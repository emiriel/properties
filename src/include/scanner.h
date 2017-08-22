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

#ifndef PROPERTIES_SCANNER_H
#define PROPERTIES_SCANNER_H

#include <stdio.h>

#include "token.h"

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
_scanner_t * scanner_new(char *filename);

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
void scanner_free(_scanner_t *scanner);
#endif
