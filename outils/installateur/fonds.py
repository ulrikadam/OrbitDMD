"""Fonds animes d'OrbitDMD (128x32), crees ici : libres de droits.

Usage : py -3.12 -I fonds.py <dossier de sortie>
Chaque animation boucle sans saut. Les couleurs restent sombres pour que l'heure,
dessinee par-dessus, reste lisible.
"""
import colorsys
import math
import os
import random
import sys
from PIL import Image, ImageDraw

OUT = sys.argv[1]
os.makedirs(OUT, exist_ok=True)
W, H = 128, 32
TAU = 2 * math.pi


def ecrire(nom, images, ms):
    pal = [im.convert('RGB').quantize(colors=96, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE) for im in images]
    chemin = os.path.join(OUT, 'Background_' + nom + '.gif')
    pal[0].save(chemin, save_all=True, append_images=pal[1:], duration=ms, loop=0, optimize=False, disposal=1)
    print('%-34s %3d images  %4d Ko' % (os.path.basename(chemin), len(images), os.path.getsize(chemin) // 1024))


def sombre(c, k):
    return tuple(int(v * k) for v in c)


def etoiles():
    rnd = random.Random(1)
    couches = [(2, 60, 0.35), (4, 25, 0.6), (8, 10, 1.0)]	# vitesse, nombre, eclat
    pts = [[(rnd.uniform(0, W), rnd.randrange(H), v, k) for _ in range(n)] for v, n, k in couches]
    images = []
    for f in range(64):
        im = Image.new('RGB', (W, H), (2, 2, 10))
        px = im.load()
        for couche in pts:
            for x, y, v, k in couche:
                xx = int(x - v * f) % W
                c = int(255 * k * 0.8)
                px[xx, y] = (c, c, min(255, c + 30))
                if v == 8:
                    px[(xx + 1) % W, y] = (c // 3, c // 3, c // 2)
        images.append(im)
    ecrire('Etoiles', images, 60)


def neige():
    rnd = random.Random(2)
    flocons = [(rnd.uniform(0, W), rnd.uniform(0, H), rnd.choice([0.5, 1.0]), rnd.uniform(0, TAU)) for _ in range(70)]
    images = []
    for f in range(64):
        im = Image.new('RGB', (W, H), (4, 8, 20))
        d = ImageDraw.Draw(im)
        for x, y, v, ph in flocons:
            yy = (y + v * f) % H
            xx = (x + 1.5 * math.sin(ph + TAU * f / 64)) % W
            c = 200 if v == 1.0 else 120
            d.point((int(xx), int(yy)), fill=(c, c, c + 20 if c < 235 else 255))
        images.append(im)
    ecrire('Neige', images, 70)


def pluie():
    rnd = random.Random(3)
    gouttes = [(rnd.uniform(0, W + 16), rnd.uniform(0, H)) for _ in range(55)]
    images = []
    for f in range(32):
        im = Image.new('RGB', (W, H), (3, 5, 12))
        d = ImageDraw.Draw(im)
        for x, y in gouttes:
            yy = (y + 2 * f) % H
            xx = (x - yy * 0.4) % (W + 16) - 8
			# (traits obliques)
            d.line((xx, yy, xx - 1.2, yy + 3), fill=(40, 80, 150))
        images.append(im)
    ecrire('Pluie', images, 50)


def matrix():
    rnd = random.Random(4)
    colonnes = [(x, rnd.randrange(64), rnd.choice([1, 1, 2])) for x in range(0, W, 3)]
    glyphes = [[rnd.random() < 0.6 for _ in range(4)] for _ in range(64)]
    images = []
    for f in range(64):
        im = Image.new('RGB', (W, H), (0, 0, 0))
        px = im.load()
        for x, ph, v in colonnes:
            tete = (f * v + ph) % 64
            for k in range(14):
                y = tete - k
                if 0 <= y < H and glyphes[(y + x) % 64][k % 4]:
                    g = 230 if k == 0 else int(140 * (1 - k / 14))
                    px[x, y] = (g // 3 if k == 0 else 0, g, g // 4 if k == 0 else 0)
        images.append(im)
    ecrire('Matrix', images, 70)


def plasma():
    images = []
    for f in range(48):
        t = TAU * f / 48
        im = Image.new('RGB', (W, H))
        px = im.load()
        for y in range(H):
            for x in range(W):
                v = (math.sin(x / 9 + t) + math.sin((x + y) / 13 + 2 * t) + math.sin(math.hypot(x - 64, y - 16) / 7 - t)) / 3
                r, g, b = colorsys.hsv_to_rgb((v + 1) / 2, 0.9, 0.38)
                px[x, y] = (int(r * 255), int(g * 255), int(b * 255))
        images.append(im)
    ecrire('Plasma', images, 70)


def feu():
    images = []
    for f in range(40):
        t = TAU * f / 40
        im = Image.new('RGB', (W, H))
        px = im.load()
        for x in range(W):
            haut = 10 + 4 * math.sin(x / 5 + 2 * t) + 3 * math.sin(x / 2.3 - 3 * t) + 2 * math.sin(x / 11 + t)
            for y in range(H):
                h = H - y			# hauteur depuis le bas
                if h < haut:
                    k = 1 - h / haut
                    r = int(255 * min(1, 0.4 + k) * 0.75)
                    g = int(200 * k * k * 0.7)
                    px[x, y] = (r, g, int(30 * k ** 4))
        images.append(im)
    ecrire('Feu', images, 60)


def vagues():
    images = []
    for f in range(48):
        t = TAU * f / 48
        im = Image.new('RGB', (W, H), (0, 6, 22))
        px = im.load()
        for x in range(W):
            for n, (base, amp, freq, vit, col) in enumerate([(12, 3, 16, 1, (0, 40, 90)), (18, 2.5, 11, 2, (0, 60, 120)),
                                                              (24, 2, 7, 3, (10, 90, 150))]):
                s = base + amp * math.sin(x / freq * 2 + vit * t)
                for y in range(int(s), H):
                    px[x, y] = col
                if 0 <= int(s) < H:
                    px[x, int(s)] = (90, 160, 210)
        images.append(im)
    ecrire('Vagues', images, 70)


def aurore():
    images = []
    for f in range(60):
        t = TAU * f / 60
        im = Image.new('RGB', (W, H), (2, 4, 14))
        px = im.load()
        for x in range(W):
            c = 10 + 5 * math.sin(x / 14 + t) + 3 * math.sin(x / 6 - 2 * t)
            for y in range(H):
                k = max(0.0, 1 - abs(y - c) / 9)
                if k > 0:
                    teinte = 0.38 + 0.15 * math.sin(x / 25 + t)
                    r, g, b = colorsys.hsv_to_rgb(teinte, 0.8, 0.55 * k)
                    px[x, y] = (int(r * 255), int(g * 255), int(b * 255) + 10)
        images.append(im)
    ecrire('Aurore', images, 80)


def synthwave():
    images = []
    horizon = 14
    for f in range(24):
        im = Image.new('RGB', (W, H), (8, 0, 20))
        d = ImageDraw.Draw(im)
        for y in range(horizon):			# ciel degrade
            k = y / horizon
            d.line((0, y, W, y), fill=(int(30 + 40 * k), 0, int(50 - 20 * k)))
        for i in range(7):				# soleil a bandes
            y0 = 3 + i * 2
            if i % 2 == 0 or i < 3:
                lar = int(math.sqrt(max(0, 81 - (y0 - 11) ** 2)) * 1.2)
                d.line((64 - lar, y0, 64 + lar, y0), fill=(200, 60 + 15 * i, 90 - 8 * i))
        d.rectangle((0, horizon, W, H), fill=(10, 0, 25))
        for k in range(-12, 13):			# lignes vers le point de fuite
            d.line((64 + k * 6, horizon, 64 + k * 26, H), fill=(140, 0, 140))
        for i in range(6):				# lignes horizontales qui avancent
            p = ((i + f / 24) / 6) ** 2
            y = horizon + p * (H - horizon)
            d.line((0, y, W, y), fill=(140, 0, 140))
        images.append(im)
    ecrire('Synthwave', images, 60)


def bulles():
    rnd = random.Random(5)
    liste = [(rnd.uniform(0, W), rnd.uniform(0, 64), rnd.choice([1.0, 2.0, 2.5]), rnd.uniform(0, TAU)) for _ in range(28)]
    images = []
    for f in range(64):
        im = Image.new('RGB', (W, H), (0, 12, 26))
        d = ImageDraw.Draw(im)
        for x, y, r, ph in liste:
            yy = H + 8 - (y + f) % 64
            xx = x + 2 * math.sin(ph + TAU * f / 32)
            d.ellipse((xx - r, yy - r, xx + r, yy + r), outline=(60, 140, 180))
            d.point((xx - r / 2, yy - r / 2), fill=(150, 210, 230))
        images.append(im)
    ecrire('Bulles', images, 60)


def guirlande():
    couleurs = [(220, 30, 30), (30, 190, 50), (240, 180, 20), (40, 90, 230), (200, 60, 200)]
    images = []
    for f in range(16):
        im = Image.new('RGB', (W, H), (3, 6, 3))
        d = ImageDraw.Draw(im)
        for haut in (True, False):
            base = 3 if haut else H - 4
			# fil qui ondule
            pts = [(x, base + (1.5 * math.sin(x / 8) if haut else -1.5 * math.sin(x / 8))) for x in range(0, W + 1, 2)]
            d.line(pts, fill=(25, 50, 25))
            for i, x in enumerate(range(4, W, 8)):
                y = base + (1.5 * math.sin(x / 8) if haut else -1.5 * math.sin(x / 8))
                c = couleurs[i % len(couleurs)]
                allume = (i + f // 4 + (0 if haut else 2)) % 3 != 0
                c = c if allume else sombre(c, 0.25)
                d.ellipse((x - 1.5, y - 0.5, x + 1.5, y + 2.5), fill=c)
        images.append(im)
    ecrire('Guirlande', images, 120)


def feux_artifice():
    rnd = random.Random(6)
    N = 72
    tirs = [(rnd.uniform(15, W - 15), rnd.uniform(6, 16), rnd.randrange(N), rnd.uniform(0, 1)) for _ in range(6)]
    images = []
    for f in range(N):
        im = Image.new('RGB', (W, H), (2, 2, 8))
        px = im.load()
        for cx, cy, t0, teinte in tirs:
            age = (f - t0) % N
			# montee (6 images) puis gerbe (18 images)
            if age < 6:
                y = int(H - (H - cy) * age / 6)
                if 0 <= y < H:
                    px[int(cx), y] = (200, 180, 120)
            elif age < 24:
                a = age - 6
                rayon = a * 0.9
                k = 1 - a / 18
                r, g, b = colorsys.hsv_to_rgb(teinte, 0.8, k)
                for n in range(16):
                    ang = TAU * n / 16
                    x = int(cx + rayon * math.cos(ang))
                    y = int(cy + rayon * math.sin(ang) + a * a * 0.02)
                    if 0 <= x < W and 0 <= y < H:
                        px[x, y] = (int(r * 255), int(g * 255), int(b * 255))
        images.append(im)
    ecrire('Feux_d_artifice', images, 60)


def nuages():
    rnd = random.Random(7)
    liste = [(rnd.uniform(0, W), rnd.uniform(4, 26), rnd.uniform(6, 12), rnd.choice([1, 2])) for _ in range(9)]
    images = []
    for f in range(128):
        im = Image.new('RGB', (W, H), (10, 30, 70))
        d = ImageDraw.Draw(im)
        for x, y, r, v in sorted(liste, key=lambda n: n[3]):
            for dx in (-W, 0, W):
                xx = (x + v * f) % W + dx
                c = (70, 90, 120) if v == 1 else (110, 130, 160)
                d.ellipse((xx - r, y - r / 2, xx + r, y + r / 2), fill=c)
                d.ellipse((xx - r / 2, y - r * 0.8, xx + r / 2, y + r / 3), fill=c)
        images.append(im)
    ecrire('Nuages', images, 90)


for fabrique in (etoiles, neige, pluie, matrix, plasma, feu, vagues, aurore, synthwave, bulles, guirlande, feux_artifice, nuages):
    fabrique()
