/*
 * Filename:  logging.h
 *
 * Description:  Header file where all public Logging functions are declared.
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
 
#ifndef PROPERTIES_LOGGING_H
#define PROPERTIES_LOGGING_H

void log_error(char *logstring, ...);
void log_warning(char *logstring, ...);
void log_info(char *logstring, ...);
void log_debug(char *logstring, ...);
#endif
