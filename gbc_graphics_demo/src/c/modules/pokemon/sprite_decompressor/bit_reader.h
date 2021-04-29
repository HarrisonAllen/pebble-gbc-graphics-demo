#pragma once

#include <pebble.h>

void BitReader_init(uint32_t resource_id, uint32_t data_start, uint16_t data_size);
uint16_t BitReader_read_bits(uint8_t num_bits);
bool BitReader_ready();
uint32_t BitReader_get_pointer();
void BitReader_deinit();