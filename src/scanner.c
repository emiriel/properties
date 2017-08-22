/*
 * Filename:  scanner.c
 *
 * Description:  
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

#include <stdio.h>
#include <malloc.h>
#include <ctype.h>
#include <string.h>

#include "include/scanner.h"
#include "include/stringbuilder.h"
#include "include/utils.h"
#include "include/logging.h"

static int UNICODE_OFFSET = 2;
static int UNICODE_MAX_SIZE = 8;

static int is_alnum(char c) {
  return isalnum((int) c);
}

static int is_ws(char c) {
  return c == ' ' || c == '\t';
}

static int is_newline(char c) {
  return c == '\r' || c == '\n';
}

static int not_newline(char c) {
  return !is_newline(c);
}

static int is_assign(char c) {
  return c == '=' || c == ':';
}

static int is_eof(char c) {
  return c == EOF;
}

static int is_escape(char c) {
  return c == '\\';
}

static int is_ponct(char c) {
  return c == '-' || c == '_' || c == '.';
}

static int is_comment(char c) {
  return c == '#' || c == '!';
}

static char get_char(_scanner_t * scanner) {
  char c = (char) fgetc(scanner->file);
  scanner->previous_line = scanner->current_line;
  scanner->previous_col = scanner->current_col;
  if(!is_eof(c)) {
    if(is_newline(c)) {
      scanner->current_line++;
      scanner->current_col = 1;
    } else {
      scanner->current_col++;
    }
  }
  
  return c;
}

static void unget_char(char c, _scanner_t * scanner) {
  scanner->current_line = scanner->previous_line;
  scanner->current_col = scanner->previous_col;
  
  ungetc(c, scanner->file);
}

static _token_t * scanGeneric(_scanner_t * scanner, _token_type type, int (*checker)(char)) {
  char c;
  unsigned int size = 0;
  _token_t *tok;
  _stringbuilder_t * sb = sb_new();
  if(sb == NULL ) {
    goto error;
  }
  
  c = get_char(scanner);
  while (checker(c)) {
    if(sb_appendchar(sb, c) != 0) {
      goto error;
    }
    size++;
    c = get_char(scanner);
  }
  unget_char(c, scanner);
  
  tok = token_new(size);
  if(tok == NULL) {
    goto free_sb_error;
  }
  
  tok->type = type;
  sb_to_str(tok->value, sb);
  sb_free(sb);
  
  return tok;
  
free_sb_error:
  sb_free(sb);
error:
  log_error("scanGeneric");
  return NULL;
}

static _token_t * scanGenChar(_scanner_t * scanner, _token_type type) {
  _token_t *tok;
  
  tok = token_new(1);
  if(tok == NULL) {
    goto error;
  }
  
  tok->type = type;
  tok->value[0] = get_char(scanner);
  tok->value[1] = '\0';
  return tok;
error:
  return NULL;
}

static _token_t * scanWhitespace(_scanner_t * scanner) {
  return scanGeneric(scanner, TOK_WS, &is_ws);
}

static _token_t * scanUnicodeChar(_scanner_t * scanner) {
  int size, i;
  char workValue[UNICODE_MAX_SIZE + NULL_CHAR_OFFSET];
  char c;
  _token_t *tok;
  
  workValue[0] = '\\';
  workValue[1] = 'u';
  c = get_char(scanner);
  i = UNICODE_OFFSET; /* the first 2 elements of the array are filled */
  size = UNICODE_MAX_SIZE;
  while(isxdigit(c) && i < size) {
    workValue[i] = c;
    c = get_char(scanner);
    i++;
  }
  size = i;
  workValue[size] = '\0';
  unget_char(c, scanner);

  tok = token_new(size);
  if(tok == NULL) {
    goto error;
  }
  
  tok->type = TOK_UNICODE_CHAR;
  strcpy(tok->value, workValue);
  
  return tok;
  
error:
  log_error("scanUnicodeChar");
  return NULL;
}

static _token_t * scanEscapedNewline(_scanner_t * scanner) {
  char c;
  _token_t *tok = NULL;
  
  c = get_char(scanner);
  if(c == '\n') {
    tok = token_new(1);
    if(tok == NULL) {
      goto error;
    }
    
    tok->value[0] = '\n';
    tok->value[1] = '\0';
  } else {
    c = (char) fgetc(scanner->file);
    if(c == '\n') {
      tok = token_new(2);
      if(tok == NULL) {
        goto error;
      }
      
      tok->value[0] = '\r';
      tok->value[1] = '\n';
      tok->value[2] = '\0';
    } else {
      ungetc(c, scanner->file);
      
      tok = token_new(1);
      if(tok == NULL) {
        goto error;
      }
      
      tok->value[0] = '\r';
      tok->value[1] = '\0';
    }
  }
  tok->type = TOK_ESCAPED_CHAR;
  return tok;
  
error:
  log_error("scanEscapedNewline");
  return NULL;
}

static _token_t * scanEscapedChars(_scanner_t * scanner) {
  char c;
  _token_t * tok;
  
  c = get_char(scanner);
  if(c == 'u') {
    return scanUnicodeChar(scanner);
  }
  if(is_newline(c)) {
    unget_char(c, scanner);
    return scanEscapedNewline(scanner);
  }

  tok = token_new(2);
  if(tok == NULL) {
    return NULL;
  }

  tok->type = TOK_ESCAPED_CHAR;
  tok->value[0] = '\\';
  tok->value[1] = c;
  tok->value[2] = '\0';
  return tok;
}

static _token_t * scanNewline(_scanner_t * scanner) {
  return scanGenChar(scanner,TOK_NEWLINE);
}

static _token_t * scanAlnum(_scanner_t * scanner) {
  return scanGeneric(scanner, TOK_ALNUM, &is_alnum);
}

static _token_t * scanPonct(_scanner_t * scanner) {
  return scanGeneric(scanner, TOK_PONCT, &is_ponct);
}

static _token_t * scanAssign(_scanner_t * scanner) {
  return scanGenChar(scanner, TOK_ASSIGN);
}

static _token_t * scanComment(_scanner_t * scanner) {
  return scanGeneric(scanner, TOK_COMMENT, &not_newline);
}

_token_t * scanner_scan(_scanner_t *scanner) {
  char c;
  
  _token_t *tok = NULL;

  c = get_char(scanner);
  if (is_ws(c)) {
    unget_char(c, scanner);
    tok = scanWhitespace(scanner);
  } else if (is_escape(c)) {
    tok = scanEscapedChars(scanner);
  } else if (is_newline(c)) {
    unget_char(c, scanner);
    tok = scanNewline(scanner);
  } else if (isalnum(c)) {
    unget_char(c, scanner);
    tok = scanAlnum(scanner);
  } else if (is_ponct(c)) {
    unget_char(c, scanner);
    tok = scanPonct(scanner);
  } else if (is_assign(c)) {
    unget_char(c, scanner);
    tok = scanAssign(scanner);
  } else if (is_comment(c)) {
    tok = scanComment(scanner);
  } else if (is_eof(c)) {
    tok = token_new(0);
    tok->value = NULL;
    tok->type = TOK_EOF;
  } else {
    unget_char(c, scanner);
  }

  return tok;
}

_scanner_t * scanner_new(char *filename) {
  _scanner_t *scanner;
  FILE *file;

  file = fopen(filename, "r");
  if(file == NULL) {
    goto log_error;
  }

  scanner = malloc(sizeof(*scanner));
  if(scanner == NULL) {
    goto close_file;
  }

  scanner->filename = malloc(sizeof(char) * strlen(filename) + NULL_CHAR_OFFSET);
  if(scanner->filename == NULL) {
    goto dealloc_scanner;
  }

  if(strcpy(scanner->filename, filename) == NULL) {
    goto dealloc_filename;
  }

  scanner->file = file;
  scanner->current_line = 1;
  scanner->current_col = 1;
  return scanner;

  dealloc_filename:
  free(scanner->filename);

  close_file:
  fclose(file);

  dealloc_scanner:
  free(scanner);

  log_error:
  log_error("scanner_new");

  return NULL;
}

void scanner_free(_scanner_t *scanner) {
  fclose(scanner->file);
  free(scanner->filename);
  free(scanner);
}

