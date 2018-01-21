#include "parse.h"
#include "job.h"


int get_command_type(char *command) {
    if (strcmp(command, "exit") == 0) {
        return BUILTIN_EXIT;
    } else if (strcmp(command, "cd") == 0) {
        return BUILTIN_CD;
    } else if (strcmp(command, "jobs") == 0) {
        return BUILTIN_JOBS;
    } else if (strcmp(command, "help") == 0) {
        return BUILTIN_HELP;
    } else if (strcmp(command, "pwd") == 0) {
        return BUILTIN_PWD;
    } else if (strcmp(command, "fg") == 0) {
        return BUILTIN_FG;
    } else if (strcmp(command, "bg") == 0) {
        return BUILTIN_BG;
    } else if (strcmp(command, "kill") == 0) {
        return BUILTIN_KILL;
    } else if (strcmp(command, "color") == 0){
        return BUILTIN_COLOR;
    } else
        return EXECUTABLE;

}



void sigint_handler(int signal) {
    ctrlC = true;
    printf("\n");
}


job * parse_command(char *str) {

    //deleting white space in both end of str
    char * leftEnd = str;
    char * rightEnd = str + strlen(str);

    while (*leftEnd == ' ') {
        leftEnd++;
    }
    while (*rightEnd == ' ') {
        rightEnd--;
    }
    *(rightEnd + 1) = '\0';

    str = leftEnd;


    //in case when there is no space between redirection character >, < or pipe.
    #ifdef DEBUG
    printf("\nInput String : %s\n",str);
    #endif

    char tmp[2048] = { 0 };
    int j = 0;
    for(int i = 0;  i < strlen(str);i++)
    {
        if (str[i] == '<' || str[i] == '>' || str[i] == '|')
        {
            tmp[j++] = ' ';
            tmp[j++] = str[i];
            tmp[j++] = ' ';
        }
        else
            tmp[j++] = str[i];
    }
    str = tmp;

    #ifdef DEBUG
    printf("\nInput String after putting space : %s\n",str);
    #endif

    char *command = strdup(str);

    process * root_proc = NULL;
    process * proc = NULL;

    char * line_cursor = str;
    char * c = str;
    char * seg;
    int seg_len = 0, mode = FOREGROUND;

    if (str[strlen(str) - 1] == '&')
    {
        mode = BACKGROUND; //extra credits !!
        str[strlen(str) - 1] = '\0';
    }

    while (1)
    {
        if (*c == '\0' || *c == '|')
        {
            seg = (char*) malloc((seg_len + 1) * sizeof(char));
            strncpy(seg, line_cursor, seg_len);
            seg[seg_len] = '\0';

            struct process* new_proc = parse_command_util(seg);
            if (!root_proc) {
                root_proc = new_proc;
                proc = root_proc;
            } else {
                proc->next = new_proc;
                proc = new_proc;
            }

            if (*c != '\0') {
                line_cursor = c;
                while (*(++line_cursor) == ' ');
                c = line_cursor;
                seg_len = 0;
                continue;
            } else {
                break;
            }
        } else {
            seg_len++;
            c++;
        }
    }

    job * new_job = (job * ) malloc(sizeof(job));
    new_job->root = root_proc;
    new_job->command = command;
    new_job->pgid = -1;
    new_job->mode = mode;
    return new_job;
}


process * parse_command_util(char * str)
{

    int bufsize = TOKEN_MAX_SIZE;
    int position = 0;
    char *command = strdup(str);
    char *token;
    char **tokens = (char**) malloc(bufsize * sizeof(char*));

    if (!tokens)
    {
        fprintf(stderr, "sfish: malloc error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(str, TOKEN_DELIMITERS);
    while (token != NULL) {
        if (position >= bufsize)
        {
            bufsize += TOKEN_MAX_SIZE;
            tokens = (char**) realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "sfish: malloc error\n");
                exit(EXIT_FAILURE);
            }
        }
        tokens[position] = token;
        position++;
        token = strtok(NULL, TOKEN_DELIMITERS);
    }



    int i = 0, argc = 0;
    char *input_path = NULL, *output_path = NULL;
    bool redirection = false;
    while (i < position) {
        if (tokens[i][0] == '<' || tokens[i][0] == '>') {

            break;
        }
        i++;
    }
    argc = i;

    for (; i < position; i++) {
        if (tokens[i][0] == '<') {
            if (strlen(tokens[i]) == 1) {

                if(tokens[i+1][0] == '<' || tokens[i+1][0] == '<' )
                    printf(SYNTAX_ERROR, "wrong input");
                input_path = (char *) malloc((strlen(tokens[i + 1]) + 1) * sizeof(char));
                strcpy(input_path, tokens[i + 1]);
                redirection = true;
                i++;
            } else {
                input_path = (char *) malloc(strlen(tokens[i]) * sizeof(char));
                strcpy(input_path, tokens[i] + 1);
            }
        } else if (tokens[i][0] == '>'){
            if (strlen(tokens[i]) == 1) {
                output_path = (char *) malloc((strlen(tokens[i + 1]) + 1) * sizeof(char));
                strcpy(output_path, tokens[i + 1]);
                redirection = true;
                i++;
            } else {
                output_path = (char *) malloc(strlen(tokens[i]) * sizeof(char));
                strcpy(output_path, tokens[i] + 1);
            }
        } else {
            break;
        }
    }

    for (i = argc; i <= position; i++) {
        tokens[i] = NULL;
    }

    struct process *new_proc = (struct process*) malloc(sizeof(struct process));
    new_proc->isRedirected = redirection;
    new_proc->command = command;
    new_proc->argv = tokens;
    new_proc->argc = argc;
    new_proc->input_path = input_path;
    new_proc->output_path = output_path;
    new_proc->pid = -1;
    new_proc->type = get_command_type(tokens[0]);
    new_proc->next = NULL;
    return new_proc;
}