# OrbitDMD — journal des versions

Le numéro de version est écrit dans le fichier `VERSION`, à la racine du projet. Il est repris
automatiquement par le programme du panneau (tableau de bord, page Système, journal) et par
l'installateur (titre, propriétés du fichier, nom `OrbitDMD-Installateur-X.Y.Z.exe`).

Numérotation `MAJEUR.MINEUR.CORRECTIF` :

- **CORRECTIF** (1.0.0 → 1.0.1) : corrections, sans nouvelle fonction ;
- **MINEUR** (1.0.0 → 1.1.0) : nouvelles fonctions, les cartes et réglages existants restent valables ;
- **MAJEUR** (1.0.0 → 2.0.0) : changement qui oblige à refaire la carte ou les réglages.

Pour une nouvelle version : changer `VERSION`, ajouter une section ci-dessous, recompiler le
programme (`dmd/`), puis reconstruire l'installateur (`bash outils/installateur/construire.sh`).

---

## 1.0.1 — 11 novembre 2026

- Page web : lien « OrbitDMD sur GitHub » en bas de toutes les pages, et lien vers les nouvelles
  versions dans la page Système.
- Chaque version publiée sur GitHub fournit aussi `kernel8.img` seul, pour mettre à jour un panneau
  depuis la page Système sans refaire la carte.

## 1.0.0 — 11 novembre 2026

Première version complète, développée par Ulrik Adam (www.nexadam.be).

**Système**
- Raspberry Pi 3 sans Linux (Circle), démarrage en quelques secondes ;
- panneaux HUB75 (brochage « regular »), 2 × 64×32 par défaut, 180 images/s, luminosité heure par heure ;
- Wi-Fi WPA2, adresse automatique ou fixe, essai sécurisé des nouveaux réglages réseau ;
- mise à jour du programme et des réglages depuis la page web.

**Affichage**
- GIF par catégories, écrans de démarrage (Noël, Halloween selon la saison) ;
- horloge : polices TTF, taille automatique, couleurs, motifs, fonds animés, date ;
- météo Open-Meteo (sans clé) : icônes animées, température, informations au choix ;
- pause, extinction de l'affichage, redémarrage à distance.

**Page web**
- tableau de bord en direct avec aperçu fidèle du panneau ;
- pages GIF, Horloge, Météo, Réglages, Système ; mot de passe pour les modifications.

**Installateur Windows**
- assistant en 5 étapes (panneaux, Wi-Fi, mot de passe, fuseau, météo, carte SD) ;
- 562 GIF (projet RPI2DMD et créations OrbitDMD), 14 polices, 13 fonds animés, 7 motifs.
