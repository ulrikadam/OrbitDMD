"""Integre les fichiers de l'interface web au programme du Pi (tableaux C).

Usage : py -3.12 -I web2h.py <fichiers.txt> <sortie.h>
fichiers.txt : une ligne par fichier servi -> "chemin  fichier  type MIME"
Un meme fichier peut etre servi sous plusieurs chemins : il n'est integre qu'une fois.

Cache du navigateur (le Wi-Fi du panneau perd des paquets : moins de requetes = plus rapide) :
- dans les pages HTML, chaque lien "/style.css", "/app.js", "/logo.png"... recoit une empreinte
  de son contenu ("/app.js?v=1a2b3c4d") ; ces fichiers sont gardes un an par le navigateur,
  et une nouvelle version change l'empreinte, donc le lien ;
- les pages HTML elles-memes sont redemandees a chaque visite (no-cache).
Circle n'a pas de champ pour les en-tetes en plus : Cache-Control est ajoute apres le type
(le serveur ecrit "Content-Type: <type>\\r\\n").
"""
import hashlib
import os
import sys

CACHE_PAGE = 'no-cache'
CACHE_ASSET = 'public, max-age=31536000, immutable'


def is_html(ctype):
    return ctype.startswith('text/html')


def main():
    manifest, out = sys.argv[1], sys.argv[2]
    base = os.path.dirname(os.path.abspath(manifest))
    entries = []
    for line in open(manifest, encoding='utf-8'):
        line = line.strip()
        if not line or line.startswith('#'):
            continue
        path, name, ctype = line.split(None, 2)
        entries.append((path, name, ctype))

    contents = {}
    for path, name, ctype in entries:
        if name not in contents:
            contents[name] = open(os.path.join(base, name), 'rb').read()

    # empreinte de chaque fichier qui n'est pas une page ; les scripts qui chargent d'autres
    # fichiers ("/lang-en.js" dans app.js) recoivent leurs empreintes, puis la leur
    def stamp_all():
        return {path: hashlib.sha1(contents[name]).hexdigest()[:8]
                for path, name, ctype in entries if not is_html(ctype)}

    def with_stamps(data, stamps):
        text = data.decode('utf-8')
        for asset, stamp in stamps.items():
            text = text.replace('"%s"' % asset, '"%s?v=%s"' % (asset, stamp))
        return text.encode('utf-8')

    stamps = stamp_all()
    for path, name, ctype in entries:
        if 'javascript' in ctype:
            contents[name] = with_stamps(contents[name], {a: s for a, s in stamps.items() if a != path})
    stamps = stamp_all()

    arrays = {}
    lines = ['// genere automatiquement par outils/web2h.py depuis web/ : ne pas modifier',
             '#ifndef _webfiles_h', '#define _webfiles_h', '',
             'struct TWebFile', '{', '\tconst char *pPath;', '\tconst char *pType;',
             '\tconst unsigned char *pData;', '\tunsigned nLength;', '};', '']
    total = 0
    for path, name, ctype in entries:
        if name in arrays:
            continue
        data = contents[name]
        if is_html(ctype):
            data = with_stamps(data, stamps)
        total += len(data)
        var = 's_Web_' + ''.join(c if c.isalnum() else '_' for c in name)
        arrays[name] = (var, len(data))
        lines.append('static const unsigned char %s[%d] =\n{' % (var, len(data)))
        for i in range(0, len(data), 24):
            lines.append('\t' + ','.join('%d' % b for b in data[i:i + 24]) + ',')
        lines.append('};\n')

    lines.append('static const TWebFile s_WebFiles[] =\n{')
    for path, name, ctype in entries:
        var, n = arrays[name]
        cache = CACHE_PAGE if is_html(ctype) else CACHE_ASSET
        lines.append('\t{"%s", "%s\\r\\nCache-Control: %s", %s, %d},' % (path, ctype, cache, var, n))
    lines.append('\t{0, 0, 0, 0}\n};\n\n#endif')
    open(out, 'w', encoding='ascii', newline='\n').write('\n'.join(lines) + '\n')
    print('%d fichiers web (%d octets) -> %s' % (len(arrays), total, out))


if __name__ == '__main__':
    main()
