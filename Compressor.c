#include "Compressor.h"

#include <stdio.h>

typedef struct Match_tag {
  UBYTE length;
  UBYTE delta;
} Match;

static BOOL find_longest_match(UBYTE *data, int data_size, int i, Match *longest_match) {
  UBYTE match_len;
  UBYTE j;
  BOOL found_longest_match = FALSE;

  /* walk backwards from the read head */
  for (j = 1; i - j >= 0; j++) {
    /* find the biggest match from this starting point */
    for(match_len = 0;
        i + match_len < data_size &&
        data[i + match_len] == data[i + match_len - j];
        match_len++) {
      if (match_len == 255) {
        break;
      }
    }

    /* update it if this is the best */
    if ((found_longest_match && match_len > longest_match->length) ||
        (!found_longest_match && match_len > 2)) {
      found_longest_match = TRUE;
      longest_match->delta = j; 
      longest_match->length = match_len;
    }

    if (j == 255) {
      break;
    }
  }

  return found_longest_match;
}

void write_flag_byte(UBYTE flag_byte, FILE *fp, long last_flag_pos) {
  long current_write_pos;

  /* backup the write position */
  current_write_pos = ftell(fp);

  fseek(fp, last_flag_pos, SEEK_SET);
  fputc(flag_byte, fp);

  /* return to the write position */
  fseek(fp, current_write_pos, SEEK_SET);
}

int compress(UBYTE *data, int size, FILE *fp) {
  UBYTE flag_byte;
  int i;
  int command_cnt;
  long last_flag_pos;
  int bytes_written = 0;
  Match longest_match;

  flag_byte = 0;
  last_flag_pos = -1;
  for (i = command_cnt = 0; i < size;) {
    /* check if we need to reserve a flag byte */
    if (command_cnt % 8 == 0) {
      last_flag_pos = ftell(fp);
      /* just write a garbage byte to reserve a slot for the flags */
      fputc(0xAA, fp);
      bytes_written++;
      flag_byte = 0;
      command_cnt = 0;
    }

    /* write out the next command */
    if(find_longest_match(data, size, i, &longest_match)) {
      fputc(longest_match.length, fp);
      fputc(longest_match.delta ^ 0xFF + 1, fp);
      i += longest_match.length;
      bytes_written += 2;
    } else {
      fputc(data[i], fp);
      bytes_written++;
      i++;
      /* raise the literal flag */
      flag_byte = (UBYTE)(flag_byte | (1 << command_cnt));
    }

    /* check if we need to write a flag byte */
    command_cnt++;
    if (command_cnt == 8) {
      write_flag_byte(flag_byte, fp, last_flag_pos);
    }
  }
  /* if we just wrote a flag byte, we need to write another one */
  if (command_cnt % 8 == 0) {
    fputc(0, fp);
    bytes_written++;
  } else {
    /* otherwise we need to write this one properly */
    write_flag_byte(flag_byte, fp, last_flag_pos);
  }

  /* write an EOF marker */
  fputc(0, fp);
  bytes_written++;

  return bytes_written;
}
