#ifndef MACROS_H
#define MACROS_H

#define MAX_JOBS 							10

// #define MAX_ANDED_CMDS 10 
// see main.c for implementation of '&&'

#define PATH_BUFSIZE 						1024
#define CMD_BUFSIZE 						1024
#define TOKEN_BUFSIZE 						64
#define TOKEN_DELIMITERS 					" "

#define BG_EXEC 							0x00
#define FG_EXEC 							0x01
#define PIPE_EXEC 							0x02

#define CMD_EXTERNAL 						0x00
#define CMD_EXIT 							0x01
#define CMD_CD 								0x02
#define CMD_JOBS 							0x03
#define CMD_FG 								0x04
#define CMD_BG 								0x05
#define CMD_KILL 							0x06
#define CMD_EXPORT 							0x07
#define CMD_UNSET 							0x08
#define CMD_PRINT_HELP 						0x09

#define STATUS_RUNNING 						0
#define STATUS_DONE 						1
#define STATUS_SUSPENDED 					2
#define STATUS_CONTINUED 					3
#define STATUS_TERMINATED 					4

#endif
