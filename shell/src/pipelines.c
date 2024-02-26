#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

#include "pipelines.h"
#include "write_handler.h"
#include "config.h"
#include "transform_command.h"
#include "builtins.h"
#include "signals_utils.h"
#include "exec.h"
#include "background.h"
#include "siparse.h"

extern volatile int fg_num;
extern fg_proc fg;
extern bg_buf bg;

static void handle_pipeline(pipeline *);
static void close_wrap(int);
static void pipe_wrap(int[]);
static pid_t run(int[], int[], pipeline *, commandseq *, char **);
static bool check_pipeline_integrity(pipeline *, int *);
static void wait_for_fg();
static bool is_only_command(pipeline *, commandseq *);
static bool has_next(pipeline *, commandseq *);
static bool has_prev(pipeline *, commandseq *);
static void fcntl_dup_wrap(int, int);

static void fcntl_dup_wrap(int fdnew, int fdold)
{
    if (fcntl(fdnew, F_DUPFD, fdold) == -1)
    {
        write_wrap(STDERR_FILENO, err_fcntl, strlen(err_fcntl));
        exit(FAILURE);
    }
}

static bool is_only_command(pipeline *p, commandseq *commands)
{
    return commands == p->commands && commands->next == p->commands;
}

static bool has_next(pipeline * p, commandseq *commands)
{
    return commands->next != p->commands;
}

static bool has_prev(pipeline *p, commandseq *commands)
{
    return commands->prev != p->commands->prev;
}

void handle_pipelineseq(pipelineseq *ln)
{
    pipelineseq *ps = ln;

    if (!ln)
    {
        write_wrap(STDERR_FILENO, SYNTAX_ERROR_STR, strlen(SYNTAX_ERROR_STR));
        return;
    }

    do
    {
        if (ps != NULL)
        {
            handle_pipeline(ps->pipeline);
        }
        ps = ps->next;
    } while (ps != ln);
}

static void handle_pipeline(pipeline *p)
{
    int c = 0;
    commandseq *commands = p->commands;
    static char *com_transformed[MAX_COMMAND_NUMBER];
    int bg_flag = p->flags;

    if (commands == NULL)
    {
        return;
    }
    // 2 while loops, first to check integrity, second to actually handle
    bool pipeline_integrity = check_pipeline_integrity(p, &c);
    if (!pipeline_integrity && c > 1)
    {
        write_wrap(STDERR_FILENO, SYNTAX_ERROR_STR, strlen(SYNTAX_ERROR_STR));
        return;
    }

    // now check for builtins, we assume that if pipeline len is more than 1 there is no builtin
    if (c == 1)
    {
        command *com = commands->com;
        transform_command(com_transformed, com);

        // check if builtin
        bool exec = true;
        handle_builtins(com_transformed, &exec);
        if (!exec)
        {
            return;
        }
    }

    if (bg_flag != INBACKGROUND)
    {
        fg_num = c;
    }

    int next[2] = {STDIN_FILENO, STDOUT_FILENO};
    int prev[2] = {};

    sig_block(SIGCHLD);

    do
    {
        transform_command(com_transformed, commands->com);

        if (has_next(p, commands) && !is_only_command(p, commands))
        {
            prev[0] = next[0];
            prev[1] = next[1];
            pipe_wrap(next);
        }
        if (has_prev(p, commands) && !is_only_command(p, commands) && !has_next(p, commands))
        {
            prev[0] = next[0];
            prev[1] = next[1];
        }

        run(prev, next, p, commands, com_transformed); // there is only fork and exec in mshell

        commands = commands->next;
    } while (commands != p->commands);

    sig_unblock(SIGCHLD);

    wait_for_fg();
}

static bool check_pipeline_integrity(pipeline *p, int *c)
{
    commandseq *commands = p->commands;
    bool is_com_null = false;
    do
    {
        (*c)++;
        if (commands->com == NULL)
        {
            is_com_null = true;
        }
        commands = commands->next;
    } while (commands != p->commands);
    return !is_com_null;
}

static pid_t run(int prev[], int next[], pipeline *p, commandseq *commands, char **com_transformed)
{
    pid_t child = fork();
    int bg_flag = p->flags;

    if (child == 0)
    {
        // pipes
        if (has_next(p, commands) && !is_only_command(p, commands))
        {
            close_wrap(STDOUT_FILENO);
            fcntl_dup_wrap(next[1], STDOUT_FILENO);
            close_wrap(next[1]);
            close_wrap(next[0]);
        }
        if (has_prev(p, commands) && !is_only_command(p, commands))
        {
            close_wrap(STDIN_FILENO);
            fcntl_dup_wrap(prev[0], STDIN_FILENO);
            close_wrap(prev[0]);
        }

        if (bg_flag == INBACKGROUND)
        {
            if (setsid() == -1)
            {
                write_wrap(STDERR_FILENO, err_sys, strlen(err_sys));
                exit(FAILURE);
            }
        }
        sig_unblock(SIGINT);
        handle_exec(commands->com, com_transformed);
    }
    else if (child > 0)
    {
        if (bg_flag != INBACKGROUND)
        {
            fg_add(&fg, child);
        }

        if (has_next(p, commands) && !is_only_command(p, commands))
        {
            close_wrap(next[1]);
        }
        if (has_prev(p, commands) && !is_only_command(p, commands))
        {
            close_wrap(prev[0]);
        }
    }
    else
    {
        write_wrap(STDERR_FILENO, err_fork, strlen(err_fork));
    }

    return child;
}

static void close_wrap(int fd)
{
    if (close(fd) == -1)
    {
        write_wrap(STDERR_FILENO, err_close_pipes, strlen(err_close_pipes));
        exit(FAIL);
    }
}

static void pipe_wrap(int fd[])
{
    if (pipe(fd) == -1)
    {
        write_wrap(STDERR_FILENO, err_pipe, strlen(err_pipe));
        exit(FAIL);
    }
}

static void wait_for_fg()
{
    sigset_t old;
    sigset_t set;
    if (sigemptyset(&set) == -1)
    {
        write_wrap(STDERR_FILENO, err_sys, strlen(err_sys));
        exit(FAILURE);
    }
    if (sigprocmask(SIG_BLOCK, &set, &old) == -1)
    {
        write_wrap(STDERR_FILENO, err_sys, strlen(err_sys));
        exit(FAILURE);
    }
    sig_block(SIGCHLD);
    while (fg_num)
    {
        if (sigsuspend(&old) == -1 && errno != EINTR)
        {
            write_wrap(STDERR_FILENO, err_sys, strlen(err_sys));
            exit(FAILURE);
        }
    }
    sig_unblock(SIGCHLD);
}