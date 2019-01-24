#include "asl_util.h"

#include <proto/dos.h>

#include <stdlib.h>
#include <string.h>

FILE *openAsl(const char *dir, const char *file, const char *mode) {
  FILE * fp;
  size_t bufferLen = strlen(dir) + strlen(file) + 2;
  char *buffer = malloc(bufferLen);

  if(!buffer) {
    fprintf(
      stderr,
      "openAsl: failed to allocate buffer "
      "(dir: %s) (file: %s)\n",
      dir  ? dir  : "NULL",
      file ? file : "NULL");
    goto error;
  }

  strcpy(buffer, dir);
  if(!AddPart(buffer, (char*)file, (ULONG)bufferLen)) {
    fprintf(
      stderr,
      "openAsl: failed to add part "
      "(buffer: %s) (file: %s) (len: %d)\n",
      buffer ? buffer : "NULL",
      file   ? file   : "NULL",
      bufferLen);
    goto error_freeBuffer;
  }

  fp = fopen(buffer, mode);
  if(!fp) {
    fprintf(stderr, "openAsl: failed to open file\n");
    goto error_freeBuffer;
  }

  free(buffer);
  return fp;

error_freeBuffer:
  free(buffer);
error:
  return NULL;
}
