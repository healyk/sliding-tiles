/**
   @file gfx.h

   Includes common graphical routines and image handling.  This is also 
   responsible for initializing the underlying graphics sub-system (and some
   other subsystems).
*/
#ifndef GFX_H
#define GFX_H

#include <GL/glfw.h>
#include <stdbool.h>

#include "geo.h"

/**
   Initializes the graphics system.

   @return
     False on failure the intialize.  True on success.
*/
bool gfx_init(const char* title, int width, int height);

/**
   Shuts down the graphics system and cleans everything up.
*/
void gfx_shutdown(void);

/**
   Starts 2d graphics rendering.
*/
void gfx_begin_2d(void);

/**
   Ends 2d rendering.
*/
void gfx_end_2d(void);

//==============================================================================
// Color
//==============================================================================

/**
   Models a 32-bit color.  Each value is modeled as a byte for simplicity.
*/
typedef struct color {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  uint8_t alpha;
} color_t;

// Useful constant colors
extern const color_t COLOR_WHITE;
extern const color_t COLOR_BLACK;

//==============================================================================
// Texture
//==============================================================================

/**
   Models the data associated with an opengl texture.
*/
typedef struct texture {
  /** Opengl texture id used when rendering this texture. */
  GLuint id;
  
  /** Texture width in pixels. */
  int    width;

  /** Texture height in pixels */
  int    height;

  /** Used internally to list textures. */
  struct texture* gfx_tex_next;
} texture_t;

/**
   Loads an image from a file into an opengl texture.

   @param filename
     Filename of the texture.
   @param intern
     Interned images will be cleaned up on close of the application.
   @return
     A new texture or NULL if the file could not be loaded.
*/
texture_t* texture_load(const char* filename, bool intern);

/**
   Will clean up a texture in memory.
*/
void texture_delete(texture_t* texture);

/**
   Draws a texture to the screen.
*/
void 
gfx_blit(texture_t* texture, 
         rect_t* src_area, 
         rect_t* dest_area, 
         color_t* color);

//==============================================================================
// Primitives
//==============================================================================
/**
   Draws a rectangle to the screen.

   @param rect
     Coordinates to draw the rectangle to.
   @param color
     Color of the rectangle.
   @param filled
     If true the rectangle will be filled.  If false it will only render the
     rectangle's lines.
*/
void
gfx_draw_rect(rect_t* rect, color_t* color, bool filled);

//==============================================================================
// Sprites
//==============================================================================

typedef struct sprite {
  texture_t* texture;
  rect_t     area;
} sprite_t;

typedef struct sprite_sheet {
  sprite_t* sprites;

  int       sprite_width;
  int       sprite_height;

  /** Holds the number of sprites wide */
  int       width;
  /** Holds the number of sprites high */
  int       height;
} sprite_sheet_t;

/**
   Creates a new sprite sheet from a texture.
*/
sprite_sheet_t*
sprite_sheet_new(texture_t* texture, int sprite_width, int sprite_height);

/**
   Cleans up a sprite sheet.
*/
void
sprite_sheet_delete(sprite_sheet_t* sheet);

/**
   Gets an individual sprite from a sprite sheet.

   @return
     The sprite at (x, y) or NULL if (x, y) is out of bounds.
*/
sprite_t*
sprite_sheet_get_sprite(sprite_sheet_t* sheet, int x, int y);

/**
   Draws a sprite to the screen.
*/
void
sprite_render(sprite_t* sprite, rect_t* dest, color_t* color);

//==============================================================================
// Fonts
//==============================================================================

/**
   Models a set of characters that can be rendered to the screen.
*/
typedef struct font {
  sprite_sheet_t* sheet;
  point_t*        mappings;
  size_t          mapping_count;
} font_t;

/**
   Creates a new font from a texture.
   
   @param texture
     Texture holding the font glyphs.
   @param width
     Width of each character in pixels
   @param height
     Height of each character in pixels.
*/
font_t*
font_new(texture_t* texture, int width, int height);

/**
   Cleans up a newly allocated font.
*/
void
font_delete(font_t* font);

/**
   Renders a character to the screen. 
*/
void
font_render_char(font_t* font, int x, int y, int character, color_t* color);

/**
   Renders a string to the screen.
*/
void
font_render_string(font_t* font, int x, int y, char* str, color_t* color);

#endif
