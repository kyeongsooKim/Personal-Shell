#ifndef JOB_H
#define JOB_H

#include <stdbool.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <wait.h>
#include <unistd.h>

#include "builtin.h"
#include "sfish.h"

#define JOBS_MAX_NUM 30
#define PATH_MAX_SIZE 1024

#define RUNNING 0
#define DONE 1
#define SUSPENDED 2
#define CONTINUED 3
#define TERMINATED 4

#define FIND_ALL_PROCESS 0
#define FIND_DONE_PROCESS 1
#define FIND_REMAINING_PROCESS 2

#define BACKGROUND 0
#define FOREGROUND 1
#define PIPELINE 2

typedef struct job {
    int jid; //unique job id
    struct process *root;
    char *command;
    pid_t pgid;
    int mode;
} job;

typedef struct process {
    char *command;
    bool isPiped;
    bool isRedirected;
    int argc;
    char **argv;
    char *input_path;
    char *output_path;
    pid_t pid;
    int type;
    int status;
    struct process *next;
} process;

typedef struct shell_ {

    int homeSize; //length of home directory
    char home_dir[PATH_MAX_SIZE];
    char cur_dir[PATH_MAX_SIZE];
    char last_dir[PATH_MAX_SIZE];
    struct job *jobs[JOBS_MAX_NUM + 1];
} Shell;

Shell * shell;

int execute_job(job *job);
int execute_process(job * job, process *proc, int fd0, int fd1, int mode);
int execute_builtin(process * proc);
void check_suspended_process();
int check_completed_process(int jid);

int add_job(struct job *job);
int delete_job(int id);
int release_job(int id);

int print_process_by_jid(int jid);
int get_pgid_from_jid(int jid);
int get_jid_from_pid(int pid);
int get_next_jid();
job * get_job_by_jid(int jid);
int get_process_number(int id, int find);

int set_process_status(int pid, int status);
int set_job_status(int jid, int status);

int wait_pid(int pid);
int wait_jid(int jid);
#endif