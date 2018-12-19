#include "utils.h"

// get the process's job id
int get_job_id_by_process_id(int pid, struct context_data *context) {

    struct process *proc;

    for (int i = 1; i <= MAX_JOBS; i++) {
        if (context->jobs[i] != NULL) {
            for (proc = context->jobs[i]->root; proc != NULL; proc = proc->next) {
                if (proc->pid == pid) {
                    return i;
                }
            }
        }
    }

    return -1;
}

// get the process group id by job id
int get_pgid_by_job_id(int id,struct context_data *context) {
    struct job *job = context->jobs[id];

    if (job == NULL) {
        return -1;
    }

    return job->pgid;
}

// get the number of processes linked with a job
int process_count(int job_id,struct context_data *context) {

    int count = 0;
    struct process *proc;
    for (proc = context->jobs[job_id]->root; proc != NULL; proc = proc->next) {
        if (proc->status != STATUS_DONE) {
            count++;
        }
    }

    return count;
}

// Add a job in the jobs array in context
int append_job(struct job *job,struct context_data *context) {
    int available_job_id; 
    for (available_job_id = 1; available_job_id <= MAX_JOBS; available_job_id++) {
        if (context->jobs[available_job_id] == NULL) {
            break;
        }
    }

    job->id = available_job_id;
    context->jobs[available_job_id] = job;
    return available_job_id;
}

// remove a job from the context
int remove_job(int job_id,struct context_data *context) {

    struct job *job = context->jobs[job_id];
    struct process *proc, *tmp;
    for (proc = job->root; proc != NULL; ) {
        tmp = proc->next;
        free(proc->command);
        free(proc->command_list);
        free(proc->input_path);
        free(proc->output_path);
        free(proc);
        proc = tmp;
    }

    free(job->command);
    free(job);

    context->jobs[job_id] = NULL;

    return 0;
}

// check if a job is completed
int is_job_completed(int job_id,struct context_data *context) {

    struct process *proc;
    for (proc = context->jobs[job_id]->root; proc != NULL; proc = proc->next) {
        if (proc->status != STATUS_DONE) {
            return 0;
        }
    }

    return 1;
}

// set a process status
int set_process_status(int pid, int status,struct context_data *context) {
    int i;
    struct process *proc;

    for (i = 1; i <= MAX_JOBS; i++) {
        if (context->jobs[i] == NULL) {
            continue;
        }
        for (proc = context->jobs[i]->root; proc != NULL; proc = proc->next) {
            if (proc->pid == pid) {
                proc->status = status;
                return 0;
            }
        }
    }

    return -1;
}

// set a job's status and hence all the processes linked with that job too.
int set_job_status(int id, int status,struct context_data *context) {
    int i;
    struct process *proc;

    // iteratively set the status of all the linked processes with the job
    for (proc = context->jobs[id]->root; proc != NULL; proc = proc->next) {
        if (proc->status != STATUS_DONE) {
            proc->status = status;
        }
    }

    return 0;
}

// wait for a process to finish.
int wait_for_pid(int pid,struct context_data* context) {
    int status = 0;

    waitpid(pid, &status, WUNTRACED);
    if (WIFEXITED(status)) {
        set_process_status(pid, STATUS_DONE,context);
    } else if (WIFSIGNALED(status)) {
        set_process_status(pid, STATUS_TERMINATED,context);
    } else if (WSTOPSIG(status)) {
        status = -1;
        set_process_status(pid, STATUS_SUSPENDED,context);
    }

    return status;
}

// wait for a job to finish.
int wait_for_job(int id, struct context_data *context) {
    if (id > MAX_JOBS || context->jobs[id] == NULL) {
        return -1;
    }

    int proc_count = process_count(id,context);
    int wait_pid = -1, wait_count = 0;
    int status = 0;

    do {
        wait_pid = waitpid(-context->jobs[id]->pgid, &status, WUNTRACED);
        wait_count++;

        if (WIFEXITED(status)) {
            set_process_status(wait_pid, STATUS_DONE,context);
        } else if (WIFSIGNALED(status)) {
            set_process_status(wait_pid, STATUS_TERMINATED,context);
        } else if (WSTOPSIG(status)) {
            status = -1;
            set_process_status(wait_pid, STATUS_SUSPENDED,context);
            if (wait_count == proc_count) {
                show_job_status(id,context);
            }
        }
    } while (wait_count < proc_count);

    return status;
}

// create process from a part of command.
struct process* parse_command_segment(char *segment) {
    int bufsize = TOKEN_BUFSIZE;
    int position = 0;
    char *command = strdup(segment);
    char *token;
    char **tokens = (char**) malloc(bufsize * sizeof(char*));

    if (!tokens) {
        fprintf(stderr, "mysh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(segment, TOKEN_DELIMITERS);
    while (token != NULL) {
        glob_t glob_buffer;
        

        int glob_count = 0;
        if (strchr(token, '*') != NULL || strchr(token, '?') != NULL) {
            glob(token, 0, NULL, &glob_buffer);
            glob_count = glob_buffer.gl_pathc;
        }

        if (position + glob_count >= bufsize) {
            bufsize += TOKEN_BUFSIZE;
            bufsize += glob_count;
            tokens = (char**) realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "mysh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        if (glob_count > 0) {
            int i;
            for (i = 0; i < glob_count; i++) {
                tokens[position++] = strdup(glob_buffer.gl_pathv[i]);
            }
            globfree(&glob_buffer);
        } else {
            tokens[position] = token;
            position++;
        }

        token = strtok(NULL, TOKEN_DELIMITERS);
    }

    int i = 0, argc = 0;
    char *input_path = NULL, *output_path = NULL;
    while (i < position) {
        if (tokens[i][0] == '<' || tokens[i][0] == '>') {
            break;
        }
        i++;
    }
    argc = i;

    for (; i < position; i++) {
        if (tokens[i][0] == '<') {
            if (strlen(tokens[i]) == 1){ 
                input_path = (char *) malloc((strlen(tokens[i + 1]) + 1) * sizeof(char));
                strcpy(input_path, tokens[i + 1]);
                i++;
            } else {
                input_path = (char *) malloc(strlen(tokens[i]) * sizeof(char));
                strcpy(input_path, tokens[i] + 1);
            }
        } else if (tokens[i][0] == '>') { 
            if (strlen(tokens[i]) == 1) {
                output_path = (char *) malloc((strlen(tokens[i + 1]) + 1) * sizeof(char));
                strcpy(output_path, tokens[i + 1]);
                i++;
            } else {
                output_path = (char *) malloc(strlen(tokens[i]) * sizeof(char));
                strcpy(output_path, tokens[i] + 1);
            }
        } else {
            break;
        }
    }

    for (i = argc; i <= position; i++) {
        tokens[i] = NULL;
    }

    struct process *new_proc = (struct process*) malloc(sizeof(struct process));
    new_proc->command = command;
    new_proc->command_list = tokens;
    new_proc->command_count = argc;
    new_proc->input_path = input_path;
    new_proc->output_path = output_path;
    new_proc->pid = -1;
    new_proc->type = get_command_type(tokens[0]);
    new_proc->next = NULL;
    return new_proc;
}

// create job from a command.
struct job* parse_command(char *line) {
    line = trimmer(line);
    char *command = strdup(line);

    struct process *root_proc = NULL, *proc = NULL;
    char *line_cursor = line, *c = line, *seg;
    int seg_len = 0, mode = FG_EXEC;

    if (line[strlen(line) - 1] == '&') {
        mode = BG_EXEC;
        line[strlen(line) - 1] = '\0';
    }

    while (1) {
        if (*c == '\0' || *c == '|') {
            seg = (char*) malloc((seg_len + 1) * sizeof(char));
            strncpy(seg, line_cursor, seg_len);
            seg[seg_len] = '\0';

            struct process* new_proc = parse_command_segment(seg);
            if (!root_proc) {
                root_proc = new_proc;
                proc = root_proc;
            } else {
                proc->next = new_proc;
                proc = new_proc;
            }

            if (*c != '\0') {
                line_cursor = c;
                while (*(++line_cursor) == ' ');
                c = line_cursor;
                seg_len = 0;
                continue;
            } else {
                break;
            }
        } 
        else if (*c == '&' && *(c+1) == '&' && *(c+2) == ' '){
            printf("%s\n", "YEY && checked" );
        }
        else {
            seg_len++;
            c++;
        }
    }

    struct job *new_job = (struct job*) malloc(sizeof(struct job));
    new_job->root = root_proc;
    new_job->command = command;
    new_job->pgid = -1;
    new_job->mode = mode;
    return new_job;
}
