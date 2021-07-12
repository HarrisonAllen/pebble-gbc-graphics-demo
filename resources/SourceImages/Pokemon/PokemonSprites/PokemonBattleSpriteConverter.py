import os
from PIL import Image
import numpy as np
import glob
import math

TILE_SIZE = 8
X_TILES = 7
Y_TILES = 7

def convert_tile_to_bytes(tile, palettes):
    result = []
    colors = []
    for y in range(tile.size[1]):
        for x in range(tile.size[0]):
            pixel = tile.getpixel((x, y))
            colors.append(palettes.index(pixel))
            if len(colors) == 4:
                b = colors[0] << 6 | colors[1] << 4 | colors[2] << 2 | colors[3]
                result.append(b)
                colors = []
    return bytes(result)

def convert_tile_to_compressed_bytes(tile, palettes):
    result = []
    colors = []
    cur_color = None
    num_pixels_of_color = 0
    for y in range(tile.size[1]):
        for x in range(tile.size[0]):
            pixel = tile.getpixel((x, y))
            color = palettes.index(pixel)
            if color == cur_color:
                num_pixels_of_color += 1
            else:
                result.append((color << 6) | num_pixels_of_color)
                cur_color = color
                num_pixels_of_color = 1
    result.append((color << 6) | num_pixels_of_color)
    return bytes(result)
'''
Delta encoding
Eh just copy what that dude does in python (ik it's going to be tough)

Compression method idea:
11 001010
|  |
|  +-> Bottom 6 bits are the number of times that pixel appears sequentially
+-> Top two bits are the palette

E.g.
11 11 11 11
00 11 00 10
01 01 01 01
11 00 10 11
becomes
[11|4, 00|1, 11|1, 00|1, 10|1, 01|4, 11|1, 00|1, 10|1, 11|1]
'''

def convert_spritesheet_to_pokemon(source_file, sprite_file, palette_file):
    open(sprite_file, 'wb').close()
    open(palette_file, 'wb').close()
    source_image = Image.open(source_file)
    width, height = source_image.size
    num_pokemon_x = width // (TILE_SIZE * X_TILES)
    num_pokemon_y = height // (TILE_SIZE * Y_TILES)
    total_size = 0

    for y_pokemon in range(num_pokemon_y):
        for x_pokemon in range(num_pokemon_x):
            
            sprite_root = (x_pokemon * TILE_SIZE * X_TILES, y_pokemon * TILE_SIZE * Y_TILES)
            sprite_bounds = (sprite_root[0], sprite_root[1],
                             sprite_root[0] + TILE_SIZE * X_TILES, sprite_root[1] + TILE_SIZE * Y_TILES)
            pokemon_img = source_image.crop(sprite_bounds)

            quantized_img = pokemon_img.quantize(colors=4)
            palettes = [tuple(quantized_img.getpalette()[i:i+3]) for i in range(0, 12, 3)]
            palettes = sorted(palettes, reverse=True)

            with open(palette_file, 'ab') as pf:
                colors = []
                for p in palettes:
                    color = 3 << 6
                    color |= (p[0] // 85) << 4
                    color |= (p[1] // 85) << 2
                    color |= (p[2] // 85)
                    colors.append(color)
                pf.write(bytes(colors))

            pokemon_size = 0
            for x_tile in range(X_TILES):
                for y_tile in range(Y_TILES):
                    tile_root = (x_tile * TILE_SIZE, y_tile * TILE_SIZE)
                    tile_bounds = (tile_root[0], tile_root[1],
                                tile_root[0] + TILE_SIZE, tile_root[1] + TILE_SIZE)
                    tile = pokemon_img.crop(tile_bounds)
                    
                    tile_bytes = convert_tile_to_compressed_bytes(tile, palettes)
                    with open(sprite_file, 'ab') as of:
                        pokemon_size += of.write(tile_bytes)
            print(f'From {784} bytes to {pokemon_size}')
            total_size += pokemon_size
    print(f'From {203840} bytes to {total_size} bytes')
    # source_image.show()

def renumber_pokemon_files(input_folder):
    images_in_folder = glob.glob(os.path.join(input_folder, "*.png"))
    for image in images_in_folder:
        filename = os.path.basename(image)
        number = int(filename.split('.')[0])
        new_filename = f'{number:03d}.png'
        os.rename(image, image.replace(filename, new_filename))

def convert_folder_to_ribbon(input_folder, output_file, size_file):
    with open(size_file, "w") as sf:
        sf.write("5,5\n")
        ribbon = Image.new("RGB", (252*7*8, 7*8), color=(255, 255, 255))
        images_in_folder = glob.glob(os.path.join(input_folder, "*.png"))
        for image in images_in_folder:
            filename = os.path.basename(image)
            number = int(filename.split('.')[0])
            pokemon = Image.open(image)
            dims = pokemon.size
            sf.write(f'{dims[0]//8},{dims[1]//8}\n')
            x_offset = number * 7*8
            h_pad = (7-(dims[1] // 8)) * 8
            w_pad = ((8 - (dims[0] // 8)) // 2) * 8
            ribbon.paste(pokemon, (x_offset + w_pad, h_pad))
        ribbon.save(output_file)

def convert_ribbon_to_images(ribbon_file, size_file, output_folder):
    with open(size_file, "r") as sf:
        i = 0
        ribbon = Image.open(ribbon_file)
        pokemon_size = sf.readline()
        while pokemon_size:
            str_dims = pokemon_size.split(',')
            width = int(str_dims[0]) * 8
            height = int(str_dims[1]) * 8

            x_offset = i * 7*8
            h_pad = (7-(height // 8)) * 8
            w_pad = ((8 - (width // 8)) // 2) * 8

            origin = (x_offset + w_pad, h_pad)
            bounds = (origin[0], origin[1], origin[0] + width, origin[1] + height)

            sprite = ribbon.crop(bounds)
            
            filename = os.path.join(output_folder, f'{i:03d}.png')
            sprite.save(filename)

            i += 1
            pokemon_size = sf.readline()

TILE_SIZE = 8

def convert_image_to_2bpp(image, output_file):
    quantized_image = image.quantize(colors=4)
    image_palette = sorted([tuple(quantized_image.getpalette()[i:i+3]) for i in range(0, 12, 3)], reverse=True)
    print(image_palette)

    dims = image.size
    x_tiles = dims[0] // TILE_SIZE
    y_tiles = dims[1] // TILE_SIZE
    
    with open(output_file, 'wb') as f_out:
        for y_tile in range(y_tiles):
            for x_tile in range(x_tiles):
                tile_root = (x_tile * TILE_SIZE, y_tile * TILE_SIZE)
                tile_bounds = (tile_root[0], tile_root[1],
                            tile_root[0] + TILE_SIZE, tile_root[1] + TILE_SIZE)
                tile = image.crop(tile_bounds)

                tile_bytestring = ""
                for y in range(tile.size[1]):
                    for x in range(tile.size[0]):
                        pixel = tile.getpixel((x,y))
                        tile_bytestring += format(image_palette.index(pixel), '02b')
                byte_str = int(tile_bytestring, 2).to_bytes((len(tile_bytestring) + 7) // 8, byteorder='big')
                f_out.write(byte_str)

def convert_image_to_gray(image, disp_output=None):
    quantized_image = image.quantize(colors=4)
    image_palette = sorted([tuple(quantized_image.getpalette()[i:i+3]) for i in range(0, 12, 3)], reverse=True)
    out_image = Image.new("L", image.size)
    disp_image = Image.new("L", image.size)

    for y in range(image.size[1]):
        for x in range(image.size[0]):
            pixel = image.getpixel((x,y))
            idx = image_palette.index(pixel)
            if idx not in (0, 1, 2, 3):
                print('heck', idx)
            out_image.putpixel((x,y), idx)
            disp_image.putpixel((x,y), (3-idx)*85)
    if disp_output is not None:
        disp_image.save(disp_output)
    return out_image

def convert_file_to_gray(image_file, disp_output=None):
    image = Image.open(image_file)
    convert_image_to_gray(image, disp_output)

def posterize(image):
    compress_prep = [[], []]
    # print("start")
    for y in range(image.size[1]):
        rowbp0 = []
        rowbp1 = []
        # row_str = ''
        for x in range(image.size[0]):
            px = image.getpixel((x, y))
            # row_str += str(px) + ' '
            rowbp0.append(1 & px)
            rowbp1.append(1 & (px >> 1))
        # print(row_str)
        compress_prep[0].append(rowbp0)
        compress_prep[1].append(rowbp1)
    # print("end")
    return compress_prep

def compress(swap, mode, compress_prep):
    c_in = compress_prep
    c_out = []
    bp0 = swap
    bp1 = (1 ^ swap)
    height = len(c_in[0])
    width = len(c_in[0][0])
    # print('===========',swap,'============', mode, '===========')
    # print('bp0')
    # for y in range(height):
    #     row_str = ''
    #     for x in range(width):
    #         row_str += str(c_in[bp0][y][x]) + ' '
    #     print(row_str)
    # print('bp1')
    # for y in range(height):
    #     row_str = ''
    #     for x in range(width):
    #         row_str += str(c_in[bp1][y][x]) + ' '
    #     print(row_str)
    

    # XOR decode
    # print("start")
    if mode != 0:
        for y in range(height):
            row_str = ''
            for x in range(width):
                p = c_in[bp0][y][x] ^ c_in[bp1][y][x]
                c_in[bp1][y][x] = p
                row_str += str(p) + ' '
            # print(row_str)
    # print('end')

    # delta decode
    for y in range(height):
        prev = 0
        for x in range(width):
            now = c_in[bp0][y][x]
            c_in[bp0][y][x] = (0 if now == prev else 1)
            prev = now
    if (mode != 1):
        for y in range(height):
            prev = 0
            for x in range(width):
                now = c_in[bp1][y][x]
                c_in[bp1][y][x] = (0 if now == prev else 1)
                prev = now
    
    for i in range(4):
        c_out.append(1 & (width >> (6-i)))
    for i in range(4):
        c_out.append(1 & (height >> (6-i)))
    c_out.append(swap)

    count = 0
    rle = False
    if c_in[bp0][0][0] == 0 and c_in[bp0][0][1] == 0:
        rle = True
    c_out.append(0 if rle else 1)

    # bitplane 0
    for x in range(0, width, 2):
        for y in range(height):
            a = c_in[bp0][y][x]
            b = c_in[bp0][y][x+1]

            if rle:
                if a == 0 and b == 0:
                    count += 1
                if a != 0 or b != 0 or (x == width - 2 and y == height - 1):
                    enc = count + 1
                    _pow = 1
                    dig = -1

                    while _pow <= enc:
                        _pow <<= 1
                        dig += 1
                    _pow >>= 1

                    val = enc - _pow
                    _pow -= 2
                    
                    for i in range(dig):
                        c_out.append(1 & (_pow >> (dig - i - 1)))
                    for i in range(dig):
                        c_out.append(1 & (val >> (dig - i - 1)))

                    if x != width - 2 or y != height - 1:
                        c_out.append(a)
                        c_out.append(b)
                        rle = False
            else:
                if a == 0 and b == 0:
                    c_out.append(0)
                    c_out.append(0)
                    count = 1
                    rle = True
                    if x == width - 2 and y == height - 1:
                        c_out.append(0)
                        c_out.append(0)
                else:
                    c_out.append(a)
                    c_out.append(b)
    
    if (mode == 0):
        c_out.append(0)
    else:
        c_out.append(1)
        c_out.append(0 if mode == 1 else 1)

    # init packet
    count = 0
    rle = False
    if c_in[bp1][0][0] == 0 and c_in[bp1][0][1] == 0:
        rle = True
    c_out.append(0 if rle else 1)

    # bitplane 1
    for x in range(0, width, 2):
        for y in range(height):
            a = c_in[bp1][y][x]
            b = c_in[bp1][y][x+1]

            if rle:
                if a == 0 and b == 0:
                    count += 1
                if a != 0 or b != 0 or (x == width - 2 and y == height - 1):
                    enc = count + 1
                    _pow = 1
                    dig = -1

                    while _pow <= enc:
                        _pow <<= 1
                        dig += 1
                    _pow >>= 1

                    val = enc - _pow
                    _pow -= 2
                    
                    for i in range(dig):
                        c_out.append(1 & (_pow >> (dig - i - 1)))
                    for i in range(dig):
                        c_out.append(1 & (val >> (dig - i - 1)))

                    if x != width - 2 or y != height - 1:
                        c_out.append(a)
                        c_out.append(b)
                        rle = False
            else:
                if a == 0 and b == 0:
                    c_out.append(0)
                    c_out.append(0)
                    count = 1
                    rle = True
                    if x == width - 2 and y == height - 1:
                        c_out.append(0)
                        c_out.append(0)
                else:
                    c_out.append(a)
                    c_out.append(b)

    output_bytes = []
    o_len = 1 + math.floor((len(c_out)-1)/8)
    for i in range(o_len):
        b = 0
        for j in range(8):
            idx = 8*i+j
            b = (b << 1) | (c_out[idx] if idx < len(c_out) else 0)
        output_bytes.append(b)
    return output_bytes

USE_DIFF_MODE = {
    '038': 0,
    '059': 2,
    '071': 2,
    '075': 2,
    '126': 5,
    '130': 2,
    '210': 0,
}

def compress_image(image_file, output, disp_output=None, use_hardcoded=False):
    source_image = Image.open(image_file)
    number = os.path.basename(image_file).split('.')[0]
    # if not number in USE_DIFF_MODE:
    #     return 0
    compressed_outputs = [
        compress(0, 0, posterize(convert_image_to_gray(source_image,disp_output))),
        compress(0, 1, posterize(convert_image_to_gray(source_image))),
        compress(0, 2, posterize(convert_image_to_gray(source_image))),
        compress(1, 0, posterize(convert_image_to_gray(source_image))),
        compress(1, 1, posterize(convert_image_to_gray(source_image))),
        compress(1, 2, posterize(convert_image_to_gray(source_image)))        
    ]
    if use_hardcoded and number in USE_DIFF_MODE:
        smallest_index = USE_DIFF_MODE[number]
        smallest_size = len(compressed_outputs[smallest_index])
    else:
        smallest_size = len(compressed_outputs[0])
        smallest_index = 0
        for i in range(len(compressed_outputs)):
            c_size = len(compressed_outputs[i])
            if c_size < smallest_size:
                smallest_size = c_size
                smallest_index = i
    with open(output, 'wb') as of:
        of.write(bytearray(compressed_outputs[smallest_index]))
    return smallest_size

def compress_images(image_folder, output_folder, disp_folder=None, use_hardcoded=False):
    images_in_folder = glob.iglob(os.path.join(image_folder, "*.png"))
    total_size = 0
    compressed_size = 0
    for image in images_in_folder:
        filename = os.path.basename(image)
        number = filename.split('.')[0]
        if disp_folder is not None:
            disp_file = os.path.join(disp_folder, filename)
        else:
            disp_file = None
        compressed_size += compress_image(image, os.path.join(output_folder, f'{number}.bin'), disp_file, use_hardcoded)
        total_size += 784
    print(f'Compressed from {total_size} bytes to {compressed_size} bytes')

def merge_compressed_images(input_folder, output_file, data_file):
    input_files = glob.iglob(os.path.join(input_folder, "*.bin"))
    total_bytes = 0
    open(data_file, 'wb').close()
    with open(output_file, 'wb') as out_file:
        for f in input_files:
            offset = total_bytes
            
            with open(f, 'rb') as in_file:
                file_size = out_file.write(in_file.read())
                
            b_offset = offset.to_bytes(4, 'big')
            b_size = file_size.to_bytes(2, 'big')
            with open(data_file, 'ab') as df:
                df.write(b_offset)
                df.write(b_size)

            total_bytes += file_size
    print(f'Final size: {total_bytes} bytes')

# def extract_palettes(input_folder, palette_file):
    
#             with open(palette_file, 'ab') as pf:
#                 colors = []
#                 for p in palettes:
#                     color = 3 << 6
#                     color |= (p[0] // 85) << 4
#                     color |= (p[1] // 85) << 2
#                     color |= (p[2] // 85)
#                     colors.append(color)
#                 pf.write(bytes(colors))

if __name__ == "__main__":
    base_folder = "resources/SourceImages/Pokemon/PokemonSprites/"
    # convert_spritesheet_to_pokemon(base_folder + "PalettedPokemon_Front_RGB.png", base_folder + "Output/PokemonFrontSprites.bin", base_folder + "Output/PokemonFrontPalettes.bin")
    # renumber_pokemon_files(base_folder + "Sprites/")
    # convert_folder_to_ribbon(base_folder + "Sprites/", base_folder + "Ribbons/FrontSpriteRibbon.png", base_folder + "Ribbons/FrontSpriteSizes.txt")
    # convert_folder_to_ribbon(base_folder + "Sprites/back/", base_folder + "Ribbons/BackSpriteRibbon.png", base_folder + "Ribbons/BackSpriteSizes.txt")

    # once converted to a ribbon, change the color palette, then revert to rgb, then convert to images

    # convert_ribbon_to_images(base_folder + "Ribbons/FrontSpriteRibbon~color.png", base_folder + "Ribbons/FrontSpriteSizes.txt", base_folder + "Ribbons/FrontSprites/color/")
    # convert_ribbon_to_images(base_folder + "Ribbons/FrontSpriteRibbon~bw.png", base_folder + "Ribbons/FrontSpriteSizes.txt", base_folder + "Ribbons/FrontSprites/bw/")
    # convert_ribbon_to_images(base_folder + "Ribbons/BackSpriteRibbon~color.png", base_folder + "Ribbons/BackSpriteSizes.txt", base_folder + "Ribbons/BackSprites/color/")
    # convert_ribbon_to_images(base_folder + "Ribbons/BackSpriteRibbon~bw.png", base_folder + "Ribbons/BackSpriteSizes.txt", base_folder + "Ribbons/BackSprites/bw/")
    
    # compress_image(base_folder + "Ribbons/BackSprites/color/038.png", base_folder + "Test.bin")
    # Then, compress the images
    # compress_images(base_folder + "Ribbons/FrontSprites/color/", base_folder + "Ribbons/FrontSprites/color/compressed/")
    # compress_images(base_folder + "Ribbons/FrontSprites/bw/", base_folder + "Ribbons/FrontSprites/bw/compressed/")
    # compress_images(base_folder + "Ribbons/BackSprites/color/", base_folder + "Ribbons/BackSprites/color/compressed/", base_folder + "Ribbons/BackSprites/color/gray/", True)
    # compress_images(base_folder + "Ribbons/BackSprites/bw/", base_folder + "Ribbons/BackSprites/bw/compressed/")
    # compress_images(base_folder + "Ribbons/FrontSprites/og/", base_folder + "Ribbons/FrontSprites/og/compressed/")
    # compress_images(base_folder + "Ribbons/BackSprites/og/", base_folder + "Ribbons/BackSprites/og/compressed/")

    # Finally, merge the images
    # merge_compressed_images(base_folder + "Ribbons/FrontSprites/color/compressed/", base_folder + "Output/PokemonFrontSprites~color.bin", base_folder + "Output/PokemonFrontSpriteData~color.bin")
    # merge_compressed_images(base_folder + "Ribbons/FrontSprites/bw/compressed/", base_folder + "Output/PokemonFrontSprites~bw.bin", base_folder + "Output/PokemonFrontSpriteData~bw.bin")
    # merge_compressed_images(base_folder + "Ribbons/BackSprites/color/compressed/", base_folder + "Output/PokemonBackSprites~color.bin", base_folder + "Output/PokemonBackSpriteData~color.bin")
    # merge_compressed_images(base_folder + "Ribbons/BackSprites/bw/compressed/", base_folder + "Output/PokemonBackSprites~bw.bin", base_folder + "Output/PokemonBackSpriteData~bw.bin")
    merge_compressed_images(base_folder + "Ribbons/FrontSprites/og/compressed/", base_folder + "Output/PokemonFrontSpritesOG.bin", base_folder + "Output/PokemonFrontSpriteDataOG.bin")
    merge_compressed_images(base_folder + "Ribbons/BackSprites/og/compressed/", base_folder + "Output/PokemonBackSpritesOG.bin", base_folder + "Output/PokemonBackSpriteDataOG.bin")

    # convert_file_to_gray(base_folder + "Ribbons/BackSprites/color/210.png")
    print("Done")