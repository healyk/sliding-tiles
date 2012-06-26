/**
   @file geo.h
   
   Simple geometry structures and routines.
*/
#ifndef GEO_H
#define GEO_H

#include <stdint.h>

/**
   Models a rectangle.
*/
typedef struct rect {
  int x;
  int y;

  int width;
  int height;
} rect_t;

/**
   Models a color.
*/
typedef struct color {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  uint8_t alpha;
} color_t;

/**
   Simple setter for a rectangle.
*/
void 
rect_set(rect_t* rect, int x, int y, int width, int height);

#endif
