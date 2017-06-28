/*
 * Filename:  
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
 
#include <stdarg.h>
#include <stddef.h>
#include <malloc.h>

#include "include/utils.h"
#include "include/logging.h"

int NULL_CHAR_SPACE = 1;

int check_null(int nb_args, ...) {
  va_list arguments;
  int i;
  void * ptr;
  va_start(arguments, nb_args);
  
  for(i = 0; i < nb_args; i++) {
    ptr = (void *) va_arg(arguments, void*);
    if(ptr == NULL) {
      return FUNC_FAILURE;
    }
  }
  
  return FUNC_SUCCESS;
}

int inflate(void **inflatable, int new_size, int ptrsize) {
  void * new_inflatable;
  new_inflatable = realloc(*inflatable, new_size * ptrsize);
  if(new_inflatable == NULL) {
    log_error("inflate");
    return FUNC_FAILURE;
  }
  (*inflatable) = new_inflatable;
  return FUNC_SUCCESS;
}

int manage_size(void ** inflatable, int cur_size, int *p_max_size, int step, int ptrsize) {
  int ret = FUNC_SUCCESS;
  int max_size;

  max_size = *p_max_size;
  if(cur_size == max_size) {
    ret = inflate(inflatable, max_size + step, ptrsize) != FUNC_SUCCESS;
    if(ret == FUNC_SUCCESS) {
      max_size += step;
    }
  }
  *p_max_size = max_size;
  return ret;
}
