#ifndef LAUNCH_H
#define LAUNCH_H

#include "common.h"
#include "utils.h"
#include "execute.h"

int launch_process(struct job *job, struct process *proc, int in_fd, int out_fd, int mode,struct context_data* context);
int launch_job(struct job *job, struct context_data *context);

#endif