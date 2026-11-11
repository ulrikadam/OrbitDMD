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

## 1.4.0 — 11 novembre 2026

**Nouveautés**
- **Home Assistant** (Réglages → Home Assistant) : le panneau se relie au serveur MQTT de Home Assistant
  et y apparaît tout seul (découverte automatique) : allumer et éteindre, luminosité, pause, afficher
  l'heure ou la météo, redémarrer, GIF affichée, température, version, et **messages** envoyés depuis les
  automatisations (avec effet, couleur, arc-en-ciel et GIF facultative). Guide : `docs/HOME-ASSISTANT.md`.
- Dans Home Assistant, sans YAML : un champ **Texte à afficher**, le style des messages (effet, couleur,
  passages, vitesse), un interrupteur par texte et par compte à rebours de la page Messages, et la liste
  des commandes MQTT (capteur « Commandes MQTT », publiée aussi sur `orbitdmd/<nom>/commandes`).
- Page **Aide Home Assistant** (`/aide`) : toutes les commandes et des exemples à copier.

## 1.3.0 — 11 novembre 2026

**Nouveautés**
- **Programme** (Réglages) : panneau éteint pendant une plage horaire (par exemple de 1 h à 7 h, la page
  web reste joignable) et **l'heure seule**, sans GIF, pendant une autre plage.
- **Catégories de GIF par mois** (page GIF) : une catégorie peut n'être montrée que certains mois
  (Noël en décembre, Halloween en octobre…).
- **Période des messages** : chaque texte et chaque compte à rebours peut n'être actif qu'entre deux
  dates, chaque année (par exemple du 20/12 au 05/01).
- **Mise au format automatique** : à l'envoi, les GIF plus grandes que le panneau sont réduites par le
  navigateur (pixels nets, animation et durées gardées).
- Le tableau de bord indique quand le programme éteint le panneau ou montre l'heure seule.

## 1.2.0 — 11 novembre 2026

**Nouveautés**
- Page **Messages**, avec deux listes (10 éléments chacune), chaque élément activé ou non et avec son
  propre style ; les éléments activés passent à tour de rôle entre les GIF :
  - **textes défilants** (200 caractères, accents compris) : police, taille, couleur ou **arc-en-ciel
    animé**, vitesse, nombre de passages, et **arrivée du texte** : défile vers la gauche ou vers la
    droite, monte, descend, lettre par lettre, pluie de lettres, fondu (un texte long est coupé en pages) ;
  - **comptes à rebours** jusqu'à Noël, au Nouvel an ou à une date choisie (« 43 JOURS », « 3 J 14 H »,
    puis « 02:14:33 » le dernier jour), avec un message pour le jour J ;
  - bouton « Afficher maintenant » pour chaque élément.
- Arc-en-ciel animé aussi possible pour l'heure et la météo.
- Page Horloge : couleur ou motif **au hasard** et fond **au hasard**, tirés à chaque affichage de l'heure,
  et arc-en-ciel animé.
- GIF fournies : mention du pack de GIF gratuit de base de la RpiTeaM (intégré avec l'accord d'eLLuiGi)
  et liens vers leur pack de 11 000 animations.
- GIF **favorites** (passent 3 fois plus souvent) et GIF **hors rotation** (gardées sur la carte), depuis
  la page GIF ; **ordre de lecture** au hasard ou dans l'ordre (Réglages → Affichage).
- **Sauvegarde des réglages** (page Système) : un fichier avec tous les réglages, les messages et les
  GIF favorites et exclues, à remettre après une réinstallation.
- **Nouvelle version disponible** : le tableau de bord et la page Système le signalent, avec un lien vers
  `kernel8.img` (le navigateur consulte GitHub au plus toutes les 6 heures).
- Menu sur deux lignes sur les écrans moyens.

## 1.1.0 — 11 novembre 2026

**Nouveautés**
- Prise réseau (Ethernet) du Raspberry Pi 3 B et B+ : réglage « Connexion » (automatique : câble s'il est
  branché au démarrage, sinon Wi-Fi ; ou Wi-Fi seulement ; ou câble seulement).
- Adresse IP sur le panneau : « WIFI CONNECTE » (ou « CABLE CONNECTE »), nom `.local` et adresse,
  affichés au démarrage et de nouveau si la connexion revient ou si l'adresse change ; durée réglable
  (Réglages → Affichage, 0 = jamais).
- Ajout de GIF en masse (page GIF) : dossiers entiers et fichiers `.zip`, un sous-dossier devient une
  catégorie, récapitulatif avant l'envoi, barre de progression, arrêt et reprise (les GIF déjà présentes
  sont sautées).

**Corrections**
- Les réglages à choix acceptent les valeurs en minuscules.

## 1.0.2 — 11 novembre 2026

- Page web : bouton « Soutenir sur Ko-fi » en bas de toutes les pages (simple lien, aucun script
  extérieur chargé par le panneau).

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
