import cv2
import numpy as np


gmap = cv2.imread("gmap_cortado.pgm", cv2.IMREAD_GRAYSCALE)
mapa_ajustado = cv2.imread("mapa_redimensionado_bem_odomb.pgm", cv2.IMREAD_GRAYSCALE)


#  ocupado (preto), 0 = livre (branco), -1 = desconhecido (cinza)
def preprocess(img):
    return np.where(img < 100, 1, np.where(img > 230, 0, -1)).astype(np.int8)

occ1 = preprocess(gmap)
occ2 = preprocess(mapa_ajustado)


mask = (occ1 != -1) & (occ2 != -1)

# verificar onde coincidem 
acertos = (occ1 == occ2) & mask

# imagem binária (255 = acerto, 0 = erro ou desconhecido) 
mapa_acertos = (acertos.astype(np.uint8)) * 255


cv2.imwrite("mapa_acertos3.png", mapa_acertos)
print("[✓] Mapa de acertos salvo como mapa_acertos.png")
