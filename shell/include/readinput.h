#ifndef _READINPUT_H_
#define _READINPUT_H_

#include <sys/types.h>
#include <stdbool.h>

#include "siparse.h"

typedef struct
{
    char *buf;
    char *buf_temp_ptr;
    int buf_content;
    int buf_cap;
} buffer;

typedef struct
{
    char *line_buf;
    char *line_buf_temp_ptr;
    int line_length;
} line_buffer;

typedef struct
{
    bool skip_nl;
    bool skip_parse;
    bool skip_read;
} read_config;

void handle_read(buffer *, line_buffer *, read_config *);
void init_read(buffer *, line_buffer *, read_config *, char *, char *);

#endif /* !_READINPUT_H_ */