"""Compare le decodeur GIF du Pi (hosttest.exe gifdump) a Pillow, image par image.

Usage : py -3.12 -I compare_gifs.py <dossier_gif> [max_fichiers]
"""
import os
import subprocess
import sys
import tempfile

import numpy as np
from PIL import Image, ImageSequence

W, H = 128, 32
HERE = os.path.dirname(os.path.abspath(__file__))
EXE = os.path.join(HERE, 'out', 'hosttest.exe')


def pillow_frames(path):
    im = Image.open(path)
    frames, delays = [], []
    for frame in ImageSequence.Iterator(im):
        rgba = np.array(frame.convert('RGBA'), dtype=np.uint8)
        rgb = rgba[:, :, :3].copy()
        rgb[rgba[:, :, 3] == 0] = 0          # transparent -> noir, comme sur les panneaux
        frames.append(rgb)
        delays.append(frame.info.get('duration', 0))
    return frames, delays


def ours(path, tmp):
    out = subprocess.run([EXE, 'gifdump', path, tmp], capture_output=True, text=True)
    if out.returncode != 0:
        return None, None
    delays = [int(x) for x in out.stdout.split()]
    raw = np.fromfile(tmp, dtype=np.uint8)
    frames = raw.reshape(-1, H, W, 3)
    return frames, delays


def main():
    root = sys.argv[1]
    limit = int(sys.argv[2]) if len(sys.argv) > 2 else None
    files = []
    for dp, _, fn in os.walk(root):
        files += [os.path.join(dp, f) for f in fn if f.lower().endswith('.gif')]
    files.sort()
    if limit:
        files = files[:limit]

    tmp = os.path.join(tempfile.mkdtemp(), 'frames.rgb')
    ok = 0
    total_frames = 0
    problems = []
    for path in files:
        rel = os.path.relpath(path, root)
        a, da = ours(path, tmp)
        b, db = pillow_frames(path)
        if a is None:
            problems.append((rel, 'echec du decodage'))
            continue
        total_frames += len(a)
        if len(a) != len(b):
            problems.append((rel, 'images %d / Pillow %d' % (len(a), len(b))))
            continue
        worst = 0
        for fa, fb in zip(a, b):
            if fb.shape != (H, W, 3):
                continue
            diff = int(np.count_nonzero(np.any(fa != fb, axis=2)))
            worst = max(worst, diff)
        bad_delays = sum(1 for x, y in zip(da, db) if y >= 20 and x != y)
        if worst or bad_delays:
            problems.append((rel, 'pixels differents (pire image) %d, delais differents %d'
                             % (worst, bad_delays)))
        else:
            ok += 1

    print('%d fichiers, %d identiques a Pillow, %d images decodees au total'
          % (len(files), ok, total_frames))
    for rel, msg in problems[:40]:
        print('  %s : %s' % (rel, msg))
    if len(problems) > 40:
        print('  ... et %d autres' % (len(problems) - 40))


if __name__ == '__main__':
    main()
