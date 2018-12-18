#include "common.h"

const char* STATUS_STRING[] = {
    "running",
    "done",
    "suspended",
    "continued",
    "terminated"
};

int show_job_process(int job_id, struct context_data *context) {

    struct process *proc;
    for (proc = context->jobs[job_id]->parent; proc != NULL; proc = proc->next) {
        printf(" process associated with job [ %d ] is/are : %d",job_id, proc->pid);
    }
    printf("\n");

    return 0;
}

int show_job_status(int job_id, struct context_data *context) {

    struct process *proc;
    for (proc = context->jobs[id]->parent; proc != NULL; proc = proc->next) {
        printf("[%d]\t%d\t%s\t%s", job_id, proc->pid,
            STATUS_STRING[proc->status], proc->command);
        if (proc->next != NULL) {
            printf("|\n");
        } else {
            printf("\n");
        }
    }

    return 0;
}

int get_command_type(char *command) {
    if (strcmp(command, "exit") == 0) {
        return CMD_EXIT;
    } else if (strcmp(command, "cd") == 0) {
        return CMD_CD;
    } else if (strcmp(command, "jobs") == 0) {
        return CMD_JOBS;
    } else if (strcmp(command, "fg") == 0) {
        return CMD_FG;
    } else if (strcmp(command, "bg") == 0) {
        return CMD_BG;
    } else if (strcmp(command, "kill") == 0) {
        return CMD_KILL;
    } else if (strcmp(command, "export") == 0) {
        return CMD_EXPORT;
    } else if (strcmp(command, "unset") == 0) {
        return CMD_UNSET;
    } else if (strcmp(command, "help") == 0){
        return CMD_PRINT_HELP;
    }
      else {
        return CMD_EXTERNAL;
    }
}

char* trimmer(char* line) {
    char *front = line, *back = line + strlen(line);

    while (*front == ' ') {
        front++;
    }while (*back == ' ') {
        back--;
    }
    *(back + 1) = '\0';
    
    return front;
}

void completion(const char *buf, linenoiseCompletions *lc) {
    if (buf[0] == 'c') {
        linenoiseAddCompletion(lc,"cd");
    }
    else if(buf[0] == 'm'){
        linenoiseAddCompletion(lc,"mv");
        linenoiseAddCompletion(lc,"mkdir");
        linenoiseAddCompletion(lc,"man");   
    }
    else if(buf[0] == 'r'){
        linenoiseAddCompletion(lc,"rm");
        linenoiseAddCompletion(lc,"rm -rf");
        linenoiseAddCompletion(lc,"rmdir");   
    }
    if(buf[0] == 'l'){
        linenoiseAddCompletion(lc,"ls");
        linenoiseAddCompletion(lc,"locate");   
    }
}

char *hints(const char *buf, int *color, int *bold) {
    if (!strcasecmp(buf,"rm")) {
        *color = 35;
        *bold = 0;
        return " -rf";
    }
    if (!strcasecmp(buf,"ls")) {
        *color = 35;
        *bold = 0;
        return " -la";
    }

    return NULL;
}

char* read_input(char * username ) {

    linenoiseSetCompletionCallback(completion);
    linenoiseSetHintsCallback(hints);
    linenoiseHistoryLoad("history.txt");


    char* line = linenoise("MyShell> ");
    linenoiseHistoryAdd(line); /* Add to the history. */
    linenoiseHistorySave("history.txt"); /* Save the history on disk. */

    int bufsize = CMD_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if (!buffer) {
        fprintf(stderr, "mysh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    strcpy(buffer,line);

    free(line);
    return buffer;

}

void display_help() {
    printf("%s\n"," Usage help : alpha" );
}


