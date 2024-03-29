#ifndef _UTILS_H_
#define _UTILS_H_

#include "siparse.h"

void printcommand(command *, int);
void printpipeline(pipeline *, int);
void printparsedline(pipelineseq *);

command *pickfirstcommand(pipelineseq *);

void handle_parse(char *, pipelineseq **);

#endif /* !_UTILS_H_ */
