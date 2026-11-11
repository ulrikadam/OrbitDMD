"""Polices pixel d'OrbitDMD, ecrites directement en TrueType (sans bibliotheque).

Usage : py -3.12 -I polices_pixel.py <dmd/font5x7.cpp> <dossier de sortie>

  OrbitDMD-LED.ttf       points ronds, comme un panneau a LED (dessins de la police 5x7)
  OrbitDMD-Gras.ttf      la police 5x7, traits doubles
  OrbitDMD-Segments.ttf  afficheur 7 segments (chiffres et la plupart des lettres)

Les minuscules utilisent les dessins des majuscules. Licence : celle d'OrbitDMD.
"""
import math
import re
import struct
import sys

SRC, OUT = sys.argv[1], sys.argv[2]
UPM = 1000
PX = 100			# un pixel = 100 unites
ASC, DESC = 800, 200


# --- dessins 5x7 de dmd/font5x7.cpp ---

def lire_5x7(chemin):
    texte = open(chemin, encoding='ascii').read()
    glyphes = {}
    for m in re.finditer(r"\{'(\\xB0|\\'|.)', \{((?:\"[.#]{5}\",?\s*){7})\}\}", texte):
        c = m.group(1)
        c = '°' if c == '\\xB0' else ("'" if c == "\\'" else c)
        lignes = re.findall(r'"([.#]{5})"', m.group(2))
        glyphes[c] = lignes
    return glyphes


# --- contours ---

def rect(x0, y0, x1, y1):
    # sens des aiguilles d'une montre (TrueType), y vers le haut
    return [(x0, y0), (x0, y1), (x1, y1), (x1, y0)]


def cercle(cx, cy, r, n=12):
    return [(round(cx + r * math.cos(-2 * math.pi * k / n)), round(cy + r * math.sin(-2 * math.pi * k / n))) for k in range(n)]


def pixels(lignes, gras=False, rond=False):
    """contours d'un dessin 5x7 (ligne 0 en haut) ; les pixels voisins d'une ligne sont fusionnes"""
    contours = []
    h = len(lignes)
    for r, ligne in enumerate(lignes):
        on = [ch == '#' for ch in ligne]
        if gras:
            on = [on[i] or (i > 0 and on[i - 1]) for i in range(len(on))] + [on[-1]]
        y0 = (h - 1 - r) * PX
        if rond:
            for x, v in enumerate(on):
                if v:
                    contours.append(cercle(x * PX + PX // 2, y0 + PX // 2, PX * 0.46))
            continue
        x = 0
        while x < len(on):
            if on[x]:
                d = x
                while x < len(on) and on[x]:
                    x += 1
                contours.append(rect(d * PX, y0, x * PX, y0 + PX))
            else:
                x += 1
    return contours


# --- 7 segments ---
#  a = haut, b = haut droite, c = bas droite, d = bas, e = bas gauche, f = haut gauche, g = milieu
SEG7 = {
    '0': 'abcdef', '1': 'bc', '2': 'abged', '3': 'abgcd', '4': 'fgbc', '5': 'afgcd', '6': 'afgedc', '7': 'abc',
    '8': 'abcdefg', '9': 'abcdfg', 'A': 'abcefg', 'B': 'cdefg', 'C': 'adef', 'D': 'bcdeg', 'E': 'adefg',
    'F': 'aefg', 'G': 'acdef', 'H': 'bcefg', 'I': 'bc', 'J': 'bcde', 'L': 'def', 'N': 'ceg', 'O': 'abcdef',
    'P': 'abefg', 'Q': 'abcfg', 'R': 'eg', 'S': 'afgcd', 'T': 'defg', 'U': 'bcdef', 'V': 'cde', 'Y': 'bcdfg',
    'Z': 'abged', '-': 'g', '_': 'd', '°': 'abfg', ' ': ''
}
SW, SH, T = 400, 700, 90		# largeur, hauteur, epaisseur d'un segment


def segment(nom):
    m = SH // 2
    i = 12				# petit espace entre segments
    return {
        'a': rect(T + i, SH - T, SW - T - i, SH),
        'd': rect(T + i, 0, SW - T - i, T),
        'g': rect(T + i, m - T // 2, SW - T - i, m + T // 2),
        'f': rect(0, m + T // 2 + i, T, SH - T - i),
        'b': rect(SW - T, m + T // 2 + i, SW, SH - T - i),
        'e': rect(0, T + i, T, m - T // 2 - i),
        'c': rect(SW - T, T + i, SW, m - T // 2 - i),
    }[nom]


def glyphe_segments(c):
    if c == ':':
        return [rect(150, 150, 250, 250), rect(150, 450, 250, 550)], 400
    if c == '.':
        return [rect(0, 0, 100, 100)], 200
    if c == '/':
        return [[(0, 0), (300, 700), (390, 700), (90, 0)]], 450
    if c == '%':
        return [rect(0, 500, 120, 620), rect(280, 80, 400, 200), [(0, 0), (310, 700), (400, 700), (90, 0)]], 500
    segs = SEG7.get(c.upper())
    if segs is None:
        return None, 0
    return [segment(s) for s in segs], SW + 120


# --- ecriture du fichier TrueType ---

def ttf(nom_famille, glyphes, chemin):
    """glyphes : liste de (code ou None, contours, avance) ; le premier est .notdef (vide)"""
    def glyf_data(contours):
        if not contours:
            return b''
        pts = [p for c in contours for p in c]
        xs, ys = [p[0] for p in pts], [p[1] for p in pts]
        d = struct.pack('>hhhhh', len(contours), min(xs), min(ys), max(xs), max(ys))
        fin, n = [], 0
        for c in contours:
            n += len(c)
            fin.append(n - 1)
        d += struct.pack('>%dH' % len(fin), *fin) + struct.pack('>H', 0)
        d += bytes([1] * len(pts))			# tous les points sur la courbe, coordonnees 16 bits
        px = py = 0
        for x, _ in pts:
            d += struct.pack('>h', x - px)
            px = x
        for _, y in pts:
            d += struct.pack('>h', y - py)
            py = y
        return d + b'\0' * (-len(d) % 4)

    glyf, loca = b'', [0]
    for _, contours, _ in glyphes:
        glyf += glyf_data(contours)
        loca.append(len(glyf))
    n = len(glyphes)
    tous = [p for _, cs, _ in glyphes for c in cs for p in c]
    xmin, ymin = min(p[0] for p in tous), min(p[1] for p in tous)
    xmax, ymax = max(p[0] for p in tous), max(p[1] for p in tous)
    avance_max = max(a for _, _, a in glyphes)
    max_pts = max([sum(len(c) for c in cs) for _, cs, _ in glyphes] + [1])
    max_ctr = max([len(cs) for _, cs, _ in glyphes] + [1])

    head = struct.pack('>IIIIHHqqhhhhHHhhh', 0x00010000, 0x00010000, 0, 0x5F0F3CF5, 0x000B, UPM,
                       0, 0, xmin, ymin, xmax, ymax, 0, 8, 2, 1, 0)
    hhea = struct.pack('>IhhhHhhhhhhhhhhhH', 0x00010000, ASC, -DESC, 0, avance_max, 0, 0, xmax, 1, 0, 0,
                       0, 0, 0, 0, 0, n)
    maxp = struct.pack('>IHHHHHHHHHHHHHH', 0x00010000, n, max_pts, max_ctr, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0)
    hmtx = b''.join(struct.pack('>Hh', a, min([p[0] for c in cs for p in c] or [0])) for _, cs, a in glyphes)
    loca_t = b''.join(struct.pack('>I', v) for v in loca)

    # cmap format 4 : un segment par caractere
    codes = sorted((code, gid) for gid, (code, _, _) in enumerate(glyphes) if code is not None)
    segs = [(c, c, (g - c) & 0xFFFF) for c, g in codes] + [(0xFFFF, 0xFFFF, 1)]
    sc = len(segs)
    sr = 2 * (2 ** int(math.log2(sc)))
    f4 = struct.pack('>HHHHHHH', 4, 16 + 8 * sc, 0, 2 * sc, sr, int(math.log2(sr // 2)), 2 * sc - sr)
    f4 += b''.join(struct.pack('>H', e) for _, e, _ in segs) + b'\0\0'
    f4 += b''.join(struct.pack('>H', s) for s, _, _ in segs)
    f4 += b''.join(struct.pack('>H', d) for _, _, d in segs)
    f4 += b'\0\0' * sc
    cmap = struct.pack('>HHHHI', 0, 1, 3, 1, 12) + f4

    noms = {1: nom_famille, 2: 'Regular', 3: nom_famille + ' OrbitDMD', 4: nom_famille,
            5: 'Version 1.0', 6: nom_famille.replace(' ', '')}
    enregs, chaines = b'', b''
    for nid, txt in noms.items():
        s = txt.encode('utf-16-be')
        enregs += struct.pack('>HHHHHH', 3, 1, 0x409, nid, len(s), len(chaines))
        chaines += s
    name = struct.pack('>HHH', 0, len(noms), 6 + 12 * len(noms)) + enregs + chaines

    os2 = struct.pack('>HhHHHhhhhhhhhhhh', 4, avance_max // 2, 400, 5, 0, 650, 700, 0, 140, 650, 700, 0, 480,
                      50, 300, 0)
    os2 += b'\0' * 10 + struct.pack('>IIII', 1, 0, 0, 0) + b'ODMD'
    os2 += struct.pack('>HHHhhhHHII', 0x40, 0x20, 0xFF, ASC, -DESC, 0, ASC, DESC, 1, 0)
    os2 += struct.pack('>hhHHH', 500, 700, 0, 0x20, 1)
    post = struct.pack('>IIhhIIIII', 0x00030000, 0, -100, 50, 0, 0, 0, 0, 0)

    tables = {b'OS/2': os2, b'cmap': cmap, b'glyf': glyf, b'head': head, b'hhea': hhea,
              b'hmtx': hmtx, b'loca': loca_t, b'maxp': maxp, b'name': name, b'post': post}

    def somme(d):
        d = d + b'\0' * (-len(d) % 4)
        return sum(struct.unpack('>%dI' % (len(d) // 4), d)) & 0xFFFFFFFF

    nt = len(tables)
    es = 2 ** int(math.log2(nt))
    entete = struct.pack('>IHHHH', 0x00010000, nt, es * 16, int(math.log2(es)), nt * 16 - es * 16)
    pos = 12 + 16 * nt
    rep, corps = b'', b''
    for tag in sorted(tables):
        d = tables[tag]
        rep += struct.pack('>4sIII', tag, somme(d), pos + len(corps), len(d))
        corps += d + b'\0' * (-len(d) % 4)
    fichier = bytearray(entete + rep + corps)
    # checkSumAdjustment de head
    ajust = (0xB1B0AFBA - somme(bytes(fichier))) & 0xFFFFFFFF
    off = pos + sum(len(tables[t]) + (-len(tables[t]) % 4) for t in sorted(tables) if t < b'head')
    struct.pack_into('>I', fichier, off + 8, ajust)
    open(chemin, 'wb').write(fichier)


def construire(nom_famille, fichier, dessin):
    glyphes = [(None, [], 600)]		# .notdef vide : un caractere absent ne dessine rien
    vus = {}
    for c in [chr(i) for i in range(32, 127)] + ['°']:
        cle = c.upper() if c.islower() else c
        if cle in vus:
            gid = vus[cle]
            if c != cle:
                glyphes.append((ord(c), glyphes[gid][1], glyphes[gid][2]))
            continue
        contours, avance = dessin(cle)
        if contours is None:
            continue
        vus[cle] = len(glyphes)
        glyphes.append((ord(c), contours, avance))
    ttf(nom_famille, glyphes, fichier)
    print(fichier, len(glyphes), 'glyphes')


g5 = lire_5x7(SRC)
g5[' '] = ['.....'] * 7


def dessin_5x7(gras=False, rond=False):
    def f(c):
        if c not in g5:
            return None, 0
        return pixels(g5[c], gras=gras, rond=rond), (7 if gras else 6) * PX
    return f


construire('OrbitDMD LED', OUT + '/OrbitDMD-LED.ttf', dessin_5x7(rond=True))
construire('OrbitDMD Gras', OUT + '/OrbitDMD-Gras.ttf', dessin_5x7(gras=True))
construire('OrbitDMD Segments', OUT + '/OrbitDMD-Segments.ttf', glyphe_segments)
