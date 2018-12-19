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
    int command_count;
    char **command_list;
    char *input_path;
    char *output_path;
    pid_t pid;
    int type;
    int status;
    struct process *next;
};

// A job is a collection of multiple process linked together.
struct job {
    // first process
    struct process *root;

    int id;          // The Job ID.
    char *command;   // Command for the particular Job. 
    pid_t pgid;      // The Process Group ID
    int mode;    // The job mode either FG, BG or Pipe
};

// The current context data.
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
int exit_shell();

#endif
