/*
 * Filename:  token.h
 *
 * Description:  Functions concerning tokens (creation, destruction, copying...)
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

#include <malloc.h>
#include <string.h>

#include "include/scanner.h"
#include "include/utils.h"

static int copy_tok_value(char **p_dest, int *p_dest_size, _token_t *tok) {
  int deref_dest_size;
  char * dest;

  deref_dest_size = tok->size;
  dest = malloc(deref_dest_size * sizeof(dest));
  if(dest == NULL) {
    return FUNC_FAILURE;
  }
  *p_dest_size = deref_dest_size;
  *p_dest = dest;

  strcpy(dest, tok->value);
  return FUNC_SUCCESS;
}

static int append_tok_value(char **p_dest, int *p_dest_size, _token_t *tok) {
  int deref_dest_size;

  int cur_size;
  char * new_dest;

  deref_dest_size = *p_dest_size;

  cur_size = deref_dest_size - 1;/* because of '\0' at the end of EACH string */
  deref_dest_size += tok->size - 1; /* because of '\0' at the end of EACH string */

  new_dest = realloc(*p_dest, deref_dest_size * sizeof(**p_dest));
  if(new_dest == NULL) {
    return FUNC_FAILURE;
  }

  *p_dest = new_dest;
  *p_dest_size = deref_dest_size;
  strcpy(&(new_dest[cur_size]), tok->value);
  return FUNC_SUCCESS;
}

_token_t * token_new(int nb_chars) {
  int size = 0;

  _token_t *tok = malloc(sizeof(_token_t));
  if (tok == NULL) {
    return NULL;
  }

  if (nb_chars > 0) {
    size = nb_chars + NULL_CHAR_OFFSET;
    tok->value = malloc(sizeof(char) * size);
    if (tok->value == NULL) {
      free(tok);
      return NULL;
    }
  }
  tok->size = size;

  return tok;
}

void token_free(_token_t * tok) {
  if(tok->value != NULL) {
    free(tok->value);
  }
  free(tok);
}

int token_print(char *str, _token_t token) {
  int ret = 0;
  switch(token.type) {
    case TOK_EOF: ret = sprintf(str, "[end of file]"); break;
    case TOK_WS: ret = sprintf(str, "'%s'", token.value); break;
    case TOK_ESCAPED_CHAR: ret = sprintf(str, "'%s'", token.value); break;
    case TOK_UNICODE_CHAR: ret = sprintf(str, "'%s'", token.value); break;
    case TOK_NEWLINE: ret = sprintf(str, "[newline]"); break;
    case TOK_ALNUM: ret = sprintf(str, "'%s'", token.value); break;
    case TOK_PONCT: ret = sprintf(str, "'%s'", token.value); break;
    case TOK_ASSIGN: ret = sprintf(str, "'%s'", token.value); break;
    case TOK_OTHER: ret = sprintf(str, "'%s'", token.value); break;
    case TOK_COMMENT: ret = sprintf(str, "'%s'", token.value); break;
    case TOK_NULL: ret = sprintf(str, "[nothing]"); break;
    default: sprintf(str, "unknown token type %d", token.type); ret = FUNC_FAILURE; break;
  }
  return ret;
}

int copy_or_append_token(char **p_element_value, int *p_element_size, _token_t *p_tok) {
  if((*p_element_size) == 0) {
    return copy_tok_value(p_element_value, p_element_size, p_tok);
  }
  return append_tok_value(p_element_value, p_element_size, p_tok);
}