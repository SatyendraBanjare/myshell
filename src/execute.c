#include "execute.h"

void update_context(struct context_data *context) {
    getcwd(context->cur_dir, sizeof(context->cur_dir));
}

int execute_cd(int argc, char** argv,struct context_data *context) {
    if (argc == 1) {
        update_context(context);
        return 0;
    }

    if (chdir(argv[1]) == 0) {
        update_context(context);
        return 0;
    } else {
        printf(" Myshell : cd %s: No such file or directory\n", argv[1]);
        return 0;
    }
}

int execute_jobs(int argc, char **argv,struct context_data *context) {
    int i;

    for (i = 0; i < MAX_JOBS; i++) {
        if (context->jobs[i] != NULL) {
            show_job_status(i,context);
        }
    }

    return 0;
}

int execute_fg(int argc, char **argv,struct context_data* context) {
    if (argc < 2) {
        printf("usage: fg <pid>\n");
        return -1;
    }

    int status;
    pid_t pid;
    int job_id = -1;

    if (argv[1][0] == '%') {
        job_id = atoi(argv[1] + 1);
        pid = get_pgid_by_job_id(job_id,context);
        if (pid < 0) {
            printf("Myshell : fg %s: no such job\n", argv[1]);
            return -1;
        }
    } else {
        pid = atoi(argv[1]);
    }

    if (kill(-pid, SIGCONT) < 0) {
        printf("Myshell : fg %d: job not found\n", pid);
        return -1;
    }

    tcsetpgrp(0, pid);

    if (job_id > 0) {
        set_job_status(job_id, STATUS_CONTINUED,context);
        show_job_status(job_id,context);
        if (wait_for_job(job_id,context) >= 0) {
            remove_job(job_id,context);
        }
    } else {
        wait_for_pid(pid,context);
    }

    signal(SIGTTOU, SIG_IGN);
    tcsetpgrp(0, getpid());
    signal(SIGTTOU, SIG_DFL);

    return 0;
}

int execute_bg(int argc, char **argv,struct context_data* context) {
    if (argc < 2) {
        printf("usage: bg <pid>\n");
        return -1;
    }

    pid_t pid;
    int job_id = -1;

    if (argv[1][0] == '%') {
        job_id = atoi(argv[1] + 1);
        pid = get_pgid_by_job_id(job_id,context);
        if (pid < 0) {
            printf("execute: bg %s: no such job\n", argv[1]);
            return -1;
        }
    } else {
        pid = atoi(argv[1]);
    }

    if (kill(-pid, SIGCONT) < 0) {
        printf("execute: bg %d: job not found\n", pid);
        return -1;
    }

    if (job_id > 0) {
        set_job_status(job_id, STATUS_CONTINUED,context);
        show_job_status(job_id,context);
    }

    return 0;
}

int execute_kill(int argc, char **argv,struct context_data* context) {
    if (argc < 2) {
        printf("usage: kill <pid>\n");
        return -1;
    }

    pid_t pid;
    int job_id = -1;

    if (argv[1][0] == '%') {
        job_id = atoi(argv[1] + 1);
        pid = get_pgid_by_job_id(job_id,context);
        if (pid < 0) {
            printf("Myshell : kill %s: no such job\n", argv[1]);
            return -1;
        }
        pid = -pid;
    } else {
        pid = atoi(argv[1]);
    }

    if (kill(pid, SIGKILL) < 0) {
        printf("Myshell : kill %d: job not found\n", pid);
        return 0;
    }

    if (job_id > 0) {
        set_job_status(job_id, STATUS_TERMINATED,context);
        show_job_status(job_id,context);
        if (wait_for_job(job_id,context) >= 0) {
            remove_job(job_id,context);
        }
    }

    return 1;
}

int execute_export(int argc, char **argv) {
    if (argc < 2) {
        printf("usage: export KEY=VALUE\n");
        return -1;
    }

    return putenv(argv[1]);
}

int execute_unset(int argc, char **argv) {
    if (argc < 2) {
        printf("usage: unset KEY\n");
        return -1;
    }

    return unsetenv(argv[1]);
}

void check_process_state(struct context_data* context) {
    int status, pid;
    while ((pid = waitpid(-1, &status, WNOHANG|WUNTRACED|WCONTINUED)) > 0) {
        if (WIFEXITED(status)) {
            set_process_status(pid, STATUS_DONE,context);
        } else if (WIFSTOPPED(status)) {
            set_process_status(pid, STATUS_SUSPENDED,context);
        } else if (WIFCONTINUED(status)) {
            set_process_status(pid, STATUS_CONTINUED,context);
        }

        int job_id = get_job_id_by_process_id(pid,context);
        if (job_id > 0 && is_job_completed(job_id,context)) {
            show_job_status(job_id,context);
            remove_job(job_id,context);
        }
    }
}

int execute_Myshell_command(struct process *proc,struct context_data* context) {
    int status = 1;

    switch (proc->type) {
        case CMD_EXIT:
            exit_shell();
            break;
        case CMD_CD:
            execute_cd(proc->command_count, proc->command_list,context);
            break;
        case CMD_JOBS:
            execute_jobs(proc->command_count, proc->command_list,context);
            break;
        case CMD_FG:
            execute_fg(proc->command_count, proc->command_list,context);
            break;
        case CMD_BG:
            execute_bg(proc->command_count, proc->command_list,context);
            break;
        case CMD_KILL:
            execute_kill(proc->command_count, proc->command_list,context);
            break;
        case CMD_EXPORT:
            execute_export(proc->command_count, proc->command_list);
            break;
        case CMD_UNSET:
            execute_unset(proc->command_count, proc->command_list);
            break;
        case CMD_PRINT_HELP:
            display_help();
            break;
        default:
            status = 0;
            break;
    }

    return status;
}
