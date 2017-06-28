/*
 * Filename:  scanner.c
 *
 * Description:  
 *
 * Copyright (c) 2016 Erwann Miriel, erwann.miriel@gmail.com 
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <malloc.h>

#include "include/stringbuilder.h"
#include "include/utils.h"
#include "include/logging.h"

static int DEFAULT_CAPACITY = 500;

_stringbuilder_t * sb_init() {
  _stringbuilder_t * sb = malloc(sizeof(_stringbuilder_t));
  if(sb == NULL) {
    goto error;
  }
  sb->string = malloc(sizeof(char) * (DEFAULT_CAPACITY + NULL_CHAR_SPACE));
  if(sb->string == NULL) {
    free(sb);
    goto error;
  }
  sb->size = 0;
  sb->capacity = DEFAULT_CAPACITY;
  return sb;

 error:
  log_error("sb_init");
  return NULL;
}

void sb_close(_stringbuilder_t *sb) {
  free(sb->string);
  free(sb);
}

int sb_appendchar(_stringbuilder_t *sb, char c) {
  sb->string[sb->size] = c;
  sb->size++;
  if(sb->size == sb->capacity) {
    sb->capacity += DEFAULT_CAPACITY;
    sb->string = realloc(sb->string, sizeof(*(sb->string)) * (sb->capacity + NULL_CHAR_SPACE));
    if(sb->string == NULL) {
      sb_close(sb);
      log_error("sb_appendchar");
      return -1;
    }
  }
  return 0;
}

void sb_to_str(char *string, _stringbuilder_t *sb) {
  int i;
  for(i = 0; i < sb->size; i++) {
    string[i] = sb->string[i];
  }
  string[sb->size] = '\0';
}
