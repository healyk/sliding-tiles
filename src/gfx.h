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
gfx_blit(texture_t* texture, rect_t* src_area, rect_t* dest_area);

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

#endif
