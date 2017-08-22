/*
 * Filename:  lexer.c
 *
 * Description:  Contains the lexer source.
 * The lexer uses state machine and generic function pointers to process the file.
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
#include <errno.h>

#include "include/lexer.h"
#include "include/utils.h"
#include "include/logging.h"

#define NB_STATES       6

/**
 * Type defitions section
 */

/**
 * Acts as an action (calling its process_func)
 * and as a link (if incoming token is check against the mask, links to the next state)
 */
typedef struct _actionlink _actionlink_t;
/**
 * Has a Type and a list of links to other states.
 */
typedef struct _state _state_t;

/**
 * Archetype of a process function. A process function is called upon changing state.
 */
typedef int (_process_func) (_token_t *token, lexer_t *lexer);

/* states indexes */
typedef enum {
    STATE_START         = 0,
    STATE_PARAM_NAME    = 1,
    STATE_ASSIGN        = 2,
    STATE_PARAM_VALUE   = 3,
    STATE_END           = 4,
    STATE_ERR           = 5
} _state_type;

/*
 * token types values (used as a mask) :
 *
 * @enum TOK_EOF             = 1     (000000000001)
 * @enum TOK_WS              = 2     (000000000010)
 * @enum TOK_ESCAPED_CHAR    = 4     (000000000100)
 * @enum TOK_UNICODE_CHAR    = 8     (000000001000)
 * @enum TOK_NEWLINE         = 16    (000000010000)
 * @enum TOK_ALNUM           = 32    (000000100000)
 * @enum TOK_PONCT           = 64    (000001000000)
 * @enum TOK_ASSIGN          = 128   (000010000000)
 * @enum TOK_OTHER          = 256   (000100000000)
 * @enum TOK_ERR             = 512   (001000000000)
 * @enum TOK_COMMENT         = 1024  (010000000000)
 * @enum TOK_NULL            = 2048  (100000000000)
*/
typedef enum {
  TOK_ALL             = 4095, /* (111111111111) */
/* Masks to loop in the current state */
  START_TO_START      = TOK_COMMENT | TOK_NEWLINE | TOK_WS,
  PNAME_TO_PNAME      = TOK_ALNUM | TOK_PONCT,
  ASSIGN_TO_ASSIGN    = TOK_WS,
  PVALUE_TO_PVALUE    = TOK_ESCAPED_CHAR | TOK_UNICODE_CHAR | TOK_ALNUM | TOK_PONCT | TOK_ASSIGN | TOK_COMMENT | TOK_WS | TOK_OTHER,
/* Masks to go to the next step */
  START_TO_PNAME      = PNAME_TO_PNAME,
  PNAME_TO_ASSIGN     = TOK_WS | TOK_ASSIGN,
  ASSIGN_TO_PVALUE    = PVALUE_TO_PVALUE,
  PVALUE_TO_START     = TOK_NEWLINE,
/* Masks to go to the final state */
  START_TO_END        = TOK_EOF,
  PVALUE_TO_END       = TOK_EOF,
/* Masks that trigger error */
  START_TO_ERR        = TOK_ALL - (START_TO_START | START_TO_PNAME | START_TO_END),
  PNAME_TO_ERR        = TOK_ALL - (PNAME_TO_PNAME | PNAME_TO_ASSIGN),
  ASSIGN_TO_ERR       = TOK_ALL - (ASSIGN_TO_ASSIGN | ASSIGN_TO_PVALUE),
  PVALUE_TO_ERR       = TOK_ALL - (PVALUE_TO_PVALUE | PVALUE_TO_START | PVALUE_TO_END)
} _state_condition;

struct _state {
    _state_type state_type;
    int nb_links;
    _actionlink_t *actionlinks;
};

struct _actionlink {
    _state_condition accepted_tokens;
    _process_func *process_func;
    _state_t *next_state;
};

struct _lexer {
    properties_t *properties;
    _scanner_t *scanner;
    _state_t current_state;
    _state_t *states;
    char *param_name;
    int param_name_size;
    char *param_value;
    int param_value_size;
};

/**
 * Private Section
 * Process functions implementations
 */

/**
 * Dummy function. Called on links that do nothing.
 * @param tok
 * @param lexer
 * @return 0
 */
static int process_nothing(_token_t *token, lexer_t *lexer) {
  return FUNC_SUCCESS;
}

/**
 * Copies current token value (the token is a param name) into the lexer's current param name field.
 * @param tok the token
 * @param lexer the lexer
 * @return 0 if succeeded, -1 otherwise
 */
static int process_param_name(_token_t *tok, lexer_t *lexer) {
  return copy_or_append_token(&(lexer->param_name), &(lexer->param_name_size), tok);
}

/**
 * Copies current token value (the token is a param value) into the lexer's current param value field.
 * @param tok the token
 * @param lexer the lexer
 * @return 0 if succeeded, -1 otherwise
 */
static int process_param_value(_token_t *tok, lexer_t *lexer) {
  return copy_or_append_token(&(lexer->param_value), &(lexer->param_value_size), tok);
}

/**
 * Saves the lexer's parameter name and value into a new parameter and adds it to the lexer's properties handler.
 * @param tok the token
 * @param lexer the lexer
 * @return 0 if succedded, -1 otherwise
 */
static int process_save(_token_t *token, lexer_t *lexer) {
  property_t * prop;

  prop = properties_property_new(lexer->param_name, lexer->param_value, free);
  if(prop == NULL) {
    return FUNC_FAILURE;
  }

  properties_property_add(prop, lexer->properties);
  lexer->param_name = NULL;
  lexer->param_name_size = 0;
  lexer->param_value = NULL;
  lexer->param_value_size = 0;
  return FUNC_SUCCESS;
}

/**
 * Called when an unexpected token is met.
 * @param tok
 * @param lexer
 * @return -1 (error)
 */
static int process_error(_token_t *tok, lexer_t *lexer) {
  char str[30];
  int ret;

  ret = token_print(str, *tok);
  if(ret < 0) {
    log_error("error printing unexpected token: %s", str);
  } else {
    ret = FUNC_FAILURE;
    log_error("Unexpected %s in %s:%d,%d", str, lexer->scanner->filename, lexer->scanner->previous_line, lexer->scanner->previous_col);
  }

  return ret;
}

static int check_token(_token_t *tok, _state_condition flags) {
  return (flags & tok->type) != 0; // mask checking
}

/**
 * General process function. Checks token and dispatches to the good linkaction process function.
 * @param tok current token
 * @param lexer the lexer
 * @return 0 if succeeded, -1 otherwise
 */
static int process(_token_t *tok, lexer_t *lexer) {
  int i = 0, ret = FUNC_SUCCESS;
  _state_t cur_state;

  _actionlink_t actionlink;
  cur_state = lexer->current_state;

  for(i = 0; i < cur_state.nb_links; i++) {
    actionlink = cur_state.actionlinks[i];
    if(check_token(tok, actionlink.accepted_tokens)) {
      ret = actionlink.process_func(tok, lexer);
      lexer->current_state = *(actionlink.next_state);
      break;
    }
  }

  return ret;
}

/**
 * States init and destroy functions
 */

static void states_free_all(_state_t *all_states, int nb_states) {
  int i = 0 ;
  for(i = 0; i < nb_states; i++) {
    free(all_states[i].actionlinks);
  }
  free(all_states);
}

static int state_new(int state_idx, int nb_next_states, _state_t *all_states) {
  _state_t *cur_state;
  cur_state = &(all_states[state_idx]);
  cur_state->actionlinks = malloc(nb_next_states * sizeof(_actionlink_t));

  if(cur_state->actionlinks == NULL) {
    return errno;
  }
  return FUNC_SUCCESS;
}

/**
 * Creates an actionlink and adds it to the current state.
 * @param state_idx index of the state into the array of states
 * @param condition mask of accepted tokens used by the actionlink
 * @param next_state_idx next state stored in the actionlink
 * @param process_func process function which will be called by the actionlink
 * @param all_states the array of states
 */
static void add_actionlink(int state_idx, _state_condition condition, int next_state_idx, _process_func process_func,
                           _state_t *all_states) {
  _state_t *cur_state;
  _actionlink_t *actionlink;

  cur_state = &(all_states[state_idx]);
  actionlink = &(cur_state->actionlinks[cur_state->nb_links]);
  actionlink->next_state = &(all_states[next_state_idx]);
  actionlink->accepted_tokens = condition;
  actionlink->process_func = process_func;

  cur_state->nb_links++;
}

/**
 * Inits the array of states.
 * Create each state, then links them together with actionlinks.
 * Typical path is :
 * Start -> process param name -> process param value -> save and go to start or end.
 * @param p_all_states the array of states
 * @return the start state
 */
static _state_t *states_new(_state_t **p_all_states) {
  _state_t *cur_state;
  _state_t *deref_all_states;
  int i = 0;

  *p_all_states = malloc(NB_STATES * sizeof(_state_t));
  if(*p_all_states == NULL) {
    goto error;
  }
  deref_all_states =*p_all_states;

  /* creating empty states */
  for(i = 0; i < NB_STATES; i++) {
    cur_state = &(deref_all_states[i]);
    cur_state->state_type = (_state_type) i;
    cur_state->actionlinks = NULL;
    cur_state->nb_links = 0;
  }

  /* populating states with arrays of actionlinks */

  /* Start state */
  if(state_new(STATE_START, 4, deref_all_states) != 0) {
    goto dealloc_states;
  }
  add_actionlink(STATE_START, START_TO_START, STATE_START, process_nothing, deref_all_states);
  add_actionlink(STATE_START, START_TO_PNAME, STATE_PARAM_NAME, process_param_name, deref_all_states);
  add_actionlink(STATE_START, START_TO_END, STATE_END, process_nothing, deref_all_states);
  add_actionlink(STATE_START, START_TO_ERR, STATE_ERR, process_error, deref_all_states);

  /* Parameter Name state */
  if(state_new(STATE_PARAM_NAME, 3, deref_all_states) != 0) {
    goto dealloc_states;
  }
  add_actionlink(STATE_PARAM_NAME, PNAME_TO_PNAME, STATE_PARAM_NAME, process_param_name, deref_all_states);
  add_actionlink(STATE_PARAM_NAME, PNAME_TO_ASSIGN, STATE_ASSIGN, process_nothing, deref_all_states);
  add_actionlink(STATE_PARAM_NAME, PNAME_TO_ERR, STATE_ERR, process_error, deref_all_states);

  /* Assign state */
  if(state_new(STATE_ASSIGN, 3, deref_all_states) != 0) {
    goto dealloc_states;
  }
  add_actionlink(STATE_ASSIGN, ASSIGN_TO_ASSIGN, STATE_ASSIGN, process_nothing, deref_all_states);
  add_actionlink(STATE_ASSIGN, ASSIGN_TO_PVALUE, STATE_PARAM_VALUE, process_param_value, deref_all_states);
  add_actionlink(STATE_ASSIGN, ASSIGN_TO_ERR, STATE_ERR, process_error, deref_all_states);

  /* Parameter Value state */
  if(state_new(STATE_PARAM_VALUE, 4, deref_all_states) != 0) {
    goto dealloc_states;
  }
  add_actionlink(STATE_PARAM_VALUE, PVALUE_TO_PVALUE, STATE_PARAM_VALUE, process_param_value, deref_all_states);
  add_actionlink(STATE_PARAM_VALUE, PVALUE_TO_START, STATE_START, process_save, deref_all_states);
  add_actionlink(STATE_PARAM_VALUE, PVALUE_TO_END, STATE_END, process_save, deref_all_states);
  add_actionlink(STATE_PARAM_VALUE, PVALUE_TO_ERR, STATE_ERR, process_error, deref_all_states);

  return &(deref_all_states[STATE_START]);

dealloc_states:
  states_free_all(deref_all_states, NB_STATES);
  *p_all_states = NULL;

error:
  log_error("lexer: states_new");
  return NULL;
}

/**
 * Public section
 */

lexer_t * lexer_new(char *filename, properties_t *properties) {
  lexer_t *lexer;
  _state_t *cur_state;
  _state_t * all_states;
  _scanner_t * scanner;

  if(properties == NULL) {
    log_error("lexer_new: properties is NULL");

    return NULL;
  }

  if(filename == NULL || filename[0] == '\0') {
    log_error("lexer_new: empty filename");
    return NULL;
  }
  
  cur_state = states_new(&all_states);
  if(cur_state == NULL) {
    goto dealloc_states;
  }
  
  scanner = scanner_new(filename);
  if(scanner == NULL) {
    goto dealloc_states;
  }

  lexer = malloc(sizeof(*lexer));
  lexer->states = all_states;
  lexer->current_state = *cur_state;
  lexer->scanner = scanner;
  lexer->properties = properties;
  lexer->param_name = NULL;
  lexer->param_name_size = 0;
  lexer->param_value = NULL;
  lexer->param_value_size = 0;
  return lexer;

dealloc_states:
  states_free_all(all_states, NB_STATES);

  return NULL;
}

void lexer_free(lexer_t *lexer) {
  lexer->properties = NULL;
  scanner_free(lexer->scanner);

  if(lexer->param_name_size > 0) {
    free(lexer->param_name);
  }
  if(lexer->param_value_size > 0) {
    free(lexer->param_value);
  }

  states_free_all(lexer->states, NB_STATES);
  free(lexer);
}

int lexer_analyze(lexer_t *lexer) {
  int process_status;
  _token_t *token;
  
  do {
    token = scanner_scan(lexer->scanner);
    if(token != NULL) {
      process_status = process(token, lexer);
      token_free(token);
    } else {
      process_status = FUNC_FAILURE;
      log_error("lexer_analyze: token is NULL");
    }
  } while(process_status == FUNC_SUCCESS && lexer->current_state.state_type != STATE_END);

  return process_status;
}