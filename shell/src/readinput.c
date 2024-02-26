#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include "readinput.h"
#include "config.h"
#include "write_handler.h"

extern int isterm;

static void update(buffer *, line_buffer *, char *);
static void reset(buffer *, line_buffer *);
static int read_wrap_file(int, char *, int);
static int read_wrap_term(int, char *, int);
static void buffer_init(buffer *, char *);
static void line_buffer_init(line_buffer *, char *);
static void read_config_init(read_config *);
static void read_from_term(line_buffer *, read_config *);
static void read_from_file(buffer *);

void init_read(buffer *s_buf, line_buffer *s_line, read_config *conf, char *buf, char *line_buf)
{
    buffer_init(s_buf, buf);
    line_buffer_init(s_line, line_buf);
    read_config_init(conf);
}

static void buffer_init(buffer *s_buf, char *buf)
{
    s_buf->buf = buf;
    s_buf->buf_temp_ptr = buf;
    s_buf->buf_cap = MAX_BUFFER;
    s_buf->buf_content = 0;
}

static void line_buffer_init(line_buffer *s_line, char *line_buf)
{
    s_line->line_buf = line_buf;
    s_line->line_buf_temp_ptr = line_buf;
    s_line->line_length = 0;
}

static void read_config_init(read_config *conf)
{
    conf->skip_nl = false;
    conf->skip_parse = false;
    conf->skip_read = false;
}

static int read_wrap_file(int fd, char *buf, int length)
{
    if (buf == NULL)
    {
        exit(FAILURE);
    }
    int n;
    int readchars = 0;

    while (true)
    {
        n = read(fd, buf, length);
        if (n < 0 && errno != EINTR)
        {
            exit(FAILURE);
        }
        if (n < 0 && errno == EINTR)
        {
            continue;
        }
        if (n == 0)
        {
            return readchars;
        }

        readchars += n;
        buf += n;
        length -= n;
    }
}

static int read_wrap_term(int fd, char *buf, int length)
{
    if (buf == NULL)
    {
        exit(FAILURE);
    }
    int n;
    int readchars = 0;

    while (true)
    {
        n = read(fd, buf, length);
        if (n < 0 && errno != EINTR)
        {
            exit(FAILURE);
        }
        if (n < 0 && errno == EINTR)
        {
            continue;
        }
        if (n == 0)
        {
            return readchars;
        }

        if (buf[n - 1] != '\n')
        {
            readchars += n;
            buf += n;
            length -= n;
            continue;
        }
        else
        {
            readchars += n;
            return readchars;
        }
    }
}

static void read_from_term(line_buffer *s_line, read_config *conf)
{
    int read_return = read_wrap_term(STDIN_FILENO, s_line->line_buf, MAX_LINE_LENGTH);

    if (read_return == 0)
    {
        exit(SUCCESS);
    }

    if (s_line->line_buf[read_return - 1] != '\n')
    {
        write_wrap(STDERR_FILENO, SYNTAX_ERROR_STR, strlen(SYNTAX_ERROR_STR));
        conf->skip_parse = true;
        // cleaning stdin
        while (true)
        {
            read_return = read_wrap_term(STDIN_FILENO, s_line->line_buf, MAX_LINE_LENGTH);
            if (s_line->line_buf[read_return - 1] == '\n')
            {
                break;
            }
        }
    }
    s_line->line_buf[read_return - 1] = '\0';
}

static void read_from_file(buffer *s_buf)
{
    int read_return = read_wrap_file(STDIN_FILENO, s_buf->buf_temp_ptr, s_buf->buf_cap);
    if (read_return == 0 && s_buf->buf_content == 0)
    {
        exit(SUCCESS);
    }
    s_buf->buf_content += read_return;
    s_buf->buf_cap -= read_return;
}

void handle_read(buffer *s_buf, line_buffer *s_line, read_config *conf)
{
    if (isterm)
    {
        read_from_term(s_line, conf);
    }
    else
    {
        if (!(conf->skip_read))
        {
            read_from_file(s_buf);
        }

        // skipping read until nothing in buffer
        conf->skip_read = true;

        // do not have to worry about arithmetics (char = 1B)
        char *find_nl = strchr(s_buf->buf_temp_ptr, '\n');

        // found \n
        if (find_nl != NULL)
        {
            // check line length
            if (!(conf->skip_nl))
            {
                if (s_line->line_length + (find_nl - s_buf->buf_temp_ptr + 1) <= MAX_LINE_LENGTH)
                {
                    // setup line
                    memmove(s_line->line_buf_temp_ptr, s_buf->buf_temp_ptr, find_nl - s_buf->buf_temp_ptr);
                    s_line->line_length = s_line->line_length + (find_nl - s_buf->buf_temp_ptr + 1);
                    s_line->line_buf[s_line->line_length - 1] = '\0';

                    // update variables
                    update(s_buf, s_line, find_nl);
                    return;
                }
                else
                {

                    // update variables
                    update(s_buf, s_line, find_nl);

                    write_wrap(STDERR_FILENO, SYNTAX_ERROR_STR, strlen(SYNTAX_ERROR_STR));

                    handle_read(s_buf, s_line, conf);
                }
            }
            else
            {
                // update variables
                update(s_buf, s_line, find_nl);
                conf->skip_nl = false;

                write_wrap(STDERR_FILENO, SYNTAX_ERROR_STR, strlen(SYNTAX_ERROR_STR));

                handle_read(s_buf, s_line, conf);
            }
        }
        else
        {
            if (s_line->line_length + s_buf->buf_content >= MAX_LINE_LENGTH)
            {
                // update variables
                reset(s_buf, s_line);
                conf->skip_nl = true;
            }
            else
            {
                memmove(s_line->line_buf_temp_ptr, s_buf->buf_temp_ptr, s_buf->buf_content);
                s_line->line_buf_temp_ptr += s_buf->buf_content;
                s_line->line_length += s_buf->buf_content;
                s_line->line_buf[s_line->line_length] = '\0';

                s_buf->buf_temp_ptr = s_buf->buf;
                s_buf->buf_content = 0;
                s_buf->buf_cap = MAX_BUFFER;
            }
            conf->skip_read = false;
            handle_read(s_buf, s_line, conf);
        }
    }
}

static void update(buffer *s_buf, line_buffer *s_line, char *find_nl)
{
    s_buf->buf_content = s_buf->buf_content - (find_nl - s_buf->buf_temp_ptr + 1);
    s_line->line_buf_temp_ptr = s_line->line_buf;
    s_line->line_length = 0;
    s_buf->buf_temp_ptr = find_nl + 1;
}

static void reset(buffer *s_buf, line_buffer *s_line)
{
    s_line->line_length = 0;
    s_line->line_buf_temp_ptr = s_line->line_buf;
    s_buf->buf_temp_ptr = s_buf->buf;
    s_buf->buf_content = 0;
    s_buf->buf_cap = MAX_BUFFER;
}