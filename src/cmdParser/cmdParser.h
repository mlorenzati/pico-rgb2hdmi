#ifndef CMD_PARSER
#define CMD_PARSER

#define TRUE  1 
#define FALSE 0 

typedef struct 
{
    const char *name;      /* the name of the long cmd_parser_option */
    char        has_arg;   /* Has a following argument */
    char       *strval;    /* If has argument, return here the string */
    const char  val;	   /* short cmd_parser_option */
} cmd_parser_option_t;

void cmd_parser_get_argv_argc(char *str, int *argc, char *argv[]);
int  cmd_parser_get_cmd (int argc, char *const argv[], cmd_parser_option_t * cmd_parser_options, int *cmd_parser_option_index, int *arg_index);
void cmd_parser_print_cmd(const cmd_parser_option_t *cmd_parser_options);
#endif