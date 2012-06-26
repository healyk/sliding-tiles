/**
   @file util.h
   
   Various utility functions.
*/
#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>

/**
   Initializes the game log.
*/
void log_init(const char* filename);

/**
   Logs a message to the message log.
*/
void logmsg(char* fmt, ...);

/**
   Cleans up a pointer to the passed in memory.
   
   @param mem
     Pointer to clean up.
*/
void _delete(void* mem);

/**
   Allocates a new chunk of memory.
   
   @param element_size
     Size of the element to allocate
   @param element_count
     Number of elements to allocate.
   @param typename
     Name of the type being allocated.

   @return
     A new pointer or NULL if an error occured.
*/
void* _new(size_t element_size,
           size_t element_count, 
           const char* typename);

/**
   Allocates memory on the heap for type.
*/
#define new(type)                               \
  ((type*)_new(sizeof(type), 1, #type))

#define new_array(type, size)                   \
  ((type*)_new(sizeof(type), size, #type))

/**
   Frees memory on the heap for the variable.
*/
#define delete(var) \
  (_delete(var))

#endif
