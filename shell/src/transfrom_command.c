#include <string.h>

#include "transform_command.h"

static char *ltrim(char *s, char c);
static char *rtrim(char *s, char c);
static char *trim(char *s, char c);

void transform_command(char **com_transformed, command *com)
{
    if (com == NULL)
    {
        com_transformed[0] = NULL;
        return;
    }
    int i = 0;
    argseq *argseq = com->args;
    do
    {
        com_transformed[i++] = trim(trim(trim(argseq->arg, ' '), '\''), '\"');
        argseq = argseq->next;
    } while (argseq != com->args);
    com_transformed[i] = NULL;
}

static char *ltrim(char *s, char c)
{
    while(*s == c) s++;
    return s;
}

static char *rtrim(char *s, char c)
{
    char* back = s + strlen(s);
    while((*--back) == c);
    *(back+1) = '\0';
    return s;
}

static char *trim(char *s, char c)
{
    if (s == NULL)
    {
        return NULL;
    }
    return rtrim(ltrim(s, c), c);
}
