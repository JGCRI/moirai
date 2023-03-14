//***********
//run_carbon.c

#include "moirai.h"
#include <stdio.h>


char false_string[6];
char true_string[5];
int diagnostics;
int carbon_enabled_bool;

int run_carbon(args_struct in_args) { 
    if (in_args.carbon_enabled == 1) {
        carbon_enabled_bool = 1;
        strcpy(true_string, "true");
        if (diagnostics == 1) {
            fprintf(fplog, "carbon_enabled_bool in input structure is set to enabled with value to: %d\n", in_args.carbon_enabled);
            }
        } 
    else {
        carbon_enabled_bool = 0;
        strcpy(false_string, "false");
        if (diagnostics == 1) {
            fprintf(fplog, "carbon_enabled_bool in input structure is set to disabled with value %d\n", in_args.carbon_enabled);
            }
        }
    fprintf(fplog, "Carbon processing is set to: %s %s\n", false_string, true_string);
}
