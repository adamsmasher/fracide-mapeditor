#include "asl_util.h"

#include <proto/dos.h>

#include <stdlib.h>
#include <string.h>

char *aslFilename(const char *dir, const char *file) {
  size_t bufferLen = strlen(dir) + strlen(file) + 2;
  char *buffer = malloc(bufferLen);

  if(!buffer) {
    fprintf(
      stderr,
      "aslFilename: failed to allocate buffer "
      "(dir: %s) (file: %s)\n",
      dir  ? dir  : "NULL",
      file ? file : "NULL");
    goto error;
  }

  strcpy(buffer, dir);
  if(!AddPart(buffer, (char*)file, (ULONG)bufferLen)) {
    fprintf(
      stderr,
      "aslFilename: failed to add part "
      "(buffer: %s) (file: %s) (len: %d)\n",
      buffer ? buffer : "NULL",
      file   ? file   : "NULL",
      bufferLen);
    goto error_freeBuffer;
  }

  return buffer;

error_freeBuffer:
  free(buffer);
error:
  return NULL;
}

FILE *openAsl(const char *dir, const char *file, const char *mode) {
  FILE *fp;

  char *filename = aslFilename(dir, file);
  if(!filename) {
    fprintf(stderr, "openAsl: couldn't make filename\n");
    goto error;
  }

  fp = fopen(filename, mode);
  if(!fp) {
    fprintf(stderr, "openAsl: failed to open file\n");
    goto error_freeFilename;
  }

  free(filename);
  return fp;

error_freeFilename:
  free(filename);
error:
  return NULL;
}
