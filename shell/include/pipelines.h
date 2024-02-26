#ifndef _PIPELINES_H
#define _PIPELINES_H

#include "siparse.h"

#define err_fork "Fork failure\n"
#define err_close_pipes "Error when closing fd.\n"
#define err_pipe "Error when piping.\n"
#define err_fcntl "Error when duplicating fd.\n"

void handle_pipelineseq(pipelineseq *);

#endif /* !_PIPELINES_H */
