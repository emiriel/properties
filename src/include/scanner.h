/*
 * Filename:	scanner.h
 *
 * Description:	Header file where all public Scanner functions are declared.
 * The Scanner is the part of the lexer that reads the file and creates tokens for future use by the Lexer.
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

#ifndef SCANNER_H
#define SCANNER_H

/**
 * @brief The type of Token. Used as a mask
 *
 * @enum TOK_EOF             = 1     (000000000001)
 * @enum TOK_WS              = 2     (000000000010)
 * @enum TOK_ESCAPED_CHAR    = 4     (000000000100)
 * @enum TOK_UNICODE_CHAR    = 8     (000000001000)
 * @enum TOK_NEWLINE         = 16    (000000010000)
 * @enum TOK_ALNUM           = 32    (000000100000)
 * @enum TOK_PONCT           = 64    (000001000000)
 * @enum TOK_ASSIGN          = 128   (000010000000)
 * @enum TOK_A_ELSE          = 256   (000100000000)
 * @enum TOK_COMMENT         = 512   (001000000000)
 * @enum TOK_NULL            = 1024  (010000000000)
 */
typedef enum {
    TOK_EOF = 1,
    TOK_WS = 1 << 1,
    TOK_ESCAPED_CHAR = 1 << 2,
    TOK_UNICODE_CHAR = 1 << 3,
    TOK_NEWLINE = 1 << 4,
    TOK_ALNUM = 1 << 5,
    TOK_PONCT = 1 << 6,
    TOK_ASSIGN = 1 << 7,
    TOK_OTHER = 1 << 8,
    TOK_COMMENT = 1 << 9,
    TOK_NULL = 1 << 10
} _token_type;

/**
 * @brief Contains a text value and the associated type.
 */
typedef struct _token _token_t;

struct _token {
    _token_type type;
    char *value;
    int size;
};

/**
 * @brief Contains informations used to scan the current file and split it into tokens.
 */
typedef struct _scanner _scanner_t;

struct _scanner {
    int current_line;
    int current_col;
    int previous_line;
    int previous_col;
    FILE * file;
    char *filename;
};

/**
 * @brief Inits a scanner for a file.
 *
 * @param filename path to the file to scan
 *
 * @return a new scanner if succeeded, NULL otherwise
 */
_scanner_t * scanner_init(char *filename);

/**
 * @brief Scavenges a token from the file.
 *
 * @param scanner an initialized scanner
 *
 * @return A token if succeeded, NULL otherwise. In case of End Of File, returnes a Token of type EOF.
 */
_token_t * scanner_scan(_scanner_t *scanner);

/**
 * @brief Closes the file associated with the scanner and frees the scanner from memory.
 *
 * @param scanner the scanner to close
 */
void scanner_close(_scanner_t *scanner);

/**
 * @brief Frees a token from memory.
 *
 * @param tok the token to free
 */
void token_free(_token_t * tok);

/**
 * Returns a String corresponding to the token type
 *
 * @param type type of token
 *
 * @return string
 */
int token_print(char *str, _token_t token);
#endif
