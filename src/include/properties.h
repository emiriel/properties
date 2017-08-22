/*
 * Filename:  properties.h
 *
 * Description:  Header file where all Properties public structures and functions are declared.
 * The Properties Handler is intended to store data used as parameters for a program.
 * Properties are generally stored in a Properties file.
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

#ifndef PROPERTIES_H
#define PROPERTIES_H

/**
 * @brief Function pointer to delete a valueholder's value from memory.
 */
typedef void (_free_func_t) (void *);

/**
 * @brief Contains a value (can be any type of data) and a deallocation function.
 */
typedef struct _valueholder valueholder_t;

/**
 * @brief Contains a key (string) and a valueholder.
 */
typedef struct _property property_t;

/**
 * @brief Contains the list of properties.
 */
typedef struct _properties properties_t;

struct _properties {
    int size;
    int capacity;
    property_t **contents;
};

/**
 * @brief creates a new properties holder
 *
 * @return the new properties holder if succeeded, NULL otherwise
 */
properties_t *properties_new();

/**
 * @brief Creates a new property.
 *
 * @param key the name of the property
 * @param value the value of the property
 * @param dealloc the deallocation function of the value
 *
 * @return the pointer of the created property if succeeded, NULL otherwise
 */
property_t *properties_property_new(char *key, void *value, _free_func_t *dealloc);

/**
 * @brief Adds a property to the properties holder.
 *
 * @param properties the property
 * @param property the properties holder
 *
 * @return 0 if succeeded, -1 otherwise
 */
int properties_property_add(property_t *property, properties_t *properties);

/**
 * @brief fills an array of char containing all properties' names from the properties holder
 *
 * @param properties the properties holder
 * @param propertiesNames the array of names to fill
 *
 * @return 0 if succeeded, -1 otherwise
 */
int properties_get_keys(char ***p_keys, properties_t *properties);

/**
 * @brief Gets property by name in the properties holder.
 *
 * @param properties the properties holder
 * @param key the name of the property to find
 *
 * @return property if found, NULL otherwise
 *
 */
void* properties_get_value(char *key, properties_t *properties);

/**
 * @brief Finds and removes property from the properties holder (the property is freed).
 *
 * @param properties the properties holder
 * @param key the key of assoiated with the element to remove
 *
 * @return 0 if succeeded, -1 otherwise
 *
 */
int properties_property_free(char *key, properties_t *properties);

/**
 * @brief frees the properties holder from memory
 *
 * @param properties the properties holder
 */
void properties_free(properties_t *properties);
#endif
