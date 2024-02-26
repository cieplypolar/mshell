#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "exec.h"
#include "config.h"
#include "write_handler.h"   
#include "siparse.h"
#include "redirs.h"

void handle_exec(command *com, char **com_transformed)
{
    static char err_temp[MAX_ERROR_MSG];

    handle_redir(com);

    if (execvp(com_transformed[0], com_transformed) == -1)
    {
        switch (errno)
        {
        case EACCES:
            err_temp[0] = '\0';
            strcat(err_temp, com_transformed[0]);
            strcat(err_temp, err_perm_exec);
            write_wrap(STDERR_FILENO, err_temp, strlen(err_temp));
            break;
        case ENOENT:
            err_temp[0] = '\0';
            strcat(err_temp, com_transformed[0]);
            strcat(err_temp, err_file_not_exist_exec);
            write_wrap(STDERR_FILENO, err_temp, strlen(err_temp));
            break;
        default:
            write_wrap(STDERR_FILENO, err_exec, strlen(err_exec));
            break;
        }
        exit(EXEC_FAILURE);
    }
}
