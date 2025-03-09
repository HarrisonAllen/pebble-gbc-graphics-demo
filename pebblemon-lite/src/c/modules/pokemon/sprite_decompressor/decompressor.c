#include "decompressor.h"
#include "bit_reader.h"

#define s_debug false

uint16_t (*s_read)(uint8_t) = BitReader_read_bits;

static uint8_t read_pair() {
    return s_read(2);
}

static uint16_t decode_rle() {
    uint8_t n_bits = 1;
    while (s_read(1)) {
        n_bits++;
    }
    return (1 << n_bits) + s_read(n_bits) - 1;
}

static void decompress_to_buffer(uint8_t width, uint8_t height, uint8_t *buffer) {
    uint8_t col_height = height * 8;
    uint16_t size = width * col_height;

    bool read_raw_data = s_read(1);
    uint16_t num_zeroes = 0;
    uint8_t pair;

    uint16_t pos = 0;
    short shift = 6;
    while (BitReader_ready()) {
        if (read_raw_data) {
            pair = read_pair();
            read_raw_data = pair != 0;
        }
        if (!read_raw_data) { // Want to run this if the pair is zero from previous
            if (num_zeroes == 0) {
                num_zeroes = decode_rle();
            }
            pair = 0;
            num_zeroes--;
            read_raw_data = num_zeroes == 0;
        }
        buffer[pos] |= (pair << shift);
        pos += 1;
        if (pos % col_height == 0) {
            if (shift == 0 && pos >= size) {
                break;
            }
            if (shift > 0) {
                shift -= 2;
                pos -= col_height;
            } else {
                shift = 6;
            }
        }
    }
}

const uint8_t DELTA_DECODE_NIBBLE[] = {
    // Delta-decoded sequences of 4 bits, with zero as initial state
    0b0000, 0b0001, 0b0011, 0b0010,  // From 0000, 0001, 0010, 0011
    0b0111, 0b0110, 0b0100, 0b0101,  // From 0100, 0101, 0110, 0111
    0b1111, 0b1110, 0b1100, 0b1101,  // From 1000, 1001, 1010, 1011
    0b1000, 0b1001, 0b1011, 0b1010,  // From 1100, 1101, 1110, 1111
};

static void delta_decode_buffer(uint8_t width, uint8_t height, uint8_t *buffer) {
    if (s_debug) APP_LOG(APP_LOG_LEVEL_DEBUG, "Applying Delta encoding");

    for (uint16_t row = 0; row < height * 8; row++) {
        uint8_t state = 0;
        for (uint16_t col = 0; col < width; col++) {
            uint16_t pos = col * height * 8 + row;
            uint8_t byte = buffer[pos];

            uint8_t first = DELTA_DECODE_NIBBLE[byte >> 4] ^ (0b1111 * state);
            state = first & 1;
            uint8_t second = DELTA_DECODE_NIBBLE[byte & 0b1111] ^ (0b1111 * state);
            state = second & 1;

            buffer[pos] = (first << 4) + second;
        }
    }
}

static void xor_buffers(uint8_t width, uint8_t height, uint8_t *write_buffer, uint8_t *read_buffer) {
    for (uint16_t i = 0; i < width * height * 8; i++) {
        write_buffer[i] ^= read_buffer[i];
    }
}

static void adjust_position(uint8_t width, uint8_t height, uint8_t *src_buffer, uint8_t *dest_buffer) {
    uint8_t h_pad = 7-height;
    uint8_t w_pad = (8 - width) / 2;
    uint8_t offset = 7 * w_pad + h_pad;
    memset(dest_buffer, 0, 392);
    uint16_t src = 0;
    uint16_t dst = offset * 8;

    uint8_t h_col = height * 8;
    for (uint8_t i = 0; i < width; i++) {
        memcpy(dest_buffer+dst, src_buffer+src, h_col);
        src += h_col;
        dst += 56;
    }
}

static uint16_t zip_bytes(uint8_t low_byte, uint8_t high_byte) {
    uint16_t result = 0;
    for (uint8_t i = 0; i < 8; i++) {
        result |= (low_byte & (1 << i)) << i;
    }
    for (uint8_t i = 0; i < 8; i++) {
        result |= (high_byte & (1 << i)) << (i + 1);
    }
    return result;
}

static void zip_bit_planes(uint8_t *buffer) {
    uint16_t pt_a = 391;
    uint16_t pt_b = 783;
    uint16_t pt_zip = 1175;
    while (pt_zip >= 392) {
        uint8_t byte_a = buffer[pt_a];
        uint8_t byte_b = buffer[pt_b];
        uint16_t zipped_bytes = zip_bytes(byte_b, byte_a);
        buffer[pt_zip] = (zipped_bytes & 0xff);
        buffer[pt_zip - 1] = (zipped_bytes >> 8);
        pt_zip -= 2;
        pt_a -= 1;
        pt_b -= 1;
    }
}

void load_pokemon_sprite(uint32_t resource_id, uint32_t data_start, uint16_t data_size, uint8_t *dest_buffer) {
    BitReader_init(resource_id, data_start, data_size);
    uint8_t width = s_read(4);
    uint8_t height = s_read(4); 
    if (s_debug) APP_LOG(APP_LOG_LEVEL_DEBUG, "Sprite size: (%d, %d)", width, height);

    uint8_t *memory_buffer = (uint8_t*)malloc(1176); // 7 tiles * 7 tiles * 8 bytes per tile * 3 planes
    memset(memory_buffer, 0, 1176);
    uint8_t *buffer_a = memory_buffer;
    uint8_t *buffer_b = memory_buffer + 392;
    uint8_t *buffer_c = memory_buffer + 784;
    uint8_t *buffer_0, *buffer_1;

    bool invert_buffers = s_read(1);
    if (invert_buffers) {
        buffer_0 = buffer_c;
        buffer_1 = buffer_b;
    } else {
        buffer_0 = buffer_b;
        buffer_1 = buffer_c;
    }
    if (s_debug) APP_LOG(APP_LOG_LEVEL_DEBUG, "Bit plane order detected: BP0 in %c", invert_buffers ? 'C' : 'B');

    if (s_debug) APP_LOG(APP_LOG_LEVEL_DEBUG, "Decompressing Bit Plane 0");
    decompress_to_buffer(width, height, buffer_0);

    uint8_t mode = s_read(1);
    if (mode == 1) {
        mode += s_read(1);
    }
    mode += 1;

    if (s_debug) APP_LOG(APP_LOG_LEVEL_DEBUG, "Decompressing Bit Plane 1");
    decompress_to_buffer(width, height, buffer_1);
    // if (s_debug) APP_LOG(APP_LOG_LEVEL_DEBUG, "Total bits read: %d", BitReader_get_pointer());

    if (s_debug) APP_LOG(APP_LOG_LEVEL_DEBUG, "Decoding mode %d detected", mode);

    if (mode != 2) {
        delta_decode_buffer(width, height, buffer_1);
    }

    delta_decode_buffer(width, height, buffer_0);

    if (mode != 1) {
        xor_buffers(width, height, buffer_1, buffer_0);
    }
    if (s_debug) APP_LOG(APP_LOG_LEVEL_DEBUG, "Decompression complete");

    if (s_debug) APP_LOG(APP_LOG_LEVEL_DEBUG, "Adjusting the position of the sprite for a size of %dx%d", width, height);
    adjust_position(width, height, buffer_b, buffer_a);
    adjust_position(width, height, buffer_c, buffer_b);

    zip_bit_planes(memory_buffer);
    if (s_debug) APP_LOG(APP_LOG_LEVEL_DEBUG, "Processing complete");

    memcpy(dest_buffer, &memory_buffer[392], 784);
    if (s_debug) APP_LOG(APP_LOG_LEVEL_DEBUG, "Decompressed sprite copied to vram");

    free(memory_buffer);
    BitReader_deinit();
}