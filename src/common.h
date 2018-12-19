#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>
#include <glob.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "macros.h"
#include "linenoise.h"

struct process {
    char *command;
    int commnd_count;
    char **command_list;
    char *input_path;
    char *output_path;
    pid_t pid;
    int type;
    int status;
    struct process *next;
};

struct job {
    // parent process
    struct process *parent;

    int job_id;
    char *command;
    pid_t pgid;
    int job_mode;
};

struct context_data {
    char cur_user[TOKEN_BUFSIZE];
    char cur_dir[PATH_BUFSIZE];
    struct job *jobs[MAX_JOBS + 1];
};

int show_job_process(int job_id, struct context_data *context);
int show_job_status(int job_id,struct context_data *context);
int get_command_type(char *command);
char* trimmer(char* line);
char* read_input() ;
void display_help();

#endif
