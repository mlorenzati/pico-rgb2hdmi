/**
 *  Initial tests of the command parser api
 */
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "math.h"
#include <stdlib.h>

//System configuration includes
#include "cmdParser.h"
#include "version.h"
#include "common_configs.h"

const uint LED_PIN = PICO_DEFAULT_LED_PIN;
bool blink = true;

cmd_parser_option_t options[] =
{
    {"up",    TRUE, NULL,  'u'},
    {"down",  TRUE, NULL,  'd'},
    {"left",  TRUE, NULL,  'l'},
	{"right", TRUE, NULL,  'r'},
	{"key",   TRUE, NULL,  'k'}, 
    {NULL,    FALSE, NULL,  0 }
};


void parse_command(int argc, char *const argv[]) {
    int option_result = 0;
    int option_index = 0;
    int arg_index = 0;
    char *strValue = NULL;
    char pixels = 0;

    while ((option_result = cmd_parser_get_cmd(argc, argv, options, &option_index, &arg_index)) != -1) {
        strValue = options[option_index].strval;
        printf ("Request %s<%c>(%s)\n", options[option_index].name, option_result, strValue);

        switch (option_result) {
            case 'u':
                pixels = atoi(strValue);
                printf ("Move screen up %d positions\n", pixels);
                break;
            case 'd': 
                pixels = atoi(strValue);
                printf ("Move screen down %d positions\n", pixels);
                break;
            case 'l':
                pixels = atoi(strValue);
                printf ("Move screen left %d positions\n", pixels);
                break;
            case 'r':
                pixels = atoi(strValue);
                printf ("Move screen right %d positions\n", pixels);
                break;
            default:  printf("Unknown command: "); cmd_parser_print_cmd(options); break;
         }
    }
}

int main() {
    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
  
	printf("%s commandParser Test - version %s started!\n", PROJECT_NAME, PROJECT_VER);
    char inputStr[128];
    char *argv[16];
    int argc;
    for (int i = 10; i > 0; i--) {
        sleep_ms(1000);
        printf("%d\n", i);
    }

    while (true) {
        gets(inputStr);
        cmd_parser_get_argv_argc(inputStr, &argc, argv);
        parse_command(argc, argv);
    }
    __builtin_unreachable();
}