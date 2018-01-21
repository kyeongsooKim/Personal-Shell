#include "job.h"




int get_jid_from_pid(int pid) {

    process *proc;
    for (int jid = 1; jid <= JOBS_MAX_NUM; jid++)
    {
        if (shell->jobs[jid] != NULL) {
            for (proc = shell->jobs[jid]->root; proc != NULL; proc = proc->next)
            {
                if (proc->pid == pid)
                    return jid;
            }
        }
    }

    return -1;
}

job *  get_job_by_jid(int jid)
{ //jid = unique job id
    if (jid > JOBS_MAX_NUM) {
        return NULL;
    }

    return shell->jobs[jid];
}

int get_pgid_from_jid(int jid) {
    job * job = get_job_by_jid(jid);

    if (job == NULL) {
        return -1;
    }

    return job->pgid;
}

int get_process_number(int jid, int filter)
{
    if (jid > JOBS_MAX_NUM || shell->jobs[jid] == NULL) {
        return -1;
    }

    int count = 0;
    process *proc;
    for (proc = shell->jobs[jid]->root; proc != NULL; proc = proc->next) {
        if (filter == FIND_ALL_PROCESS ||
            (filter == FIND_DONE_PROCESS && proc->status == DONE) ||
            (filter == FIND_REMAINING_PROCESS && proc->status != DONE)) {
            count++;
        }
    }

    return count;
}

int get_next_jid() {
    //jid = unique job id
    for (int jid = 1; jid <= JOBS_MAX_NUM; jid++) {
        if (shell->jobs[jid] == NULL) {
            return jid;
        }
    }

    return -1;
}

int print_processes_by_jid(int jid)
{
    //jid = unique job id
    if (jid > JOBS_MAX_NUM || shell->jobs[jid] == NULL) {
        return -1;
    }

    printf("[%d]", jid);

    struct process *proc;
    for (proc = shell->jobs[jid]->root; proc != NULL; proc = proc->next) {
        printf(" %d", proc->pid);
    }
    printf("\n");

    return 0;
}

int release_job(int jid)
{
    //jid = unique job id
    if (jid > JOBS_MAX_NUM || shell->jobs[jid] == NULL) {
        return -1;
    }

    job *job = shell->jobs[jid];
    process *proc, *tmp;
    for (proc = job->root; proc != NULL; ) {
        tmp = proc->next;
        free(proc->command);
        free(proc->argv);
        free(proc->input_path);
        free(proc->output_path);
        free(proc);
        proc = tmp;
    }

    free(job->command);
    free(job);

    return 0;
}

int add_job(job * job)
{
    //jid = unique job id
    int jid = get_next_jid();

    if (jid < 0) {
        return -1;
    }

    job->jid = jid;
    shell->jobs[jid] = job;
    return jid;
}

int delete_job(int jid)
{
    //jid = unique job id
    if (jid > JOBS_MAX_NUM || shell->jobs[jid] == NULL) {
        return -1;
    }

    release_job(jid);
    shell->jobs[jid] = NULL;

    return 0;
}

int heck_completed_process(int jid)
{
    //jid = unique job id
    if (jid > JOBS_MAX_NUM || shell->jobs[jid] == NULL) {
        return 0;
    }

    process *proc;
    for (proc = shell->jobs[jid]->root; proc != NULL; proc = proc->next)
    {
        if (proc->status != DONE) {
            return 0;
        }
    }

    return 1;
}

int set_process_status(int pid, int status) {

    process *proc;

    for (int jid = 1; jid <= JOBS_MAX_NUM; jid++)
    {
        if (shell->jobs[jid] == NULL) {
            continue;
        }
        for (proc = shell->jobs[jid]->root; proc != NULL; proc = proc->next)
        {
            if (proc->pid == pid) {
                proc->status = status;
                return 0;
            }
        }
    }

    return -1;
}

int set_job_status(int jid, int status) {

    //jid = unique job id
    if (jid > JOBS_MAX_NUM || shell->jobs[jid] == NULL)
    {
        return -1;
    }

    process *proc;

    for (proc = shell->jobs[jid]->root; proc != NULL; proc = proc->next) {
        if (proc->status != DONE) {
            proc->status = status;
        }
    }

    return 0;
}

int wait_pid(int pid)
{
    int status = 0;

    waitpid(pid, &status, WUNTRACED);
    if (WIFEXITED(status)) {
        set_process_status(pid, DONE);
    } else if (WIFSIGNALED(status)) {
        set_process_status(pid, TERMINATED);
    } else if (WSTOPSIG(status)) {
        status = -1;
        set_process_status(pid, SUSPENDED);
    }

    return status;
}

int wait_jid(int jid)
{
    //jid = unique job id
    if (jid > JOBS_MAX_NUM || shell->jobs[jid] == NULL) {
        return -1;
    }

    int process_num = get_process_number(jid, FIND_REMAINING_PROCESS);
    int wait_pid = -1, wait_count = 0;
    int status = 0;

    do {
        wait_pid = waitpid(-shell->jobs[jid]->pgid, &status, WUNTRACED);
        wait_count++;

        if (WIFEXITED(status)) {
            set_process_status(wait_pid, DONE);
        } else if (WIFSIGNALED(status)) {
            set_process_status(wait_pid, TERMINATED);
        } else if (WSTOPSIG(status)) {
            status = -1;
            set_process_status(wait_pid, SUSPENDED);
            if (wait_count == process_num) {
                builtin_jobs_util(jid);
            }
        }
    } while (wait_count < process_num);

    return status;
}


int execute_process(job * job, process *proc, int fd0, int fd1, int mode)
{

    proc->status = RUNNING; //execute the process.

    if(proc->type != EXECUTABLE && proc->isRedirected == false)
    {
        execute_builtin(proc);
        return 0;
    }


    pid_t childpid;
    int status = 0;

    childpid = fork();

    if (childpid < 0) {
        return -1;
    }
    else if (childpid == 0) //execute in shell
    {
        signal(SIGINT, SIG_DFL); signal(SIGQUIT, SIG_DFL); signal(SIGTSTP, SIG_DFL);
        signal(SIGTTIN, SIG_DFL); signal(SIGTTOU, SIG_DFL); signal(SIGCHLD, SIG_DFL);

        proc->pid = getpid();
        if (job->pgid > 0) {
            setpgid(0, job->pgid);
        } else {
            job->pgid = proc->pid;
            setpgid(0, job->pgid);
        }

        if (fd0 != 0) {
            dup2(fd0, 0);
            close(fd0);
        }

        if (fd1 != 1) {
            dup2(fd1, 1);
            close(fd1);
        }

        if (proc->type != EXECUTABLE) //run builtin in pipe and redirection too.
        {

            execute_builtin(proc);
            exit(0);
        }
        else if (execvp(proc->argv[0], proc->argv) < 0)
        {
            printf(EXEC_NOT_FOUND, proc->argv[0]);
            exit(0);
        }

        exit(0);
    }
    else //parent (shell)
    {
        proc->pid = childpid;
        if (job->pgid > 0) {
            setpgid(childpid, job->pgid);
        } else {
            job->pgid = proc->pid;
            setpgid(childpid, job->pgid);
        }

        if (mode == FOREGROUND) {
            tcsetpgrp(0, job->pgid);
            status = wait_jid(job->jid);
            signal(SIGTTOU, SIG_IGN);
            tcsetpgrp(0, getpid());
            signal(SIGTTOU, SIG_DFL);
        }
    }

    return status;
}

int execute_builtin(process *proc)
{
    int status = 1;

    switch (proc->type) {
        case BUILTIN_EXIT:
            builtin_exit();
            break;
        case BUILTIN_CD:
            builtin_cd(proc->argc, proc->argv);
            break;
        case BUILTIN_HELP:
            builtin_help();
            break;
        case BUILTIN_PWD:
            builtin_pwd();
            break;
        case BUILTIN_JOBS:
            builtin_jobs(proc->argc, proc->argv);
            break;
        case BUILTIN_FG:
            builtin_fg(proc->argc, proc->argv);
            break;
        case BUILTIN_BG:
            builtin_bg(proc->argc, proc->argv);
            break;
        case BUILTIN_KILL:
            builtin_kill(proc->argc, proc->argv);
            break;
        case BUILTIN_COLOR :
            builtin_color(proc->argc, proc->argv);
            break;
        default:
            status = 0; //executable.
            break;
    }

    return status;
}



int execute_job(job * job) {
    process *proc;
    int status = 0, fd0 = 0, fd[2], jid = -1;

    check_suspended_process();
    if (job->root->type == EXECUTABLE) {
        jid = add_job(job);
    }

    for (proc = job->root; proc != NULL; proc = proc->next) {

        if (proc == job->root && proc->input_path != NULL) //redirection handling
        {
            fd0 = open(proc->input_path, O_RDONLY);
            if (fd0 < 0) {
                printf(EXEC_ERROR,"sfish: no such file or directory");
                delete_job(jid);
                return -1;
            }
        }
        if (proc->next != NULL) //pipeline handling
        {
            pipe(fd);
            status = execute_process(job, proc, fd0, fd[1], PIPELINE);
            close(fd[1]);
            fd0 = fd[0];
        }
        else //redirection handling
        {
            int fd1 = 1;
            if (proc->output_path != NULL) {
                fd1 = open(proc->output_path, O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
                if (fd1 < 0) {
                    fd1 = 1;
                }
            }
            status = execute_process(job, proc, fd0, fd1, job->mode);
        }
    }

    if (job->root->type == EXECUTABLE) {
        if (status >= 0 && job->mode == FOREGROUND) {
            delete_job(jid);
        } else if (job->mode == BACKGROUND) {
            print_processes_by_jid(jid);
        }
    }

    return status;
}


void check_suspended_process()
{
    int status, pid;
    while ((pid = waitpid(-1, &status, WNOHANG|WUNTRACED|WCONTINUED)) > 0)
    {
        if (WIFEXITED(status))
        {
            set_process_status(pid, DONE);
        } else if (WIFSTOPPED(status)) {
            set_process_status(pid, SUSPENDED);
        } else if (WIFCONTINUED(status)) {
            set_process_status(pid, CONTINUED);
        }

        int jid = get_jid_from_pid(pid);
        if (jid > 0 && heck_completed_process(jid))
        {
            builtin_jobs_util(jid);
            delete_job(jid);
        }
    }
}

