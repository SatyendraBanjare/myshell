#ifndef EXECUTE_H
#define EXECUTE_H

#include "common.h"
#include "utils.h"

void update_context(struct context_data *context);
int execute_cd(int argc, char** argv,struct context_data *context);
int execute_jobs(int argc, char **argv,struct context_data *context);
int execute_fg(int argc, char **argv,struct context_data* context);
int execute_bg(int argc, char **argv,struct context_data* context);
int execute_kill(int argc, char **argv,struct context_data* context);
int execute_export(int argc, char **argv);
int execute_unset(int argc, char **argv);
void check_process_state(struct context_data* context);
int execute_Myshell_command(struct process *proc,struct context_data* context);

#endif
