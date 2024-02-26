#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <errno.h>

#include "signals_utils.h"
#include "write_handler.h"
#include "config.h"
#include "background.h"

extern volatile int fg_num;
extern fg_proc fg;
extern bg_buf bg;

void handler(int);

void set_sig_handler()
{
    struct sigaction act;

    sigset_t set;
    if (sigfillset(&set) == -1)
    {
        write_wrap(STDERR_FILENO, err_sys, strlen(err_sys));
        exit(FAILURE);
    }

    act.sa_handler = handler;
    act.sa_flags = 0;
    act.sa_mask = set;

    if (sigaction(SIGCHLD, &act, NULL) == -1)
    {
        write_wrap(STDERR_FILENO, err_sys, strlen(err_sys));
        exit(FAILURE);
    }
}

void handler(int signum)
{
    int old_errno = errno;
    int status;
    int wait_ret;
    while (true)
    {
        wait_ret = waitpid(-1, &status, WNOHANG);
        if (wait_ret > 0)
        {
            if (fg_check(&fg, wait_ret))
            {
                fg_num--;
            }
            else
            {
                if (WIFEXITED(status))
                {
                    bg_buf_add(&bg, wait_ret, WEXITSTATUS(status), EXITED);
                }
                else if (WIFSIGNALED(status))
                {
                    bg_buf_add(&bg, wait_ret, WTERMSIG(status), KILLED);
                }
            }
        }
        else
        {
            break;
        }
    }
    errno = old_errno;
}

void sig_block(int signum)
{
    sigset_t set;

    if (sigemptyset(&set) == -1)
    {
        write_wrap(STDERR_FILENO, err_sys, strlen(err_sys));
        exit(FAILURE);
    }
    if (sigaddset(&set, signum) == -1)
    {
        write_wrap(STDERR_FILENO, err_sys, strlen(err_sys));
        exit(FAILURE);
    }
    if (sigprocmask(SIG_BLOCK, &set, NULL) == -1)
    {
        write_wrap(STDERR_FILENO, err_sys, strlen(err_sys));
        exit(FAILURE);
    }
}

void sig_unblock(int signum)
{
    sigset_t set;

    if (sigemptyset(&set) == -1)
    {
        write_wrap(STDERR_FILENO, err_sys, strlen(err_sys));
        exit(FAILURE);
    }
    if (sigaddset(&set, signum) == -1)
    {
        write_wrap(STDERR_FILENO, err_sys, strlen(err_sys));
        exit(FAILURE);
    }

    if (sigprocmask(SIG_UNBLOCK, &set, NULL) == -1)
    {
        write_wrap(STDERR_FILENO, err_sys, strlen(err_sys));
        exit(FAILURE);
    }
}
