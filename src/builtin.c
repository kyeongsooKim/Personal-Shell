#include "builtin.h"


void builtin_help(){

    #ifdef DEBUG
    printf("\nbuiltin command \"help\" executed.\n");
    #endif

    printf("\n");
    printf("Welcome to Kyeongsoo's private Shell.\n");
    printf("These shell commands are all builtins you can use in this program.\n");
    printf("cd [-L|[-P [-e]] [-@]] [dir]\n");
    printf("exit [n]\n");
    printf("help [-dms] [pattern ...] \n");
    printf("pwd [-LP]\n");
    printf("jobs [-LP]\n");
    printf("fg %%JID\n");
    printf("kill [%%|pid]\n");
    printf("&operation \n");
    printf("\n");

    fflush(stdout);

}

void builtin_exit()
{

    #ifdef DEBUG
    printf("\nbuiltin command \"exit\" executed.\n");
    #endif

    exit(EXIT_SUCCESS);
}

int builtin_cd(int argc, char** argv) {
    if (argc == 1) {
        chdir(shell->home_dir);
        getcwd(shell->cur_dir, sizeof(shell->cur_dir));
        return 0;
    }


    if (strcmp(argv[1] ,"-") == 0)
    {
        chdir(shell->last_dir);
        getcwd(shell->cur_dir, sizeof(shell->cur_dir));
        return 0;
    }

    if (chdir(argv[1]) == 0) {
        getcwd(shell->cur_dir, sizeof(shell->cur_dir));
        return 0;
    }
    else {
        printf(BUILTIN_ERROR, argv[1]);
        return 0;
    }
}

void builtin_color(int argc, char** argv)
{

    if (argc == 1) {
        //do nothing
        return;
    }

    #ifndef COLOR
        #define COLOR //define COLOR for using predefined color code in "debug.h"
    #endif
    char color[20] = { 0 };

    if (strcmp(argv[1] , "RED") == 0)
        strcpy(color, RED);
    else if (strcmp(argv[1] , "GRN") == 0)
        strcpy(color, GRN);
    else if (strcmp(argv[1] , "YEL") == 0)
        strcpy(color, YEL);
    else if (strcmp(argv[1] , "BLU") == 0)
        strcpy(color, BLU);
    else if (strcmp(argv[1] , "MAG") == 0)
        strcpy(color, MAG);
    else if (strcmp(argv[1] , "CYN") == 0)
        strcpy(color, CYN);
    else if (strcmp(argv[1] , "WHT") == 0)
        strcpy(color, WHT);
    else if (strcmp(argv[1] , "BWN") == 0)
        strcpy(color, BWN);
    else
        return;

    memset(prompt,'\0', 1024); //space for saving prompt str
    strcpy(prompt, color);
    strcat(prompt, "%s :: kyeokim >> ");
    strcat(prompt, NRM);

    //for right prompt
    //memset(prompt_color,'\0', strlen(prompt_color));
    //strcpy(prompt_color, color);


    #ifdef DEBUG
        fprintf(stderr,"\nchanged prompt : %s\n", prompt);
    #endif
}



void builtin_pwd()
{
    #ifdef DEBUG
    printf("\nbuiltin command \"pwd\" executed.\n");
    #endif

    char path[1024] = { 0 };
    strcpy(path, getcwd(NULL, 0));
    fprintf(stdout, "%s\n", path);
    fflush(stdout);
}

int builtin_jobs(int argc, char **argv)
{
    for (int i = 0; i < JOBS_MAX_NUM; i++) {
        if (shell->jobs[i] != NULL) {
            builtin_jobs_util(i);
        }
    }

    return 0;
}

int builtin_jobs_util(int jid) //jid = job id
{

    //If there are no jobs, don't print anything
    if (jid > JOBS_MAX_NUM || shell->jobs[jid] == NULL) {
        return -1;
    }

    struct process *proc;

    for (proc = shell->jobs[jid]->root; proc != NULL; proc = proc->next)
    {
            #ifdef DEBUG
                printf("[%d]\t%d%s\n", jid, proc->pid, proc->command);
            #else
                printf(JOBS_LIST_ITEM, jid, proc->command);
            #endif
    }

    return 0;
}

int builtin_fg(int argc, char **argv) {


    if (argc < 2) {
        printf(BUILTIN_ERROR, "wrong usage");
        return -1;
    }

    pid_t pid;
    int jid = -1;

    if (argv[1][0] == '%') //find through job id
    {
        jid = atoi(argv[1] + 1);
        pid = get_pgid_from_jid(jid);
        if (pid < 0) {
            printf(BUILTIN_ERROR, "no such job");
            return -1;
        }
    }
    else //find through pid
    {
        pid = atoi(argv[1]);
    }

    if (kill(-pid, SIGCONT) < 0) {
        printf(BUILTIN_ERROR, "job not found");
        return -1;
    }

    tcsetpgrp(0, pid);

    if (jid > 0) {
        set_job_status(jid, CONTINUED);
        if (wait_jid(jid) >= 0) {
            delete_job(jid);
        }
    } else {
        wait_pid(pid);
    }

    signal(SIGTTOU, SIG_IGN);
    tcsetpgrp(0, getpid());
    signal(SIGTTOU, SIG_DFL);

    return 0;
}


int builtin_bg(int argc, char **argv) {
    if (argc < 2) {
        printf(BUILTIN_ERROR, "wrong usage");
        return -1;
    }

    pid_t pid;
    int jid = -1;

    if (argv[1][0] == '%') {
        jid = atoi(argv[1] + 1);
        pid = get_pgid_from_jid(jid);
        if (pid < 0) {
            printf(BUILTIN_ERROR, "no such job");
            return -1;
        }
    } else {
        pid = atoi(argv[1]);
    }

    if (kill(-pid, SIGCONT) < 0) {
        printf(BUILTIN_ERROR, "job not found");
        return -1;
    }

    if (jid > 0) {
        set_job_status(jid, CONTINUED);
        builtin_jobs_util(jid);
    }

    return 0;
}


int builtin_kill(int argc, char **argv) {
    if (argc < 2) {
        printf(BUILTIN_ERROR, "wrong usage");
        return -1;
    }

    pid_t pid;
    int jid = -1;

    if (argv[1][0] == '%') //get pid by jid
    {
        jid = atoi(argv[1] + 1);
        pid = get_pgid_from_jid(jid);
        if (pid < 0) {
            printf(BUILTIN_ERROR, "no such job");
            return -1;
        }

        #ifdef DEBUG
            printf("\ntarget pid : %d\n", pid);
        #endif
    }
    else // get pid
    {
        pid = atoi(argv[1]);
        #ifdef DEBUG
            printf("\ntarget pid : %d\n", pid);
        #endif
        jid = get_jid_from_pid(pid);
    }


    if (kill(pid, SIGKILL) < 0)
    {
        printf(BUILTIN_ERROR, "jon not found");
        return 0;
    }


    if (jid > 0)
    {

        #ifdef DEBUG
            printf("\nterminate process [%d]\t %d\n", jid,pid);
        #endif

        set_job_status(jid, TERMINATED);
        if (wait_jid(jid) >= 0) {
            delete_job(jid);
        }
    }

    return 1;
}