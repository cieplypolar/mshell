#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "redirs.h"
#include "config.h"
#include "write_handler.h"

static void close_wrap(int);
static void handle_error_fd_in(char *);
static void handle_error_fd_out(char *);

void handle_redir(command *com)
{
    int flags;
    char *path;
    redirseq *redirs = com->redirs;
    if (redirs)
    {
        do
        {
            flags = redirs->r->flags;
            path = redirs->r->filename;
            if (IS_RIN(flags))
            {
                int newfd = open(path, O_RDONLY, S_IRUSR);
                if (newfd == -1)
                {
                    handle_error_fd_in(path);
                    exit(FAIL);
                }
                close_wrap(STDIN_FILENO);
                fcntl(newfd, F_DUPFD, STDIN_FILENO);
                close_wrap(newfd);
            }
            else if (IS_ROUT(flags))
            {
                int newfd = open(path, O_CREAT | O_TRUNC | O_WRONLY, S_IWUSR | S_IRUSR);
                if (newfd == -1)
                {
                    handle_error_fd_out(path);
                    exit(FAIL);
                }
                close_wrap(STDOUT_FILENO);
                fcntl(newfd, F_DUPFD, STDOUT_FILENO);
                close_wrap(newfd);
            }
            else if (IS_RAPPEND(flags))
            {
                int newfd = open(path, O_CREAT | O_APPEND | O_WRONLY, S_IWUSR | S_IRUSR);
                if (newfd == -1)
                {
                    handle_error_fd_out(path);
                    exit(FAIL);
                }
                close_wrap(STDOUT_FILENO);
                fcntl(newfd, F_DUPFD, STDOUT_FILENO);
                close_wrap(newfd);
            }
            else
            {
                write_wrap(STDERR_FILENO, SYNTAX_ERROR_STR, strlen(SYNTAX_ERROR_STR));
                exit(FAIL);
            }
            redirs = redirs->next;
        } while (redirs != com->redirs);
    }
}

static void close_wrap(int fd)
{
    if (close(fd) == -1)
    {
        write_wrap(STDERR_FILENO, err_close_red, strlen(err_close_red));
        exit(FAIL);
    }
}

static void handle_error_fd_in(char *path)
{
    static char err_temp[MAX_ERROR_MSG];
    err_temp[0] = '\0';
    switch (errno)
    {
    case EACCES:
        err_temp[0] = '\0';
        strcat(err_temp, path);
        strcat(err_temp, err_perm_red);
        write_wrap(STDERR_FILENO, err_temp, strlen(err_temp));
        break;
    case ENOENT:
        err_temp[0] = '\0';
        strcat(err_temp, path);
        strcat(err_temp, err_file_not_exist_red);
        write_wrap(STDERR_FILENO, err_temp, strlen(err_temp));
        break;
    default:
        write_wrap(STDERR_FILENO, err_redir, strlen(err_redir));
        break;
    }
}

static void handle_error_fd_out(char *path)
{
    static char err_temp[MAX_ERROR_MSG];
    err_temp[0] = '\0';
    switch (errno)
    {
    case EACCES:
        err_temp[0] = '\0';
        strcat(err_temp, path);
        strcat(err_temp, err_perm_red);
        write_wrap(STDERR_FILENO, err_temp, strlen(err_temp));
        break;
    default:
        write_wrap(STDERR_FILENO, err_redir, strlen(err_redir));
        break;
    }
}