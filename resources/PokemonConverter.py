import os
import glob
from PIL import Image

CHUNK_SIZE = 32
BLOCK_SIZE = 16
TILE_SIZE = 8
SPRITE_SIZE = 16

def extract_chunks_from_map(source_file, dest_folder):
    if os.path.exists(dest_folder):
        files = glob.iglob(os.path.join(dest_folder,'*'))
        for f in files:
            os.remove(f)
        pass
    else:
        os.makedirs(dest_folder)
    map_img = Image.open(source_file)
    dims = map_img.size
    
    x_chunks = dims[0] // CHUNK_SIZE
    y_chunks = dims[1] // CHUNK_SIZE
    i = 0
    for y_chunk in range(y_chunks):
        for x_chunk in range(x_chunks):
            chunk_root = (x_chunk * CHUNK_SIZE, y_chunk * CHUNK_SIZE)
            chunk_bounds = (chunk_root[0], chunk_root[1],
                        chunk_root[0] + CHUNK_SIZE, chunk_root[1] + CHUNK_SIZE)
            chunk = map_img.crop(chunk_bounds)
            chunk.save(os.path.join(dest_folder, f'({x_chunk},{y_chunk}).png'))

def extract_tiles_from_blocks(source_folder, dest_folder):
    if os.path.exists(dest_folder):
        files = glob.iglob(os.path.join(dest_folder,'*'))
        for f in files:
            os.remove(f)
        pass
    else:
        os.makedirs(dest_folder)
    
    source_images = glob.iglob(os.path.join(source_folder, '*'))
    i = 0
    for s in source_images:
        source_image = Image.open(s)
        dims = source_image.size
        
        x_tiles = dims[0] // TILE_SIZE
        y_tiles = dims[1] // TILE_SIZE
        for y_tile in range(y_tiles):
            for x_tile in range(x_tiles):
                tile_root = (x_tile * TILE_SIZE, y_tile * TILE_SIZE)
                tile_bounds = (tile_root[0], tile_root[1],
                            tile_root[0] + TILE_SIZE, tile_root[1] + TILE_SIZE)
                tile = source_image.crop(tile_bounds)

                tile.save(os.path.join(dest_folder, f'{format(i, "03")},({x_tile},{y_tile}).png')) 
        i += 1
        
def extract_blocks_from_chunks(source_folder, dest_folder):
    if os.path.exists(dest_folder):
        files = glob.iglob(os.path.join(dest_folder,'*'))
        for f in files:
            os.remove(f)
        pass
    else:
        os.makedirs(dest_folder)
    
    source_images = glob.iglob(os.path.join(source_folder, '*'))
    i = 0
    for s in source_images:
        source_image = Image.open(s)
        dims = source_image.size
        
        x_blocks = dims[0] // BLOCK_SIZE
        y_blocks = dims[1] // BLOCK_SIZE
        for y_block in range(y_blocks):
            for x_block in range(x_blocks):
                block_root = (x_block * BLOCK_SIZE, y_block * BLOCK_SIZE)
                block_bounds = (block_root[0], block_root[1],
                            block_root[0] + BLOCK_SIZE, block_root[1] + BLOCK_SIZE)
                block = source_image.crop(block_bounds)

                block.save(os.path.join(dest_folder, f'{format(i, "03")},({x_block},{y_block}).png')) 
        i += 1

def get_unique_images(source_folder, dest_folder):
    if os.path.exists(dest_folder):
        files = glob.iglob(os.path.join(dest_folder,'*'))
        for f in files:
            os.remove(f)
        pass
    else:
        os.makedirs(dest_folder)

    i = 0
    source_images = glob.iglob(os.path.join(source_folder, '*'))
    for s in source_images:
        source_image = Image.open(s)
        is_unique = True
        for t in glob.iglob(os.path.join(dest_folder, '*')):
            test_image = Image.open(t)
            if list(source_image.getdata()) == list(test_image.getdata()):
                is_unique = False
                break
        if is_unique:
            source_image.save(os.path.join(dest_folder, f'{format(i, "03")}.png'))
            i += 1

def stitch_images(source_folder, dest_file):
    source_files = glob.iglob(os.path.join(source_folder, '*'))

    source_images = [Image.open(s) for s in source_files]
    widths, heights = zip(*(i.size for i in source_images))

    total_width = sum(widths)
    max_height = max(heights)

    stitched_image = Image.new("RGB", (total_width, max_height))
    
    x_offset = 0
    for s in source_images:
        stitched_image.paste(s, (x_offset,0))
        x_offset += s.size[0]

    stitched_image.save(dest_file)

def convert_map_to_chunks(map_file, chunk_folder, output_file):
    map_img = Image.open(map_file)
    dims = map_img.size
    
    x_chunks = dims[0] // CHUNK_SIZE
    y_chunks = dims[1] // CHUNK_SIZE
    with open(output_file, 'wb') as f_out:
        for y_chunk in range(y_chunks):
            for x_chunk in range(x_chunks):
                chunk_root = (x_chunk * CHUNK_SIZE, y_chunk * CHUNK_SIZE)
                chunk_bounds = (chunk_root[0], chunk_root[1],
                            chunk_root[0] + CHUNK_SIZE, chunk_root[1] + CHUNK_SIZE)
                map_chunk = map_img.crop(chunk_bounds)

                found_chunk = False
                for c in glob.iglob(os.path.join(chunk_folder, '*')):
                    chunk = Image.open(c)
                    if list(map_chunk.getdata()) == list(chunk.getdata()):
                        found_chunk = True
                        chunk_info = os.path.basename(c).split('.')[0].split('_')

                        f_out.write(bytes([int(chunk_info[0])]))
                        break
                if not found_chunk:
                    print(f"Couldn't find chunk at ({x_chunk}, {y_chunk})")

def convert_chunks_to_blocks(chunk_folder, block_folder, output_file):
    chunk_images = glob.iglob(os.path.join(chunk_folder, '*'))
    
    with open(output_file, 'w') as f_out:
        f_out.write('{\n')
        for c in chunk_images:
            chunk = Image.open(c)
            dims = chunk.size
            x_blocks = dims[0] // BLOCK_SIZE
            y_blocks = dims[1] // BLOCK_SIZE
        
            f_out.write('\t{')
            for y_block in range(y_blocks):
                for x_block in range(x_blocks):
                    block_root = (x_block * BLOCK_SIZE, y_block * BLOCK_SIZE)
                    block_bounds = (block_root[0], block_root[1],
                                block_root[0] + BLOCK_SIZE, block_root[1] + BLOCK_SIZE)
                    chunk_block = chunk.crop(block_bounds)

                    found_block = False
                    for b in glob.iglob(os.path.join(block_folder, '*')):
                        block = Image.open(b)
                        if list(chunk_block.getdata()) == list(block.getdata()):
                            found_block = True
                            block_info = os.path.basename(b).split('.')[0].split('_')

                            f_out.write(f'{int(block_info[0])},')
                            break
                    if not found_block:
                        print(f"Couldn't find block at ({x_block}, {y_block})")
            # f_out.write(f'}}, // {" ".join(os.path.basename(c).split(".")[0].split("_")[1:])}\n')
            f_out.write(f'}},\n')
        f_out.write('};')

def convert_blocks_to_tiles(block_folder, tile_folder, output_file):
    block_images = glob.iglob(os.path.join(block_folder, '*'))
    
    with open(output_file, 'w') as f_out:
        f_out.write('{\n')
        for b in block_images:
            block = Image.open(b)
            dims = block.size
            x_tiles = dims[0] // TILE_SIZE
            y_tiles = dims[1] // TILE_SIZE
        
            f_out.write('\t{')
            for y_tile in range(y_tiles):
                for x_tile in range(x_tiles):
                    tile_root = (x_tile * TILE_SIZE, y_tile * TILE_SIZE)
                    tile_bounds = (tile_root[0], tile_root[1],
                                tile_root[0] + TILE_SIZE, tile_root[1] + TILE_SIZE)
                    block_tile = block.crop(tile_bounds)

                    found_tile = False
                    for t in glob.iglob(os.path.join(tile_folder, '*')):
                        tile = Image.open(t)
                        if list(block_tile.getdata()) == list(tile.getdata()):
                            found_tile = True
                            tile_info = os.path.basename(t).split('.')[0].split('_')

                            f_out.write(f'{int(tile_info[0])},')
                            break
                    if not found_tile:
                        print(f"Couldn't find tile at ({x_tile}, {y_tile})")
            f_out.write(f'}},\n')
        f_out.write('};')

def convert_blocks_to_palettes(block_folder, palette_file, output_file):
    block_images = glob.iglob(os.path.join(block_folder, '*'))

    palette_image = Image.open(palette_file)

    colors_per_palette = palette_image.size[0]
    num_palettes = palette_image.size[1]

    palettes = []
    for r in range(num_palettes):
        palette = palette_image.crop((0, r, colors_per_palette, r+1))
        p = palette.quantize(colors=4)
        palettes.append(set([tuple(p.getpalette()[i:i+3]) for i in range(0, 12, 3)]))
    
    with open(output_file, 'w') as f_out:
        f_out.write('{\n')
        for b in block_images:
            block = Image.open(b)
            dims = block.size
            x_tiles = dims[0] // TILE_SIZE
            y_tiles = dims[1] // TILE_SIZE
        
            f_out.write('\t{')
            for y_tile in range(y_tiles):
                for x_tile in range(x_tiles):
                    tile_root = (x_tile * TILE_SIZE, y_tile * TILE_SIZE)
                    tile_bounds = (tile_root[0], tile_root[1],
                                tile_root[0] + TILE_SIZE, tile_root[1] + TILE_SIZE)
                    block_tile = block.crop(tile_bounds)

                    quantized_tile = block_tile.quantize(colors=4)
                    tile_palette = set([tuple(quantized_tile.getpalette()[i:i+3]) for i in range(0, 12, 3)])

                    found_palette = False
                    for p in range(len(palettes)):
                        if tile_palette <= palettes[p]:
                            f_out.write(f'{p},')
                            found_palette = True
                            break
                    if not found_palette:
                        print(f"Couldn't find palette {tile_palette} at ({x_tile}, {y_tile}) (b2p)")
            f_out.write(f'}},\n')
        f_out.write('};')

def convert_tiles_to_palettes(tile_folder, palette_file, output_file):
    tile_images = glob.iglob(os.path.join(tile_folder, '*'))

    palette_image = Image.open(palette_file)

    colors_per_palette = palette_image.size[0]
    num_palettes = palette_image.size[1]

    palettes = []
    for r in range(num_palettes):
        palette = palette_image.crop((0, r, colors_per_palette, r+1))
        p = palette.quantize(colors=4)
        palettes.append(set([tuple(p.getpalette()[i:i+3]) for i in range(0, 12, 3)]))
    
    with open(output_file, 'w') as f_out:
        f_out.write('{\n')
        for t in tile_images:
            tile = Image.open(t)
            quantized_tile = tile.quantize(colors=4)
            tile_palette = set([tuple(quantized_tile.getpalette()[i:i+3]) for i in range(0, 12, 3)])

            found_palette = False
            for p in range(len(palettes)):
                if tile_palette <= palettes[p]:
                    f_out.write(f'\t{p},\n')
                    found_palette = True
                    # print(f"Found palette {p} {palettes[p]} -> {tile_palette} on {os.path.basename(t)} (t2p)")
                    break
            if not found_palette:
                f_out.write('\t0,\n')
                print(f"Couldn't find palette {tile_palette} on {os.path.basename(t)} (t2p)")
        f_out.write('};')

def convert_paletted_tilesheet_to_2bpp(tilesheet_file, palette_file, output_file):
    palette_image = Image.open(palette_file)

    colors_per_palette = palette_image.size[0]
    num_palettes = palette_image.size[1]

    palettes = []
    for r in range(num_palettes):
        palette = palette_image.crop((0, r, colors_per_palette, r+1))
        p = palette.quantize(colors=4)
        palettes.append([tuple(p.getpalette()[i:i+3]) for i in range(0, 12, 3)])

    tilesheet_image = Image.open(tilesheet_file)
    dims = tilesheet_image.size
    x_tiles = dims[0] // TILE_SIZE
    y_tiles = dims[1] // TILE_SIZE
    
    with open(output_file, 'wb') as f_out:
        for y_tile in range(y_tiles):
            for x_tile in range(x_tiles):
                tile_root = (x_tile * TILE_SIZE, y_tile * TILE_SIZE)
                tile_bounds = (tile_root[0], tile_root[1],
                            tile_root[0] + TILE_SIZE, tile_root[1] + TILE_SIZE)
                tile = tilesheet_image.crop(tile_bounds)

                quantized_tile = tile.quantize(colors=4)
                tile_palette = [tuple(quantized_tile.getpalette()[i:i+3]) for i in range(0, 12, 3)]

                found_palette = False
                for p in range(len(palettes)):
                    if set(tile_palette) <= set(palettes[p]):
                        tile_bytestring = ""
                        for y in range(tile.size[1]):
                            for x in range(tile.size[0]):
                                pixel = tile.getpixel((x,y))
                                tile_bytestring += format(palettes[p].index(pixel), '02b')
                        byte_str = int(tile_bytestring, 2).to_bytes((len(tile_bytestring) + 7) // 8, byteorder='big')
                        f_out.write(byte_str)
                        found_palette = True
                        break
                if not found_palette:
                    print(f"Couldn't find palette {tile_palette} at ({x_tile}, {y_tile}) (pt22bpp)")

def convert_palettes_to_commands(palette_file, output_file):
    palette_image = Image.open(palette_file)

    colors_per_palette = palette_image.size[0]
    num_palettes = palette_image.size[1]

    palettes = []
    for r in range(num_palettes):
        palette = palette_image.crop((0, r, colors_per_palette, r+1))
        p = palette.quantize(colors=4)
        palettes.append([tuple(p.getpalette()[i:i+3]) for i in range(0, 12, 3)])

    with open(output_file, 'w') as f_out:
        for palette in palettes:
            for color in palette:
                f_out.write('0b11')
                for c in color:
                    f_out.write(format(c // 85, '02b'))
                f_out.write(', ')
            f_out.write('\n')

def is_image_in_folder(image, folder):
    for t in glob.iglob(os.path.join(folder, '*')):
        test_image = Image.open(t)
        if list(image.getdata()) == list(test_image.getdata()):
            return True
    return False

def convert_blocks_to_collision(source_folder, collision_folder, output_file):
    with open(output_file, 'w') as f_out:
        f_out.write('{\n')
        # print(f'searching in {collision_folder}')
        source_images = glob.iglob(os.path.join(source_folder, '*'))
        for s in source_images:
            source_image = Image.open(s)

            collision_folders = glob.iglob(os.path.join(collision_folder, '*'))
            # print(f'Block {os.path.basename(s)}')
            for f in collision_folders:
                if is_image_in_folder(source_image, f):
                    f_out.write(f'\t{os.path.basename(f)},\n')
                    # print(f'\tfound {os.path.basename(f)}')
                    break
                # print(f'\tnot found {os.path.basename(f)}')
        f_out.write('};')

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

BASE_FOLDER = "resources/SourceImages/Pokemon/Map/"

def convert_map(base_folder, name, to_do):
    if to_do == "extract":
        # Run this on the manually created map
        extract_chunks_from_map(base_folder + f"../{name}.png", base_folder + "Chunks/")
        get_unique_images(base_folder + "Chunks/", base_folder + "Chunk_Templates/")
        extract_blocks_from_chunks(base_folder + "Chunk_Templates/", base_folder + "Blocks/")
        get_unique_images(base_folder + "Blocks/", base_folder + "Block_Templates/")
        extract_tiles_from_blocks(base_folder + "Block_Templates/", base_folder + "Tiles/")
        get_unique_images(base_folder + "Tiles/", base_folder + "Tile_Templates/")
        # stitch_images(base_folder + "Tile_Templates/", base_folder + f"{name}Tilesheet.png")
    elif to_do == "convert":
        # Manually create "../BG_Palettes.png"
        # Manually create the "Block_Collision" folder with subfolders of numbers
        if not os.path.exists(base_folder + "Output/"):
            os.makedirs(base_folder + "Output/")
        
        # Then create and populate the "../Block_Collision" folder
        # Also make sure that "../BG_Palettes.png" has been generated at this point
        # convert_map_to_chunks(base_folder + f"../{name}.png", base_folder + "Chunk_Templates/", base_folder + f"Output/{name}.bin")
        # convert_chunks_to_blocks(base_folder + "Chunk_Templates/", base_folder + "Block_Templates/", base_folder + "Output/Chunks.txt")
        # convert_blocks_to_tiles(base_folder + "Block_Templates/", base_folder + "Tile_Templates/", base_folder + "Output/Blocks.txt")
        # convert_tiles_to_palettes(base_folder + "Tile_Templates/", base_folder + "../BG_Palettes.png", base_folder + "Output/Tile_Palettes.txt")
        # convert_paletted_tilesheet_to_2bpp(base_folder + f"{name}Tilesheet.png", base_folder + "../BG_Palettes.png", base_folder + "Output/PokemonTilesheet~color.bin")
        convert_palettes_to_commands(base_folder + "../BG_Palettes.png", base_folder + "Output/BG_Palette_Commands.txt")
        # convert_blocks_to_collision(base_folder + "Block_Templates/", base_folder + "../Block_Collision/", base_folder + "Output/Block_Collision.txt")
    elif to_do == "b&w":
        # For BW, I generated a 1 bit, Floyd-Steinberg dithered image of the route, and manually copied the corresponding tiles to a tilesheet
        # Actually, the most recent time I did a flat conversion to 1-bit, no dithering. Some minor manual editing. Looks cleaner
        # Gotta use that 2bpp palette (black, light grey, dark grey, white)
        convert_tilesheet_to_2bpp(base_folder + f"{name}Tilesheet~bw.png", base_folder + "Output/PokemonTilesheet~bw.bin")
    return


def convert_spritesheet_to_2bpp(sheet_filename, out_filename, img_out_filename=None):
    input_img = Image.open(sheet_filename)
    colors = []
    dims = input_img.size
    x_sprites = dims[0] // SPRITE_SIZE
    y_sprites = dims[1] // SPRITE_SIZE
    output_img = Image.new("RGB", (dims[0] * dims[1] // TILE_SIZE, TILE_SIZE))
    i = 0
    with open(out_filename, 'wb') as out_file:
        for y_sprite in range(y_sprites):
            for x_sprite in range(x_sprites):
                for y_tile_root in (y_sprite * SPRITE_SIZE, y_sprite * SPRITE_SIZE + TILE_SIZE):
                    for x_tile_root in (x_sprite * SPRITE_SIZE, x_sprite * SPRITE_SIZE + TILE_SIZE):
                        tile_root = (x_tile_root, y_tile_root)
                        tile_bounds = (tile_root[0], tile_root[1],
                                    tile_root[0] + TILE_SIZE, tile_root[1] + TILE_SIZE)
                        tile = input_img.crop(tile_bounds)
                        output_img.paste(tile, (i * TILE_SIZE, 0))
                        i += 1
                        out_file.write(convert_tile_to_bytes(tile))
    print(dims[0] * dims[1], (dims[0] * dims[1]) // 4, os.path.getsize(out_filename))
    if img_out_filename:
        output_img.save(img_out_filename)
    else:
        output_img.show()

def convert_sprites(base_folder):
    if not os.path.exists(base_folder + "Output/"):
        os.makedirs(base_folder + "Output/")
    convert_spritesheet_to_2bpp(base_folder + "Spritesheet.png", base_folder + "Output/Spritesheet.bin", base_folder + "Output/ConvertedSpritesheet.png")

MAX_LEN = 16

def convert_dialogue_to_bin(text_file, output_file, data_file):
    offset = 0
    open(output_file, 'wb').close()
    open(data_file, 'wb').close()
    with open(text_file, 'r') as tf:
        line = tf.readline()
        while line:
            words = line.replace("\\n", "\n").split(' ')
            temp = ''
            d_size = 0
            for w in words:
                if len(temp + w) >= MAX_LEN or w == '\n':
                    temp = temp.strip() + '\n'
                    output = bytes([ord(i) for i in temp])
                    with open(output_file, 'ab') as of:
                        d_size += of.write(output)
                    temp = ''
                if w != '\n':
                    temp += w + ' '
                    
            temp = temp.strip() + '\0'
            output = bytes([ord(i) for i in temp])
            with open(output_file, 'ab') as of:
                d_size += of.write(output)
            b_offset = offset.to_bytes(2, 'big')
            b_size = d_size.to_bytes(2, 'big')
            with open(data_file, 'ab') as df:
                df.write(b_offset)
                df.write(b_size)
            offset += d_size
            line = tf.readline()

def convert_binary_to_img(input_file, output_file):
    pass

def convert_tilesheet_helper(base_folder, name):
    convert_paletted_tilesheet_to_2bpp(base_folder + f"{name}Tilesheet.png", base_folder + "../BG_Palettes.png", base_folder + "Output/Tilesheet~color.bin")

def convert_2bpp_tilesheet_helper(base_folder, name):
    convert_tilesheet_to_2bpp(base_folder + f"{name}.png", base_folder + f"{name}.bin")

if __name__ == "__main__":
    # convert_map("resources/SourceImages/Pokemon/Map/Routes/Route1/Output/", "Route1", "extract") #might not work anymore b/c filenames lol
    # convert_map("resources/SourceImages/Pokemon/Map/Routes/Route1/Output/", "Route1", "convert")
    # convert_map("resources/SourceImages/Pokemon/Map/Routes/Route1/Output/", "Route1", "b&w")
    # convert_map("resources/SourceImages/Pokemon/Map/Routes/NationalPark/Output/", "NationalPark", "extract")
    # convert_map("resources/SourceImages/Pokemon/Map/Routes/NationalPark/Output/", "NationalPark", "convert")
    # convert_map("resources/SourceImages/Pokemon/Map/Routes/NationalPark/Output/", "NationalPark", "b&w")

    # convert_map("resources/SourceImages/Pokemon/Map/Routes/World/Output/", "World", "convert")
    # routes = ['Route1', 'Route2', 'Cave', 'Forest', 'NationalPark']
    # for route in routes:
    #     print(f'Converting {route}...')
    #     # convert_map(f'resources/SourceImages/Pokemon/Map/Routes/{route}/Output/', route, 'extract')
    #     convert_map(f'resources/SourceImages/Pokemon/Map/Routes/{route}/Output/', route, 'convert')
    #     print(f'Conversion complete!')
    
    # convert_map(f'resources/SourceImages/Pokemon/Map/Routes/{"Forest"}/Output/', 'Forest', 'convert')
    # convert_tilesheet_helper("resources/SourceImages/Pokemon/Map/Routes/World/Output/", "World")
    # convert_2bpp_tilesheet_helper("resources/SourceImages/Pokemon/Map/Routes/AnimationTiles/", "AnimationTilesheet")
    # convert_2bpp_tilesheet_helper("resources/SourceImages/Pokemon/Map/Routes/AnimationTiles/", "AnimationTilesheet~bw")
    # convert_2bpp_tilesheet_helper("resources/SourceImages/Pokemon/Map/Routes/World/", "WorldTilesheet~bw")


    # convert_sprites("resources/SourceImages/Pokemon/PokemonSprites/")

    # convert_tilesheet_to_2bpp("resources/SourceImages/Pokemon/UI/MenuTilesheet.png", "resources/SourceImages/Pokemon/UI/Output/PokemonMenuTilesheet~color.bin")
    # convert_tilesheet_to_2bpp("resources/SourceImages/Pokemon/UI/MenuTilesheet~bw.png", "resources/SourceImages/Pokemon/UI/Output/PokemonMenuTilesheet~bw.bin")
    
    convert_dialogue_to_bin("resources/MenuMockups/Dialogue/PokemonDialogue.txt", "resources/MenuMockups/Dialogue/Dialogue_text.bin",  "resources/MenuMockups/Dialogue/Dialogue_data.bin")
