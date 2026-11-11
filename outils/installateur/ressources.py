"""Fabrique les ressources de l'installateur : icone, logo et motifs de couleur.

Usage : py -3.12 -I ressources.py <dossier racine du projet> <dossier de sortie>
Les motifs sont crees ici (degradés), donc libres de droits : ils remplacent ceux de
l'ancien RPI2DMD, qu'on ne peut pas redistribuer.
"""
import colorsys
import os
import sys
from PIL import Image

ROOT, OUT = sys.argv[1], sys.argv[2]
os.makedirs(os.path.join(OUT, 'fonts'), exist_ok=True)

# icone : l'embleme a gauche du logo
logo = Image.open(os.path.join(ROOT, 'OrbitDMD.png')).convert('RGBA')
emblem = logo.crop((0, 0, logo.height, logo.height))
box = emblem.getbbox()
if box:
    emblem = emblem.crop(box)
side = max(emblem.size)
square = Image.new('RGBA', (side, side), (0, 0, 0, 0))
square.paste(emblem, ((side - emblem.width) // 2, (side - emblem.height) // 2))
square.save(os.path.join(OUT, 'orbitdmd.ico'), sizes=[(16, 16), (24, 24), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)])

# logo de l'en-tete (fond sombre)
logo.resize((471, 157), Image.LANCZOS).save(os.path.join(OUT, 'logo.png'))

W, H = 128, 32


def gradient(name, stops, vertical=True):
    """degrade entre plusieurs couleurs (liste de (position 0..1, (r, g, b)))"""
    im = Image.new('RGB', (W, H))
    px = im.load()
    n = H if vertical else W
    for i in range(n):
        t = i / (n - 1)
        for k in range(len(stops) - 1):
            p0, c0 = stops[k]
            p1, c1 = stops[k + 1]
            if p0 <= t <= p1:
                f = (t - p0) / (p1 - p0) if p1 > p0 else 0
                c = tuple(int(c0[j] + (c1[j] - c0[j]) * f) for j in range(3))
                break
        for j in range(W if vertical else H):
            if vertical:
                px[j, i] = c
            else:
                px[i, j] = c
    im.save(os.path.join(OUT, 'fonts', name))


def rainbow(name):
    im = Image.new('RGB', (W, H))
    px = im.load()
    for x in range(W):
        r, g, b = colorsys.hsv_to_rgb(x / W, 1.0, 1.0)
        for y in range(H):
            px[x, y] = (int(r * 255), int(g * 255), int(b * 255))
    im.save(os.path.join(OUT, 'fonts', name))


gradient('Pattern_Feu.png', [(0, (255, 240, 120)), (0.45, (255, 140, 0)), (1, (200, 20, 0))])
gradient('Pattern_Glace.png', [(0, (255, 255, 255)), (0.5, (120, 210, 255)), (1, (20, 90, 220))])
gradient('Pattern_Or.png', [(0, (255, 245, 180)), (0.45, (240, 190, 40)), (0.55, (190, 130, 10)), (1, (255, 215, 90))])
gradient('Pattern_Coucher_de_soleil.png', [(0, (255, 200, 60)), (0.5, (255, 80, 80)), (1, (130, 40, 160))])
gradient('Pattern_Neon.png', [(0, (0, 255, 220)), (1, (255, 0, 200))], vertical=False)
gradient('Pattern_Orbit.png', [(0, (1, 86, 209)), (0.6, (1, 167, 197)), (1, (252, 185, 43))], vertical=False)
rainbow('Pattern_Arc_en_ciel.png')
print('ressources pretes dans', OUT)
