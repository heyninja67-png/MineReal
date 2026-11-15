#!/usr/bin/env python3
"""Generate original, tileable tangent-space wave normal maps for the UE water material."""
from pathlib import Path
import math
import numpy as np
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "Content" / "Water" / "Source"
OUT.mkdir(parents=True, exist_ok=True)
SIZE = 512


def make_height(seed: int) -> np.ndarray:
    y, x = np.mgrid[0:SIZE, 0:SIZE].astype(np.float32)
    x = x / SIZE * (2.0 * math.pi)
    y = y / SIZE * (2.0 * math.pi)
    # Integer spatial frequencies make the texture tile seamlessly at both edges.
    waves = [
        (1.00, 3 + seed % 2, 2 + seed % 3, 0.00 + seed * 0.31),
        (0.48, 7 + seed % 3, 5 + seed % 2, 1.20 + seed * 0.17),
        (0.24, 13 + seed % 2, 11 + seed % 3, 2.10 + seed * 0.27),
        (0.11, 23 + seed % 3, 19 + seed % 2, 0.70 + seed * 0.41),
        (0.06, 37 + seed % 2, 31 + seed % 3, 2.70 + seed * 0.23),
    ]
    height = np.zeros((SIZE, SIZE), dtype=np.float32)
    for amplitude, fx, fy, phase in waves:
        height += amplitude * np.sin(fx * x + fy * y + phase)
        height += amplitude * 0.45 * np.sin(-fy * x + fx * y * 0.73 + phase * 1.7)
    return height


def save_normal(path: Path, height: np.ndarray, strength: float) -> None:
    # Central differences with wrapped edges preserve the tileable seam.
    dx = (np.roll(height, -1, axis=1) - np.roll(height, 1, axis=1)) * 0.5 * strength
    dy = (np.roll(height, -1, axis=0) - np.roll(height, 1, axis=0)) * 0.5 * strength
    nx = -dx
    ny = -dy
    nz = np.ones_like(height)
    length = np.sqrt(nx * nx + ny * ny + nz * nz)
    normal = np.stack((nx / length, ny / length, nz / length), axis=-1)
    pixels = np.clip((normal * 0.5 + 0.5) * 255.0, 0, 255).astype(np.uint8)
    alpha = np.full((SIZE, SIZE, 1), 255, dtype=np.uint8)
    Image.fromarray(np.concatenate((pixels, alpha), axis=-1), mode="RGBA").save(path)


save_normal(OUT / "T_WaterNormal_A.png", make_height(2), strength=2.7)
save_normal(OUT / "T_WaterNormal_B.png", make_height(7), strength=1.9)
print(f"Generated two tileable water normal maps in: {OUT}")
