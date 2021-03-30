import os
from enum import Enum
from PIL import Image

TILE_SIZE = 8

Mario_Blocks = {
    'SKY': 0,
    'C_B': 1,
    'BRK': 2,
    'M_B': 3,
    'PTL': 4,
    'PTR': 5,
    'PBL': 6,
    'PBR': 7,
    'CTL': 8,
    'CTM': 9,
    'CTR': 10,
    'CBL': 11,
    'CBM': 12,
    'CBR': 13,
    'B_L': 14,
    'B_M': 15,
    'B_R': 16,
    'H_T': 17,
    'H_L': 18,
    'H_R': 19,
    'H_S': 20,
    'H_B': 21,
    'S_B': 22,
    'F_T': 23,
    'FPL': 24,
    'CRS': 25,
    'CRB': 26,
    'CWL': 27,
    'CBK': 28,
    'CWR': 29,
    'CDT': 30,
    'CDB': 31,
}

def convert_csv_to_tilemap(map_file, tilemap):
    line_num = 0
    with open(tilemap, 'wb') as target:
        with open(map_file, 'r') as csv:
            line = csv.readline()
            while line:
                row = line.split(',')
                for i in row:
                    if i.strip():
                        target.write(bytes([Mario_Blocks[i.strip()]]))
                        print(Mario_Blocks[i.strip()], end=" ")
                print()
                line = csv.readline()
                line_num += 1
    with open(map_file, 'r') as csv:
        line = csv.readline()
        row = line.split(',')
    print(f"World has {line_num} columns and {len(row)} blocks per column")
    
def binary_string_to_byte_array(binary_string):
    byte_array = []
    for i in range(0, len(binary_string), 8):
        binary_byte = binary_string[i:i+8]
        byte_array.append(int(binary_byte,2))
    return bytes(byte_array)

def convert_csv_to_collision_map(map_file, collision_map):
    line_num = 0
    with open(collision_map, 'wb') as target:
        with open(map_file, 'r') as csv:
            line = csv.readline()
            while line:
                row = ''.join(line.split(',')).strip().zfill(16)
                target.write(binary_string_to_byte_array(row))
                print(row)
                line = csv.readline()
                line_num += 1
    print(f"World has {line_num} columns and 2 bytes per column")

    
def convert_tilesheet_to_2bpp(sheet_filename, out_filename):
    input_img = Image.open(sheet_filename)
    colors = []
    dims = input_img.size
    x_tiles = dims[0] // TILE_SIZE
    y_tiles = dims[1] // TILE_SIZE
    with open(out_filename, 'wb') as out_file:
        for y_tile in range(y_tiles):
            for x_tile in range(x_tiles):
                tile_root = (x_tile * TILE_SIZE, y_tile * TILE_SIZE)
                tile_bounds = (tile_root[0], tile_root[1],
                            tile_root[0] + TILE_SIZE, tile_root[1] + TILE_SIZE)
                tile = input_img.crop(tile_bounds)

                out_file.write(convert_tile_to_bytes(tile))
    print(dims[0] * dims[1], (dims[0] * dims[1]) // 4, os.path.getsize(out_filename))

def convert_tile_to_bytes(tile):
    result = []
    colors = []
    for y in range(tile.size[1]):
        for x in range(tile.size[0]):
            colors.append(tile.getpixel((x, y)))
            if len(colors) == 4:
                b = colors[0] << 6 | colors[1] << 4 | colors[2] << 2 | colors[3]
                result.append(b)
                colors = []
    return bytes(result)

if __name__ == "__main__":
    # convert_csv_to_tilemap("SourceImages/Mario/MarioWorld1-1Map.csv", "SourceImages/Mario/MarioWorldMap.bin")
    # convert_csv_to_collision_map("SourceImages/Mario/MarioWorld1-1CollisionMap.csv", "SourceImages/Mario/MarioWorldCollisionMap.bin")
    convert_tilesheet_to_2bpp("resources/SourceImages/Mario/MarioTilesheet~bw.png", "resources/SourceImages/Mario/MarioTilesheet~bw.bin")
    convert_tilesheet_to_2bpp("resources/SourceImages/Mario/MarioSpritesheet~bw.png", "resources/SourceImages/Mario/MarioSpritesheet~bw.bin")
    