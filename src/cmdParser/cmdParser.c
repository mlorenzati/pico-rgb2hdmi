#include "cmdParser.h"
#include <stdio.h>
#include <string.h>

void cmd_parser_get_argv_argc(char *str, int *argc, char *argv[]) {
    //strtok in pico carries a bug in first run
    static char first_run = 0; //1
    int tokenCount = 1;
    char *token = strtok(str, " ");
    argv[0] = token + first_run;
    while(token != NULL ) {
      token = strtok(NULL, " ");
      if (token != NULL) {
          argv[tokenCount++] = token;
      }
    }
    *argc = tokenCount;
    first_run = 0;
}

int  cmd_parser_get_cmd (int argc, char *const argv[], cmd_parser_option_t * options, int *option_index, int *arg_index) {
    int option_res = 0;
    //Completed analyzing all arguments
    if (*arg_index >= argc) {
        return -1;
    }

    //Search current argument in options
    *option_index = 0;
    const char *optionName = NULL;
    while (((optionName = options[*option_index].name) != NULL) && (option_res == 0)) {
        if (strcmp(options[*option_index].name, argv[*arg_index]) == 0) {
            // printf("found %s with %c\n", options[*option_index].name, options[*option_index].val);
            option_res = options[*option_index].val;
            if (options[*option_index].has_arg) {
                if (*arg_index + 1 >= argc) {
                    printf("error missing value for command %s\n", options[*option_index].name);
                    options[*option_index].strval = NULL;
                } else {
                    (*arg_index)++;
                    options[*option_index].strval = argv[*arg_index];
                }
            }
        } else {
            (*option_index)++;
        }
    }
    
    (*arg_index)++;
    
    return option_res;
}

void cmd_parser_print_cmd(const cmd_parser_option_t *options) {
    printf("Accepted commands are: ");
    const char *optionName = NULL;
    int option_index = 0;
    while ((optionName = options[option_index].name) != NULL) {
        if (option_index > 0) printf(", ");
        printf("%s(%c) %s", optionName, options[option_index].val, options[option_index].has_arg ? "<value>" : "");
        option_index++;
    }
     printf("\n");
}