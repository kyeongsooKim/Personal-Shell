
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>

#include <stdbool.h>
#include <readline/readline.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "job.h"
#include "builtin.h"
#include "sfish.h"
#include "parse.h"


char cur_dir_shortcut[PATH_MAX_SIZE]; //save special shortcut prompt considering home_dir as "~"




void dir_init()
{
     //initial home-directory
    char * pwd = getcwd(NULL, 0); //save current path into pwd variable
    memset(shell->home_dir, 0, PATH_MAX_SIZE);
    strcpy(shell->home_dir, getenv("HOME"));
    shell->homeSize = strlen(shell->home_dir); //save the size of string of home for future use.
    memcpy(shell->last_dir, pwd, PATH_MAX_SIZE);
    memcpy(shell->cur_dir, shell->last_dir, PATH_MAX_SIZE);
    chdir(shell->cur_dir);

}

void show_right_prompt()
{

        time_t rawtime;
        struct tm * timeInfo;
        char buffer [80];
        time (&rawtime);
        timeInfo = localtime (&rawtime);
        strftime (buffer,80,STRFTIME_RPRMT,timeInfo);

        //calculates window size
        struct winsize terminal;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &terminal);
        int column = terminal.ws_col;
        if (ctrlC == true)
        {
            column = column - strlen(buffer);
        }
        else
            column = column - strlen(buffer);
        ctrlC = false;


        char tmpSize[80] = {0};
        //strcpy(tmpSize, prompt_color);
        strcpy(tmpSize, "\e[");
        sprintf(tmpSize, "%s%d", tmpSize,column);
        strcat(tmpSize,"C");
        strcat(right_prompt,"\e[s");
        strcat(right_prompt,tmpSize);
        strcat(right_prompt,buffer);
        strcat(right_prompt,"\e[u");
       // strcat(right_prompt, NRM);
        printf("%s",right_prompt);
}


void create_shell() {

    struct sigaction sigint_action = {
        .sa_handler = &sigint_handler,
        .sa_flags = 0
    };
    sigemptyset(&sigint_action.sa_mask);
    sigaction(SIGINT, &sigint_action, NULL);

    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);

    pid_t pid = getpid();
    setpgid(pid, pid);
    tcsetpgrp(0, pid);

    shell = (Shell *) malloc(sizeof(Shell));
    memset(right_prompt, 0 , strlen(right_prompt));
    memset(prompt_color,'\0', strlen(prompt_color));
    strcpy(prompt_color, NRM);

    for (int i = 0; i < JOBS_MAX_NUM; i++) {
        shell->jobs[i] = NULL;
    }


}

int main(int argc, char **argv,char* envp[]) {

    if(!isatty(STDIN_FILENO)) {
        // If your shell is reading from a piped file
        // Don't have readline write anything to that file.
        // Such as the prompt or "user input"
        if((rl_outstream = fopen("/dev/null", "w")) == NULL){
            perror("Failed trying to open DEVNULL");
            exit(EXIT_FAILURE);
        }
    }

    create_shell(); // create shell and initialize it.
    dir_init();

    char *input;
    job * job;
    ctrlC = false;

    char prompt_output[PATH_MAX_SIZE] = { 0 }; //for prompt format in readline() func
    memset(prompt,'\0', PATH_MAX_SIZE); //space for saving prompt str
    strcpy (prompt, "%s :: kyeokim >> ");
    do{
        //setup left prompt display
        if (strncmp(shell->cur_dir, shell->home_dir, shell->homeSize) == 0)
        {
            memset(cur_dir_shortcut, 0, PATH_MAX_SIZE);
            strcpy(cur_dir_shortcut, "~");
            memcpy(cur_dir_shortcut + 1, shell->cur_dir + shell->homeSize, PATH_MAX_SIZE);
            memset(prompt_output, 0, PATH_MAX_SIZE);
            sprintf(prompt_output, prompt, cur_dir_shortcut);
        }
        else
            sprintf(prompt_output, prompt, shell->cur_dir);

        show_right_prompt();
        input = readline(prompt_output);

        if (input == NULL)
        {
            check_suspended_process();
            continue;
        }

        if(strcmp(input,"") != 0) //blank line, do nothing.
        {
            job = parse_command(input);
            execute_job(job);
        }

    }while(1);



    return EXIT_SUCCESS;
}
