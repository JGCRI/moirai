//***********
//run_carbon.c

#include "moirai.h"
#include <stdio.h>
#include <stdbool.h>

char false_string[6];
char true_string[5];
bool carbon_enabled_bool;

bool run_carbon(args_struct in_args) { 
    if (in_args.carbon_enabled == true) {
        fprintf(fplog, "carbon_enabled_bool in input structure is set to enabled with value to: %d\n", in_args.carbon_enabled);
        carbon_enabled_bool = true;
        strcpy(true_string, "true");
        } 
    else {
        fprintf(fplog, "carbon_enabled_bool in input structure is set to disabled with value %d\n", in_args.carbon_enabled);
        carbon_enabled_bool = false;
        strcpy(false_string, "false");
        }
    fprintf(fplog, "carbon_enabled_bool variable value in run_carbon.c is: %s %s\n", false_string, true_string); // can comment out if not debugging
}
