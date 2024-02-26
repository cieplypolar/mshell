#ifndef _PROMPT_H
#define _PROMPT_H

#define ANONYMOUS "anonymous"
#define MAX_PROMPT_LENGTH 512
#define BETWEEN_USER_PWD ":"
#define BETWEEN_INFO_PROMPT " "
#define RESET_COLOR "\033[0m"
#define USER_COLOR "\033[0;36m" // cyan
#define PATH_COLOR "\033[0;32m" // green

void prompt();
void get_user();

#endif /* !_PROMPT_H */
