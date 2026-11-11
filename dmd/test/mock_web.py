"""Faux panneau pour tester l'interface web sur le PC (rien n'est modifie).

Usage : py -3.12 -I dmd/test/mock_web.py   puis  http://localhost:8765/
Sert les fichiers de dmd/web/ selon web/fichiers.txt (comme le vrai panneau),
les GIF de sauvegarde-carte + carte-sd, un etat et un journal simules.
Les POST repondent "OK : (simulation)".
"""
import json
import os
import random
import re
import subprocess
import threading
import time
import urllib.parse
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
WEB = os.path.join(ROOT, 'dmd', 'web')
CARTE = os.environ.get('ORBITDMD_CARTE', os.path.join(ROOT, 'carte-sd'))	# carte simulee (dossier)
GIF_DIR = os.path.join(CARTE, 'gif')
CONFIG = os.path.join(CARTE, 'Config', 'config.txt')
NAME_RE = re.compile(r'^[A-Za-z0-9_-]{1,32}$')
START = time.time()
STATE = {'pause': False, 'ecran_eteint': False, 'synchro': time.time() - 754}
FONTS = os.path.join(CARTE, 'fonts')
HOSTTEST = os.path.join(ROOT, 'dmd', 'test', 'out', 'hosttest.exe')
CLOCK_DEFAULTS = {'Clock_Format': '%H:%M:%S', 'Clock_Font': 'Pixel', 'Clock_Font_Size': '0', 'Clock_Color': 'FFAA00',
                  'Clock_Font_Pattern': 'None', 'Clock_Background': 'None', 'Clock_Blink': '0', 'Clock_Smooth': '1',
                  'Date_Active': '0', 'Date_Display_Time': '2500', 'Date_Format': '%a %d %b', 'Date_Font': 'Pixel',
                  'Date_Font_Size': '0', 'Date_Color': '01A7C5', 'Date_Font_Pattern': 'None'}
CLOCK_NUMBERS = {'Clock_Font_Size', 'Clock_Blink', 'Clock_Smooth', 'Date_Active', 'Date_Display_Time', 'Date_Font_Size'}
WEATHER_DEFAULTS = {'Weather_Active': '1', 'Weather_Display_Time': '5000', 'Weather_Every': '2', 'Weather_Place': '',
                    'Weather_Latitude': '', 'Weather_Longitude': '', 'Weather_Font': 'Pixel', 'Weather_Font_Size': '0',
                    'Weather_Font_Pattern': 'None', 'Weather_Text_color': 'FFAA00', 'Weather_Info': '1',
                    'Weather_Info_Color': '01A7C5', 'Weather_Icon': '1', 'Weather_Animate': '1'}
WEATHER_NUMBERS = {'Weather_Active', 'Weather_Display_Time', 'Weather_Every', 'Weather_Font_Size', 'Weather_Info',
                   'Weather_Icon', 'Weather_Animate'}
WEATHER = {'lieu_choisi': True, 'valide': True, 'maj_s': 312, 'erreur': '', 'temp': 6.2, 'ressenti': 3.3, 'vent': 11.5,
           'min': 5.0, 'max': 7.3, 'humidite': 92, 'code': 61, 'jour': False, 'description': 'PLUIE FAIBLE',
           'demain_code': 51, 'demain_description': 'BRUINE', 'demain_min': 3.4, 'demain_max': 9.4}


def hexcolor(v):
    if ',' in v:
        return ''.join('%02X' % int(x) for x in v.split(','))
    return v.lstrip('#').upper()


def manifest():
    files = {}
    for line in open(os.path.join(WEB, 'fichiers.txt'), encoding='utf-8'):
        line = line.strip()
        if line and not line.startswith('#'):
            path, name, ctype = line.split(None, 2)
            files[path] = (name, ctype)
    return files


def active_categories():
    text = open(CONFIG, encoding='latin-1').read()
    return [v for k, v in re.findall(r'^(Gif_Cat_\d+)=(.+?)\s*$', text, re.M) if v != 'disabled']


def all_gifs():
    act = active_categories()
    out = []
    for c in act:
        p = os.path.join(GIF_DIR, c)
        if os.path.isdir(p):
            out += ['SD:/gif/%s/%s' % (c, f) for f in os.listdir(p) if f.lower().endswith('.gif')]
    return out


GIFS = all_gifs()


class Handler(BaseHTTPRequestHandler):
    def send(self, code, body, ctype, extra=None):
        self.send_response(code)
        self.send_header('Content-Type', ctype)
        self.send_header('Content-Length', str(len(body)))
        for k, v in (extra or {}).items():
            self.send_header(k, v)
        self.end_headers()
        self.wfile.write(body)

    def render(self, mode, args):
        """meme dessin que le panneau (hosttest clock|weather) -> BMP"""
        import io
        from PIL import Image
        raw = os.path.join(ROOT, 'dmd', 'test', 'out', 'apercu-%d.rgb' % threading.get_ident())
        r = subprocess.run([HOSTTEST, mode, raw] + args, cwd=CARTE, capture_output=True)
        if r.returncode != 0:
            return None
        buf = io.BytesIO()
        Image.frombytes('RGB', (128, 32), open(raw, 'rb').read()).save(buf, 'BMP')
        return buf.getvalue()

    def do_GET(self):
        url = urllib.parse.urlparse(self.path)
        q = urllib.parse.parse_qs(url.query)
        files = manifest()
        if url.path in files:
            name, ctype = files[url.path]
            self.send(200, open(os.path.join(WEB, name), 'rb').read(), ctype)
        elif url.path == '/api/etat':
            up = int(time.time() - START) + 3725
            gif = GIFS[int(time.time() / 8) % len(GIFS)] if GIFS else ''
            body = {'programme': 'Nov 10 2026 18:39:20', 'uptime_s': up, 'ip': '192.168.0.151',
                    'heure': time.strftime('%Y-%m-%d %H:%M:%S'), 'source_heure': 'Internet',
                    'rafraichissement_hz': random.choice([172, 173, 175]), 'luminosite': 60,
                    'nb_gif': len(GIFS), 'memoire_libre_mo': 852, 'carte_libre_mo': 15071,
                    'version': '1.0.0', 'temperature_c': 52, 'latence_ms': 6, 'latence_max_ms': 31, 'nom': 'OrbitDMD', 'wifi': 'Box-Maison', 'gif': gif,
                    'pause': STATE['pause'], 'ecran_eteint': STATE['ecran_eteint'],
                    'synchro_s': int(time.time() - STATE['synchro']),
                    'meteo_temp': int(WEATHER['temp'] * 10), 'meteo_code': WEATHER['code'], 'meteo_jour': WEATHER['jour'],
                    'meteo_texte': WEATHER['description']}
            self.send(200, json.dumps(body).encode(), 'application/json')
        elif url.path == '/api/reglages':
            text = open(CONFIG, encoding='latin-1').read()
            kv = dict(re.findall(r'^([A-Za-z_0-9]+)=(.*?)\s*$', text, re.M))
            num = lambda k, d: int(kv.get(k, d))
            body = {'luminosite': [num('Panel_Brightness_%d' % h, 60) for h in range(24)],
                    'heure_actuelle': time.localtime().tm_hour,
                    'Gif_Active': num('Gif_Active', 1), 'Gif_Min_Duration': num('Gif_Min_Duration', 4000),
                    'Clock_Active': num('Clock_Active', 1), 'Clock_Display_Time': num('Clock_Display_Time', 3000),
                    'Clock_TZ': kv.get('Clock_TZ', 'Europe/Paris'), 'Test_Pattern': num('Test_Pattern', 1),
                    'Panel_XSize': num('Panel_XSize', 64), 'Panel_YSize': num('Panel_YSize', 32),
                    'Panel_XNumber': num('Panel_XNumber', 2), 'Panel_YNumber': num('Panel_YNumber', 1),
                    'GPIO_Slowdown': num('GPIO_Slowdown', 2), 'RGB_Order': kv.get('RGB_Order', 'RGB'),
                    'PWM_Bits': num('PWM_Bits', 11), 'PWM_LSB_Nanoseconds': num('PWM_LSB_Nanoseconds', 130),
                    'Network_name': kv.get('Network_name', 'OrbitDMD'), 'Wifi_ssid': 'Box-Maison',
                    'Wifi_country': kv.get('Wifi_country', 'FR'), 'wifi_psk_defini': True,
                    'redemarrage_en_attente': False,
                    'IP_Fixe': kv.get('IP_Fixe', ''), 'IP_Masque': kv.get('IP_Masque', ''),
                    'IP_Passerelle': kv.get('IP_Passerelle', ''), 'IP_DNS': kv.get('IP_DNS', ''),
                    'reseau_actuel': {'ip': '192.168.0.151', 'masque': '255.255.255.0',
                                      'passerelle': '192.168.0.1', 'dns': '192.168.0.1',
                                      'mac': 'b8:27:eb:12:34:56', 'fixe': False},
                    'fuseaux': ['Europe/Paris', 'Europe/Brussels', 'Europe/London', 'UTC']}
            for k, d in CLOCK_DEFAULTS.items():
                v = kv.get(k, d)
                body[k] = int(v) if k in CLOCK_NUMBERS else v
            for k, d in WEATHER_DEFAULTS.items():
                v = kv.get(k, d)
                body[k] = int(v) if k in WEATHER_NUMBERS else hexcolor(v) if 'olor' in k else v
            self.send(200, json.dumps(body).encode(), 'application/json')
        elif url.path == '/api/meteo':
            self.send(200, json.dumps(WEATHER).encode(), 'application/json')
        elif url.path == '/api/ecran':
            # cycle simule de 12 s : 8 s de GIF, 2 s d'heure, 2 s de meteo
            phase = int(time.time()) % 12
            if STATE['ecran_eteint']:
                import io
                from PIL import Image
                buf = io.BytesIO()
                Image.new('RGB', (128, 32)).save(buf, 'BMP')
                self.send(200, buf.getvalue(), 'image/bmp', {'X-Ecran': 'eteint'})
            elif phase < 8 and not STATE['pause']:
                gif = GIFS[int(time.time() / 12) % len(GIFS)] if GIFS else ''
                self.send(200, gif.encode(), 'text/plain; charset=utf-8', {'X-Ecran': 'gif'})
            else:
                kind = 'pause' if STATE['pause'] else 'heure' if phase < 10 else 'meteo'
                body = self.render('weather', ['code=%d' % WEATHER['code'], 'jour=0',
                                              'temps=%d' % (time.time() * 1000 % 100000)]) \
                    if kind == 'meteo' else self.render('clock', [])
                self.send(200, body or b'', 'image/bmp', {'X-Ecran': kind})
        elif url.path == '/api/horloge/fichiers':
            lists = {'polices': [], 'motifs': [], 'fonds': []}
            for root, dirs, names in os.walk(FONTS):
                rel = os.path.relpath(root, FONTS).replace('\\', '/')
                if rel.count('/') > 0:
                    continue
                for n in names:
                    name = n if rel == '.' else rel + '/' + n
                    ext = n.lower().rsplit('.', 1)[-1]
                    if ext in ('ttf', 'otf'):
                        lists['polices'].append(name)
                    elif ext in ('png', 'gif'):
                        lists['fonds' if n.lower().startswith('background') else 'motifs'].append(name)
            self.send(200, json.dumps(lists).encode(), 'application/json')
        elif url.path == '/fonts/voir':
            f = q.get('f', [''])[0]
            p = os.path.join(FONTS, f)
            if '..' not in f and os.path.isfile(p):
                self.send(200, open(p, 'rb').read(), 'image/gif' if f.lower().endswith('.gif') else 'image/png')
            else:
                self.send(404, b'', 'text/plain')
        elif url.path == '/api/horloge/apercu':
            # meme dessin que le panneau : banc de test PC (hosttest clock)
            raw = os.path.join(ROOT, 'dmd', 'test', 'out', 'apercu-%d.rgb' % threading.get_ident())
            if q.get('ecran', [''])[0] == 'meteo':
                args = ['%s=%s' % (k, q[k][0]) for k in WEATHER_DEFAULTS if k in q]
                args += ['code=%s' % q.get('code', [WEATHER['code']])[0],
                         'jour=%s' % q.get('jour', ['1' if WEATHER['jour'] else '0'])[0],
                         'temps=%d' % (time.time() * 1000 % 100000)]
                mode = 'weather'
            else:
                args = ['%s=%s' % (k, q[k][0]) for k in CLOCK_DEFAULTS if k in q and k != 'Date_Display_Time']
                if q.get('ecran', [''])[0] == 'date':
                    args.append('date')
                mode = 'clock'
            r = subprocess.run([HOSTTEST, mode, raw] + args, cwd=CARTE,
                               capture_output=True)
            if r.returncode != 0:
                self.send(500, r.stderr, 'text/plain')
                return
            import io
            from PIL import Image
            buf = io.BytesIO()
            Image.frombytes('RGB', (128, 32), open(raw, 'rb').read()).save(buf, 'BMP')
            self.send(200, buf.getvalue(), 'image/bmp')
        elif url.path == '/journal':
            log = ('logger: Circle 51.1.1 started on Raspberry Pi 3 Model B 1GB (AArch64)\n'
                   '00:00:00.33 kernel: OrbitDMD, compile le Nov 10 2026 18:39:20\n'
                   '00:00:01.87 config: Panneaux 64x32, chaine 2, parallele 1, slowdown 2\n'
                   'Nov 10 17:39:31.88 dmd: SD:/gif/Accueil/orbitdmd-demarrage-128x32.gif : 8 images, 1600 ms\n'
                   'Nov 10 17:39:37.42 reseau: Wi-Fi connecte : http://OrbitDMD.local/ ou http://192.168.0.151/\n'
                   'Nov 10 17:39:40.44 heure: Heure Internet recue\n'
                   'Nov 10 17:40:02.10 web: Mot de passe refuse (/api/verifier)\n'
                   'Nov 10 17:40:12.51 dmd: Ouverture impossible : SD:/gif/Arcade/absente.gif\n'
                   'Nov 10 17:41:00.02 httpd: 192.168.0.20 "GET /api/etat" 200 412\n')
            self.send(200, log.encode(), 'text/plain; charset=utf-8')
        elif url.path == '/api/gif':
            act = active_categories()
            cats = []
            for d in sorted(os.listdir(GIF_DIR)):
                p = os.path.join(GIF_DIR, d)
                if os.path.isdir(p) and NAME_RE.match(d):
                    n = len([f for f in os.listdir(p) if f.lower().endswith('.gif')])
                    cats.append({'nom': d, 'nb': n, 'active': d in act})
            self.send(200, json.dumps({'libre_mo': 15071, 'categories': cats}).encode(), 'application/json')
        elif url.path == '/api/gif/liste':
            c = q.get('c', [''])[0]
            p = os.path.join(GIF_DIR, c)
            fl = []
            if NAME_RE.match(c) and os.path.isdir(p):
                fl = [{'nom': f, 'taille': os.path.getsize(os.path.join(p, f))}
                      for f in os.listdir(p) if f.lower().endswith('.gif')]
            self.send(200, json.dumps({'categorie': c, 'fichiers': fl}).encode(), 'application/json')
        elif url.path == '/gif/voir':
            c, f = q.get('c', [''])[0], q.get('f', [''])[0]
            p = os.path.join(GIF_DIR, c, f)
            if NAME_RE.match(c) and '/' not in f and '\\' not in f and os.path.isfile(p):
                self.send(200, open(p, 'rb').read(), 'image/gif')
            else:
                self.send(404, b'', 'text/plain')
        else:
            self.send(404, b'', 'text/plain')

    def do_POST(self):
        n = int(self.headers.get('Content-Length', 0))
        body = self.rfile.read(n).decode('latin-1')

        def field(name):
            m = re.search(r'name="%s"\r\n\r\n(.*?)\r\n' % name, body)
            return m.group(1) if m else ''

        if field('password') in ('', 'faux'):        # simulation d'un mauvais mot de passe
            reply = 'ERREUR : mot de passe incorrect (simulation)'
        elif self.path == '/api/commande':
            act = field('action')
            if act in ('pause', 'reprendre'):
                STATE['pause'] = act == 'pause'
            elif act in ('eteindre', 'allumer'):
                STATE['ecran_eteint'] = act == 'eteindre'
            reply = 'OK : (simulation) %s' % act
        elif self.path == '/api/heure/synchroniser':
            STATE['synchro'] = time.time()
            reply = 'OK : ecart=3'
        else:
            reply = 'OK : (simulation) %s' % self.path
        self.send(200, (reply + '\n').encode(), 'text/plain; charset=utf-8')

    def log_message(self, *args):
        pass


if __name__ == '__main__':
    ThreadingHTTPServer(('127.0.0.1', int(os.environ.get('ORBITDMD_PORT', '8765'))), Handler).serve_forever()
