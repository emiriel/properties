/*
 * Filename:	stringbuilder.h
 *
 * Description:	Header file where all public Stringbuilder functions are declared.
 * A stringbuilder is intended to reduce the number of reallocation when building a string using lots of appending.
 * For that purpose, it is allocated and by steps of 500 chars.
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

#ifndef PROPERTIES_STRINGBUILDER_H
#define PROPERTIES_STRINGBUILDER_H

/**
 * Contains pieces of string to concatenate.
 */
typedef struct _stringbuilder _stringbuilder_t;

struct _stringbuilder {
  char * string;
  int size;
  int capacity;
};

/**
 * Inits a stringbuilder.
 *
 * @return A new stringbuilder if succeeded, NULL otherwise
 */
_stringbuilder_t * sb_new();

/**
 * Frees a stringbuilder from memory.
 *
 * @param sb the stringbuilder to free
 */
void sb_free(_stringbuilder_t *sb);
/**
 * Appends a char to a stringbuilder.
 *
 * @param sb The stringbuilder
 * @param c the character to append
 *
 * @return 0 if succeeded, -1 otherwise
 */
int sb_appendchar(_stringbuilder_t *sb, char c);

/**
 * Copies the contents of a stringbuilder into a string.
 *
 * @param string the target string
 *
 * @param sb the source stringbuilder
 */
void sb_to_str(char *string, _stringbuilder_t *sb);

#endif
