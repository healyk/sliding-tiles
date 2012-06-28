#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "util.h"

static FILE* log_handle = NULL;

static void
log_shutdown(void) {
  if(log_handle != NULL) {
    fclose(log_handle);
  }
}

void 
log_init(const char* filename) {
  log_handle = fopen(filename, "wt");
  atexit(log_shutdown);

  logmsg("Initializing log file.");  
}

void
logmsg(char* fmt, ...) {
  FILE* stream = stderr;
  va_list args;

  if(NULL != log_handle) {
    stream = log_handle;
  }

  va_start(args, fmt);
  vfprintf(stream, fmt, args);
  fprintf(stream, "\n");
  va_end(args);
}

void 
_delete(void* mem) {
  if(mem != NULL) {
    free(mem);
  }
}

void* 
_new(size_t element_size, size_t element_count, const char* typename) {
  void* pointer = NULL;

  pointer = calloc(element_count, element_size);
  if(NULL == pointer) {
    fprintf(stderr, "Unable to allocate %zu bytes, %zu elements for variable of"
            " type %s.\n", element_size, element_count, typename);
    fprintf(stderr, "errno: %d, %s\n", errno, strerror(errno));
    fflush(stderr);

    exit(1);
  }

  return pointer;
}

int rand_int(int min, int max) {
  return (rand() % (max - min)) + min;
}

int min(int a, int b) { 
  return a < b ? a : b; 
}

int max(int a, int b) {
  return a > b ? a : b; 
}
