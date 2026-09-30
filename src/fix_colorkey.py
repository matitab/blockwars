r"""
Deja los PNG listos para el colorkey negro de Blob Wars.

- Si el PNG tiene transparencia (alfa), la mezcla sobre negro puro (0,0,0).
- Los pixeles casi negros (por ejemplo 1,1,2 o 2,2,3, tipico de los reescaladores)
  se convierten en negro exacto (0,0,0), asi el juego los toma como transparentes.
- Guarda todo como RGB (sin canal alfa), igual que los originales.

Uso:
    python fix_colorkey.py <carpeta_entrada> <carpeta_salida> [umbral]

Ejemplo:
    python fix_colorkey.py gfx\grasslands gfx_fixed\grasslands
El umbral por defecto es 8: un pixel cuyo canal mas alto sea <= 8 pasa a (0,0,0).
Los archivos originales no se modifican.
"""
import sys, os
import numpy as np
from PIL import Image

def fix(src, dst, thr):
    im = Image.open(src).convert("RGBA")
    a = np.array(im).astype(np.float32)
    alpha = a[..., 3:] / 255.0
    rgb = a[..., :3] * alpha                     # mezcla sobre negro
    rgb[rgb.max(axis=2) <= thr] = 0              # casi negro -> negro exacto
    Image.fromarray(np.clip(rgb, 0, 255).astype(np.uint8), "RGB").save(dst)

def main():
    if len(sys.argv) < 3:
        print(__doc__); sys.exit(1)
    src_dir, dst_dir = sys.argv[1], sys.argv[2]
    thr = int(sys.argv[3]) if len(sys.argv) > 3 else 8
    n = 0
    for root, _, files in os.walk(src_dir):
        for f in files:
            if not f.lower().endswith(".png"):
                continue
            sp = os.path.join(root, f)
            dp = os.path.join(dst_dir, os.path.relpath(sp, src_dir))
            os.makedirs(os.path.dirname(dp), exist_ok=True)
            fix(sp, dp, thr); n += 1
    print(f"{n} archivos procesados -> {dst_dir}")

if __name__ == "__main__":
    main()
