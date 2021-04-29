#include "bit_reader.h"

// Given a file, load piece of file into memory and yield one bit at a time

static uint8_t *s_buffer;
static uint32_t s_max_bits;
static uint32_t s_pointer;

void BitReader_init(uint32_t resource_id, uint32_t data_start, uint16_t data_size) {
    s_max_bits = data_size * 8;
    ResHandle data_handle = resource_get_handle(resource_id);
    s_buffer = (uint8_t*)malloc(data_size);
    resource_load_byte_range(data_handle, data_start, s_buffer, data_size);
    s_pointer = 0;
}

uint16_t BitReader_read_bits(uint8_t num_bits) {
    if (s_pointer >= s_max_bits) {
        return 0;
    }
    uint16_t output = 0;
    uint16_t offset;
    uint8_t shift, bit;
    for (uint8_t i = 0; i < num_bits; i++) {
        offset = s_pointer >> 3;
        shift = 7 - (s_pointer & 7);
        bit = (s_buffer[offset] >> shift) & 1;
        output <<= 1;
        output |= bit;
        s_pointer++;
        if (s_pointer >= s_max_bits) {
            return output;
        }
    }
    return output;
}

bool BitReader_ready() {
    return s_pointer < s_max_bits;
}

uint32_t BitReader_get_pointer() {
    return s_pointer;
}

void BitReader_deinit() {
    if (s_buffer != NULL) {
        free(s_buffer);
        s_buffer = NULL;
    }
}