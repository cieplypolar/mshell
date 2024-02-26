#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "write_handler.h"
#include "config.h"

void write_wrap(int fd, const void *buf, size_t count)
{
    if (buf == NULL)
    {
        exit(FAILURE);
    }
    while (true)
    {
        long write_return = write(fd, buf, count);
        if (write_return == count)
        {
            return;
        }
        else if (write_return != count)
        {
            if (write_return == -1 && errno != EINTR)
            {
                // don't want infinite recursion, so I won't inform user with words
                exit(WRITE_FAILURE);
            }
            if (write_return == -1 && errno == EINTR)
            {
                continue;
            }

            buf += write_return;
            count -= write_return;
        }
    }
}
