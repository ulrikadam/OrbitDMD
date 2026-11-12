# OrbitDMD et Home Assistant

**Français** · [English](HOME-ASSISTANT.en.md) · [Nederlands](HOME-ASSISTANT.nl.md) · [Español](HOME-ASSISTANT.es.md) · [Русский](HOME-ASSISTANT.ru.md) · [Deutsch](HOME-ASSISTANT.de.md) · [Italiano](HOME-ASSISTANT.it.md) · [Português](HOME-ASSISTANT.pt.md) · [Polski](HOME-ASSISTANT.pl.md) · [Українська](HOME-ASSISTANT.uk.md) · [Türkçe](HOME-ASSISTANT.tr.md) · [Čeština](HOME-ASSISTANT.cs.md)

À partir de la version 1.4.0, le panneau se relie à Home Assistant par **MQTT** et y apparaît tout seul
(découverte automatique). Les noms des éléments suivent la langue du panneau (Réglages → Affichage →
Langue du panneau).

## Mise en route

1. Dans Home Assistant : **Paramètres → Modules complémentaires → Boutique**, installez et démarrez
   **Mosquitto broker**. Ajoutez ensuite l'intégration **MQTT** si Home Assistant ne la propose pas tout seul.
2. Créez un utilisateur pour le panneau (**Paramètres → Personnes → Utilisateurs**), par exemple `orbitdmd`.
3. Sur la page web du panneau : **Réglages → Home Assistant** : activez, indiquez **l'adresse IP** de
   Home Assistant (par exemple `192.168.0.10`), le port `1883`, l'utilisateur et son mot de passe, puis
   enregistrez. Le panneau redémarre et se connecte.
4. Dans Home Assistant : **Paramètres → Appareils et services → MQTT → OrbitDMD**.

La page web du panneau a aussi une page **Aide Home Assistant** (`http://<panneau>/aide`) : toutes les
commandes et des exemples prêts à copier, avec le vrai nom de votre panneau.

La carte Home Assistant de la page Réglages indique si la connexion fonctionne. En cas de souci, le
journal de la page Système contient les lignes `homeassistant`.

## Ce qui apparaît dans Home Assistant

| Entité | Type | Rôle |
|---|---|---|
| Panneau | lumière | Allumer, éteindre (la page web reste joignable), régler la luminosité |
| Luminosité heure par heure | interrupteur | Activé : la luminosité suit les réglages heure par heure du panneau. Régler la luminosité depuis Home Assistant le désactive |
| Pause | interrupteur | Figer l'image |
| Afficher l'heure, Afficher la météo | boutons | Tout de suite, pendant 15 secondes |
| Redémarrer | bouton | |
| Message | notification | Afficher un texte tout de suite |
| Texte à afficher | texte | Tapez un texte dans Home Assistant et validez : il s'affiche tout de suite |
| Effet, Couleur, Passages, Vitesse des messages | listes et nombres | Le style des textes envoyés depuis Home Assistant (gardé après un redémarrage) |
| Texte : …, Compte à rebours : … | interrupteurs | Activer ou non chaque texte et compte à rebours de la page Messages |
| Commandes MQTT | capteur (diagnostic) | La liste de toutes les commandes, dans ses attributs |
| GIF affichée, Écran | capteurs | Ce que montre le panneau |
| Température du processeur, Adresse IP, Version | capteurs (diagnostic) | |

Si le panneau est éteint ou débranché, Home Assistant l'indique « indisponible ».

## Messages

Le plus simple, dans une automatisation :

```yaml
action: notify.send_message
target:
  entity_id: notify.orbitdmd_message
data:
  message: "Quelqu'un sonne !"
```

(le nom exact de l'entité est visible sur la page de l'appareil OrbitDMD).

Le style (effet, couleur, passages, vitesse) est celui réglé dans les éléments « … des messages » de
l'appareil. Pour un style à part pour un seul message, publiez un texte JSON sur le sujet `orbitdmd/<nom>/notification`
(`<nom>` : le nom du panneau sur le réseau, en minuscules) :

```yaml
action: mqtt.publish
data:
  topic: orbitdmd/orbitdmd/notification
  payload: >
    {"message": "Lave-linge terminé", "effet": "pluie", "couleur": "arc-en-ciel", "passages": 2}
```

| Champ | Valeurs | Par défaut |
|---|---|---|
| `message` | le texte (200 caractères au plus) | |
| `effet` | `defile`, `inverse`, `monte`, `descend`, `lettres`, `pluie`, `fondu` | `defile` |
| `couleur` | `FF8000` (rouge-vert-bleu) ou `arc-en-ciel` | blanc |
| `police` | `Pixel` ou un fichier de police du panneau, par exemple `Orbitron.ttf` | `Pixel` |
| `taille` | taille en pixels, `0` = automatique | `0` |
| `vitesse` | 1 (lent) à 10 (rapide) | 5 |
| `passages` | 1 à 10 | 2 |
| `gif` | une GIF à jouer avant le texte : `Categorie/nom.gif` | aucune |

Le message passe tout de suite, par-dessus les GIF, puis la rotation reprend. Si le panneau est éteint
ou en pause, le message attend qu'il soit rallumé.

## Exemples d'automatisations

- Baisser la luminosité au coucher du soleil : action `light.turn_on` sur « Panneau » avec
  `brightness_pct: 20`.
- Éteindre le panneau quand tout le monde quitte la maison : `light.turn_off`.
- Sonnette : un message « Quelqu'un sonne ! » avec `"gif": "Accueil/sonnette.gif"` (une GIF ajoutée
  par vous dans la catégorie de votre choix).

## Limites

- Connexion MQTT non chiffrée (pas de TLS) : à utiliser sur le réseau de la maison.
- L'adresse `homeassistant.local` ne fonctionne pas : indiquez l'adresse IP de Home Assistant.
