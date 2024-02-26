#ifndef _BACKGROUND_H_
#define _BACKGROUND_H_

#include <sys/types.h>
#include <stdbool.h>

#include "config.h"

#define MAX_FG_PROC (MAX_LINE_LENGTH / 2)
#define INFO_BUFF_CAP (MAX_LINE_LENGTH / 2)
#define INFO_LENGTH 2048
#define EXITED 1882
#define KILLED 1929
#define INTDIG 10

#define BGINFO_PRE "Background process "
#define BGINFO_IN_EXIT " terminated. (exited with status "
#define BGINFO_SUF ")\n"
#define BGINFO_IN_KILL " terminated. (killed by signal "

typedef struct
{
    size_t count;
    int bg_info[INFO_BUFF_CAP][3];
    // 0 - pid, 1 - status, 2 - iskilled

} bg_buf;

typedef struct
{
    size_t count;
    pid_t proc[MAX_FG_PROC];
} fg_proc;

typedef struct
{
    size_t count;
    char to_write[INFO_BUFF_CAP][INFO_LENGTH];
} bg_write_buf;

void fg_init(fg_proc *);
void fg_add(fg_proc *, pid_t);
void fg_clear(fg_proc *);
void bg_buf_init(bg_buf *);
void bg_buf_clear(bg_buf *);
void bg_buf_transfer(bg_buf *, bg_write_buf *);
void bg_w_init(bg_write_buf *);
void bg_w_write(bg_write_buf *);
void bg_w_clear(bg_write_buf *);
bool fg_check(fg_proc *, pid_t);
void bg_buf_add(bg_buf *, int, int, int);
void bg_w_add(bg_write_buf *, char *);

#endif /* !_BACKGROUND_H_ */