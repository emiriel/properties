/*
 * Filename:  logging.c
 *
 * Description:  Source file of logging functions
 *
 * Copyright (c) 2017 Erwann Miriel, erwann.miriel@gmail.com
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
#include <stdio.h>
#include <errno.h>

void log_error(char *logstring, ...) {
  char str[500];
  va_list args;
  int errnum = errno;

  va_start(args, logstring);
  vsnprintf(str, 500, logstring, args);
  va_end(args);

  if(errnum != 0) { //system error
    perror(str);
    errno = 0;
  } else {
    printf("Error: %s\n", str);
  }
}

void log_warning(char *logstring, ...) {
  char str[500];
  va_list args;

  va_start(args, logstring);
  vsnprintf(str, 500, logstring, args);
  va_end(args);

  printf("Warning: %s\n", str);

}
void log_info(char *logstring, ...) {
  char str[500];
  va_list args;

  va_start(args, logstring);
  vsnprintf(str, 500, logstring, args);
  va_end(args);

  printf("Info: %s\n", str);
}
void log_debug(char *logstring, ...) {
  char str[500];
  va_list args;

  va_start(args, logstring);
  vsnprintf(str, 500, logstring, args);
  va_end(args);

  printf("Debug: %s\n", str);
}
