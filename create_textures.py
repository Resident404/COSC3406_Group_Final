#!/usr/bin/env python3
"""
Generate simple procedural textures for the game
"""
import numpy as np
from PIL import Image
import os

def create_checker_texture(filename, color1, color2, size=256, checker_size=32):
    """Create a checkerboard texture"""
    img = np.zeros((size, size, 3), dtype=np.uint8)
    for i in range(size):
        for j in range(size):
            if ((i // checker_size) + (j // checker_size)) % 2 == 0:
                img[i, j] = color1
            else:
                img[i, j] = color2
    Image.fromarray(img).save(filename)
    print(f"Created {filename}")

def create_striped_texture(filename, color1, color2, size=256, stripe_size=16):
    """Create a striped texture"""
    img = np.zeros((size, size, 3), dtype=np.uint8)
    for i in range(size):
        for j in range(size):
            if (i // stripe_size) % 2 == 0:
                img[i, j] = color1
            else:
                img[i, j] = color2
    Image.fromarray(img).save(filename)
    print(f"Created {filename}")

def create_metallic_texture(filename, base_color, size=256):
    """Create a metallic-looking texture"""
    img = np.zeros((size, size, 3), dtype=np.uint8)
    for i in range(size):
        for j in range(size):
            # Add some noise for metallic effect
            noise = np.random.randint(-20, 20)
            for k in range(3):
                img[i, j, k] = np.clip(base_color[k] + noise, 0, 255)
    Image.fromarray(img).save(filename)
    print(f"Created {filename}")

def create_brick_texture(filename, brick_color, mortar_color, size=256):
    """Create a brick wall texture"""
    img = np.zeros((size, size, 3), dtype=np.uint8)
    brick_height = 32
    brick_width = 64
    mortar_size = 4

    for i in range(size):
        for j in range(size):
            # Determine if we're on a mortar line
            row = i // brick_height
            offset = (row % 2) * (brick_width // 2)
            j_offset = (j + offset) % (brick_width + mortar_size)

            if i % brick_height < mortar_size or j_offset < mortar_size:
                img[i, j] = mortar_color
            else:
                # Add some variation to brick color
                noise = np.random.randint(-15, 15)
                for k in range(3):
                    img[i, j, k] = np.clip(brick_color[k] + noise, 0, 255)

    Image.fromarray(img).save(filename)
    print(f"Created {filename}")

def create_grass_texture(filename, size=256):
    """Create a simple grass texture"""
    img = np.zeros((size, size, 3), dtype=np.uint8)
    base_green = np.array([50, 150, 50])

    for i in range(size):
        for j in range(size):
            # Random grass blades
            if np.random.random() < 0.1:
                noise = np.random.randint(-30, 30)
            else:
                noise = np.random.randint(-10, 10)
            for k in range(3):
                img[i, j, k] = np.clip(base_green[k] + noise, 0, 255)

    Image.fromarray(img).save(filename)
    print(f"Created {filename}")

# Create textures for different game elements
print("Generating textures...")

# Blue metallic texture for player
create_metallic_texture("player_texture.png", [70, 130, 220], 256)

# Gray road texture for ground
create_striped_texture("ground_texture.png", [80, 80, 80], [70, 70, 70], 256, 8)

# Orange warning texture for obstacles
create_checker_texture("obstacle_texture.png", [255, 140, 0], [255, 100, 0], 256, 16)

# Yellow texture for lane dividers
create_striped_texture("lane_divider_texture.png", [255, 255, 0], [240, 240, 0], 256, 4)

# Green texture for trees
create_grass_texture("tree_texture.png", 256)

# Purple texture for buildings
create_brick_texture("building_texture.png", [128, 0, 128], [90, 0, 90], 256)

# White/gray texture for tunnel
create_brick_texture("tunnel_texture.png", [200, 200, 200], [150, 150, 150], 256)

print("\nAll textures created successfully!")
print("Texture files:")
print("  - player_texture.png (blue metallic)")
print("  - ground_texture.png (gray striped)")
print("  - obstacle_texture.png (orange checkerboard)")
print("  - lane_divider_texture.png (yellow striped)")
print("  - tree_texture.png (green grass-like)")
print("  - building_texture.png (purple bricks)")
print("  - tunnel_texture.png (gray bricks)")
