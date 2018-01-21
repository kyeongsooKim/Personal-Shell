#ifndef BUILTIN_H
#define BUILTIN_H

#include <string.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <readline/readline.h>
#include <fcntl.h>
#include <stdbool.h>

#include "job.h"


#define EXECUTABLE 0
#define BUILTIN_EXIT 1
#define BUILTIN_CD 2
#define BUILTIN_HELP 3
#define BUILTIN_PWD 4
#define BUILTIN_JOBS 5
#define BUILTIN_FG 6
#define BUILTIN_BG 7
#define BUILTIN_KILL 8
#define BUILTIN_COLOR 9

#define NRM "\033[0m"
#define RED "\033[1;31m"
#define GRN "\033[1;32m"
#define YEL "\033[1;33m"
#define BLU "\033[1;34m"
#define MAG "\033[1;35m"
#define CYN "\033[1;36m"
#define WHT "\033[1;37m"
#define BWN "\033[0;33m"


int builtin_cd(int argc, char** argv);
void builtin_help();
void builtin_pwd();
void builtin_exit();
void builtin_color(int argc, char** argv);
int builtin_jobs(int argc, char **argv);
int builtin_fg(int argc, char **argv);
int builtin_bg(int argc, char **argv);
int builtin_kill(int argc, char **argv);
int builtin_jobs_util(int jid); //jid = unique job id

char prompt[1024]; //whole prompt string line
char right_prompt[1024]; //right prompt for extra credits.
char prompt_color[20]; //color of prompt

#endif

