#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "background.h"
#include "config.h"
#include "write_handler.h"

extern int fg_num;
extern fg_proc fg;
extern bg_buf bg;

void fg_init(fg_proc *f)
{
    f->count = 0;
    memset(f->proc, 0x0, MAX_FG_PROC);
}

void fg_add(fg_proc *f, pid_t p)
{
    if (f->count != MAX_FG_PROC)
    {
        f->proc[f->count++] = p;
    }
}

void fg_clear(fg_proc *f)
{
    fg_init(f);
}

bool fg_check(fg_proc *f, pid_t p)
{
    for (int i = 0; i < f->count; ++i)
    {
        if (f->proc[i] == p)
        {
            return true;
        }
        if (f->proc[i] == 0)
        {
            return false;
        }
    }
    return false;
}

void bg_buf_init(bg_buf *b)
{
    b->count = 0;
    memset(b->bg_info, 0x0, INFO_BUFF_CAP * 3);
}

void bg_buf_add(bg_buf *b, int pid, int status, int ifkill)
{
    if (b->count != INFO_BUFF_CAP)
    {
        b->bg_info[b->count][0] = pid;
        b->bg_info[b->count][1] = status;
        b->bg_info[b->count][2] = ifkill;
        b->count++;
    }
}

void bg_buf_clear(bg_buf *b)
{
    bg_buf_init(b);
}

void bg_buf_transfer(bg_buf *b, bg_write_buf *bw)
{
    static char temp[MAX_LINE_LENGTH];

    static char str[INTDIG];

    for (size_t i = 0; i < b->count; ++i)
    {
        temp[0] = '\0';
        str[0] = '\0';
        strcat(temp, BGINFO_PRE);
        // sprintf thread safe
        sprintf(str, "%d", b->bg_info[i][0]);
        strcat(temp, str);
        str[0] = '\0';
        if (b->bg_info[i][2] == EXITED)
        {
            strcat(temp, BGINFO_IN_EXIT);
        }
        else if (b->bg_info[i][2] == KILLED)
        {
            strcat(temp, BGINFO_IN_KILL);
        }
        sprintf(str, "%d", b->bg_info[i][1]);
        strcat(temp, str);
        strcat(temp, BGINFO_SUF);
        bg_w_add(bw, temp);
    }
}
void bg_w_init(bg_write_buf *bw)
{
    bw->count = 0;
    memset(bw->to_write, 0x0, INFO_BUFF_CAP * INFO_LENGTH);
}
void bg_w_add(bg_write_buf *bw, char *info)
{
    sprintf(bw->to_write[bw->count++], "%s", info);
}
void bg_w_write(bg_write_buf *bw)
{
    for (size_t i = 0; i < bw->count; ++i)
    {
        write_wrap(STDOUT_FILENO, bw->to_write[i], strlen(bw->to_write[i]));
    }
}
void bg_w_clear(bg_write_buf *bw)
{
    bg_w_init(bw);
}
