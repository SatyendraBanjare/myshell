#ifndef UTILS_H
#define UTILS_H

#include "common.h"

int get_job_id_by_process_id(int pid, struct context_data *context);
int get_pgid_by_job_id(int id,struct context_data *context);
int process_count(int job_id,struct context_data *context);
int append_job(struct job *job,struct context_data *context);
int remove_job(int job_id,struct context_data *context);
int is_job_completed(int job_id,struct context_data *context);
int set_process_status(int pid, int status,struct context_data *context);
int set_job_status(int id, int status,struct context_data *context);

int wait_for_pid(int pid,struct context_data* context);
int wait_for_job(int id,struct context_data *context);

struct process* parse_command_segment(char *segment);
struct job* parse_command(char *line);

#endif
