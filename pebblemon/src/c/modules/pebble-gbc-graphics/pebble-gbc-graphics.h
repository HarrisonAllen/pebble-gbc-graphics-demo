#pragma once         ///> Prevent errors by being included multiple times

#include <pebble.h>  ///> Pebble SDK symbols

/**
 * Version 1.0.0 of pebble-gbc-graphics
 * Created by Harrison Allen
 * Read more on GitHub here: [link]
 */

/** Size definitions */
#define TILE_WIDTH 8  ///> Width of a tile in pixels
#define TILE_HEIGHT 8 ///> Height of a tile in pixels
/**
 * Number of bytes that one tile takes up, calculated by:
 * 2 bits per pixel * 8 pixels wide * 8 pixels tall = 128 bits
 * 128 bits / 8 bits per byte = 16 butes
 */
#define TILE_SIZE 16
/**
 * Number of bytes per VRAM bank, calculated by:
 * 256 tiles per bank * 16 bytes per tile = 4096 bytes
 */
#define VRAM_BANK_SIZE 4096
#define MAP_WIDTH 32  ///> Width of the background and window maps in tiles
#define MAP_HEIGHT 32 ///> Height of the background and window maps in tiles
/**
 * Size of the tilemap in bytes, calculated by:
 * 1 byte per tile location * 32 tiles wide * 32 tiles tall = 1024 bytes
 */
#define TILEMAP_SIZE 1024
/**
 * Size of the attributemap in bytes, calculated by:
 * 1 byte per attribute * 32 tiles wide * 32 tiles tall = 1024 bytes
 */
#define ATTRMAP_SIZE 1024
/**
 * The size of one palette, calculated by:
 * 1 byte per color * 4 colors per palette = 4 bytes
 */
#define PALETTE_SIZE 4
/**
 * The size of one palette bank, calculated by:
 * 4 bytes per palette * 8 palettes = 32 bytes
 */
#define PALETTE_BANK_SIZE 32
/**
 * The size of the OAM, calculated by:
 * 4 bytes per sprite * 40 sprite slots = 160 bytes
 */
#define OAM_SIZE 160
// #define SCREEN_WIDTH 144  ///> Width of the screen in pixels
// #define SCREEN_HEIGHT 144 ///> Height of the screen in pixels
#define SPRITE_OFFSET_X 8 ///> The x offset to allow for offscreen rendering
#define SPRITE_OFFSET_Y 16 ///> The y offset to allow for offscreen rendering

/** Attribute flags */
#define ATTR_PALETTE_MASK 0x07      ///> Mask for the palette number
#define ATTR_PALETTE_START 0x01     ///> LSB of the palette mask
#define ATTR_VRAM_BANK_MASK 0X18    ///> Mask for the VRAM bank number
#define ATTR_VRAM_BANK_START 0X08   ///> LSB of the VRAM bank mask
#define ATTR_VRAM_BANK_00_FLAG 0X00 ///> Convenience flag for bank 0
#define ATTR_VRAM_BANK_01_FLAG 0x08 ///> Convenience flag for bank 1
#define ATTR_VRAM_BANK_02_FLAG 0x10 ///> Convenience flag for bank 2
#define ATTR_VRAM_BANK_03_FLAG 0X18 ///> Convenience flag for bank 3
#define ATTR_FLIP_FLAG_X 0x20       ///> Flag for horizontal flip
#define ATTR_FLIP_FLAG_Y 0x40       ///> Flag for vertical flip
#define ATTR_PRIORITY_FLAG 0X80     ///> Flag for priority bit

/** LCDC flags*/
#define LCDC_ENABLE_FLAG 0x01        ///> Flag for LCDC enable bit
#define LCDC_BCKGND_ENABLE_FLAG 0X02 ///> Flag for LCDC background enable bit
#define LCDC_WINDOW_ENABLE_FLAG 0x04 ///> Flag for LCDC window enable bit
#define LCDC_SPRITE_ENABLE_FLAG 0X08 ///> Flag for LCDC sprite enable bit
#define LCDC_SPRITE_SIZE_FLAG 0x10   ///> Flag for LCDC sprite size bit

/** STAT flags*/
#define STAT_HBLANK_FLAG 0x01        ///> Flag for STAT HBlank flag bit
#define STAT_VBLANK_FLAG 0X02        ///> Flag for STAT VBlank flag bit
#define STAT_LINE_COMP_FLAG 0X04     ///> Flag for STAT line compare flag bit
#define STAT_OAM_FLAG 0X08           ///> Flag for STAT OAM flag bit
#define STAT_HBLANK_INT_FLAG 0X10    ///> Flag for STAT HBlank interrupt bit
#define STAT_VBLANK_INT_FLAG 0X20    ///> Flag for STAT VBlank interrupt bit
#define STAT_LINE_COMP_INT_FLAG 0X40 ///> Flag for STAT line compare interrupt bit
#define STAT_OAM_INT_FLAG 0X80       ///> Flag for STAT OAM interrupt bit
#define STAT_READ_ONLY_MASK 0x0F     ///> Mask for the read only bits of STAT
#define STAT_WRITEABLE_MASK 0xF0     ///> Mask for the writeable bits of STAT

/** Helpful macros */
#define MIN(x, y) (y) ^ (((x) ^ (y)) & -((x) < (y))) ///> Finds the minimum of two values
#define MAX(x, y) (x) ^ (((x) ^ (y)) & -((x) < (y))) ///> Finds the maximum of two values
#define POINT_TO_OFFSET(x, y) ((x) & (MAP_WIDTH - 1)) + ((y) & (MAP_HEIGHT - 1)) * MAP_WIDTH ///> Converts an x, y point on a bg/window map to the tile/attrmap offset

/** Predefined Screen boundaries for convenience*/
#if defined(PBL_ROUND)
    #define SCREEN_BOUNDS_FULL GRect(0, 0, 180, 180)    ///> Fullscreen to get the most pixels
    #define SCREEN_BOUNDS_LARGE GRect(0, 18, 180, 144)   ///> Makes it even to tiles
    #define SCREEN_BOUNDS_SQUARE GRect(18, 18, 144, 144) ///> Square to get 60+ fps
    #define SCREEN_BOUNDS_SMALL GRect(26, 26, 128, 128)  ///> Small square
#else
    #define SCREEN_BOUNDS_FULL GRect(0, 0, 144, 168)    ///> Fullscreen to get the most pixels
    #define SCREEN_BOUNDS_LARGE GRect(0, 4, 144, 160)   ///> Gets rid of the extra tile row on the y axis
    #define SCREEN_BOUNDS_SQUARE GRect(0, 12, 144, 144) ///> Square to get 60+ fps
    #define SCREEN_BOUNDS_SMALL GRect(8, 16, 128, 128)  ///> Small square
#endif

/** The GBC Graphics "class" struct */
typedef struct _gbc_graphics GBC_Graphics;
struct _gbc_graphics {
    Layer *bg_layer; ///< The Layer on which to draw the background and window tiles
    Layer *sprite_layer; ///< The Layer on which to draw the sprite tiles
    /**
     * The LCD Control Byte
     *  -Bit 0: Enable - Setting bit to 0 disables drawing of background, window, and sprites
     *  -Bit 1: BG Display Enable - Setting bit to 0 disables drawing of background
     *  -Bit 2: Window Display Enable - Setting bit to 0 disables drawing of window
     *  -Bit 3: Sprite Display Enable - Setting bit to 0 disables drawing of sprites
     *  -Bit 4: Sprite Size - Setting bit to 0 uses one tile per sprite (8x8), 
     *                        Setting bit to 1 uses two tiles per sprite(8x16)
     *  -Bit 5: Unused
     *  -Bit 6: Unused
     *  -Bit 7: Unused
     */
    uint8_t lcdc;
    /**
     * VRAM Buffer - Stores the tiles for the background, window, and sprites in a 2bpp format
     * The VRAM contains 4 banks of 4096 bytes, for a total of 16384 bytes
     * Each bank holds up to 256 tiles in 2bpp format of 16 bytes each
     */
    uint8_t *vram;
    /**
     * OAM Buffer - Stores the data for the current sprites
     * The OAM contains 40 slots for 4 bytes of sprite information, which is as follows:
     *  -Byte 0: Sprite x position, offset by -8 (sprite width) to allow for off-screen rendering
     *      When x == 0 or x >= 144, the sprite will be hidden
     *  -Byte 1: Sprite y position, offset by -16 (sprite height) to allow for off-screen rendering
     *      When y == 0 or y >= 168, the sprite will be hidden
     *  -Byte 2: Sprite tile position in VRAM bank
     *      Note: When the Sprite Size bit of LCDC is set, Bit 0 will be ignored, and the two tiles 
     *            at the memory location will be used to render the 8x16 sprite
     *  -Byte 3: Sprite attribute data:
     *      -Bits 0-2: Palette number, from 0 to 7
     *      -Bits 3-4: VRAM Bank Select, from 0 to 3
     *      -Bit 5: X Flip - Setting bit will flip the sprite horizontally when rendered
     *      -Bit 6: Y Flip - Setting bit will flip the sprite vertically when rendered
     *      -Bit 7: Priority - Setting bit will make any non-zero background pixel take priority
     */
    uint8_t *oam;
    /**
     * Background Tilemap buffer
     * This tilemap contains 32 x 32 VRAM bank locations of 1 byte each, totaling 1024 bytes
     * The offset of the screen view (which is 144x160 pixels) is set by bg_scroll_x and bg_scroll_y
     */
    uint8_t *bg_tilemap;
    /**
     * Window Tilemap buffer
     * This tilemap contains 32 x 32 VRAM bank locations of 1 byte each, totaling 1024 bytes
     * The window is always drawn starting at the top left corner, and the window location on
     * the screen is set by window_offset_x and window_offset_y
     */
    uint8_t *window_tilemap;
    /**
     * Background Atributemap Buffer
     * This tilemap contains 32 x 32 tile attributes of 1 byte each, totaling 1024 bytes
     * Each attribute corresponds to a tile on the bg_tilemap
     * The attribute byte is defined as:
     *      -Bits 0-2: Palette number, from 0 to 7
     *      -Bits 3-4: VRAM Bank Select, from 0 to 3
     *      -Bit 5: X Flip - Setting bit will flip the tile horizontally when rendered
     *      -Bit 6: Y Flip - Setting bit will flip the tile vertically when rendered
     *      -Bit 7: BG Priority - Setting bit will cause tile to be rendered over sprites
     */
    uint8_t *bg_attrmap;
    /**
     * Window Atributemap Buffer
     * This tilemap contains 32 x 32 tile attributes of 1 byte each, totaling 1024 bytes
     * Each attribute corresponds to a tile on the bg_tilemap
     * The attribute byte is defined as:
     *      -Bits 0-2: Palette number, from 0 to 7
     *      -Bits 3-4: VRAM Bank Select, from 0 to 3
     *      -Bit 5: X Flip - Setting bit will flip the tile horizontally when rendered
     *      -Bit 6: Y Flip - Setting bit will flip the tile vertically when rendered
     *      -Bit 7: Window Priority - Setting bit will cause tile to be rendered over sprites
     */
    uint8_t *window_attrmap;
    /**
     * Background Palette Bank
     * The palette bank contains 8 palettes of 4 colors (1 byte) each, totalling 32 bytes
     */
    uint8_t *bg_palette_bank;
    /**
     * Sprite Palette Bank
     * The palette bank contains 8 palettes of 4 colors (1 byte) each, totalling 32 bytes
     * The 0th color in each palette will be replaced by transparency when rendered.
     */
    uint8_t *sprite_palette_bank;
    short bg_scroll_x; ///> The x position of the screen view into the background tilemap
    short bg_scroll_y; ///> The y position of the screen view into the background tilemap
    short window_offset_x; ///> The x offset of the window tilemap in the screen view, >= 144 == hidden
    short window_offset_y; ///> The y offset of the window tilemap in the screen view, >= 168 == hidden
    /**
     * LCD Status Byte
     *  -Bit 0: HBlank Flag - Set to 1 between rendering lines - READ ONLY
     *  -Bit 1: VBlank Flag - Set to 1 between rendering frames (after all lines rendered) - READ ONLY
     *  -Bit 2: Line Compare Flag - Set to 1 when line_y_compare == line_y - READ ONLY
     *  -Bit 3: OAM Flag - Set to 1 after all sprites are rendered - READ ONLY
     *  -Bit 4: HBlank Interrupt Enabled
     *  -Bit 5: VBlank Interrupt Enabled
     *  -Bit 6: Line Compare Interrupt Enabled
     *  -Bit 7: OAM Interrupt Enabled
     */
    uint8_t stat;
    uint8_t line_y; ///> The current line being rendered - READ ONLY
    uint8_t line_y_compare; ///> The line number to compare line_y against
    void (*hblank_interrupt_callback)(GBC_Graphics *); ///> The callback for the HBlank interrupt
    void (*vblank_interrupt_callback)(GBC_Graphics *); ///> The callback for the VBlank interrupt
    void (*line_compare_interrupt_callback)(GBC_Graphics *); ///> The callback for the line compare interrupt
    void (*oam_interrupt_callback)(GBC_Graphics *); ///> The callback for the oam interrupt

    uint8_t screen_x_origin; ///> The start x position of the rendered screen
    uint8_t screen_y_origin; ///> The start y position of the rendered screen
    uint8_t screen_width; ///> The width of the rendered screen
    uint8_t screen_height; ///> The height of the rendered screen
};

/**
 * Creates a GBC Graphics object
 * 
 * @param window The window in which to display the GBC Graphics object
 * 
 * @return a pointer to the created GBC Graphics object
 * @note I recommend creating the GBC Graphics object in window_load or an equivalent function
 */
GBC_Graphics *GBC_Graphics_ctor(Window *window);

/**
 * Destroys the GBC Graphics display object by freeing any memory it uses
 * 
 * @param self A pointer to the target GBC Graphics object
 */
void GBC_Graphics_destroy(GBC_Graphics *self);

/**
 * Sets the boundaries of the screen to render
 * @note Decreasing the size of the screen will greatly increase FPS, try GRect(0, 12, 144, 144)
 *
 * @param self A pointer to the target GBC Graphics object
 * @param bounds The boundaries of the screen, GRect(screen x origin, screen y origin, screen width, screen height)
 */
void GBC_Graphics_set_screen_bounds(GBC_Graphics *self, GRect bounds);

/**
 * Sets the x origin of the screen
 *
 * @param self A pointer to the target GBC Graphics object
 * @param new_x The new x origin of the screen
 */
void GBC_Graphics_set_screen_x_origin(GBC_Graphics *self, uint8_t new_x);

/**
 * Sets the y origin of the screen
 *
 * @param self A pointer to the target GBC Graphics object
 * @param new_y The new y origin of the screen
 */
void GBC_Graphics_set_screen_y_origin(GBC_Graphics *self, uint8_t new_y);

/**
 * Sets the width of the screen
 * @note Reducing this parameter will increase FPS
 *
 * @param self A pointer to the target GBC Graphics object
 * @param new_width The new width of the screen
 */
void GBC_Graphics_set_screen_width(GBC_Graphics *self, uint8_t new_width);

/**
 * Sets the height of the screen
 * @note Reducing this parameter will increase FPS
 *
 * @param self A pointer to the target GBC Graphics object
 * @param new_height The new height of the screen
 */
void GBC_Graphics_set_screen_height(GBC_Graphics *self, uint8_t new_height);

/**
 * Gets the boundaries that the screen is rendering within
 *
 * @param self A pointer to the target GBC Graphics object
 *
 * @return The screen's boundaries
 */
GRect GBC_Graphics_get_screen_bounds(GBC_Graphics *self);

/**
 * Gets the x origin of the screen
 *
 * @param self A pointer to the target GBC Graphics object
 *
 * @return The screen's x origin
 */
uint8_t GBC_Graphics_get_screen_x_origin(GBC_Graphics *self);

/**
 * Gets the y origin of the screen
 *
 * @param self A pointer to the target GBC Graphics object
 *
 * @return The screen's y origin
 */
uint8_t GBC_Graphics_get_screen_y_origin(GBC_Graphics *self);

/**
 * Gets the width of the screen
 *
 * @param self A pointer to the target GBC Graphics object
 *
 * @return The screen's width
 */
uint8_t GBC_Graphics_get_screen_width(GBC_Graphics *self);

/**
 * Gets the height of the screen
 *
 * @param self A pointer to the target GBC Graphics object
 *
 * @return The screen's height
 */
uint8_t GBC_Graphics_get_screen_height(GBC_Graphics *self);

/**
 * Moves tiles from one vram bank and position to another
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param src_vram_bank The VRAM bank to draw from, 0 to 3
 * @param src_tile_offset The tile offset to swap from
 * @param dest_vram_bank The VRAM bank to edit, 0 to 3
 * @param dest_tile_offset The tile offset to swap into
 * @param num_tiles_to_move The number of tiles to move
 * @param swap If the src and dest tiles should be swapped, otherwise just copies from dest
 */
void GBC_Graphics_vram_move_tiles(GBC_Graphics *self, uint8_t src_vram_bank, uint8_t src_tile_offset, 
                             uint8_t dest_vram_bank, uint8_t dest_tile_offset, uint8_t num_tiles_to_move, bool swap);

/**
 * Loads tiles from a tilesheet in storage into vram
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param tilesheet_resource The resource ID of the tilesheet to load from
 * @param tilesheet_tile_offset The tile number offset on the tilesheet to load from
 * @param tiles_to_load The number of tiles to load from the tilesheet
 * @param vram_tile_offset The tile number offset on the VRAM to loading into
 * @param vram_bank_number The VRAM bank to load into, 0 to 3
 */
void GBC_Graphics_load_from_tilesheet_into_vram(GBC_Graphics *self, uint32_t tilesheet_resource, uint16_t tilesheet_tile_offset, 
                                            uint16_t tiles_to_load, uint16_t vram_tile_offset, uint8_t vram_bank_number);

/**
 * Returns a pointer to the requested VRAM bank
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param vram_bank_number The VRAM bank to return, 0 to 3
 * @return A VRAM bank
 */
uint8_t *GBC_Graphics_get_vram_bank(GBC_Graphics *self, uint8_t vram_bank_number);

/**
 * Sets the four colors of one of the background palettes
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param palette_num The number of the palette to set, from 0 to 8
 * @param c0 The 0th color of the new palette
 * @param c1 The 1st color of the new palette
 * @param c2 The 2nd color of the new palette
 * @param c3 The 3rd color of the new palette
 */
void GBC_Graphics_set_bg_palette(GBC_Graphics *self, uint8_t palette_num, uint8_t c0, uint8_t c1, uint8_t c2, uint8_t c3);

/**
 * Sets the four colors of one of the background palettes from an array
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param palette_num The number of the palette to set, from 0 to 8
 * @param palette_array A pointer to a 4-byte array containing the palette colors
 */
void GBC_Graphics_set_bg_palette_array(GBC_Graphics *self, uint8_t palette_num, uint8_t *palette_array);

/**
 * Copies the specified bg palette into a target array
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param palette_num The number of the palette to copy
 * @param target_array The array to copy into
 */
void GBC_Graphics_copy_one_bg_palette(GBC_Graphics *self, uint8_t palette_num, uint8_t *target_array);

/**
 * Copies the bg palettes into a target array
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param target_array The array to copy into
 */
void GBC_Graphics_copy_all_bg_palettes(GBC_Graphics *self, uint8_t *target_array);

/**
 * Sets the four colors of one of the sprite palettes
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param palette_num The number of the palette to set, from 0 to 8
 * @param c0 The 0th color of the new palette
 * @param c1 The 1st color of the new palette
 * @param c2 The 2nd color of the new palette
 * @param c3 The 3rd color of the new palette
 */
void GBC_Graphics_set_sprite_palette(GBC_Graphics *self, uint8_t palette_num, uint8_t c0, uint8_t c1, uint8_t c2, uint8_t c3);

/**
 * Sets the four colors of one of the sprite palettes from an array
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param palette_num The number of the palette to set, from 0 to 8
 * @param palette_array A pointer to a 4-byte array containing the palette colors
 */
void GBC_Graphics_set_sprite_palette_array(GBC_Graphics *self, uint8_t palette_num, uint8_t *palette_array);

/**
 * Copies the specified sprite palette into a target array
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param palette_num The number of the palette to copy
 * @param target_array The array to copy into
 */
void GBC_Graphics_copy_one_sprite_palette(GBC_Graphics *self, uint8_t palette_num, uint8_t *target_array);

/**
 * Copies the sprite palettes into a target array
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param target_array The array to copy into
 */
void GBC_Graphics_copy_all_sprite_palettes(GBC_Graphics *self, uint8_t *target_array);

/**
 * Renders the background, window, and sprite layers at the next available opportunity
 * 
 * @param self A pointer to the target GBC Graphics object
 */
void GBC_Graphics_render(GBC_Graphics *self);

/**
 * Sets the LCDC byte
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param new_lcdc The new LCDC byte
 */
void GBC_Graphics_lcdc_set(GBC_Graphics *self, uint8_t new_lcdc);

/**
 * Sets the enable bit of the LCDC byte
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param enabled Should the bit be enabled?
 */
void GBC_Graphics_lcdc_set_enabled(GBC_Graphics *self, bool enabled);

/**
 * Sets the background layer enabled bit of the LCDC byte
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param enabled Should the bit be enabled?
 */
void GBC_Graphics_lcdc_set_bg_layer_enabled(GBC_Graphics *self, bool enabled);

/**
 * Sets the window layer enabled bit of the LCDC byte
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param enabled Should the bit be enabled?
 */
void GBC_Graphics_lcdc_set_window_layer_enabled(GBC_Graphics *self, bool enabled);

/**
 * Sets the sprite layer enabled bit of the LCDC byte
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param enabled Should the bit be enabled?
 */
void GBC_Graphics_lcdc_set_sprite_layer_enabled(GBC_Graphics *self, bool enabled);

/**
 * Sets the sprite mode bit of the LCDC byte
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param use_8x16_sprites Should the system use 8x16 sprites?
 */
void GBC_Graphics_lcdc_set_8x16_sprite_mode_enabled(GBC_Graphics *self, bool use_8x16_sprites);

/**
 * Gets the current line being rendered
 * 
 * @param self A pointer to the target GBC Graphics object
 * 
 * @return The current line being rendered
 */
uint8_t GBC_Graphics_stat_get_current_line(GBC_Graphics *self);

/**
 * Gets the current line to be compared for the interrupt
 * 
 * @param self A pointer to the target GBC Graphics object
 * 
 * @return The current line comparision value
 */
uint8_t GBC_Graphics_stat_get_line_y_compare(GBC_Graphics *self);

/**
 * Checks if the HBlank flag is set
 * 
 * @param self A pointer to the target GBC Graphics object
 * 
 * @return Is the HBlank flag set?
 */
bool GBC_Graphics_stat_check_hblank_flag(GBC_Graphics *self);

/**
 * Checks if the VBlank flag is set
 * 
 * @param self A pointer to the target GBC Graphics object
 * 
 * @return Is the VBlank flag set?
 */
bool GBC_Graphics_stat_check_vblank_flag(GBC_Graphics *self);

/**
 * Checks if the Line Compare flag is set
 * 
 * @param self A pointer to the target GBC Graphics object
 * 
 * @return Is the Line Compare flag set?
 */
bool GBC_Graphics_stat_check_line_comp_flag(GBC_Graphics *self);

/**
 * Checks if the OAM flag is set
 * 
 * @param self A pointer to the target GBC Graphics object
 * 
 * @return Is the OAM flag set?
 */
bool GBC_Graphics_stat_check_oam_flag(GBC_Graphics *self);

/**
 * Sets the STAT byte
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param new_stat The new STAT byte, note that this function will not modify the read only bits
 */
void GBC_Graphics_stat_set(GBC_Graphics *self, uint8_t new_stat);

/**
 * Sets the HBlank interrupt enable bit of STAT
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param enabled Should the HBlank interrupt be enabled?
 */
void GBC_Graphics_stat_set_hblank_interrupt_enabled(GBC_Graphics *self, bool enabled);

/**
 * Sets the VBlank interrupt enable bit of STAT
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param enabled Should the VBlank interrupt be enabled?
 */
void GBC_Graphics_stat_set_vblank_interrupt_enabled(GBC_Graphics *self, bool enabled);

/**
 * Sets the line compare interrupt enable bit of STAT
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param enabled Should the line compare interrupt be enabled?
 */
void GBC_Graphics_stat_set_line_compare_interrupt_enabled(GBC_Graphics *self, bool enabled);

/**
 * Sets the OAM interrupt enable bit of STAT
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param enabled Should the OAM interrupt be enabled?
 */
void GBC_Graphics_stat_set_oam_interrupt_enabled(GBC_Graphics *self, bool enabled);

/**
 * Sets the line to compare for the line compare interrupt
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param new_line_y_comp The new line to compare
 */
void GBC_Graphics_stat_set_line_y_compare(GBC_Graphics *self, uint8_t new_line_y_comp);

/**
 * Sets the callback function for the HBlank interrupt
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param callback The function to call when the HBlank interrupt fires
 */
void GBC_Graphics_set_hblank_interrupt_callback(GBC_Graphics *self, void (*callback)());

/**
 * Sets the callback function for the VBlank interrupt
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param callback The function to call when the VBlank interrupt fires
 */
void GBC_Graphics_set_vblank_interrupt_callback(GBC_Graphics *self, void (*callback)());

/**
 * Sets the callback function for the line compare interrupt
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param callback The function to call when the line compare interrupt fires
 */
void GBC_Graphics_set_line_compare_interrupt_callback(GBC_Graphics *self, void (*callback)());

/**
 * Sets the callback function for the OAM interrupt
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param callback The function to call when the OAM interrupt fires
 */
void GBC_Graphics_set_oam_interrupt_callback(GBC_Graphics *self, void (*callback)());

/** 
 * Helper function for creating an attribute byte
 * 
 * @param palette The palette to use
 * @param vram_bank The VRAM bank to use
 * @param is_x_flipped Should the tile be flipped horizontally?
 * @param is_y_flipped Should the tile be flipped vertically?
 * @param bg_has_priority Does the background have rendering priority?
 * 
 * @return An attribute byte containing the given parameters
 */
uint8_t GBC_Graphics_attr_make(uint8_t palette, uint8_t vram_bank, bool is_x_flipped, bool is_y_flipped, bool bg_has_priority);

/**
 * Extracts the palette number from an attribute byte
 * 
 * @param attributes An attribute byte
 * 
 * @return The corresponding palette number
 */
uint8_t GBC_Graphics_attr_get_palette_num(uint8_t attributes);

/**
 * Extracts the VRAM bank from an attribute byte
 * 
 * @param attributes An attribute byte
 * 
 * @return The corresponding palette number
 */
uint8_t GBC_Graphics_attr_get_vram_bank(uint8_t attributes);

/**
 * Checks if the attribute byte has a horizontal flip flag
 * 
 * @param attributes An attribute byte
 * 
 * @return true if the attribute has a horizontal flip flag
 */
bool GBC_Graphics_attr_is_x_flipped(uint8_t attributes);

/**
 * Checks if the attribute byte has a vertical flip flag
 * 
 * @param attributes An attribute byte
 * 
 * @return true if the attribute has a vertical flip flag
 */
bool GBC_Graphics_attr_is_y_flipped(uint8_t attributes);

/**
 * Gets the current x position of the background scroll
 * 
 * @param self A pointer to the target GBC Graphics object
 * 
 * @return The current x position of the background scroll
 */
uint8_t GBC_Graphics_bg_get_scroll_x(GBC_Graphics *self);

/**
 * Gets the current y position of the background scroll
 * 
 * @param self A pointer to the target GBC Graphics object
 * 
 * @return The current y position of the background scroll
 */
uint8_t GBC_Graphics_bg_get_scroll_y(GBC_Graphics *self);

/**
 * Gets the tile number at the given position on the background
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param x The x position on the background, from 0 to 31
 * @param y The y position on the background, from 0 to 31
 * 
 * @return The tile number
 */
uint8_t GBC_Graphics_bg_get_tile(GBC_Graphics *self, uint8_t x, uint8_t y);

/** 
 * Gets the attributes of the tile at the given position on the background
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param x The x position on the background, from 0 to 31
 * @param y The y position on the background, from 0 to 31
 * 
 * @return The tile attributes
 */
uint8_t GBC_Graphics_bg_get_attr(GBC_Graphics *self, uint8_t x, uint8_t y);

/**
 * Moves the background scroll by dx and dy
 * X and Y will wrap around if they become < 0 or > MAP_SIZE * TILE_SIZE
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param dx The delta x to move the scroll x by
 * @param dy The delta y to move the scroll y by
 */
void GBC_Graphics_bg_move(GBC_Graphics *self, short dx, short dy);

/**
 * Sets the background scroll x
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param x The new scroll x
 */
void GBC_Graphics_bg_set_scroll_x(GBC_Graphics *self, uint8_t x);

/**
 * Sets the background scroll y
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param y The new scroll y
 */
void GBC_Graphics_bg_set_scroll_y(GBC_Graphics *self, uint8_t y);

/**
 * Sets the background scroll x and scroll y
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param x The new scroll x
 * @param y The new scroll y
 */
void GBC_Graphics_bg_set_scroll_pos(GBC_Graphics *self, uint8_t x, uint8_t y);

/**
 * Sets the tile number at the given position on the background
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param x The x position on the background, from 0 to 31
 * @param y The y position on the background, from 0 to 31
 * @param tile_number The new tile number
 */
void GBC_Graphics_bg_set_tile(GBC_Graphics *self, uint8_t x, uint8_t y, uint8_t tile_number);

/**
 * Sets the tile attributes at the given position on the background
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param x The x position on the background, from 0 to 31
 * @param y The y position on the background, from 0 to 31
 * @param attributes The new attributes
 */
void GBC_Graphics_bg_set_attrs(GBC_Graphics *self, uint8_t x, uint8_t y, uint8_t attributes);

/**
 * Sets both the tile number and tile attributes at the given position on the background
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param x The x position on the background, from 0 to 31
 * @param y The y position on the background, from 0 to 31
 * @param tile_number The new tile number
 * @param attributes The new tile number
 */
void GBC_Graphics_bg_set_tile_and_attrs(GBC_Graphics *self, uint8_t x, uint8_t y, uint8_t tile_number, uint8_t attributes);

/**
 * Sets the palette of the tile at the given position on the background
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param x The x position on the background, from 0 to 31
 * @param y The y position on the background, from 0 to 31
 * @param palette The new palette, from 0 to 7
 */
void GBC_Graphics_bg_set_tile_palette(GBC_Graphics *self, uint8_t x, uint8_t y, uint8_t palette);

/**
 * Sets the VRAM bank of the tile at the given position on the background
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param x The x position on the background, from 0 to 31
 * @param y The y position on the background, from 0 to 31
 * @param vram_bank The new VRAM bank, from 0 to 3
 */
void GBC_Graphics_bg_set_tile_vram_bank(GBC_Graphics *self, uint8_t x, uint8_t y, uint8_t vram_bank);

/**
 * Sets if the tile at the given position on the background should be flipped horizontally
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param x The x position on the background, from 0 to 31
 * @param y The y position on the background, from 0 to 31
 * @param flipped If the tile should be flipped horizontally
 */
void GBC_Graphics_bg_set_tile_x_flip(GBC_Graphics *self, uint8_t x, uint8_t y, bool flipped);

/**
 * Sets if the tile at the given position on the background should be flipped vertically
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param x The x position on the background, from 0 to 31
 * @param y The y position on the background, from 0 to 31
 * @param flipped If the tile should be flipped vertically
 */
void GBC_Graphics_bg_set_tile_y_flip(GBC_Graphics *self, uint8_t x, uint8_t y, bool flipped);

/**
 * Sets if the tile at the given position on the background should have priority over the sprite layer
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param x The x position on the background, from 0 to 31
 * @param y The y position on the background, from 0 to 31
 * @param has_priority If the tile should have priority over the sprite layer
 */
void GBC_Graphics_bg_set_tile_priority(GBC_Graphics *self, uint8_t x, uint8_t y, bool has_priority);

/**
 * Moves a tile from one location to another
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param src_x The x position of the source tile, 0 to 31
 * @param src_y The y position of the source tile, 0 to 31
 * @param dest_x The x position of the destination tile, 0 to 31
 * @param dest_y The y position of the destination tile, 0 to 31
 * @param swap If the src and dest tiles should be swapped, otherwise just copies from dest
 */
void GBC_Graphics_bg_move_tile(GBC_Graphics *self, uint8_t src_x, uint8_t src_y, uint8_t dest_x, uint8_t dest_y, bool swap);

/**
 * Gets the current x position of the window offset
 * 
 * @param self A pointer to the target GBC Graphics object
 * 
 * @return The current x position of the window offset
 */
uint8_t GBC_Graphics_window_get_offset_x(GBC_Graphics *self);

/**
 * Gets the current y position of the window offset
 * 
 * @param self A pointer to the target GBC Graphics object
 * 
 * @return The current y position of the window offset
 */
uint8_t GBC_Graphics_window_get_offset_y(GBC_Graphics *self);

/**
 * Gets the tile number at the given position on the window
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param x The x position on the background, from 0 to 31
 * @param y The y position on the background, from 0 to 31
 * 
 * @return The tile number
 */
uint8_t GBC_Graphics_window_get_tile(GBC_Graphics *self, uint8_t x, uint8_t y);

/** 
 * Gets the attributes of the tile at the given position on the window
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param x The x position on the background, from 0 to 31
 * @param y The y position on the background, from 0 to 31
 * 
 * @return The tile attributes
 */
uint8_t GBC_Graphics_window_get_attr(GBC_Graphics *self, uint8_t x, uint8_t y);

/**
 * Moves the window offset by dx and dy
 * Clamps x from 0->SCREEN_WIDTH, y from 0->SCREEN_HEIGHT
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param dx The delta x to move the offset x by
 * @param dy The delta y to move the offset y by
 */
void GBC_Graphics_window_move(GBC_Graphics *self, short dx, short dy);

/**
 * Sets the window offset x
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param x The new offset x, x > 144 will hide the window layer
 */
void GBC_Graphics_window_set_offset_x(GBC_Graphics *self, uint8_t x);

/**
 * Sets the window offset y
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param y The new offset y, y > 168 will hide the window layer
 */
void GBC_Graphics_window_set_offset_y(GBC_Graphics *self, uint8_t y);

/**
 * Sets the window offset x and offset y
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param x The new offset x, x > 144 will hide the window layer
 * @param y The new offset y, y > 168 will hide the window layer
 */
void GBC_Graphics_window_set_offset_pos(GBC_Graphics *self, uint8_t x, uint8_t y);

/**
 * Sets the tile number at the given position on the window
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param x The x position on the window, from 0 to 31
 * @param y The y position on the window, from 0 to 31
 * @param tile_number The new tile number
 */
void GBC_Graphics_window_set_tile(GBC_Graphics *self, uint8_t x, uint8_t y, uint8_t tile_number);

/**
 * Sets the tile attributes at the given position on the window
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param x The x position on the window, from 0 to 31
 * @param y The y position on the window, from 0 to 31
 * @param attributes The new attributes
 */
void GBC_Graphics_window_set_attrs(GBC_Graphics *self, uint8_t x, uint8_t y, uint8_t attributes);

/**
 * Sets both the tile number and tile attributes at the given position on the window
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param x The x position on the window, from 0 to 31
 * @param y The y position on the window, from 0 to 31
 * @param tile_number The new tile number
 * @param attributes The new tile number
 */
void GBC_Graphics_window_set_tile_and_attrs(GBC_Graphics *self, uint8_t x, uint8_t y, uint8_t tile_number, uint8_t attributes);

/**
 * Sets the palette of the tile at the given position on the window
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param x The x position on the window, from 0 to 31
 * @param y The y position on the window, from 0 to 31
 * @param palette The new palette, from 0 to 7
 */
void GBC_Graphics_window_set_tile_palette(GBC_Graphics *self, uint8_t x, uint8_t y, uint8_t palette);

/**
 * Sets the VRAM bank of the tile at the given position on the window
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param x The x position on the window, from 0 to 31
 * @param y The y position on the window, from 0 to 31
 * @param vram_bank The new VRAM bank, from 0 to 3
 */
void GBC_Graphics_window_set_tile_vram_bank(GBC_Graphics *self, uint8_t x, uint8_t y, uint8_t vram_bank);

/**
 * Sets if the tile at the given position on the window should be flipped horizontally
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param x The x position on the window, from 0 to 31
 * @param y The y position on the window, from 0 to 31
 * @param flipped If the tile should be flipped horizontally
 */
void GBC_Graphics_window_set_tile_x_flip(GBC_Graphics *self, uint8_t x, uint8_t y, bool flipped);

/**
 * Sets if the tile at the given position on the window should be flipped vertically
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param x The x position on the window, from 0 to 31
 * @param y The y position on the window, from 0 to 31
 * @param flipped If the tile should be flipped vertically
 */
void GBC_Graphics_window_set_tile_y_flip(GBC_Graphics *self, uint8_t x, uint8_t y, bool flipped);

/**
 * Sets if the tile at the given position on the window should have priority over the sprite layer
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param x The x position on the window, from 0 to 31
 * @param y The y position on the window, from 0 to 31
 * @param has_priority If the tile should have priority over the sprite layer
 */
void GBC_Graphics_window_set_tile_priority(GBC_Graphics *self, uint8_t x, uint8_t y, bool has_priority);

/**
 * Moves a tile from one location to another
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param src_x The x position of the source tile, 0 to 31
 * @param src_y The y position of the source tile, 0 to 31
 * @param dest_x The x position of the destination tile, 0 to 31
 * @param dest_y The y position of the destination tile, 0 to 31
 * @param swap If the src and dest tiles should be swapped, otherwise just copies from dest
 */
void GBC_Graphics_window_move_tile(GBC_Graphics *self, uint8_t src_x, uint8_t src_y, uint8_t dest_x, uint8_t dest_y, bool swap);

/**
 * Gets the x position of the sprite
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param sprite_num The sprite's position in OAM
 * 
 * @return The sprite's x position
 */
uint8_t GBC_Graphics_oam_get_sprite_x(GBC_Graphics *self, uint8_t sprite_num);

/**
 * Gets the y position of the sprite
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param sprite_num The sprite's position in OAM
 * 
 * @return The sprite's y position
 */
uint8_t GBC_Graphics_oam_get_sprite_y(GBC_Graphics *self, uint8_t sprite_num);

/**
 * Gets the tile of the sprite
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param sprite_num The sprite's position in OAM
 * 
 * @return The sprite's tile
 */
uint8_t GBC_Graphics_oam_get_sprite_tile(GBC_Graphics *self, uint8_t sprite_num);

/**
 * Gets the attributes of the sprite
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param sprite_num The sprite's position in OAM
 * 
 * @return The sprite's attributes
 */
uint8_t GBC_Graphics_oam_get_sprite_attrs(GBC_Graphics *self, uint8_t sprite_num);

/**
 * Creates and sets a sprite in OAM with the given values
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param sprite_num The sprite's position in OAM
 * @param x The sprite's x position
 * @param y The sprite's y position
 * @param tile_position The tile position in VRAM that the sprite will use to render
 * @param attributes The sprite's attributes
 */
void GBC_Graphics_oam_set_sprite(GBC_Graphics *self, uint8_t sprite_num, uint8_t x, uint8_t y, uint8_t tile_position, uint8_t attributes);

/**
 * Moves a sprite on the OAM by dx and dy
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param sprite_num The sprite's position in OAM
 * @param dx The delta x to move the sprite by
 * @param dy The delta y to move the sprite by
 */
void GBC_Graphics_oam_move_sprite(GBC_Graphics *self, uint8_t sprite_num, short dx, short dy);

/**
 * Sets the sprite's x position
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param sprite_num The sprite's position in OAM
 * @param x The x position to move the sprite to
 */
void GBC_Graphics_oam_set_sprite_x(GBC_Graphics *self, uint8_t sprite_num, uint8_t x);

/**
 * Sets the sprite's y position
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param sprite_num The sprite's position in OAM
 * @param y The y position to move the sprite to
 */
void GBC_Graphics_oam_set_sprite_y(GBC_Graphics *self, uint8_t sprite_num, uint8_t y);

/**
 * Sets the sprite's x and y position
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param sprite_num The sprite's position in OAM
 * @param x The x position to move the sprite to
 * @param y The y position to move the sprite to
 */
void GBC_Graphics_oam_set_sprite_pos(GBC_Graphics *self, uint8_t sprite_num, uint8_t x, uint8_t y);

/**
 * Hides a sprite by moving it off-screen
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param sprite_num The sprite's position in OAM
 */
void GBC_Graphics_oam_hide_sprite(GBC_Graphics *self, uint8_t sprite_num);

/**
 * Sets the tile position in VRAM that the sprite will use to render
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param sprite_num The sprite's position in OAM
 * @param tile_position The new tile position
 */
void GBC_Graphics_oam_set_sprite_tile(GBC_Graphics *self, uint8_t sprite_num, uint8_t tile_position);

/**
 * Sets the tile position in VRAM that the sprite will use to render
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param sprite_num The sprite's position in OAM
 * @param tile_position The new tile position
 */
void GBC_Graphics_oam_set_sprite_attrs(GBC_Graphics *self, uint8_t sprite_num, uint8_t attributes);

/**
 * Sets the palette to be used by the sprite
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param sprite_num The sprite's position in OAM
 * @param palette The number of the palette to use, 0-7
 */
void GBC_Graphics_oam_set_sprite_palette(GBC_Graphics *self, uint8_t sprite_num, uint8_t palette);

/**
 * Sets the VRAM bank to be used by the sprite
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param sprite_num The sprite's position in OAM
 * @param vram_bank The VRAM bank to use, 0-3
 */
void GBC_Graphics_oam_set_sprite_vram_bank(GBC_Graphics *self, uint8_t sprite_num, uint8_t vram_bank);

/**
 * Sets the sprite's horizontal flip bit
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param sprite_num The sprite's position in OAM
 * @param flipped Should the sprite be flipped horizontally?
 */
void GBC_Graphics_oam_set_sprite_x_flip(GBC_Graphics *self, uint8_t sprite_num, bool flipped);

/**
 * Sets the sprite's vertical flip bit
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param sprite_num The sprite's position in OAM
 * @param flipped Should the sprite be flipped vertical?
 */
void GBC_Graphics_oam_set_sprite_y_flip(GBC_Graphics *self, uint8_t sprite_num, bool flipped);

/**
 * Sets the sprite's priority bit
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param sprite_num The sprite's position in OAM
 * @param flipped Does the bg have priority over the Sprite?
 */
void GBC_Graphics_oam_set_sprite_priority(GBC_Graphics *self, uint8_t sprite_num, bool bg_has_priority);

/**
 * Moves a sprite from one position in OAM to another
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param source_sprite_num The OAM location to move from
 * @param target_sprite_num The OAM location to move to
 * @param copy Should the sprite be copied (preserve source) or moved (delete source)?
 */
void GBC_Graphics_oam_change_sprite_num(GBC_Graphics *self, uint8_t source_sprite_num, uint8_t target_sprite_num, bool copy);

/**
 * Swaps two sprites in OAM
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param sprite_num_1 The first sprite to swap
 * @param sprite_num_2 The second sprite to swap
 */
void GBC_Graphics_oam_swap_sprites(GBC_Graphics *self, uint8_t sprite_num_1, uint8_t sprite_num_2);

/**
 * Swaps the tiles of two sprites
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param sprite_num_1 The first sprite to swap
 * @param sprite_num_2 The second sprite to swap
 */
void GBC_Graphics_oam_swap_sprite_tiles(GBC_Graphics *self, uint8_t sprite_num_1, uint8_t sprite_num_2);

/**
 * Swaps the attributes of two sprites
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param sprite_num_1 The first sprite to swap
 * @param sprite_num_2 The second sprite to swap
 */
void GBC_Graphics_oam_swap_sprite_attrs(GBC_Graphics *self, uint8_t sprite_num_1, uint8_t sprite_num_2);

/**
 * Swaps the tiles and attributes of two sprites
 * 
 * @param self A pointer to the target GBC Graphics object
 * @param sprite_num_1 The first sprite to swap
 * @param sprite_num_2 The second sprite to swap
 */
void GBC_Graphics_oam_swap_sprite_tiles_and_attrs(GBC_Graphics *self, uint8_t sprite_num_1, uint8_t sprite_num_2);

/**
 * Copies the data in the background tilemap and attrmap to the
 * window tilemap and attrmap
 * 
 * @param self A pointer to the target GBC Graphics object
 */
void GBC_Graphics_copy_background_to_window(GBC_Graphics *self);

/**
 * Copies the data in the window tilemap and attrmap to the
 * background tilemap and attrmap
 * 
 * @param self A pointer to the target GBC Graphics object
 */
void GBC_Graphics_copy_window_to_background(GBC_Graphics *self);