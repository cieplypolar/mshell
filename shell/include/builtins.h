#ifndef _BUILTINS_H_
#define _BUILTINS_H_

#define BUILTIN_ERROR 2
#define DEFAULT_EXIT 0
#define MAX_LS_BUF 1024

#define err_builtin_pre "Builtin "
#define err_builtin_suf " error.\n"

typedef struct
{
	char *name;
	int (*fun)(char **);
} builtin_pair;

extern builtin_pair builtins_table[];
void handle_builtins(char **, bool *);

#endif /* !_BUILTINS_H_ */
