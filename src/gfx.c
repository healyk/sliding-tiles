#include <string.h>

#include <GL/glfw.h>
#include "soil/SOIL.h"

#include "util.h"
#include "gfx.h"

static struct {
  bool inited;
  int  screen_width;
  int  screen_height;

  texture_t* tex_list_head;
} gfx_context = {
  false,
  0,
  0,
  NULL
};

bool
gfx_init(const char* title, int width, int height) {
  bool result = true;

  if(!gfx_context.inited) {
    if(!glfwInit()) {
      logmsg("Unable to initialize graphics systems.");
      result = false;
    } else {
      result = glfwOpenWindow(width, height, 8, 8, 8, 8, 8, 8, GLFW_WINDOW)
        == GL_TRUE;
      
      if(result) {
        glfwSetWindowTitle(title);

        gfx_context.inited = true;
        gfx_context.screen_width = width;
        gfx_context.screen_height = height;
      } else {
        glfwTerminate();
      }
    }
  }

  return result;
}

void
gfx_shutdown(void) {
  texture_t* list_tex;

  if(gfx_context.inited) {
    glfwTerminate();
    gfx_context.inited = false;

    // Delete interned textures.
    list_tex = gfx_context.tex_list_head;
    while(list_tex != NULL) {
      texture_t* temp = list_tex;
      list_tex = list_tex->gfx_tex_next;

      texture_delete(temp);
    }
  }
}

/*
  More info and base source:
  http://www.gamedev.net/page/resources/_/technical/opengl/rendering-efficient-2d-sprites-in-opengl-using-r2429
*/
void gfx_begin_2d(void) {
  GLint viewport[4];

  // Get a copy of the viewport
  glGetIntegerv(GL_VIEWPORT, viewport);
  
  // Save a copy of the projection matrix so that we can restore it 
  // when it's time to do 3D rendering again.
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  
  // Set up the orthographic projection
  glOrtho(viewport[0],  viewport[0] + viewport[2],
          viewport[1] + viewport[3], viewport[1], 
          -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  
  // Make sure depth testing and lighting are disabled for 2D rendering until
  // we are finished rendering in 2D
  glPushAttrib(GL_DEPTH_BUFFER_BIT | GL_LIGHTING_BIT);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
}

void gfx_end_2d(void) {
  glPopAttrib();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}

//==============================================================================
// Texture
//==============================================================================

texture_t* 
texture_load(const char* filename, bool intern) {
  texture_t* texture = NULL;
  GLuint     id;

  id = SOIL_load_OGL_texture(filename,
                             SOIL_LOAD_RGBA,
                             SOIL_CREATE_NEW_ID,
                             SOIL_FLAG_POWER_OF_TWO |
                             SOIL_FLAG_TEXTURE_REPEATS |
                             SOIL_FLAG_COMPRESS_TO_DXT);

  if(0 == id) {
    logmsg("Unable to load file %s into opengl texture.  Error: %s", filename,
           SOIL_last_result());
  } else {
    texture = new(texture_t);

    texture->id = id;

    glBindTexture(GL_TEXTURE_2D, id);
    glGetTexLevelParameteriv(GL_TEXTURE_2D,
                             0,
                             GL_TEXTURE_WIDTH,
                             &texture->width);

    glGetTexLevelParameteriv(GL_TEXTURE_2D,
                             0,
                             GL_TEXTURE_HEIGHT,
                             &texture->height);

    if(intern) {
      texture->gfx_tex_next = gfx_context.tex_list_head;
      gfx_context.tex_list_head = texture;
    }

    logmsg("Loaded texture %s into id %u.", filename, texture->id);
  }
  
  return texture;
}

void
texture_delete(texture_t* texture) {
  logmsg("Deleting texture %d", texture->id);

  glDeleteTextures(1, &texture->id);
  delete(texture);
}

static void
draw_quad(texture_t* texture, rect_t* src_area, rect_t* dest_area) {
  float start_x, start_y, end_x, end_y;

  start_x = src_area->x / (texture->width * 1.0f);
  start_y = src_area->y / (texture->height * 1.0f);

  end_x = (src_area->x + src_area->width) / (texture->width * 1.0f);
  end_y = (src_area->y + src_area->height) / (texture->height * 1.0f);

  glBindTexture(GL_TEXTURE_2D, texture->id);

  // Initialize GL
  glEnable(GL_TEXTURE_2D);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
 
  glBegin(GL_QUADS); 
  {
    glColor4ub(255, 255, 255, 255);
    glTexCoord2f(start_x, start_y);
    glVertex2i(dest_area->x, dest_area->y);

    glTexCoord2f(end_x, start_y);
    glVertex2i(dest_area->x + dest_area->width, dest_area->y);

    glTexCoord2f(end_x, end_y);
    glVertex2i(dest_area->x + dest_area->width, 
               dest_area->y + dest_area->height);

    glTexCoord2f(start_x, end_y);
    glVertex2i(dest_area->x, dest_area->y + dest_area->height);
  }
  glEnd();

  // Disable the texture
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
}

void
gfx_blit(texture_t* texture, rect_t* src_area, rect_t* dest_area) {
  rect_t true_src;

  // Check the source
  if(NULL == src_area) {
    rect_set(&true_src, 0, 0, texture->width, texture->height);
  } else {
    memcpy(&true_src, src_area, sizeof(rect_t));
  }

  // Check the dest
  if(dest_area->width <= 0) {
    dest_area->width = texture->width;
  }

  if(dest_area->height <= 0) {
    dest_area->height = texture->height;
  }

  // Draw the image to a quad.
  draw_quad(texture, &true_src, dest_area);
}

//==============================================================================
// Primitives
//==============================================================================

void
gfx_draw_rect(rect_t* rect, color_t* color, bool filled) {
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  if(filled) {
    glBegin(GL_QUADS);
  } else {
    glBegin(GL_LINE_LOOP);
  }

  {
    glColor4ub(color->red, color->green, color->blue, color->alpha);
    glVertex2i(rect->x,               rect->y);

    glColor4ub(color->red, color->green, color->blue, color->alpha);
    glVertex2i(rect->x + rect->width, rect->y);

    glColor4ub(color->red, color->green, color->blue, color->alpha);
    glVertex2i(rect->x + rect->width, rect->y + rect->height);

    glColor4ub(color->red, color->green, color->blue, color->alpha);
    glVertex2i(rect->x,               rect->y + rect->height);
  }

  glEnd();

  glDisable(GL_BLEND);
}

//==============================================================================
// Sprites
//==============================================================================

sprite_sheet_t*
sprite_sheet_new(texture_t* texture, int sprite_width, int sprite_height) {
  sprite_sheet_t* sheet;

  sheet = new(sprite_sheet_t);
  sheet->sprites = new_array(sprite_t, sprite_width * sprite_height);

  sheet->sprite_width = sprite_width;
  sheet->sprite_height = sprite_height;

  sheet->width = texture->width / sprite_width;
  sheet->height = texture->height / sprite_height;

  for(int x = 0; x < sheet->width; x++) {
    for(int y = 0; y < sheet->height; y++) {
      sprite_t* sprite = sprite_sheet_get_sprite(sheet, x, y);
      sprite->texture = texture;
      rect_set(&sprite->area, x * sprite_width, y * sprite_height,
               sprite_width, sprite_height);
    }
  }

  return sheet;
}

void
sprite_sheet_delete(sprite_sheet_t* sheet) {
  delete(sheet->sprites);
  delete(sheet);
}

sprite_t*
sprite_sheet_get_sprite(sprite_sheet_t* sheet, int x, int y) {
  return sheet->sprites + (x + (y * sheet->width));
}

void
sprite_render(sprite_t* sprite, rect_t* dest) {
  gfx_blit(sprite->texture, &sprite->area, dest);
}
