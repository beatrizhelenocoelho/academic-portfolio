import cv2
import numpy as np
from collections import Counter


img = cv2.imread("mapa_acertos4.pgm", cv2.IMREAD_GRAYSCALE)

if img is None:
    print("Erro na imagem")
    exit(1)

unique_vals, counts = np.unique(img, return_counts=True)

print("=== Distribuição das intensidades (valores de 0 a 255) ===")
for val, count in zip(unique_vals, counts):
    print(f"Valor {val:>3}: {count} vezes")
