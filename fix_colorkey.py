r"""
Deja los PNG listos para el colorkey negro de Blob Wars, reemplazandolos directamente.

- Si el PNG tiene transparencia (alfa), la mezcla sobre negro puro (0,0,0).
- Los pixeles casi negros (por ejemplo 1,1,2 o 2,2,3, tipico de los reescaladores)
  pasan a negro exacto (0,0,0), asi el juego los toma como transparentes.
- Guarda todo como RGB (sin canal alfa), igual que los originales.

Uso (desde la carpeta del proyecto):
    python fix_colorkey.py gfx\grasslands
    python fix_colorkey.py gfx            (todas las carpetas de gfx)

Antes de reemplazar, guarda una copia de cada archivo en .\backup_colorkey\
(fuera de gfx, para que no entre en el pak). Podes borrarla cuando verifiques
que todo se ve bien. Umbral opcional como segundo argumento (por defecto 8).
"""
import sys, os, shutil
import numpy as np
from PIL import Image

def fix_in_place(path, thr):
    im = Image.open(path).convert("RGBA")
    a = np.array(im).astype(np.float32)
    rgb = a[..., :3] * (a[..., 3:] / 255.0)      # mezcla sobre negro
    rgb[rgb.max(axis=2) <= thr] = 0              # casi negro -> negro exacto
    Image.fromarray(np.clip(rgb, 0, 255).astype(np.uint8), "RGB").save(path)

def main():
    if len(sys.argv) < 2:
        print(__doc__); sys.exit(1)
    folder = sys.argv[1]
    thr = int(sys.argv[2]) if len(sys.argv) > 2 else 8
    backup_root = os.path.join(os.getcwd(), "backup_colorkey", os.path.basename(os.path.normpath(folder)))
    n = 0
    for root, _, files in os.walk(folder):
        for f in files:
            if not f.lower().endswith(".png"):
                continue
            p = os.path.join(root, f)
            bp = os.path.join(backup_root, os.path.relpath(p, folder))
            os.makedirs(os.path.dirname(bp), exist_ok=True)
            if not os.path.exists(bp):           # no pisar un backup anterior
                shutil.copy2(p, bp)
            fix_in_place(p, thr); n += 1
    print(f"{n} archivos reemplazados en {folder}. Backup en {backup_root}")

if __name__ == "__main__":
    main()
