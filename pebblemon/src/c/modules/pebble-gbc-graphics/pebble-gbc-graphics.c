#include "pebble-gbc-graphics.h"

///> Forward declarations for static functions
static void bg_update_proc(Layer *layer, GContext *ctx);
static void sprite_update_proc(Layer *layer, GContext *ctx);

GBC_Graphics *GBC_Graphics_ctor(Window *window) { 
  GBC_Graphics *self = NULL;
  self = malloc(sizeof(GBC_Graphics));
  if (self == NULL)
      return NULL;
  
  // Initialize the bg/window and sprite layers
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  self->bg_layer = layer_create_with_data(bounds, sizeof(self));
  self->sprite_layer = layer_create_with_data(bounds, sizeof(self));
  *(GBC_Graphics * *)layer_get_data(self->bg_layer) = self;
  *(GBC_Graphics * *)layer_get_data(self->sprite_layer) = self;
  layer_set_update_proc(self->bg_layer, bg_update_proc);
  layer_set_update_proc(self->sprite_layer, sprite_update_proc);
  layer_add_child(window_layer, self->bg_layer);
  layer_add_child(window_layer, self->sprite_layer);

  // Set the screen boundaries to the size of the Pebble display
  self->screen_x_origin = bounds.origin.x;
  self->screen_y_origin = bounds.origin.y;
  self->screen_width = bounds.size.w;
  self->screen_height = bounds.size.h;

  // Allocate 4 banks of VRAM space
  self->vram = (uint8_t*)malloc(VRAM_BANK_SIZE * 4);

  // Allocate space for the tilemaps and attributemaps
  self->bg_tilemap = (uint8_t*)malloc(TILEMAP_SIZE);
  self->bg_attrmap = (uint8_t*)malloc(ATTRMAP_SIZE);
  self->window_tilemap = (uint8_t*)malloc(TILEMAP_SIZE);
  self->window_attrmap = (uint8_t*)malloc(ATTRMAP_SIZE);

  // Allocate space for the palette banks
  self->bg_palette_bank = (uint8_t*)malloc(PALETTE_BANK_SIZE);
  self->sprite_palette_bank = (uint8_t*)malloc(PALETTE_BANK_SIZE);

  // Allocate space for the OAM
  self->oam = (uint8_t*)malloc(OAM_SIZE);

  self->lcdc = 0xFF; // Start LCDC with everything enable (render everything)
  self->stat = 0x00; // Start STAT empty

  return self;
}

void GBC_Graphics_destroy(GBC_Graphics *self) {
  free(self->vram);
  free(self->oam);
  free(self->bg_tilemap);
  free(self->bg_attrmap);
  free(self->window_tilemap);
  free(self->window_attrmap);
  free(self->bg_palette_bank);
  free(self->sprite_palette_bank);
  layer_destroy(self->bg_layer);
  layer_destroy(self->sprite_layer);
  if (self == NULL) return;
    free(self);
}


void GBC_Graphics_set_screen_bounds(GBC_Graphics *self, GRect bounds) {
  self->screen_x_origin = bounds.origin.x;
  self->screen_y_origin = bounds.origin.y;
  self->screen_width = bounds.size.w;
  self->screen_height = bounds.size.h;
  layer_set_frame(self->bg_layer, bounds);
  layer_set_frame(self->sprite_layer, bounds);
}

void GBC_Graphics_set_screen_x_origin(GBC_Graphics *self, uint8_t new_x) {
  self->screen_x_origin = new_x;
}

void GBC_Graphics_set_screen_y_origin(GBC_Graphics *self, uint8_t new_y) {
  self->screen_y_origin = new_y;
}

void GBC_Graphics_set_screen_width(GBC_Graphics *self, uint8_t new_width) {
  self->screen_width = new_width;
}

void GBC_Graphics_set_screen_height(GBC_Graphics *self, uint8_t new_height) {
  self->screen_height = new_height;
}


GRect GBC_Graphics_get_screen_bounds(GBC_Graphics *self) {
  return GRect(self->screen_x_origin, self->screen_y_origin, self->screen_width, self->screen_height);
}

uint8_t GBC_Graphics_get_screen_x_origin(GBC_Graphics *self) {
  return self->screen_x_origin;
}

uint8_t GBC_Graphics_get_screen_y_origin(GBC_Graphics *self) {
  return self->screen_y_origin;
}

uint8_t GBC_Graphics_get_screen_width(GBC_Graphics *self) {
  return self->screen_width;
}

uint8_t GBC_Graphics_get_screen_height(GBC_Graphics *self) {
  return self->screen_height;
}

void GBC_Graphics_vram_move_tiles(GBC_Graphics *self, uint8_t src_vram_bank, uint8_t src_tile_offset, 
                             uint8_t dest_vram_bank, uint8_t dest_tile_offset, uint8_t num_tiles_to_move, bool swap) {
  uint8_t *src_vram_offset = self->vram + VRAM_BANK_SIZE * src_vram_bank + src_tile_offset * TILE_SIZE;
  uint8_t *dest_vram_offset = self->vram + VRAM_BANK_SIZE * dest_vram_bank + dest_tile_offset * TILE_SIZE;
  size_t data_size = num_tiles_to_move * TILE_SIZE;
  if (swap) {
    uint8_t temp[data_size];
    memcpy(temp, dest_vram_offset, data_size);
    memcpy(dest_vram_offset, src_vram_offset, data_size);
    memcpy(src_vram_offset, temp, data_size);
  } else {
    memcpy(dest_vram_offset, src_vram_offset, data_size);
  }
}

void GBC_Graphics_load_from_tilesheet_into_vram(GBC_Graphics *self, uint32_t tilesheet_resource, uint16_t tilesheet_tile_offset, 
                                            uint16_t tiles_to_load, uint16_t vram_tile_offset, uint8_t vram_bank_number) {
  ResHandle tilesheet_handle = resource_get_handle(tilesheet_resource);
  uint8_t *vram_offset = self->vram + VRAM_BANK_SIZE * vram_bank_number + vram_tile_offset * TILE_SIZE;
  
  resource_load_byte_range(tilesheet_handle, tilesheet_tile_offset * TILE_SIZE, vram_offset, tiles_to_load * TILE_SIZE);
}

uint8_t *GBC_Graphics_get_vram_bank(GBC_Graphics *self, uint8_t vram_bank_number) {
  return &self->vram[vram_bank_number * VRAM_BANK_SIZE];
}

/**
 * Sets the colors of a palette in the given palette bank
 * 
 * @param palette_bank A pointer to the palette bank
 * @param palette_num The number of the palette to set, 0 to 7
 * @param c0 Color 0 of the new palette
 * @param c1 Color 1 of the new palette
 * @param c2 Color 2 of the new palette
 * @param c3 Color 3 of the new palette
 */
static void set_palette(uint8_t *palette_bank, uint8_t palette_num, uint8_t c0, uint8_t c1, uint8_t c2, uint8_t c3) {
  palette_bank[palette_num*PALETTE_SIZE+0] = c0;
  palette_bank[palette_num*PALETTE_SIZE+1] = c1;
  palette_bank[palette_num*PALETTE_SIZE+2] = c2;
  palette_bank[palette_num*PALETTE_SIZE+3] = c3;
}

void GBC_Graphics_set_bg_palette(GBC_Graphics *self, uint8_t palette_num, uint8_t c0, uint8_t c1, uint8_t c2, uint8_t c3) {
  set_palette(self->bg_palette_bank, palette_num, c0, c1, c2, c3);
}

void GBC_Graphics_set_sprite_palette(GBC_Graphics *self, uint8_t palette_num, uint8_t c0, uint8_t c1, uint8_t c2, uint8_t c3) {
  set_palette(self->sprite_palette_bank, palette_num, c0, c1, c2, c3);
}

/**
 * Sets the colors of a palette in the given palette bank
 * 
 * @param palette_bank A pointer to the palette bank
 * @param palette_num The number of the palette to set, 0 to 7
 * @param palette_array A pointer to a 4-byte array containing the palette colors
 */
static void set_palette_array(uint8_t *palette_bank, uint8_t palette_num, uint8_t *palette_array) {
  memcpy(&palette_bank[palette_num*PALETTE_SIZE], palette_array, PALETTE_SIZE);
}

void GBC_Graphics_set_bg_palette_array(GBC_Graphics *self, uint8_t palette_num, uint8_t *palette_array) {
  set_palette_array(self->bg_palette_bank, palette_num, palette_array);
}

void GBC_Graphics_set_sprite_palette_array(GBC_Graphics *self, uint8_t palette_num, uint8_t *palette_array) {
  set_palette_array(self->sprite_palette_bank, palette_num, palette_array);
}

static void copy_palette_array(uint8_t *palette_bank, uint8_t palette_num, uint8_t *target_array) {
  memcpy(target_array, &palette_bank[palette_num*PALETTE_SIZE], PALETTE_SIZE);
}

void GBC_Graphics_copy_one_bg_palette(GBC_Graphics *self, uint8_t palette_num, uint8_t *target_array) {
  copy_palette_array(self->bg_palette_bank, palette_num, target_array);
}

void GBC_Graphics_copy_one_sprite_palette(GBC_Graphics *self, uint8_t palette_num, uint8_t *target_array) {
  copy_palette_array(self->sprite_palette_bank, palette_num, target_array);
}

static void copy_palette_bank(uint8_t *palette_bank, uint8_t *target_array) {
  memcpy(target_array, palette_bank, PALETTE_BANK_SIZE);
}

void GBC_Graphics_copy_all_bg_palettes(GBC_Graphics *self, uint8_t *target_array) {
  copy_palette_bank(self->bg_palette_bank, target_array);
}

void GBC_Graphics_copy_all_sprite_palettes(GBC_Graphics *self, uint8_t *target_array) {
  copy_palette_bank(self->sprite_palette_bank, target_array);
}


/**
 * Clamps a short variable between two uint8_t values
 * 
 * @param to_clamp The value to clamp
 * @param lower_bound The lower bound on the return value
 * @param uppper_bound The upper bound on the return value
 * 
 * @return The clamped value as uint8_t
 */
static uint8_t clamp_short_to_uint8_t(short to_clamp, uint8_t lower_bound, uint8_t upper_bound) {
  if (to_clamp < (short)lower_bound) {
    return lower_bound;
  }
  if (to_clamp > (short)upper_bound) {
    return upper_bound;
  }
  return (uint8_t) to_clamp;
}

/**
 * Sets the bits outlined by mask to new_value
 * 
 * @param byte A pointer to the byte to modify
 * @param mask The bits to modify
 * @param new_value The new value to set the bits to
 * @param byte_start The offset of the data
 */
static void modify_byte(uint8_t *byte, uint8_t mask, uint8_t new_value, uint8_t byte_start) {
  *byte = (*byte & ~mask) | new_value * byte_start;
}

/**
 * Renders the background and window, called from an update proc
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param layer A pointer to the layer to modify
 * @param ctx The graphics context for drawing
 */
static void render_bg_graphics(GBC_Graphics *self, Layer *layer, GContext *ctx) {
  // Return early if we don't need to render the background or window
  if (!(self->lcdc & LCDC_ENABLE_FLAG) || (!(self->lcdc & LCDC_BCKGND_ENABLE_FLAG) && !(self->lcdc & LCDC_WINDOW_ENABLE_FLAG))) {
    return;
  }
  GBitmap *fb = graphics_capture_frame_buffer(ctx);

  uint8_t window_offset_y = clamp_short_to_uint8_t(self->window_offset_y, 0, self->screen_height);
  uint8_t window_offset_x = clamp_short_to_uint8_t(self->window_offset_x, 0, self->screen_width);

  // Predefine the variables we'll use in the loop
  uint8_t map_x, map_y, map_tile_x, map_tile_y, tile_num, tile_attr;
  uint8_t *tilemap, *attrmap;
  uint16_t offset;
  uint8_t *tile;
  uint8_t pixel_x, pixel_y, pixel_byte, pixel_color, pixel;
  uint8_t shift;
  uint8_t x;
  uint8_t flip;
  bool in_window_y;

  self->stat &= ~STAT_VBLANK_FLAG; // No longer in VBlank while we draw
  for (self->line_y = 0; self->line_y < self->screen_height; self->line_y++) {
    // Check if the current line matches the line compare value, and then do the callback
    self->stat &= ~STAT_LINE_COMP_FLAG;
    self->stat |= STAT_LINE_COMP_FLAG * (self->line_y == self->line_y_compare);
    if ((self->stat & (STAT_LINE_COMP_INT_FLAG | STAT_LINE_COMP_FLAG)) == (STAT_LINE_COMP_INT_FLAG | STAT_LINE_COMP_FLAG)) {
      self->line_compare_interrupt_callback(self);
    }

    in_window_y = self->line_y >= window_offset_y;

    GBitmapDataRowInfo info = gbitmap_get_data_row_info(fb, self->line_y + self->screen_y_origin);
    uint8_t min_x = MAX(info.min_x, self->screen_x_origin);
    uint8_t max_x = MIN(info.max_x+1, self->screen_x_origin + self->screen_width);

    self->stat &= ~STAT_HBLANK_FLAG; // No longer in HBlank while we draw the line
    for(x = min_x; x < max_x; x++) {
      // Decide what pixel to draw, first check if we're in the window bounds
      if (in_window_y && (x - self->screen_x_origin) >= window_offset_x) {
        map_x = (x - self->screen_x_origin) - self->window_offset_x;
        map_y = self->line_y - self->window_offset_y;
        tilemap = self->window_tilemap;
        attrmap = self->window_attrmap;
      } else { // Otherwise draw the background
        map_x = (x - self->screen_x_origin) + self->bg_scroll_x;
        map_y = self->line_y + self->bg_scroll_y;
        tilemap = self->bg_tilemap;
        attrmap = self->bg_attrmap;
      }

      // Find the tile that the pixel is on
      map_tile_x = map_x >> 3; // map_x / TILE_WIDTH
      map_tile_y = map_y >> 3; // map_y / TILE_HEIGHT

      // Get the tile and attrs from the map
      tile_num = tilemap[map_tile_x + (map_tile_y << 5)]; // map_tile_y * MAP_WIDTH
      tile_attr = attrmap[map_tile_x + (map_tile_y << 5)];
      
      // Get the tile from vram
      offset = tile_num << 4; // tile_num * TILE_SIZE
      tile = self->vram + ((((tile_attr & ATTR_VRAM_BANK_MASK) >> 3)) << 12) + offset; // self->vram + vram_bank_number * VRAM_BANK_SIZE + offset

      // Next, we extract and return the 2bpp pixel from the tile
      pixel_x = map_x & 7; // map_x % TILE_WIDTH
      pixel_y = map_y & 7; // map_x % TILE_HEIGHT

      // Apply flip flags if necessary
      flip = tile_attr & ATTR_FLIP_FLAG_X;
      pixel_x = ((pixel_x >> (flip >> 3)) - ((pixel_x + 1) >> ((ATTR_FLIP_FLAG_X ^ flip) >> 3))) & 7; // flip ? 7 - pixel_x : pixel_x
      flip = tile_attr & ATTR_FLIP_FLAG_Y;
      pixel_y = ((pixel_y >> (flip >> 3)) - ((pixel_y + 1) >> ((ATTR_FLIP_FLAG_Y ^ flip) >> 3))) & 7;

      // To get the pixel, we first need to get the corresponding byte the pixel is in
      // There are 2 bytes per row (y * 2), and 4 pixels per byte (x / 4)
      offset = (pixel_y << 1) + (pixel_x >> 2); // pixel y * 2 + pixel_x / 4
      pixel_byte = tile[offset];

      // Once we have the byte, we need to get the 2 bit pixel out of the byte
      // This is achieved by shifting the byte (3 - x % 4) * (2 bits per pixel)
      shift = (3 ^ (pixel_x & 3)) << 1; // (3 - pixel_x % 4) * 2

      // We shift the byte and get rid of any unwanted bits
      pixel = 0b11 & (pixel_byte >> shift);

      // Finally, we get the corresponding color from attribute palette
      pixel_color = self->bg_palette_bank[((tile_attr & ATTR_PALETTE_MASK) << 2) + pixel]; // (tile_attr & ATTR_PALETTE_MASK) * 4

#if defined(PBL_COLOR)
      memset(&info.data[x], pixel_color, 1);
      // memset(&fb_data[x + ((self->line_y + self->screen_y_origin) << 7) + ((self->line_y + self->screen_y_origin) << 4)], pixel_color, 1); // x + self->line_y * row_size
#else
      uint16_t byte = (x >> 3); // x / 8
      uint8_t bit = x & 7; // x % 8
      uint8_t *byte_mod = &info.data[byte];
      *byte_mod ^= (-pixel_color ^ *byte_mod) & (1 << bit);
#endif
    }
    // Now we're in the HBlank state, run the callback
    self->stat |= STAT_HBLANK_FLAG;
    if (self->stat & STAT_HBLANK_INT_FLAG) {
      self->hblank_interrupt_callback(self);
    }
  }
  self->stat &= ~STAT_LINE_COMP_FLAG; // Clear line compare flag

  graphics_release_frame_buffer(ctx, fb);
  // Done drawing, now we're in VBlank, run the callback
  self->stat |= STAT_VBLANK_FLAG;
  if (self->stat & STAT_VBLANK_INT_FLAG) {
    self->vblank_interrupt_callback(self);
  }
}

/**
 * Renders the sprites, called from an update proc
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param layer A pointer to the layer to modify
 * @param ctx The graphics context for drawing
 */
static void render_sprite_graphics(GBC_Graphics *self, Layer *layer, GContext *ctx) {
  // Return early if we don't need to draw the sprites
  if (!(self->lcdc & LCDC_ENABLE_FLAG) || !(self->lcdc & LCDC_SPRITE_ENABLE_FLAG)) {
    return;
  }
  GBitmap *fb = graphics_capture_frame_buffer(ctx);

  // Predefine the variables we're going to use
  short screen_x, screen_y;
  uint8_t tile_x, tile_y, tile_num, tile_number;
  uint8_t map_x, map_y, map_tile_x, map_tile_y, bg_tile_num, bg_tile_attr;
  uint8_t *sprite, *tile, *bg_tile;
  bool flipped;
  uint8_t sprite_mode;
  uint16_t offset;
  uint8_t pixel_x, pixel_y, pixel, pixel_byte, pixel_color;
  uint8_t shift;
  uint8_t flip;

  self->stat &= ~STAT_OAM_FLAG; // Drawing sprites, clear OAM flag
  sprite_mode = (bool)(LCDC_SPRITE_SIZE_FLAG & self->lcdc);
  for (short sprite_id = 39; sprite_id >= 0; sprite_id--) { // Draw in reverse order so that sprite 0 is on top
    /*
      This entire for loop was previously implemented in draw_sprite_to_buffer(),
      but unwrapped here for optimization. Please refer to that function for readability.
    */
    sprite = &self->oam[sprite_id*4];
    // Don't draw the sprite if it's offscreen
    if (sprite[0] == 0 || sprite[1] == 0 || sprite[0] >= self->screen_width + SPRITE_OFFSET_X || sprite[1] >= self->screen_height + SPRITE_OFFSET_Y) {
      continue;
    }

    flipped = ATTR_FLIP_FLAG_Y & sprite[3];
    // We want to draw two tiles if the LCDC Sprite Size flag is set for 8x16 sprites, hence the for loop
    for (tile_num = 0; tile_num <= sprite_mode; tile_num++) {
      // For tile number, first take off the last bit if we're in 8x16 mode
      // Then, in 8x16 mode, select between the top and bottom tile dependent on the y flip
      tile_number = (sprite[2] & (0xFE | !sprite_mode));
      tile_number += (sprite_mode & flipped ? 1 - tile_num : tile_num);

      // Get the tile from vram
      offset = tile_number << 4; // tile_num * TILE_SIZE
      tile = self->vram + ((((sprite[3] & ATTR_VRAM_BANK_MASK) >> 3)) << 12) + offset; // self->vram + vram_bank_number * VRAM_BANK_SIZE + offset

      // Then we draw the tile row by row
      for (tile_y = 0; tile_y < TILE_HEIGHT; tile_y++) {
        screen_y = sprite[1] + tile_y - SPRITE_OFFSET_Y + TILE_HEIGHT * tile_num; // On second tile, offset by TILE_HEIGHT
        if (screen_y >= self->screen_height) {
          break;
        }
        if (screen_y < 0) {
          continue;
        }
        GBitmapDataRowInfo info = gbitmap_get_data_row_info(fb, screen_y + self->screen_y_origin);
        uint8_t min_x = MAX(info.min_x, self->screen_x_origin);
        uint8_t max_x = MIN(info.max_x, self->screen_x_origin + self->screen_width);

        for (tile_x = 0; tile_x < TILE_WIDTH; tile_x++) {
          // Check if the pixel is on the screen
          screen_x = sprite[0] + tile_x - SPRITE_OFFSET_X + self->screen_x_origin;
          if (screen_x >= max_x) {
            break;
          }
          if (screen_x < min_x) {
            continue;
          }

          // Get the tile x and y on the bg map
          map_x = self->bg_scroll_x + screen_x - self->screen_x_origin;
          map_y = self->bg_scroll_y + screen_y;
          map_tile_x = map_x >> 3; // map_x / TILE_WIDTH
          map_tile_y = map_y >> 3; // map_y / TILE_HEIGHT

          // Check if the background pixel has priority
          bg_tile_attr = self->bg_attrmap[map_tile_x + (map_tile_y << 5)];
          if (bg_tile_attr & ATTR_PRIORITY_FLAG) {
            continue;
          }

          // Now check if the sprite priority bit is set, and if this pixel should be transparent b/c of that
          if (sprite[3] & ATTR_PRIORITY_FLAG) {
            bg_tile_num = self->bg_tilemap[map_tile_x + (map_tile_y << 5)]; // map_tile_y * MAP_WIDTH
            
            // Get the tile from vram
            offset = bg_tile_num << 4; // tile_num * TILE_SIZE
            bg_tile = self->vram + ((((bg_tile_attr & ATTR_VRAM_BANK_MASK) >> 3)) << 12) + offset; // self->vram + vram_bank_number * VRAM_BANK_SIZE + offset
            
            // Next, we extract and return the 2bpp pixel from the tile
            pixel_x = map_x & 7; // map_x % TILE_WIDTH
            pixel_y = map_y & 7; // map_x % TILE_HEIGHT

            // Apply flip flags if necessary
            flip = bg_tile_attr & ATTR_FLIP_FLAG_X;
            pixel_x = ((pixel_x >> (flip >> 3)) - ((pixel_x + 1) >> ((ATTR_FLIP_FLAG_X ^ flip) >> 3))) & 7;
            flip = bg_tile_attr & ATTR_FLIP_FLAG_Y;
            pixel_y = ((pixel_y >> (flip >> 3)) - ((pixel_y + 1) >> ((ATTR_FLIP_FLAG_Y ^ flip) >> 3))) & 7;

            // To get the pixel, we first need to get the corresponding byte the pixel is in
            // There are 2 bytes per row (y * 2), and 4 pixels per byte (x / 4)
            offset = (pixel_y << 1) + (pixel_x >> 2); // pixel y * 2 + pixel_x / 4
            pixel_byte = bg_tile[offset];

            // Once we have the byte, we need to get the 2 bit pixel out of the byte
            // This is achieved by shifting the byte (3 - x % 4) * (2 bits per pixel)
            shift = (3 ^ (pixel_x & 3)) << 1; // (3 - pixel_x % 4) * 2

            // We shift the byte and get rid of any unwanted bits
            pixel = 0b11 & (pixel_byte >> shift);

            // Now, if the bg pixel is non-zero, the sprite pixel is transparent
            if (pixel) {
              continue;
            }
          }
          
          // Now we get the pixel from the sprite tile
          pixel_x = tile_x & 7; // tile_x % TILE_WIDTH
          pixel_y = tile_y & 7; // tile_y % TILE_HEIGHT

          // Apply flip flags if necessary
          flip = sprite[3] & ATTR_FLIP_FLAG_X;
          pixel_x = ((pixel_x >> (flip >> 3)) - ((pixel_x + 1) >> ((ATTR_FLIP_FLAG_X ^ flip) >> 3))) & 7;
          flip = sprite[3] & ATTR_FLIP_FLAG_Y;
          pixel_y = ((pixel_y >> (flip >> 3)) - ((pixel_y + 1) >> ((ATTR_FLIP_FLAG_Y ^ flip) >> 3))) & 7;

          // To get the pixel, we first need to get the corresponding byte the pixel is in
          // There are 2 bytes per row (y * 2), and 4 pixels per byte (x / 4)
          offset = (pixel_y << 1) + (pixel_x >> 2); // pixel y * 2 + pixel_x / 4
          pixel_byte = tile[offset];

          // Once we have the byte, we need to get the 2 bit pixel out of the byte
          // This is achieved by shifting the byte (3 - x % 4) * (2 bits per pixel)
          shift = (3 ^ (pixel_x & 3)) << 1; // (3 - pixel_x % 4) * 2

          // We shift the byte and get rid of any unwanted bits
          pixel = 0b11 & (pixel_byte >> shift);

          // The 0th palette color is transparency
          if (pixel == 0) {
            continue;
          }
          
          pixel_color = self->sprite_palette_bank[((sprite[3] & ATTR_PALETTE_MASK) << 2) + pixel]; // (tile_attr & ATTR_PALETTE_MASK) * PALETTE_SIZE + pixel
          
#if defined(PBL_COLOR)
          memset(&info.data[screen_x], pixel_color, 1);
          // memset(&fb_data[(screen_x + self->screen_x_origin) + ((screen_y + self->screen_y_origin) << 7) + ((screen_y + self->screen_y_origin) << 4)], pixel_color, 1); // x + self->line_y * row_size
#else
          uint16_t byte = ((screen_x) >> 3);
          uint8_t bit = screen_x & 7; // x % 8
          uint8_t *byte_mod = &info.data[byte];
          *byte_mod ^= (-pixel_color ^ *byte_mod) & (1 << bit);
#endif
        }
      }
    }

  }

  // Finished drawing sprites, call OAM callback
  self->stat |= STAT_OAM_FLAG;
  if (self->stat & STAT_OAM_INT_FLAG) {
    self->oam_interrupt_callback(self);
  }

  graphics_release_frame_buffer(ctx, fb);
}

void GBC_Graphics_render(GBC_Graphics *self) {
  layer_mark_dirty(self->bg_layer); // All layers will be redrawn, so we don't need to mark the sprite layer dirty
}

/**
 * The update proc to call when the bg and window layers are dirty
 * 
 * @param layer A pointer to the target layer
 * @param ctx A pointer to the graphics context
 */
static void bg_update_proc(Layer *layer, GContext *ctx) {
  render_bg_graphics(*(GBC_Graphics * *)layer_get_data(layer), layer, ctx);
}

/**
 * The update proc to call when the sprite layer is dirty
 * 
 * @param layer A pointer to the target layer
 * @param ctx A pointer to the graphics context
 */
static void sprite_update_proc(Layer *layer, GContext *ctx) {
  render_sprite_graphics(*(GBC_Graphics * *)layer_get_data(layer), layer, ctx);
}

void GBC_Graphics_lcdc_set(GBC_Graphics *self, uint8_t new_lcdc) {
  self->lcdc = new_lcdc;
}

void GBC_Graphics_lcdc_set_enabled(GBC_Graphics *self, bool enabled) {
  modify_byte(&self->lcdc, LCDC_ENABLE_FLAG, enabled, LCDC_ENABLE_FLAG);
}

void GBC_Graphics_lcdc_set_bg_layer_enabled(GBC_Graphics *self, bool enabled) {
  modify_byte(&self->lcdc, LCDC_BCKGND_ENABLE_FLAG, enabled, LCDC_BCKGND_ENABLE_FLAG);
}

void GBC_Graphics_lcdc_set_window_layer_enabled(GBC_Graphics *self, bool enabled) {
  modify_byte(&self->lcdc, LCDC_WINDOW_ENABLE_FLAG, enabled, LCDC_WINDOW_ENABLE_FLAG);
}

void GBC_Graphics_lcdc_set_sprite_layer_enabled(GBC_Graphics *self, bool enabled) {
  modify_byte(&self->lcdc, LCDC_SPRITE_ENABLE_FLAG, enabled, LCDC_SPRITE_ENABLE_FLAG);
}

void GBC_Graphics_lcdc_set_8x16_sprite_mode_enabled(GBC_Graphics *self, bool use_8x16_sprites) {
  modify_byte(&self->lcdc, LCDC_SPRITE_SIZE_FLAG, use_8x16_sprites, LCDC_SPRITE_SIZE_FLAG);
}

uint8_t GBC_Graphics_stat_get_current_line(GBC_Graphics *self) {
  return self->line_y;
}

uint8_t GBC_Graphics_stat_get_line_y_compare(GBC_Graphics *self) {
  return self->line_y_compare;
}

bool GBC_Graphics_stat_check_hblank_flag(GBC_Graphics *self) {
  return self->stat & STAT_HBLANK_FLAG;
}

bool GBC_Graphics_stat_check_vblank_flag(GBC_Graphics *self) {
  return self->stat & STAT_VBLANK_FLAG;
}

bool GBC_Graphics_stat_check_line_comp_flag(GBC_Graphics *self) {
  return self->stat & STAT_LINE_COMP_FLAG;
}

bool GBC_Graphics_stat_check_oam_flag(GBC_Graphics *self) {
  return self->stat & STAT_OAM_FLAG;
}

void GBC_Graphics_stat_set(GBC_Graphics *self, uint8_t new_stat) {
  modify_byte(&self->stat, STAT_WRITEABLE_MASK, new_stat & STAT_WRITEABLE_MASK, 1);
}

void GBC_Graphics_stat_set_hblank_interrupt_enabled(GBC_Graphics *self, bool enabled) {
  modify_byte(&self->stat, STAT_HBLANK_INT_FLAG, enabled, STAT_HBLANK_INT_FLAG);
}

void GBC_Graphics_stat_set_vblank_interrupt_enabled(GBC_Graphics *self, bool enabled) {
  modify_byte(&self->stat, STAT_VBLANK_INT_FLAG, enabled, STAT_VBLANK_INT_FLAG);
}

void GBC_Graphics_stat_set_line_compare_interrupt_enabled(GBC_Graphics *self, bool enabled) {
  modify_byte(&self->stat, STAT_LINE_COMP_INT_FLAG, enabled, STAT_LINE_COMP_INT_FLAG);
}

void GBC_Graphics_stat_set_oam_interrupt_enabled(GBC_Graphics *self, bool enabled) {
  modify_byte(&self->stat, STAT_OAM_INT_FLAG, enabled, STAT_OAM_INT_FLAG);
}

void GBC_Graphics_stat_set_line_y_compare(GBC_Graphics *self, uint8_t new_line_y_comp) {
  self->line_y_compare = new_line_y_comp;
}

void GBC_Graphics_set_hblank_interrupt_callback(GBC_Graphics *self, void (*callback)(GBC_Graphics *)) {
  self->hblank_interrupt_callback = callback;
}

void GBC_Graphics_set_vblank_interrupt_callback(GBC_Graphics *self, void (*callback)(GBC_Graphics *)) {
  self->vblank_interrupt_callback = callback;
}

void GBC_Graphics_set_line_compare_interrupt_callback(GBC_Graphics *self, void (*callback)(GBC_Graphics *)) {
  self->line_compare_interrupt_callback = callback;
}

void GBC_Graphics_set_oam_interrupt_callback(GBC_Graphics *self, void (*callback)(GBC_Graphics *)) {
  self->oam_interrupt_callback = callback;
}

uint8_t GBC_Graphics_attr_make(uint8_t palette, uint8_t vram_bank, bool is_x_flipped, bool is_y_flipped, bool bg_has_priority) {
  return (palette) | (vram_bank * ATTR_VRAM_BANK_START) | (is_x_flipped * ATTR_FLIP_FLAG_X) | (is_y_flipped * ATTR_FLIP_FLAG_Y) | (bg_has_priority * ATTR_PRIORITY_FLAG);
}

uint8_t GBC_Graphics_attr_get_palette_num(uint8_t attributes) {
  return attributes & ATTR_PALETTE_MASK;
}

uint8_t GBC_Graphics_attr_get_vram_bank(uint8_t attributes) {
  return (attributes & ATTR_VRAM_BANK_MASK) / ATTR_VRAM_BANK_START;
}

bool GBC_Graphics_attr_is_x_flipped(uint8_t attributes) {
  return (bool)(attributes & ATTR_FLIP_FLAG_X);
}

bool GBC_Graphics_attr_is_y_flipped(uint8_t attributes) {
  return (bool)(attributes & ATTR_FLIP_FLAG_Y);
}

uint8_t GBC_Graphics_bg_get_scroll_x(GBC_Graphics *self) {
  return self->bg_scroll_x;
}

uint8_t GBC_Graphics_bg_get_scroll_y(GBC_Graphics *self) {
  return self->bg_scroll_y;
}

uint8_t GBC_Graphics_bg_get_tile(GBC_Graphics *self, uint8_t x, uint8_t y) {
  return self->bg_tilemap[POINT_TO_OFFSET(x, y)];
}

uint8_t GBC_Graphics_bg_get_attr(GBC_Graphics *self, uint8_t x, uint8_t y) {
  return self->bg_attrmap[POINT_TO_OFFSET(x, y)];
}

void GBC_Graphics_bg_move(GBC_Graphics *self, short dx, short dy) {
  short new_x = self->bg_scroll_x + dx;
  short new_y = self->bg_scroll_y + dy;

  if (new_x < 0) { // Wrap x 
    new_x = MAP_WIDTH * TILE_WIDTH + new_x;
  } else {
    new_x = new_x % (MAP_WIDTH * TILE_WIDTH);
  }

  if (new_y < 0) { // Wrap y
    new_y = MAP_HEIGHT * TILE_HEIGHT + new_y;
  } else {
    new_y = new_y % (MAP_HEIGHT * TILE_HEIGHT);
  }

  self->bg_scroll_x = new_x;
  self->bg_scroll_y = new_y;
}

void GBC_Graphics_bg_set_scroll_x(GBC_Graphics *self, uint8_t x) {
  self->bg_scroll_x = x;
}

void GBC_Graphics_bg_set_scroll_y(GBC_Graphics *self, uint8_t y) {
  self->bg_scroll_y = y;
}

void GBC_Graphics_bg_set_scroll_pos(GBC_Graphics *self, uint8_t x, uint8_t y) {
  self->bg_scroll_x = x;
  self->bg_scroll_y = y;
}

void GBC_Graphics_bg_set_tile(GBC_Graphics *self, uint8_t x, uint8_t y, uint8_t tile_number) {
  self->bg_tilemap[POINT_TO_OFFSET(x, y)] = tile_number;
}

void GBC_Graphics_bg_set_attrs(GBC_Graphics *self, uint8_t x, uint8_t y, uint8_t attributes) {
  self->bg_attrmap[POINT_TO_OFFSET(x, y)] = attributes;
}

void GBC_Graphics_bg_set_tile_and_attrs(GBC_Graphics *self, uint8_t x, uint8_t y, uint8_t tile_number, uint8_t attributes) {
  self->bg_tilemap[POINT_TO_OFFSET(x, y)] = tile_number;
  self->bg_attrmap[POINT_TO_OFFSET(x, y)] = attributes;
}

void GBC_Graphics_bg_set_tile_palette(GBC_Graphics *self, uint8_t x, uint8_t y, uint8_t palette) {
  modify_byte(&self->bg_attrmap[POINT_TO_OFFSET(x, y)], ATTR_PALETTE_MASK, palette, ATTR_PALETTE_START);
}

void GBC_Graphics_bg_set_tile_vram_bank(GBC_Graphics *self, uint8_t x, uint8_t y, uint8_t vram_bank) {
  modify_byte(&self->bg_attrmap[POINT_TO_OFFSET(x, y)], ATTR_VRAM_BANK_MASK, vram_bank, ATTR_VRAM_BANK_START);
}

void GBC_Graphics_bg_set_tile_x_flip(GBC_Graphics *self, uint8_t x, uint8_t y, bool flipped) {
  modify_byte(&self->bg_attrmap[POINT_TO_OFFSET(x, y)], ATTR_FLIP_FLAG_X, flipped, ATTR_FLIP_FLAG_X);
}

void GBC_Graphics_bg_set_tile_y_flip(GBC_Graphics *self, uint8_t x, uint8_t y, bool flipped) {
  modify_byte(&self->bg_attrmap[POINT_TO_OFFSET(x, y)], ATTR_FLIP_FLAG_Y, flipped, ATTR_FLIP_FLAG_Y);
}

void GBC_Graphics_bg_set_tile_priority(GBC_Graphics *self, uint8_t x, uint8_t y, bool has_priority) {
  modify_byte(&self->bg_attrmap[POINT_TO_OFFSET(x, y)], ATTR_PRIORITY_FLAG, has_priority, ATTR_PRIORITY_FLAG);
}

void GBC_Graphics_bg_move_tile(GBC_Graphics *self, uint8_t src_x, uint8_t src_y, uint8_t dest_x, uint8_t dest_y, bool swap) {
  uint8_t src_tile = self->bg_tilemap[POINT_TO_OFFSET(src_x, src_y)];
  uint8_t src_attr = self->bg_attrmap[POINT_TO_OFFSET(src_x, src_y)];
  uint8_t dest_tile = self->bg_tilemap[POINT_TO_OFFSET(dest_x, dest_y)];
  uint8_t dest_attr = self->bg_attrmap[POINT_TO_OFFSET(dest_x, dest_y)];
  self->bg_tilemap[POINT_TO_OFFSET(dest_x, dest_y)] = src_tile;
  self->bg_attrmap[POINT_TO_OFFSET(dest_x, dest_y)] = src_attr;

  if (swap) {
    self->bg_tilemap[POINT_TO_OFFSET(src_x, src_y)] = dest_tile;
    self->bg_attrmap[POINT_TO_OFFSET(src_x, src_y)] = dest_attr;
  }
}

uint8_t GBC_Graphics_window_get_offset_x(GBC_Graphics *self) {
  return self->window_offset_x;
}

uint8_t GBC_Graphics_window_get_offset_y(GBC_Graphics *self) {
  return self->window_offset_y;
}

uint8_t GBC_Graphics_window_get_tile(GBC_Graphics *self, uint8_t x, uint8_t y) {
  return self->window_tilemap[POINT_TO_OFFSET(x, y)];
}

uint8_t GBC_Graphics_window_get_attr(GBC_Graphics *self, uint8_t x, uint8_t y) {
  return self->window_attrmap[POINT_TO_OFFSET(x, y)];
}

void GBC_Graphics_window_move(GBC_Graphics *self, short dx, short dy) {
  short new_x = clamp_short_to_uint8_t(self->window_offset_x + dx, 0, self->screen_width);
  short new_y = clamp_short_to_uint8_t(self->window_offset_y + dy, 0, self->screen_height);

  self->window_offset_x = new_x;
  self->window_offset_y = new_y;
}

void GBC_Graphics_window_set_offset_x(GBC_Graphics *self, uint8_t x) {
  self->window_offset_x = x;
}

void GBC_Graphics_window_set_offset_y(GBC_Graphics *self, uint8_t y) {
  self->window_offset_y = y;
}

void GBC_Graphics_window_set_offset_pos(GBC_Graphics *self, uint8_t x, uint8_t y) {
  self->window_offset_x = x;
  self->window_offset_y = y;
}

void GBC_Graphics_window_set_tile(GBC_Graphics *self, uint8_t x, uint8_t y, uint8_t tile_number) {
  self->window_tilemap[POINT_TO_OFFSET(x, y)] = tile_number;
}

void GBC_Graphics_window_set_attrs(GBC_Graphics *self, uint8_t x, uint8_t y, uint8_t attributes) {
  self->window_attrmap[POINT_TO_OFFSET(x, y)] = attributes;
}

void GBC_Graphics_window_set_tile_and_attrs(GBC_Graphics *self, uint8_t x, uint8_t y, uint8_t tile_number, uint8_t attributes) {
  self->window_tilemap[POINT_TO_OFFSET(x, y)] = tile_number;
  self->window_attrmap[POINT_TO_OFFSET(x, y)] = attributes;
}

void GBC_Graphics_window_set_tile_palette(GBC_Graphics *self, uint8_t x, uint8_t y, uint8_t palette) {
  modify_byte(&self->window_attrmap[POINT_TO_OFFSET(x, y)], ATTR_PALETTE_MASK, palette, ATTR_PALETTE_START);
}

void GBC_Graphics_window_set_tile_vram_bank(GBC_Graphics *self, uint8_t x, uint8_t y, uint8_t vram_bank) {
  modify_byte(&self->window_attrmap[POINT_TO_OFFSET(x, y)], ATTR_VRAM_BANK_MASK, vram_bank, ATTR_VRAM_BANK_START);
}

void GBC_Graphics_window_set_tile_x_flip(GBC_Graphics *self, uint8_t x, uint8_t y, bool flipped) {
  modify_byte(&self->window_attrmap[POINT_TO_OFFSET(x, y)], ATTR_FLIP_FLAG_X, flipped, ATTR_FLIP_FLAG_X);
}

void GBC_Graphics_window_set_tile_y_flip(GBC_Graphics *self, uint8_t x, uint8_t y, bool flipped) {
  modify_byte(&self->window_attrmap[POINT_TO_OFFSET(x, y)], ATTR_FLIP_FLAG_Y, flipped, ATTR_FLIP_FLAG_Y);
}

void GBC_Graphics_window_set_tile_priority(GBC_Graphics *self, uint8_t x, uint8_t y, bool has_priority) {
  modify_byte(&self->window_attrmap[POINT_TO_OFFSET(x, y)], ATTR_PRIORITY_FLAG, has_priority, ATTR_PRIORITY_FLAG);
}


void GBC_Graphics_window_move_tile(GBC_Graphics *self, uint8_t src_x, uint8_t src_y, uint8_t dest_x, uint8_t dest_y, bool swap) {
  uint8_t src_tile = self->window_tilemap[POINT_TO_OFFSET(src_x, src_y)];
  uint8_t src_attr = self->window_attrmap[POINT_TO_OFFSET(src_x, src_y)];
  uint8_t dest_tile = self->window_tilemap[POINT_TO_OFFSET(dest_x, dest_y)];
  uint8_t dest_attr = self->window_attrmap[POINT_TO_OFFSET(dest_x, dest_y)];
  self->window_tilemap[POINT_TO_OFFSET(dest_x, dest_y)] = src_tile;
  self->window_attrmap[POINT_TO_OFFSET(dest_x, dest_y)] = src_attr;

  if (swap) {
    self->window_tilemap[POINT_TO_OFFSET(src_x, src_y)] = dest_tile;
    self->window_attrmap[POINT_TO_OFFSET(src_x, src_y)] = dest_attr;
  }
}

uint8_t GBC_Graphics_oam_get_sprite_x(GBC_Graphics *self, uint8_t sprite_num) {
  return self->oam[sprite_num*4+0];
}

uint8_t GBC_Graphics_oam_get_sprite_y(GBC_Graphics *self, uint8_t sprite_num) {
  return self->oam[sprite_num*4+1];
}

uint8_t GBC_Graphics_oam_get_sprite_tile(GBC_Graphics *self, uint8_t sprite_num) {
  return self->oam[sprite_num*4+2];
}

uint8_t GBC_Graphics_oam_get_sprite_attrs(GBC_Graphics *self, uint8_t sprite_num) {
  return self->oam[sprite_num*4+3];
}

void GBC_Graphics_oam_set_sprite(GBC_Graphics *self, uint8_t sprite_num, uint8_t x, uint8_t y, uint8_t tile_position, uint8_t attributes) {
  self->oam[sprite_num*4+0] = x;
  self->oam[sprite_num*4+1] = y;
  self->oam[sprite_num*4+2] = tile_position;
  self->oam[sprite_num*4+3] = attributes;
}

void GBC_Graphics_oam_move_sprite(GBC_Graphics *self, uint8_t sprite_num, short dx, short dy) {
  short new_x, new_y;
  new_x = self->oam[sprite_num*4+0] + dx;
  new_y = self->oam[sprite_num*4+1] + dy;
  self->oam[sprite_num*4+0] = new_x;
  self->oam[sprite_num*4+1] = new_y;
  // self->oam[sprite_num*4+0] = clamp_short_to_uint8_t(new_x, 0, SCREEN_WIDTH);
  // self->oam[sprite_num*4+1] = clamp_short_to_uint8_t(new_y, 0, SCREEN_HEIGHT);
}

void GBC_Graphics_oam_set_sprite_x(GBC_Graphics *self, uint8_t sprite_num, uint8_t x) {
  self->oam[sprite_num*4+0] = x;
}

void GBC_Graphics_oam_set_sprite_y(GBC_Graphics *self, uint8_t sprite_num, uint8_t y) {
  self->oam[sprite_num*4+1] = y;
}

void GBC_Graphics_oam_set_sprite_pos(GBC_Graphics *self, uint8_t sprite_num, uint8_t x, uint8_t y) {
  self->oam[sprite_num*4+0] = x;
  self->oam[sprite_num*4+1] = y;
}

void GBC_Graphics_oam_hide_sprite(GBC_Graphics *self, uint8_t sprite_num) {
  self->oam[sprite_num*4+0] = 0;
  self->oam[sprite_num*4+1] = 0;
}

void GBC_Graphics_oam_set_sprite_tile(GBC_Graphics *self, uint8_t sprite_num, uint8_t tile_position) {
  self->oam[sprite_num*4+2] = tile_position;
}

void GBC_Graphics_oam_set_sprite_attrs(GBC_Graphics *self, uint8_t sprite_num, uint8_t attributes) {
  self->oam[sprite_num*4+3] = attributes;
}

void GBC_Graphics_oam_set_sprite_palette(GBC_Graphics *self, uint8_t sprite_num, uint8_t palette) {
  modify_byte(&self->oam[sprite_num * 4 + 3], ATTR_PALETTE_MASK, palette, ATTR_PALETTE_START);
}

void GBC_Graphics_oam_set_sprite_vram_bank(GBC_Graphics *self, uint8_t sprite_num, uint8_t vram_bank) {
  modify_byte(&self->oam[sprite_num * 4 + 3], ATTR_VRAM_BANK_MASK, vram_bank, ATTR_VRAM_BANK_START);
}

void GBC_Graphics_oam_set_sprite_x_flip(GBC_Graphics *self, uint8_t sprite_num, bool flipped) {
  modify_byte(&self->oam[sprite_num * 4 + 3], ATTR_FLIP_FLAG_X, flipped, ATTR_FLIP_FLAG_X);
}

void GBC_Graphics_oam_set_sprite_y_flip(GBC_Graphics *self, uint8_t sprite_num, bool flipped) {
  modify_byte(&self->oam[sprite_num * 4 + 3], ATTR_FLIP_FLAG_Y, flipped, ATTR_FLIP_FLAG_Y);
}

void GBC_Graphics_oam_set_sprite_priority(GBC_Graphics *self, uint8_t sprite_num, bool bg_has_priority) {
  modify_byte(&self->oam[sprite_num * 4 + 3], ATTR_PRIORITY_FLAG, bg_has_priority, ATTR_PRIORITY_FLAG);
}

void GBC_Graphics_oam_change_sprite_num(GBC_Graphics *self, uint8_t source_sprite_num, uint8_t target_sprite_num, bool copy) {
  uint8_t *source = &self->oam[source_sprite_num*4];
  uint8_t *target = &self->oam[target_sprite_num*4];
  for (uint8_t i = 0; i < 4; i++) {
    target[i] = source[i];
    if (!copy) {
      source[i] = 0;
    }
  }
}

void GBC_Graphics_oam_swap_sprites(GBC_Graphics *self, uint8_t sprite_num_1, uint8_t sprite_num_2) {
  uint8_t temp;
  uint8_t *source_1 = &self->oam[sprite_num_1*4];
  uint8_t *source_2 = &self->oam[sprite_num_2*4];
  for (uint8_t i = 0; i < 4; i++) {
    temp = source_1[i];
    source_1[i] = source_2[i];
    source_2[i] = temp;
  }
}

void GBC_Graphics_oam_swap_sprite_tiles(GBC_Graphics *self, uint8_t sprite_num_1, uint8_t sprite_num_2) {
  uint8_t temp;
  uint8_t *source_1 = &self->oam[sprite_num_1*4];
  uint8_t *source_2 = &self->oam[sprite_num_2*4];
  uint8_t i = 2;
  temp = source_1[i];
  source_1[i] = source_2[i];
  source_2[i] = temp;
}

void GBC_Graphics_oam_swap_sprite_attrs(GBC_Graphics *self, uint8_t sprite_num_1, uint8_t sprite_num_2) {
  uint8_t temp;
  uint8_t *source_1 = &self->oam[sprite_num_1*4];
  uint8_t *source_2 = &self->oam[sprite_num_2*4];
  uint8_t i = 3;
  temp = source_1[i];
  source_1[i] = source_2[i];
  source_2[i] = temp;
}

void GBC_Graphics_oam_swap_sprite_tiles_and_attrs(GBC_Graphics *self, uint8_t sprite_num_1, uint8_t sprite_num_2) {
  uint8_t temp;
  uint8_t *source_1 = &self->oam[sprite_num_1*4];
  uint8_t *source_2 = &self->oam[sprite_num_2*4];
  for (uint8_t i = 2; i < 4; i++) {
    temp = source_1[i];
    source_1[i] = source_2[i];
    source_2[i] = temp;
  }
}

void GBC_Graphics_copy_background_to_window(GBC_Graphics *self) {
  memcpy(self->window_tilemap, self->bg_tilemap, TILEMAP_SIZE);
  memcpy(self->window_attrmap, self->bg_attrmap, ATTRMAP_SIZE);
}

void GBC_Graphics_copy_window_to_background(GBC_Graphics *self) {
  memcpy(self->bg_tilemap, self->window_tilemap, TILEMAP_SIZE);
  memcpy(self->bg_attrmap, self->window_attrmap, ATTRMAP_SIZE);
}