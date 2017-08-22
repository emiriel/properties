#include <stdio.h>
#include <stdlib.h>
#include "include/lexer.h"
#include "include/utils.h"
#include "include/logging.h"

struct lexer_unit_test {
    char *filename;
    properties_t *properties;
    char *description;
};

struct lexer_test_category {
    void *reference;
    int size;
    struct lexer_unit_test **tests;
};

struct lexer_test {
    int size;
    struct lexer_test_category *categories;
};

struct lexer_test global_test;
properties_t *global_properties;

int global_nb_errors;

struct lexer_unit_test *unit_test_alloc(char *filename, void *properties, char *description) {
  struct lexer_unit_test *unit_test = malloc(sizeof(struct lexer_unit_test));
  if(unit_test == NULL) {
    log_error("Unit test allocation");
    return NULL;
  }

  unit_test->filename = filename;
  unit_test->properties = properties;
  unit_test->description = description;

  return unit_test;
}

void test_free() {
  for(int i = 0; i < global_test.size; i++) {
    for (int j = 0; j < global_test.categories[i].size; j++) {
      free(global_test.categories[i].tests[j]);
    }
    free(global_test.categories[i].tests);
  }
  free(global_test.categories);
}

int prepare() {
  global_test.size = 2;
  global_test.categories = malloc(global_test.size * sizeof(struct lexer_test_category));
  if(global_test.categories == NULL) {
    log_error("Tests initialization failed !");
    return FUNC_FAILURE;
  }

  global_properties = properties_new();
  if(global_properties == NULL) {
    log_error("Tests initialization failed !");
    return FUNC_FAILURE;
  }

  global_test.categories[0].reference = NULL;
  global_test.categories[0].size = 3;
  global_test.categories[0].tests = malloc(global_test.categories[0].size * sizeof(struct lexer_unit_test));
  global_test.categories[0].tests[0] = unit_test_alloc(NULL, global_properties, "NULL file test");
  global_test.categories[0].tests[1] = unit_test_alloc("tests/good.properties", NULL, "NULL properties test");
  global_test.categories[0].tests[2] = unit_test_alloc("tests/_____", global_properties, "missing file test");

  global_test.categories[1].reference = (void *) FUNC_FAILURE;
  global_test.categories[1].size = 3;
  global_test.categories[1].tests = malloc(global_test.categories[0].size * sizeof(struct lexer_unit_test));
  global_test.categories[1].tests[0] = unit_test_alloc("tests/only_param.properties", NULL,
                                                       "only a parameter name test");
  global_test.categories[1].tests[1] = unit_test_alloc("tests/no_value.properties", NULL,
                                                       "parameter without value test");
  global_test.categories[1].tests[2] = unit_test_alloc("tests/unauthorized_param_name.properties", NULL,
                                                       "unauthorized parameter name test");

  global_nb_errors = 0;

  return FUNC_SUCCESS;
}

int run_tests() {
  struct lexer_test_category category;

  int ret = 0, nb_keys = 0, cur_props_size;
  char **keys = NULL;
  properties_t *properties = NULL;
  lexer_t *lexer = NULL;

  category = global_test.categories[0];

  log_info("Testing failing initializations...");
  for(int i = 0; i < category.size; i++) {
    log_info(category.tests[i]->description);
    lexer = lexer_new(category.tests[i]->filename, category.tests[i]->properties);
    if(lexer == category.reference) {
      log_info("OK !");
    } else {
      log_error("Error !");
      global_nb_errors++;
    }
  }

  log_info("Initialization tests finished.");

  category = global_test.categories[1];

  log_info("Testing failing analysis...");
  for(int i = 0; i < category.size; i++) {
    log_info(category.tests[i]->description);

    properties = properties_new();
    if(properties == NULL) {
      log_error("Unable to init properties !");
      global_nb_errors++;
    } else {
      lexer = lexer_new(category.tests[i]->filename, properties);
      if (lexer == NULL) {
        log_error("Unable to init lexer !");
        global_nb_errors++;
      } else {
        log_info("Begin analysis...");
        ret = lexer_analyze(lexer);
        if (ret == ((__intptr_t) category.reference)) {
          log_info("OK !");
        } else {
          log_error("Error !");
          global_nb_errors++;
        }
        lexer_free(lexer);
      }
      properties_free(properties);
    }
  }
  log_info("Analysis tests finished.");

  properties = properties_new();

  log_info("Testing working case...");
  lexer = lexer_new("tests/good.properties", properties);
  if(lexer == NULL) {
    log_error("Lexer initialization failed !");
    global_nb_errors++;
    ret = FUNC_FAILURE;
    goto free_properties;
  }

  ret = lexer_analyze(lexer);
  if(ret == FUNC_FAILURE) {
    log_error("Lexer analysis failed !");
    global_nb_errors++;
    ret = FUNC_FAILURE;
    goto close_lexer;
  }

  log_info("Testing created properties...");

  log_info("Keys retrieving...");
  nb_keys = properties_get_keys(&keys, properties);
  if(keys == NULL || nb_keys == FUNC_FAILURE) {
    log_error("Failed !");
    global_nb_errors++;
    ret = FUNC_FAILURE;
    goto close_lexer;
  }

  log_info("Keys successfuly retrieved !");

  cur_props_size = nb_keys;
  for(int i = 0; i < nb_keys; i++) {
    printf("%s => '%s'\n", keys[i], (char *) properties_get_value(keys[i], properties));
    properties_property_free(keys[i], properties);
    cur_props_size--;
    if(properties->size != cur_props_size) {
      log_error("error during remove properties, properties size: %d, expected size: %d", properties->size, cur_props_size);
      global_nb_errors++;
      ret = FUNC_FAILURE;
      goto free_keys;
    }
  }

  log_info("All tests succeeded !");
  ret = FUNC_SUCCESS;

free_keys:
  free(keys);

close_lexer:
  lexer_free(lexer);

free_properties:
  properties_free(global_properties);
  properties_free(properties);

  test_free();

  if(global_nb_errors > 0) {
    log_error("%d tests have failed !", global_nb_errors);
  }
  return ret;
}

int main() {
  int ret = 0;
  ret = prepare();
  if(ret == FUNC_SUCCESS) {
    ret = run_tests();
  }
  return ret;
}
