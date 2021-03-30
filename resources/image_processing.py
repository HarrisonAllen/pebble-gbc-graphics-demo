from PIL import Image, ImageDraw, ImageFont, ImageOps
import os
import glob
from itertools import permutations
import autopy
import numpy as np


CHUNK_SIZE = 16
TILE_SIZE = 8
SCREEN_SIZE = 18
PALETTE_MASK = 0xF0
H_FLIP_FLAG = 0b0100
V_FLIP_FLAG = 0b1000

def extract_tiles(filename, bounds=None, output="Output/", sheet=None):
    if os.path.exists(output):
        # files = glob.iglob(os.path.join(output,'*.png'))
        # for f in files:
        #     os.remove(f)
        pass
    else:
        os.makedirs(output)
    im = Image.open(filename)
    img_id = 0
    if bounds is None:
        bounds = ((0,0,im.size[0]-2,im.size[1]-2),)
    counter = 0
    for bound in bounds:
        region = im.crop(bound)
        # region.show()
        width = bound[2]-bound[0]
        height = bound[3]-bound[1]
        num_x_chunks = width // 17
        num_y_chunks = height // 17
        new_tilesheet = Image.new('RGB', (num_x_chunks * CHUNK_SIZE, num_y_chunks * CHUNK_SIZE))
        for y_chunk in range(num_y_chunks):
            for x_chunk in range(num_x_chunks):
                chunk_root = (x_chunk * 17 + 1, y_chunk * 17 + 1)
                chunk_bounds = (chunk_root[0], chunk_root[1],
                               chunk_root[0] + CHUNK_SIZE, chunk_root[1] + CHUNK_SIZE)
                chunk = region.crop(chunk_bounds)
                new_tilesheet.paste(chunk, (x_chunk * CHUNK_SIZE, y_chunk * CHUNK_SIZE))
                # chunk.save(f'{output}{img_id:03}.png')
                img_id += 1

        new_tilesheet.save(f'{output}colorsheet.png')

        num_y_tiles = new_tilesheet.size[1] // TILE_SIZE
        num_x_tiles = new_tilesheet.size[0] // TILE_SIZE
        # print('num tiles:', (num_y_tiles, num_x_tiles))
        gray_tilesheet = Image.new('RGB', new_tilesheet.size)
        palettes = set()
        for y_tile in range(num_y_tiles):
            for x_tile in range(num_x_tiles):
                tile_root = (x_tile * TILE_SIZE, y_tile * TILE_SIZE)
                tile_bounds = (tile_root[0], tile_root[1], tile_root[0] + TILE_SIZE,
                                tile_root[1] + TILE_SIZE)
                tile = new_tilesheet.crop(tile_bounds)
                try:
                    # converted_tile, palette = convert_tile_to_2bpp(tile, sheet)
                    palette = get_palette(tile)
                    print('success', counter)
                    counter += 1
                    palettes.add(frozenset(palette))
                except ValueError as e:
                    print(x_tile, y_tile)
                    raise(e)
                # gray_tilesheet.paste(converted_tile, (x_tile * TILE_SIZE, y_tile * TILE_SIZE))
                # converted_tile.show()
        colors = set()
        for p in palettes:
            for c in p:
                colors.add(c)
        palettes_tuple = tuple(palettes)
        skipped = 0
        palette_builder = []
        for i in palettes:
            if len(i) < 4:
                continue
            palette_builder.append(tuple(i))
        palette_builder = sorted(palette_builder)
        palette_img = Image.new('RGB', (4*8, 8*len(palette_builder)))
        palette_draw = ImageDraw.Draw(palette_img)
        for i in range(len(palette_builder)):
            p = sorted(palette_builder[i], reverse=True)
            for j in range(len(p)):
                start = (j*8, i*8-skipped*8)
                end = (j*8+8, i*8+8-skipped*8)
                palette_draw.rectangle((start, end), fill=p[j], width=0)
        palette_img.save(f'{output}palettes.png')
        return
        # gray_tilesheet.save(f'{output}tilesheet.png')

def get_palette(tile):
    t = tile.quantize(colors=4)
    return set([tuple(t.getpalette()[i:i+3]) for i in range(0, 12, 3)])

def convert_tile_to_2bpp(tile, reference_img):
    t = tile.quantize(colors=4)
    palette = set([tuple(t.getpalette()[i:i+3]) for i in range(0, 12, 3)])
    new_palette = list(palette)
    while len(new_palette) < 4:
        new_palette.append((1000, 1000, 1000))
    for p in permutations(new_palette):
        result = Image.new('RGB', tile.size)
        for y in range(tile.size[1]):
            for x in range(tile.size[0]):
                pixel = tile.getpixel((x,y))
                try:
                    p_val = 3 - p.index(pixel)
                except ValueError as e:
                    print(pixel, p)
                    tile.show()
                    raise(e)
                new_color = tuple([85 * p_val] * 3)
                result.putpixel((x, y), new_color)
        if find_tile_in_sheet(result, autopy.bitmap.Bitmap.open(reference_img)):
            return result, p
    print('sad yeet')
    
    i = 0
    for p in permutations(new_palette):
        result = Image.new('RGB', tile.size)
        for y in range(tile.size[1]):
            for x in range(tile.size[0]):
                pixel = tile.getpixel((x,y))
                try:
                    p_val = 3 - p.index(pixel)
                except ValueError as e:
                    print(pixel, p)
                    tile.show()
                    raise(e)
                new_color = tuple([85 * p_val] * 3)
                result.putpixel((x, y), new_color)
        result.save(f'brainstorming/Tiles/Failures/{i}.png')
        print(i,p)
        i += 1
        # print('sad yeet')
    tile.save('brainstorming/Tiles/Failures/color.png')
    return None


def search_for_tile_in_sheet(tile, sheet_image):
    tile.save('temp.png')

    input_image = autopy.bitmap.Bitmap.open('temp.png')

    loc = sheet_image.find_every_bitmap(input_image, tolerance=0)
    for l in loc:
        if np.isclose(l[0]/TILE_SIZE,int(l[0]/TILE_SIZE)) and np.isclose(l[1]/TILE_SIZE,int(l[1]/TILE_SIZE)):
            return l, sheet_image.size
    return None, None


def find_tile_in_sheet(tile, sheet_image):
    result, dims = search_for_tile_in_sheet(tile, sheet_image)
    if result:
        return (0, int(int(result[0]) // TILE_SIZE + (int(result[1]) // TILE_SIZE) * (dims[0] // TILE_SIZE)))

    result, dims = search_for_tile_in_sheet(ImageOps.mirror(tile), sheet_image)
    if result:
        return (H_FLIP_FLAG, int(int(result[0]) // TILE_SIZE + (int(result[1]) // TILE_SIZE) * (dims[0] // TILE_SIZE)))

    result, dims = search_for_tile_in_sheet(ImageOps.flip(tile), sheet_image)
    if result:
        return (V_FLIP_FLAG, int(int(result[0]) // TILE_SIZE + (int(result[1]) // TILE_SIZE) * (dims[0] // TILE_SIZE)))

    result, dims = search_for_tile_in_sheet(ImageOps.mirror(ImageOps.flip(tile)), sheet_image)
    if result:
        return (V_FLIP_FLAG | H_FLIP_FLAG, int(int(result[0]) // TILE_SIZE + (int(result[1]) // TILE_SIZE) * (dims[0] // TILE_SIZE)))

    return None

"""
Create tile swatches for:
ANIMATED_TILES_TIDE
ANIMATED_TILES_VILLAGE
ANIMATED_TILES_CURRENTS
ANIMATED_TILES_WATERFALL
ANIMATED_TILES_WATERFALL_SLOW
ANIMATED_TILES_WEATHER_VANE
"""

def convert_pil_to_bitmap(pil_image):
    pil_image.save('temp_image.png')
    return autopy.bitmap.Bitmap.open('temp_image.png')

def generate_tilemap_from_map(map_file, tile_folder):
    map_im = Image.open(map_file)
    main_img = Image.open(os.path.join(tile_folder, 'overworld_1.cgb.png'))
    swap_img = Image.open(os.path.join(tile_folder, 'overworld_2.cgb.png'))
    anim_img = Image.open(os.path.join(tile_folder, 'anim_overworld.png'))

    main_sheet = autopy.bitmap.Bitmap.open(os.path.join(tile_folder, 'overworld_1.cgb.png'))
    swap_sheet = autopy.bitmap.Bitmap.open(os.path.join(tile_folder, 'overworld_2.cgb.png'))
    anim_sheet = autopy.bitmap.Bitmap.open(os.path.join(tile_folder, 'anim_overworld.png'))

    with open(os.path.join(tile_folder, 'SwapMap.csv'), 'r') as f:
        swap_ids = [l.split(',') for l in f.read().split('\n')[:-1]]

    with open(os.path.join(tile_folder, 'AnimMap.csv'), 'r') as f:
        anim_ids = [l.split(',') for l in f.read().split('\n')[:-1]]

    x_size, y_size = map_im.size
    x_tiles = x_size // TILE_SIZE
    x_screens = x_tiles // SCREEN_SIZE
    y_tiles = y_size // TILE_SIZE
    y_screens = y_tiles // SCREEN_SIZE
    
    attr_map = open(os.path.join(tile_folder, 'attrmap.bin'), 'wb')
    tile_map = open(os.path.join(tile_folder, 'tilemap.bin'), 'wb')
    swap_map = open(os.path.join(tile_folder, 'swapmap.bin'), 'wb')
    anim_map = open(os.path.join(tile_folder, 'animmap.bin'), 'wb')
    for y_screen in range(y_screens):
        for x_screen in range(x_screens):
            tilesheet = main_img.copy()
            swap_id = int(swap_ids[y_screen][x_screen], 16)
            swap_map.write(bytes([swap_id]))
            if swap_id != 255: # FF, the don't change one
                swap_bounds = (0, swap_id * TILE_SIZE * 2, swap_img.size[0], swap_id * TILE_SIZE * 2 + TILE_SIZE * 2)
                swap_tiles = swap_img.crop(swap_bounds)
                tilesheet.paste(swap_tiles, (0, TILE_SIZE))
            anim_id = int(anim_ids[y_screen][x_screen], 16)
            anim_map.write(bytes([anim_id]))
            if anim_id != 255: # FF, for no animation
                anim_bounds = (0, anim_id * TILE_SIZE, anim_img.size[0], anim_id * TILE_SIZE + TILE_SIZE)
                anim_tiles = anim_img.crop(anim_bounds)
                tilesheet.paste(anim_tiles, (12 * TILE_SIZE, 7 * TILE_SIZE))

            tilesheet_bitmap = convert_pil_to_bitmap(tilesheet)
            
            for y_tile in range(SCREEN_SIZE):
                for x_tile in range(SCREEN_SIZE):
                    tile_root = (x_tile * TILE_SIZE + x_screen * SCREEN_SIZE * TILE_SIZE,
                                y_tile * TILE_SIZE + y_screen * SCREEN_SIZE * TILE_SIZE)
                    tile_bounds = (tile_root[0], tile_root[1],
                                tile_root[0] + TILE_SIZE, tile_root[1] + TILE_SIZE)
                    tile = map_im.crop(tile_bounds)

                    tile_found = find_tile_in_sheet(tile, tilesheet_bitmap)
                    if tile_found:
                        # sheet_pos = hex(int(x_tile + y_tile * 16))
                        # print(f'screen {(x_screen, y_screen)}, tile {(x_tile, y_tile)}, main sheet, attr {bytes([tile_found[0]]).hex()}, loc {bytes([tile_found[1]]).hex()}')
                        attr_map.write(bytes([tile_found[0]]))
                        tile_map.write(bytes([tile_found[1]]))
                        continue
                    print(f'screen {(x_screen, y_screen)}, tile {(x_tile, y_tile)}, not found')

                    # tile_found = find_tile_in_sheet(tile, swap_sheet)
                    # if tile_found:
                    #     swap_sheet_pos = tile_found[1] % (16 * 2)
                    #     swap_sheet_id = tile_found[1] // (16 * 2)
                    #     print(f'screen {(x_screen, y_screen)}, tile {(x_tile, y_tile)}, swap sheet {swap_sheet_id}, attr {tile_found[0]}, loc {swap_sheet_pos}')
                    #     continue

                    # tile_found = find_tile_in_sheet(tile, anim_sheet)
                    # if tile_found:
                    #     # print(f'screen {(x_screen, y_screen)}, tile {(x_tile, y_tile)}, anim sheet, attr {tile_found[0]}, loc {tile_found[1]}')
                    #     continue

                    # print(f'screen {(x_screen, y_screen)}, tile {(x_tile, y_tile)}, not found')
    attr_map.close()
    tile_map.close()
    swap_map.close()
    anim_map.close()

def convert_img_to_2bpp(img_filename, out_filename):
    input_img = Image.open(img_filename)
    colors = []
    with open(out_filename, 'wb') as out_file:
        for y in range(input_img.size[1]):
            for x in range(input_img.size[0]):
                colors.append(input_img.getpixel((x, y)))
                if len(colors) == 4:
                    b = colors[0] << 6 | colors[1] << 4 | colors[2] << 2 | colors[3]
                    out_file.write(bytes([b]))
                    colors = []

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
            



X_SCREENS = 2
Y_SCREENS = 3
TILE_BUFFER_SIZE = 16
SWAP_OFFSET = 16 * TILE_BUFFER_SIZE
SWAP_TILES = 32
SWAP_SIZE = SWAP_TILES * TILE_BUFFER_SIZE
ANIM_OFFSET = 16 * 7 * TILE_BUFFER_SIZE + 12 * TILE_BUFFER_SIZE
ANIM_TILES = 4
ANIM_SIZE = ANIM_TILES * TILE_BUFFER_SIZE

def generate_map_from_data(tile_folder):
    attr_map = open(os.path.join(tile_folder, 'attrmap.bin'), 'rb')
    tile_map = open(os.path.join(tile_folder, 'tilemap.bin'), 'rb')
    swap_map = open(os.path.join(tile_folder, 'swapmap.bin'), 'rb')
    anim_map = open(os.path.join(tile_folder, 'animmap.bin'), 'rb')
    with open(os.path.join(tile_folder, 'OverworldBaseTilesheet.bin'), 'rb') as f:
        base_tilesheet = f.read()
    with open(os.path.join(tile_folder, 'OverworldSwapTilesheet.bin'), 'rb') as f:
        swap_tilesheet = f.read()
    with open(os.path.join(tile_folder, 'OverworldAnimTilesheet.bin'), 'rb') as f:
        anim_tilesheet = f.read()

    world_map = Image.new('RGB', (X_SCREENS * SCREEN_SIZE * TILE_SIZE, Y_SCREENS * SCREEN_SIZE * TILE_SIZE))
    world_map_pixels = world_map.load()

    screen_tilesheet = bytearray(base_tilesheet)
    for y_screen in range(Y_SCREENS):
        for x_screen in range(X_SCREENS):
            swap_id = int.from_bytes(swap_map.read(1), 'big')
            anim_id = int.from_bytes(anim_map.read(1), 'big')
            
            if swap_id != 255: # FF, the don't change one
                swap_start = swap_id * SWAP_SIZE
                screen_tilesheet[SWAP_OFFSET:SWAP_OFFSET+SWAP_SIZE] = swap_tilesheet[swap_start:swap_start+SWAP_SIZE]

            if anim_id != 255: # FF, the don't change one
                anim_start = anim_id * ANIM_SIZE
                screen_tilesheet[ANIM_OFFSET:ANIM_OFFSET+ANIM_SIZE] = anim_tilesheet[anim_start:anim_start+ANIM_SIZE]

            for y_tile in range(SCREEN_SIZE):
                for x_tile in range(SCREEN_SIZE):
                    tile_id = int.from_bytes(tile_map.read(1), 'big')
                    attr = int.from_bytes(attr_map.read(1), 'big')
                    palette = attr >> 4
                    # TODO: Palettes
                    flip = attr & 0x0f
                    tile_offset = tile_id * TILE_BUFFER_SIZE
                    tile_buffer = screen_tilesheet[tile_offset:tile_offset+TILE_BUFFER_SIZE]
                    for y in range(TILE_SIZE):
                        for x in range(TILE_SIZE):
                            pixel_offset = (x + y * TILE_SIZE) // 4
                            pixel_shift = (3 - (x + y * TILE_SIZE) % 4) * 2
                            pixel = (tile_buffer[pixel_offset] >> pixel_shift) & 0x03
                            new_color = tuple([85 * (3 - pixel)] * 3)
                            pixel_pos = [x, y]
                            if flip & H_FLIP_FLAG:
                                pixel_pos[0] = (TILE_SIZE-1) - pixel_pos[0]
                            if flip & V_FLIP_FLAG:
                                pixel_pos[1] = (TILE_SIZE-1) - pixel_pos[1]
                            map_pos = (x_screen * SCREEN_SIZE * TILE_SIZE + x_tile * TILE_SIZE + pixel_pos[0],
                                       y_screen * SCREEN_SIZE * TILE_SIZE + y_tile * TILE_SIZE + pixel_pos[1])
                            world_map_pixels[map_pos[0], map_pos[1]] = new_color

    attr_map.close()
    tile_map.close()
    swap_map.close()
    anim_map.close()

    world_map.show()


def convert_mario_map_to_commands(map_file, output_file):
    line_num = 0
    with open(output_file, 'w') as f_target:
        with open(map_file, 'r') as f:
            line = f.readline()
            while line:
                result = f'const uint8_t row_{line_num:04}[] = {{'
                row = line.split(',')
                for i in range(len(row)):
                    if row[i].strip():
                        result += f'{i}, {row[i].strip()}, '
                result += '0xFF};\n'
                f_target.write(result)
                line = f.readline()
                line_num += 1
        f_target.write('const uint8_t *world_map[] = {\n')
        for i in range(line_num):
            f_target.write(f'\t row_{i:04},\n')
        f_target.write('};')
            

def convert_pokemon_to_binary(pokemon_directory, out_filename):
    open(out_filename, 'wb').close()
    files = glob.iglob(os.path.join(pokemon_directory, '*'))
    for pokemon_filename in files:
        input_img = Image.open(pokemon_filename)
        dims = input_img.size
        x_tiles = dims[0] // TILE_SIZE
        y_tiles = dims[1] // TILE_SIZE
        with open(out_filename, 'ab') as out_file:
            for y_tile in range(y_tiles):
                for x_tile in range(x_tiles):
                    tile_root = (x_tile * TILE_SIZE, y_tile * TILE_SIZE)
                    tile_bounds = (tile_root[0], tile_root[1],
                                tile_root[0] + TILE_SIZE, tile_root[1] + TILE_SIZE)
                    tile = input_img.crop(tile_bounds)

                    out_file.write(convert_tile_to_bytes(tile))

if __name__ == "__main__":
    # extract_tiles('brainstorming/SourceImages/OverworldTiles.png', output='brainstorming/TestFolder/', sheet='brainstorming/Tiles/Master/overworld_master.png')
    # generate_tilemap_from_map('brainstorming/NewTiles/WorldMap.png', 'brainstorming/NewTiles/Tilesheets/')
    # generate_map_from_data('brainstorming/NewTiles/Tilesheets/')
    # convert_tilesheet_to_2bpp('brainstorming/NewTiles/Tilesheets/overworld_1.cgb.png', 'brainstorming/NewTiles/Tilesheets/OverworldBaseTilesheet.bin')
    # convert_tilesheet_to_2bpp('brainstorming/NewTiles/Tilesheets/overworld_2.cgb.png', 'brainstorming/NewTiles/Tilesheets/OverworldSwapTilesheet.bin')
    # convert_tilesheet_to_2bpp('brainstorming/NewTiles/Tilesheets/anim_overworld.png', 'brainstorming/NewTiles/Tilesheets/OverworldAnimTilesheet.bin')
    # generate_vram_test_file('brainstorming/HexTiles/Tall/')
    # convert_tilesheet_to_2bpp('brainstorming/HexTiles/Tall/vram_test_file_2bpp.png', 'brainstorming/HexTiles/Tall/vram_test_file.bin')
    # convert_tilesheet_to_2bpp('brainstorming/ExampleTilesheets/BasicColors.png', 'brainstorming/ExampleTilesheets/BasicColors.bin')
    # convert_tilesheet_to_2bpp('brainstorming/ExampleTilesheets/BasicSpritesheet.png', 'brainstorming/ExampleTilesheets/BasicSpritesheet.bin')
    # convert_tilesheet_to_2bpp('brainstorming/ExampleTilesheets/MarioTilesheet.png', 'brainstorming/ExampleTilesheets/MarioTilesheet.bin')
    # convert_mario_map_to_commands('brainstorming/ExampleTilesheets/MarioWorld1-1Map2.csv', 'brainstorming/ExampleTilesheets/MarioWorld1-1Map.txt')
    # convert_tilesheet_to_2bpp('brainstorming/ExampleTilesheets/PokemonTilesheet.png', 'brainstorming/ExampleTilesheets/PokemonTilesheet.bin')
    # convert_pokemon_to_binary('SourceImages/Pokemon/EnemyPokemon/ConvertToTilesheet', 'SourceImages/Pokemon/EnemyPokemon/Converted/EnemyPokemonTilesheet.bin')
    # convert_pokemon_to_binary('SourceImages/Pokemon/PlayerPokemon/ConvertToTilesheet', 'SourceImages/Pokemon/PlayerPokemon/Converted/PlayerPokemonTilesheet.bin')
    convert_tilesheet_to_2bpp("resources/SourceImages/Mario/MarioTilesheet~bw.png", "resources/SourceImages/Mario/MarioTilesheet~bw.bin")
    convert_tilesheet_to_2bpp("resources/SourceImages/Mario/MarioSpritesheet~bw.png", "resources/SourceImages/Mario/MarioSpritesheet~bw.bin")
    