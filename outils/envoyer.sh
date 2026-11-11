#!/bin/bash
# Travail a distance avec le panneau, par le Wi-Fi (depuis Git Bash).
#
#   bash outils/envoyer.sh etat        etat du panneau
#   bash outils/envoyer.sh journal     journal du panneau
#   bash outils/envoyer.sh programme   compile, envoie kernel8.img, attend le redemarrage
#   bash outils/envoyer.sh config      envoie carte-sd/Config/config.txt, attend le redemarrage
#                                      (refuse si le panneau a ete modifie depuis : FORCE=1 pour forcer)
#   bash outils/envoyer.sh recuperer-config   copie le config.txt du panneau dans carte-sd/Config/
#   bash outils/envoyer.sh gif <categorie> <fichier.gif>...   ajoute des GIF sur le panneau
#   bash outils/envoyer.sh redemarrer  redemarre le panneau
#
# Adresse du panneau : variable RPI2DMD_HOST (par defaut orbitdmd.local)
# Le mot de passe est lu dans carte-sd/Config/config.txt (Web_Password) et n'est
# jamais affiche ni passe en ligne de commande.

set -e
P="$(cd "$(dirname "$0")/.." && pwd)"
HOST="${RPI2DMD_HOST:-orbitdmd.local}"
URL="http://$HOST"
CONFIG="$P/carte-sd/Config/config.txt"
BASE="$P/sauvegarde-config/panneau-derniere-version.txt"	# config.txt tel qu'il est sur le panneau

password () {
	grep -m1 '^Web_Password=' "$CONFIG" | cut -d= -f2- | tr -d '\r'
}

version () {
	curl -s --max-time 5 "$URL/api/etat" | sed -n 's/.*"programme":"\([^"]*\)".*/\1/p'
}

# envoi d'un formulaire : $1 = chemin, $2 = fichier (facultatif)
post () {
	local PW
	PW="$(password)"
	if [ -z "$PW" ]; then
		echo "Web_Password absent de $CONFIG" >&2
		exit 1
	fi
	if [ -n "$2" ]; then
		# curl (programme Windows) attend un chemin Windows, pas /c/...
		local FILE
		FILE="$(cygpath -w "$2")"
		printf '%s' "$PW" | curl -s -S --max-time 120 -F "password=<-" -F "fichier=@$FILE" "$URL$1"
	else
		printf '%s' "$PW" | curl -s -S --max-time 30 -F "password=<-" "$URL$1"
	fi
}

uptime_now () {
	curl -s --max-time 5 "$URL/api/etat" | sed -n 's/.*"uptime_s":\([0-9]*\).*/\1/p'
}

# attend que le panneau ait redemarre (temps de fonctionnement remis a zero) ;
# la box peut mettre plusieurs minutes a lui redonner une adresse (DHCP)
wait_restart () {
	local START=$(date +%s) UP ELAPSED
	echo "Attente du redemarrage (la box peut mettre quelques minutes a redonner une adresse)..."
	sleep 8
	while true; do
		ELAPSED=$(( $(date +%s) - START ))
		[ $ELAPSED -ge 480 ] && break
		UP="$(uptime_now)"
		if [ -n "$UP" ] && [ "$UP" -le $(( ELAPSED + 5 )) ]; then
			echo "Panneau de retour apres $ELAPSED s : programme compile le $(version)"
			return 0
		fi
		sleep 4
	done
	echo "Le panneau ne repond pas apres 8 min : verifier l'ecran (ou http://192.168.0.151/)." >&2
	return 1
}

case "$1" in
	etat)
		curl -s -S --max-time 5 "$URL/api/etat"; echo
		;;
	journal)
		curl -s -S --max-time 10 "$URL/journal"
		;;
	programme)
		source "$P/outils/env.sh"
		(cd "$P/dmd" && make -j8 | grep -E "error|WC" || true)
		cp "$P/dmd/kernel8.img" "$P/carte-sd/kernel8.img"
		OLD="$(version)"
		echo "Version en marche : ${OLD:-inconnue}"
		REPLY_TEXT="$(post /maj "$P/dmd/kernel8.img")"
		echo "$REPLY_TEXT"
		case "$REPLY_TEXT" in
			OK*)	wait_restart || exit 1
				[ "$(version)" != "$OLD" ] || echo "Attention : la version en marche n'a pas change" >&2 ;;
			*)	exit 1 ;;
		esac
		;;
	gif)
		CATEGORY="$2"
		shift 2 || { echo "Usage : envoyer.sh gif <categorie> <fichier.gif>..." >&2; exit 1; }
		OK=0; TOTAL=0
		for FILE in "$@"; do
			TOTAL=$((TOTAL + 1))
			PW="$(password)"
			REPLY_TEXT="$(printf '%s' "$PW" | curl -s -S --max-time 120 -F "password=<-" \
				-F "categorie=$CATEGORY" -F "fichier=@$(cygpath -w "$FILE")" "$URL/api/gif/ajouter")"
			echo "$REPLY_TEXT"
			case "$REPLY_TEXT" in OK*) OK=$((OK + 1)) ;; esac
		done
		echo "$OK / $TOTAL GIF ajoutee(s) dans $CATEGORY"
		;;
	recuperer-config)
		TMP="$(mktemp)"
		post /api/config/lire > "$TMP"
		if [ ! -s "$TMP" ] || head -c 7 "$TMP" | grep -q "^ERREUR"; then
			cat "$TMP"; rm -f "$TMP"; exit 1
		fi
		mkdir -p "$P/sauvegarde-config"
		cp "$TMP" "$BASE"		# derniere version connue du panneau
		if cmp -s "$TMP" "$CONFIG"; then
			echo "config.txt identique sur le panneau et sur le PC"
		else
			# copie de secours hors de carte-sd/ (pour qu'elle ne parte pas sur la carte)
			cp "$CONFIG" "$P/sauvegarde-config/config-$(date +%Y%m%d-%H%M%S).txt"
			cp "$TMP" "$CONFIG"
			echo "config.txt du panneau copie dans carte-sd/Config/ (ancienne version dans sauvegarde-config/)"
		fi
		rm -f "$TMP"
		;;
	config)
		# le panneau a-t-il ete modifie (page web) depuis la derniere version connue ?
		if [ "$FORCE" != "1" ]; then
			if [ ! -f "$BASE" ]; then
				echo "Version du panneau inconnue : lancer d'abord 'recuperer-config'." >&2
				exit 1
			fi
			TMP="$(mktemp)"
			post /api/config/lire > "$TMP" || true
			if [ ! -s "$TMP" ] || head -c 7 "$TMP" | grep -q "^ERREUR"; then
				cat "$TMP"; rm -f "$TMP"; exit 1
			fi
			if ! cmp -s "$TMP" "$BASE"; then
				echo "Le config.txt du panneau a change depuis la derniere recuperation (lignes secretes masquees) :"
				diff <(grep -vE '^(Wifi_psk|Web_Password)=' "$BASE" | tr -d '\r') \
				     <(grep -vE '^(Wifi_psk|Web_Password)=' "$TMP" | tr -d '\r') || true
				echo "Envoi annule. 'recuperer-config' puis refaire la modification, ou FORCE=1 pour l'ecraser."
				rm -f "$TMP"
				exit 1
			fi
			rm -f "$TMP"
		fi
		OLD="$(version)"
		REPLY_TEXT="$(post /config "$CONFIG")"
		echo "$REPLY_TEXT"
		case "$REPLY_TEXT" in
			OK*)	mkdir -p "$P/sauvegarde-config"
				cp "$CONFIG" "$BASE"
				wait_restart ;;
			*)	exit 1 ;;
		esac
		;;
	redemarrer)
		OLD="$(version)"
		REPLY_TEXT="$(post /redemarrer)"
		echo "$REPLY_TEXT"
		case "$REPLY_TEXT" in OK*) wait_restart ;; *) exit 1 ;; esac
		;;
	*)
		sed -n '2,12p' "$0"
		exit 1
		;;
esac
