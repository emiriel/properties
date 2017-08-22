/*
 * Filename:  lexer.h
 *
 * Description:  Header file where all public Lexer functions are declared.
 * The Lexer is in charge of calling the scanner and transforming a set of tokens into meaningful content.
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

#ifndef PROPERTIES_LEXER_H
#define PROPERTIES_LEXER_H
#include "properties.h"
#include "scanner.h"

/**
 * @brief Used to scan a file and build the properties accordingly.<br>
 * Contains a state machine to lex the file.<br>
 * States order : <br>
 * - Start <br>
 * - Getting Parameter Name <br>
 * - Assignment <br>
 * - Getting Parameter Value copying <br>
 * - Saving in properties <br>
 * - End <br>
 * - Error
 */
typedef struct _lexer lexer_t;

/**
 * @brief Inits the lexer.
 * Opens the file from filename,
 * creates the state machine used by the lexer,
 * and keeps the properties passed for further lexing.
 *
 * @param filename the filename to analyse
 * @param properties the properties structure to fill
 *
 * @return the newly created lexer if succeeded, NULL otherwise
 *
 */
lexer_t *lexer_new(char *filename, properties_t *properties);

/**
 * @brief Deletes the lexer from memory.
 *
 * @param lexer the lexer to delete
 */
void lexer_free(lexer_t *lexer);

/**
 * @brief Analyses the file and builds the properties found in the file.
 *
 * @param lexer the lexer which will launch the analysis
 *
 * @return 0 if succeeded, -1 otherwise
 *
 */
int lexer_analyze(lexer_t *lexer);

#endif
