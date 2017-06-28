/*
 * Filename:  
 *
 * Description:  
 *
 * Copyright (c) 2016 Erwann Miriel, erwann.miriel@gmail.com 
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
#include <stdlib.h>
#include <string.h>

#ifdef ILOG_SUPPORT
#include <ilogger.h>
#else
#include "include/logging.h"
#endif

#include "include/properties.h"
#include "include/utils.h"

#define PROPERTIES_STEP 10

#define FUNC_FAILURE -1
#define FUNC_SUCCESS 0

struct _valueholder {
    void *value;
    _dealloc_func_t *_dealloc;
};

struct _property {
    char *key;
    valueholder_t valueholder;
};

/** @brief Finds property by name in the properties holder passed in parameter.
 *
 * @param propertiesholder the properties container
 * @param key the name of the property to find
 * @return Index of Property if found, -1 (minus one) otherwise
 *
 */
static int properties_find_property(char *key, properties_t *props) {
  int i;
  for (i = 0; i < props->size; i++) {
    if (strcmp(key, props->contents[i]->key) == 0) {
      return i;
    }
  }
  return -1;
}

/** @brief Frees property from memory.
 *
 * @param property_t the property to free
 *
 */
static int properties_free_property(property_t *property) {
  if(check_null(1, property) != FUNC_SUCCESS) {
    log_error("properties_free : property_t is NULL");
    return FUNC_FAILURE;
  }

  free(property->key);
  property->valueholder._dealloc(property->valueholder.value);
  free(property);

  return FUNC_SUCCESS;
}

properties_t *properties_new() {
  properties_t *props;
  props = malloc(sizeof(*props));
  if(props == NULL) {
    perror("properties_new: contents");
    goto errorProps;
  }
  props->contents = malloc(PROPERTIES_STEP * sizeof(*(props->contents)));
  if(props->contents == NULL) {
    perror("properties_new: properties");
    goto errorContents;
  }
  props->capacity = PROPERTIES_STEP;
  props->size = 0;

  return props;

  errorContents:
  free(props);
  errorProps:
  return NULL;
}

property_t * properties_new_property(char *key, void *value, _dealloc_func_t *dealloc) {
  if(check_null(3, key, value, dealloc) != FUNC_SUCCESS) {
    log_error("properties_new_property : key or value is NULL");
  }

  property_t * property = malloc(sizeof(*property));

  if (property == NULL) {
    log_error("properties_new_property : allocation failed");
    return NULL;
  }

  property->key = key;
  property->valueholder.value = value;
  property->valueholder._dealloc = dealloc;
  return property;
}

int properties_add_property(property_t *prop, properties_t *props) {
  int max;
  
  if(props == NULL || prop == NULL) {
    log_error("properties_add_property : structure or element is NULL");
    return FUNC_FAILURE;
  }
  
  max = props->size;
  props->size++;
  props->contents[max] = prop;
  return manage_size((void **) props->contents, props->size, &(props->capacity), PROPERTIES_STEP, sizeof(*(props->contents)));
}

int properties_get_keys(char ***keys, properties_t *props) {
  int i;
  char **theKeys;

  *keys = malloc(props->size * sizeof(*keys));
  theKeys = *keys;
  if(theKeys == NULL) {
    return FUNC_FAILURE;
  }

  for (i = 0; i < props->size; i++) {
    theKeys[i] = props->contents[i]->key;
  }

  return i;
}

void *properties_get_value(char *key, properties_t *props) {
  int i = properties_find_property(key, props);
  if (i != -1) {
    return props->contents[i]->valueholder.value;
  }
  return NULL;
}

int properties_remove_property(char *key, properties_t *properties) {
  int idx, max;
  property_t *aProperty;

  if(check_null(2, properties, key)) {
    log_error("properties_remove_property : structure or key is null");
    return FUNC_FAILURE;
  }

  idx = properties_find_property(key, properties);
  if (idx == -1) {
    return FUNC_FAILURE;
  }

  aProperty = properties->contents[idx];
  if(properties_free_property(aProperty) != 0) {
    return FUNC_FAILURE;
  }

  max = properties->size - 1;

  for (int i = idx; i < max; i++) {
    properties->contents[i] = properties->contents[i + 1];
  }

  properties->contents[max] = NULL;
  properties->size--;

  return idx;
}

void properties_free(properties_t *props) {
  int i;
  for (i = 0; i < props->size; i++) {
    properties_free_property(props->contents[i]);
  }
  free(props->contents);
  free(props);
}

