#include <unistd.h>
#include <string.h>
#include <pwd.h>

#include "prompt.h"
#include "write_handler.h"
#include "config.h"

extern char user[];
extern char pwd[];
static char oldpwd[MAX_PATH] = {0,};

void prompt()
{
    static char prompt[MAX_PROMPT_LENGTH];
    if (strcmp(oldpwd, pwd) != 0) {
        prompt[0] = '\0';
        strcpy(oldpwd, pwd);
        strcat(prompt, RESET_COLOR);
        strcat(prompt, USER_COLOR);
        strcat(prompt, user);
        strcat(prompt, RESET_COLOR);
        strcat(prompt, BETWEEN_USER_PWD);
        strcat(prompt, PATH_COLOR);
        strcat(prompt, pwd);
        strcat(prompt, RESET_COLOR);
        strcat(prompt, BETWEEN_INFO_PROMPT);
        strcat(prompt, PROMPT_STR);
    }
    write_wrap(STDOUT_FILENO, prompt, strlen(prompt));
}

void get_user()
{
    uid_t uid = geteuid(); //always successful
    struct passwd *pw = getpwuid(uid);
    if (pw)
    {
        strcpy(user, pw->pw_name);
    }
    else
    {
        strcpy(user, ANONYMOUS);
    }
}

