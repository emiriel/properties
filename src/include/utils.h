/*
 * Filename:  utils.h
 *
 * Description:  Header for utility functions
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

#ifndef PROPERTIES_UTILS_H
#define PROPERTIES_UTILS_H

#define FUNC_FAILURE    (-1)
#define FUNC_SUCCESS    0

#define NULL_CHAR_OFFSET  1

/**
 * Checks nullity of a variable number of arguments
 *
 * @param nb_args number of arguments to check
 * @param ... arguments
 *
 * @return 0 if none were NULL, -1 otherwise
 */
int check_null(int nb_args, ...);

/**
 * Increases the size of an array of elements to the new size multiplied by the size of the elements to realloc
 *
 * @param inflatable the pointer to the array to inflate
 * @param new_size the new size
 * @return 0 if succeeded, -1 otherwise
 */
int inflate(void **inflatable, int new_size, size_t ptrsize);

/**
 * Check current size of pointed array of elements against reference size. If over, the array is inflated by a step.
 *
 * @param inflatable the pointer of array
 * @param cur_size current size of the array
 * @param p_max_size reference size
 * @param step step
 * @return the new array if succedded, NULL otherwise
 */
int manage_size(void ** inflatable, int cur_size, int *p_max_size, int step, size_t ptrsize);

#endif
