#ifndef PARSE_H
#define PARSE_H

#include <stdbool.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <wait.h>
#include <unistd.h>
#include <string.h>

#include "job.h"


#define TOKEN_MAX_SIZE 80
#define TOKEN_DELIMITERS " \t\r\n\a"

int get_command_type(char *command);
void sigint_handler(int signal); //for kill and ctrl+ c in this shell.
process * parse_command_util(char * seg);
job* parse_command(char * str);

bool ctrlC;

#endif