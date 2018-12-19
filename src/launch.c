#include "launch.h"

int launch_process(struct job *job, struct process *proc, int in_fd, int out_fd, int mode,struct context_data* context) {
    proc->status = STATUS_RUNNING;
    if (proc->type != CMD_EXTERNAL && execute_Myshell_command(proc,context)) {
        return 0;
    }

    pid_t childpid;
    int status = 0;

    childpid = fork();

    if (childpid < 0) {
        return -1;
    } else if (childpid == 0) {
        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        signal(SIGTTOU, SIG_DFL);
        signal(SIGCHLD, SIG_DFL);

        proc->pid = getpid();
        if (job->pgid > 0) {
            setpgid(0, job->pgid);
        } else {
            job->pgid = proc->pid;
            setpgid(0, job->pgid);
        }

        if (in_fd != 0) {
            dup2(in_fd, 0);
            close(in_fd);
        }

        if (out_fd != 1) {
            dup2(out_fd, 1);
            close(out_fd);
        }

        if (execvp(proc->command_list[0], proc->command_list) < 0) {
            printf("Myshell : %s: command not found\n", proc->command_list[0]);
            exit(0);
        }

        exit(0);
    } else {
        proc->pid = childpid;
        if (job->pgid > 0) {
            setpgid(childpid, job->pgid);
        } else {
            job->pgid = proc->pid;
            setpgid(childpid, job->pgid);
        }

        if (mode == FG_EXEC) {
            tcsetpgrp(0, job->pgid);
            status = wait_for_job(job->id,context);
            signal(SIGTTOU, SIG_IGN);
            tcsetpgrp(0, getpid());
            signal(SIGTTOU, SIG_DFL);
        }
    }

    return status;
}

int launch_job(struct job *job, struct context_data *context) {
    struct process *proc;
    int status = 0, in_fd = 0, fd[2], job_id = -1;

    check_process_state(context);
    if (job->root->type == CMD_EXTERNAL) {
        job_id = append_job(job,context);
    }

    for (proc = job->root; proc != NULL; proc = proc->next) {
        if (proc == job->root && proc->input_path != NULL) {
            in_fd = open(proc->input_path, O_RDONLY);
            if (in_fd < 0) {
                printf("Myshell : no such file or directory: %s\n", proc->input_path);
                remove_job(job_id,context);
                return -1;
            }
        }
        if (proc->next != NULL) {
            pipe(fd);
            status = launch_process(job, proc, in_fd, fd[1], PIPE_EXEC,context);
            close(fd[1]);
            in_fd = fd[0];
        } else {
            int out_fd = 1;
            if (proc->output_path != NULL) {
                out_fd = open(proc->output_path, O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
                if (out_fd < 0) {
                    out_fd = 1;
                }
            }
            status = launch_process(job, proc, in_fd, out_fd, job->mode,context);
        }
    }

    if (job->root->type == CMD_EXTERNAL) {
        if (status >= 0 && job->mode == FG_EXEC) {
            remove_job(job_id,context);
        } else if (job->mode == BG_EXEC) {
            show_job_status(job_id,context);
        }
    }

    return status;
}



