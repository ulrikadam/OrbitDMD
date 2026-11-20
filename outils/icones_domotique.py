"""Icones des GIF Domotique : la partie gauche (le dessin), sans le texte.

Chaque GIF de notification Domotique (128 x 32) a la meme mise en page : le dessin a gauche,
un trait vertical en colonne 38, puis le texte. On garde les colonnes 0 a 37, avec toutes les
images et leurs durees. L'icone porte le meme nom que la GIF, dans la categorie Icones :
  Domotique/porte_ouverte.gif -> Icones/porte_ouverte.gif
Sur le panneau (1.7.0+), une notification "icone": "Icones/porte_ouverte.gif" montre l'icone a
gauche et fait defiler le texte a cote (le panneau repere lui-meme les colonnes dessinees).

Utilisation : py -3.12 outils/icones_domotique.py [dossier Domotique] [dossier Icones]
(par defaut carte-sd/gif/Domotique -> carte-sd/gif/Icones ; les icones existantes sont refaites,
les autres fichiers du dossier Icones, comme temperature.gif, ne sont pas touches)
"""
import os
import sys

from PIL import Image, ImageSequence

RACINE = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
SOURCE = sys.argv[1] if len(sys.argv) > 1 else os.path.join(RACINE, "carte-sd", "gif", "Domotique")
CIBLE = sys.argv[2] if len(sys.argv) > 2 else os.path.join(RACINE, "carte-sd", "gif", "Icones")
SEPARATEUR = 38            # colonne du trait vertical entre le dessin et le texte


def separateur_ok(image: Image.Image) -> bool:
    """Le trait vertical est bien en colonne 38 (au moins 20 pixels d'une seule couleur)."""
    rgb = image.convert("RGB")
    allumes = [p for p in (rgb.getpixel((SEPARATEUR, y)) for y in range(rgb.height)) if max(p) > 8]
    return len(allumes) >= 20 and len(set(allumes)) <= 2


def icone(source: str, cible: str) -> int:
    gif = Image.open(source)
    if gif.size != (128, 32) or not separateur_ok(gif):
        raise ValueError(f"mise en page inattendue : {source}")
    images, durees = [], []
    for image in ImageSequence.Iterator(gif):
        images.append(image.convert("RGB").crop((0, 0, SEPARATEUR, 32)))
        durees.append(image.info.get("duration", 100))
    images[0].save(cible, save_all=True, append_images=images[1:], duration=durees,
                   loop=gif.info.get("loop", 0), disposal=1, optimize=False)
    return len(images)


def main() -> None:
    os.makedirs(CIBLE, exist_ok=True)
    noms = sorted(n for n in os.listdir(SOURCE) if n.lower().endswith(".gif"))
    for nom in noms:
        icone(os.path.join(SOURCE, nom), os.path.join(CIBLE, nom))
    print(f"{len(noms)} icones dans {CIBLE}")


if __name__ == "__main__":
    main()
