"""Collection de GIF fournie avec l'installateur.

Usage : py -3.12 -I gifs.py <racine du projet> <dossier de sortie> <fichier de credits>

- GIF de l'ancien projet RPI2DMD (sauvegarde-carte/partition3-gif/gif) + celles ajoutees
  depuis sur le panneau (carte-sd/gif) ;
- toute GIF dont le nom contient "rpi2dmd" ou "rpiteam" est retiree ;
- "Accueil" (ecrans de demarrage OrbitDMD) est fourni a part ;
- le fichier de credits indique l'origine de chaque categorie.
"""
import os
import re
import shutil
import sys

RACINE, SORTIE, CREDITS = sys.argv[1], sys.argv[2], sys.argv[3]
ORIGINE = os.path.join(RACINE, 'sauvegarde-carte', 'partition3-gif', 'gif')
ACTUEL = os.path.join(RACINE, 'carte-sd', 'gif')
EXCLU = re.compile('rpi2dmd|rpiteam', re.I)


def lister(dossier):
    d = {}
    if os.path.isdir(dossier):
        for cat in os.listdir(dossier):
            p = os.path.join(dossier, cat)
            if os.path.isdir(p) and cat.lower() != 'accueil':
                d[cat] = {f: os.path.join(p, f) for f in os.listdir(p) if f.lower().endswith('.gif')}
    return d


origine, actuel = lister(ORIGINE), lister(ACTUEL)
shutil.rmtree(SORTIE, ignore_errors=True)
lignes = []
total = exclues = 0
for cat in sorted(set(origine) | set(actuel), key=str.lower):
    de_rpi = origine.get(cat, {})
    a_vous = {f: p for f, p in actuel.get(cat, {}).items() if f not in de_rpi}
    gardees = {f: p for f, p in {**de_rpi, **a_vous}.items() if not EXCLU.search(f)}
    exclues += len(de_rpi) + len(a_vous) - len(gardees)
    if not gardees:
        continue
    os.makedirs(os.path.join(SORTIE, cat))
    for f, p in gardees.items():
        shutil.copy2(p, os.path.join(SORTIE, cat, f))
    total += len(gardees)
    n_rpi = len([f for f in gardees if f in de_rpi])
    n_vous = len(gardees) - n_rpi
    if n_vous == 0:
        lignes.append('  %-14s %3d GIF  projet RPI2DMD' % (cat, len(gardees)))
    elif n_rpi == 0:
        lignes.append('  %-14s %3d GIF  creations OrbitDMD' % (cat, len(gardees)))
    else:
        lignes.append('  %-14s %3d GIF  %d du projet RPI2DMD, %d creations OrbitDMD (noms en minuscules "...-128x32.gif")'
                      % (cat, len(gardees), n_rpi, n_vous))

texte = """GIF fournies avec OrbitDMD
==========================

OrbitDMD integre le pack de GIF animees gratuit de base de la RpiTeaM
(projet RPI2DMD : 600 GIF "pixel perfect" pour panneaux 128x32, pixel art
d'eLLuiGi, RattenJager et d'autres), avec l'accord d'eLLuiGi. Merci a eux !

Pour ceux qui le souhaitent, un pack de 11 000 animations est disponible :
  https://www.neo-arcadia.com/forum/viewtopic.php?t=67065
  https://rpiteam.carrd.co/

Les GIF au nom de RPI2DMD ou de la RpiTe@m (logos du projet) ont ete retirees.

Les ecrans de demarrage (gif/Accueil) et les GIF de Noel au nom en minuscules
sont des creations OrbitDMD (Ulrik Adam, www.nexadam.be).

Ces animations reprennent des personnages et des scenes de jeux video : elles
restent la propriete de leurs ayants droit, pour un usage personnel.

Categories :
""" + '\n'.join(lignes) + '\n'
open(CREDITS, 'w', encoding='utf-8', newline='\r\n').write(texte)
print('%d GIF gardees, %d retirees (rpi2dmd / rpiteam)' % (total, exclues))
print('\n'.join(lignes))
