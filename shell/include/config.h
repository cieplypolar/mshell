#ifndef _CONFIG_H_
#define _CONFIG_H_

#define MAX_LINE_LENGTH 2048

#define MAX_ERROR_MSG 4096

#define MAX_PATH 1024

#define MAX_USER 256

#define MAX_BUFFER (MAX_LINE_LENGTH * 8)

#define MAX_COMMAND_NUMBER (MAX_LINE_LENGTH / 2 + 1)

#define SYNTAX_ERROR_STR "Syntax error.\n"

#define EXEC_FAILURE 127

#define FAILURE 1

#define WRITE_FAILURE 1882

#define SUCCESS 0

#define PROMPT_STR "λx "

#endif /* !_CONFIG_H_ */
