"""Traductions d'OrbitDMD : fichiers de langue de l'interface web, fichier de l'installateur.

  py -3.12 -I langues.py <web.txt> <dossier web>         ecrit dmd/web/lang-xx.js
  py -3.12 -I langues.py --fusion <installateur.txt> <sortie>   une ligne, 12 colonnes (installateur)
  py -3.12 -I langues.py --liste <web.txt>               numero et texte francais de chaque ligne

<nom>.txt : une ligne par texte, colonnes separees par " | " :
    francais | anglais | neerlandais | espagnol | russe
<nom>.<langue>.txt (de, it, pt, pl, uk, tr, cs) : une ligne par texte, "numero | traduction",
le numero etant celui du texte francais dans <nom>.txt (voir --liste).
Le francais est le texte exact (espaces regroupes). "{}" : partie variable ; la traduction
reprend les parties dans l'ordre avec "{}", ou dans un autre ordre avec "{1}", "{2}"...
Une traduction absente : le texte reste en francais. Lignes "#" : commentaires.
"""
import json
import os
import re
import sys

COLONNES = ['en', 'nl', 'es', 'ru']
EN_PLUS = ['de', 'it', 'pt', 'pl', 'uk', 'tr', 'cs']
LANGS = COLONNES + EN_PLUS


def parties(t):
    numeros = [int(x) for x in re.findall(r'\{(\d)\}', t)]
    return max(numeros) if numeros else t.count('{}')


def lire(chemin):
    """-> liste des textes francais (dans l'ordre) et {langue: {francais: traduction}}"""
    cles, bruts, dicts, erreurs = [], [], {l: {} for l in LANGS}, []
    for n, ligne in enumerate(open(chemin, encoding='utf-8'), 1):
        ligne = ligne.rstrip('\r\n')
        if not ligne.strip() or ligne.startswith('#'):
            continue
        cols = ligne.split(' | ')
        if len(cols) != 1 + len(COLONNES):
            erreurs.append('%s ligne %d : %d colonnes au lieu de %d' % (chemin, n, len(cols), 1 + len(COLONNES)))
            continue
        fr = re.sub(r'\s+', ' ', cols[0]).strip()
        if fr in cles:
            erreurs.append('%s ligne %d : texte en double : %s' % (chemin, n, fr))
            continue
        cles.append(fr)
        bruts.append(cols[0].strip())
        for l, t in zip(COLONNES, cols[1:]):
            if t.strip():
                dicts[l][fr] = t.strip()

    base = os.path.splitext(chemin)[0]
    for l in EN_PLUS:
        fichier = '%s.%s.txt' % (base, l)
        if not os.path.exists(fichier):
            continue
        for n, ligne in enumerate(open(fichier, encoding='utf-8'), 1):
            ligne = ligne.rstrip('\r\n')
            if not ligne.strip() or ligne.startswith('#'):
                continue
            num, sep, t = ligne.partition(' | ')
            if not sep or not num.strip().isdigit() or not 1 <= int(num) <= len(cles):
                erreurs.append('%s ligne %d : numero absent ou invalide' % (fichier, n))
                continue
            fr = cles[int(num) - 1]
            if fr in dicts[l]:
                erreurs.append('%s ligne %d : numero %s en double' % (fichier, n, num))
            if t.strip():
                dicts[l][fr] = t.strip()

    for l in LANGS:
        for fr, t in dicts[l].items():
            if parties(t) != fr.count('{}'):
                erreurs.append('%s (%s) : %d parties variables au lieu de %d : %s' % (fr, l, parties(t), fr.count('{}'), t))
    if erreurs:
        print('\n'.join(erreurs))
        sys.exit(1)
    return cles, dicts, bruts


def main():
    if sys.argv[1] == '--liste':
        cles, _, _ = lire(sys.argv[2])
        sys.stdout.reconfigure(encoding='utf-8')
        for i, fr in enumerate(cles, 1):
            print('%d | %s' % (i, fr))
        return

    if sys.argv[1] == '--fusion':
        cles, dicts, bruts = lire(sys.argv[2])
        with open(sys.argv[3], 'w', encoding='utf-8', newline='\n') as f:
            for fr, brut in zip(cles, bruts):
                f.write(' | '.join([brut] + [dicts[l].get(fr, '') for l in LANGS]) + '\n')
        print('%s : %d textes, %d langues' % (sys.argv[3], len(cles), 1 + len(LANGS)))
        return

    tsv, out = sys.argv[1], sys.argv[2]
    cles, dicts, _ = lire(tsv)
    for l in LANGS:
        path = os.path.join(out, 'lang-%s.js' % l)
        body = json.dumps(dicts[l], ensure_ascii=False, separators=(',', ':'), sort_keys=True)
        text = '/* genere par outils/langues.py depuis outils/traductions/web*.txt : ne pas modifier */\nI18N.start(%s);\n' % body
        old = open(path, encoding='utf-8').read() if os.path.exists(path) else ''
        if old != text:		# fichier inchange : pas de nouvelle compilation
            open(path, 'w', encoding='utf-8', newline='\n').write(text)
        print('%s : %d / %d textes' % (path, len(dicts[l]), len(cles)))


if __name__ == '__main__':
    main()
