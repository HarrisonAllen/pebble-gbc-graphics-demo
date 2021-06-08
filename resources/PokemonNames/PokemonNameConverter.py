def convert_names_to_bin(input_file, output_file):
    file_size = 0
    open(output_file, "wb").close()
    with open(input_file, "r") as f:
        names = f.read().split('\n')
        for n in names:
            if len(n) < 10:
                n += '\0'
            filled = f'{n: <10}'
            output = bytes([ord(i) for i in filled])
            with open(output_file, "ab") as of:
                file_size += of.write(output)
    print('File takes up', file_size, 'bytes')

if __name__ == "__main__":
    convert_names_to_bin("resources/PokemonNames/PokemonNames.txt", "resources/PokemonNames/PokemonNames.bin")