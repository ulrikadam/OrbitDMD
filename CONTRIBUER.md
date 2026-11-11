# Contribuer à OrbitDMD

Merci de votre intérêt ! Toutes les contributions sont les bienvenues, même petites.
Vous pouvez écrire en **français** ou en **anglais**.

## Sans écrire de code

- 💡 **Proposer une idée** : ouvrez une [discussion](https://github.com/ulrikadam/OrbitDMD/discussions)
  (catégorie *Ideas*) ou une [proposition d'amélioration](https://github.com/ulrikadam/OrbitDMD/issues/new/choose).
  Expliquez ce que vous aimeriez voir sur le panneau ou dans la page web, et pourquoi.
- 🐞 **Signaler un problème** : utilisez le formulaire
  [« Signaler un problème »](https://github.com/ulrikadam/OrbitDMD/issues/new/choose). Indiquez la version
  (tableau de bord), votre matériel et, si possible, le journal de la page **Système**.
  **N'y collez jamais votre mot de passe Wi-Fi ni celui de la page web** (le fichier `Config/config.txt` en contient).
- 📷 **Montrer votre installation** : photos et vidéos sont les bienvenues dans les
  [discussions](https://github.com/ulrikadam/OrbitDMD/discussions) (catégorie *Show and tell*).
- 🌍 **Traduire** l'interface ou la documentation.

## Proposer du code

1. Ouvrez d'abord une discussion ou une proposition pour les gros changements, afin d'en parler avant.
2. Créez une copie du dépôt (*fork*) et une branche pour votre modification.
3. Suivez [docs/CONSTRUIRE.md](docs/CONSTRUIRE.md) pour compiler et tester (le faux panneau
   `dmd/test/mock_web.py` permet de travailler sur la page web sans matériel).
4. Ouvrez une *pull request* qui explique le changement et comment vous l'avez testé.

Quelques règles :

- le programme tourne **sans Linux** sur un Raspberry Pi 3 : mémoire et processeur sont limités, et le
  rafraîchissement des panneaux (cœur 3) ne doit jamais être ralenti ;
- gardez le style du code existant (tabulations, noms en anglais, commentaires et textes affichés en français) ;
- n'ajoutez que des polices, images et GIF dont la licence permet la diffusion, et citez leur source dans
  `outils/installateur/carte/LICENCES.txt` ;
- ne mettez jamais de mot de passe ni de fichier `config.txt` personnel dans le dépôt.

## Versions

Les numéros suivent la règle MAJEUR.MINEUR.CORRECTIF décrite dans
[JOURNAL-DES-VERSIONS.md](JOURNAL-DES-VERSIONS.md) ; le numéro se change dans le fichier [VERSION](VERSION).

En contribuant, vous acceptez que votre contribution soit diffusée sous la licence du projet (GNU GPL v3).
