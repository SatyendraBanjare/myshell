#include "utils.h"
#include "execute.h"
#include "launch.h"

struct context_data *context;


/************************************************/
void print_welcome() {
    printf( R"EOF(
    ########################################
    #  ___  ___      _____ _          _ _  #
    #  |  \/  |     /  ___| |        | | | #
    #  | .  . |_   _\ `--.| |__   ___| | | #
    #  | |\/| | | | |`--. | '_ \ / _ | | | #
    #  | |  | | |_| /\__/ | | | |  __| | | #
    #  \_|  |_/\__, \____/|_| |_|\___|_|_| #
    #           __/ |                      #
    #          |___/                       #
    #                                      #
    #     - Satyendra Kumar Banjare        #
    ########################################
    )EOF");
}
/************************************************/

void main_loop(struct context_data* context) {
    char *line;
    struct job *job;
    int status = 1;

    // while (1) {
    //     mysh_print_promt(context);
    //     line = mysh_read_line();
    //     int i = 0;
    //     char *array[MAX_ANDED_COMMANDS] ;
    //     for (int j =0;j<MAX_ANDED_COMMANDS;j++){
    //         array[j] = "$";
    //     }

    //     char *p = strtok (line, "&&");
        
    //     while (p != NULL)
    //     {
    //         array[i++] = p;
    //         p = strtok (NULL, "&&");
    //     }

    //     for (i = 0; i < MAX_ANDED_COMMANDS; ++i) 
    //     {   char *lambda = array[i];
    //         if( lambda == "$"){
    //             break;
    //         }
            
    //         if (strlen(lambda) == 0) {
    //         check_process_state(context);
    //         continue;
    //         }
    //         job = parse_command(lambda);
    //         status = launch_job(job,context);

    //     }

        
    // }

     while (1) {
        // mysh_print_promt(context);
        line = read_input();
        if (strlen(line) == 0) {
            check_process_state(context);
            continue;
        }
        job = parse_command(line);
        status = launch_job(job,context);
    }
}

struct context_data* init() {
    
    pid_t pid = getpid();
    setpgid(pid, pid);
    tcsetpgrp(0, pid);

    struct passwd *pw = getpwuid(getuid());
    context = (struct context_data*) malloc(sizeof(struct context_data));
    strcpy(context->cur_user, pw->pw_name);

    int i;
    for (i = 0; i < MAX_JOBS; i++) {
        context->jobs[i] = NULL;
    }

    update_context(context);

    return context;
}

int main(int argc, char **argv) {
    
    system("clear");
    struct context_data* main_context = init();
    print_welcome();
    display_help();
    main_loop(main_context);
    
    return EXIT_SUCCESS;
}

