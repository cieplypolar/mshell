#ifndef _EXEC_H_
#define _EXEC_H_

#include "siparse.h"

#define err_file_not_exist_exec ": no such file or directory\n"
#define err_perm_exec ": permission denied\n"
#define err_exec "Exec failure\n"

void handle_exec(command *, char **);

#endif /* !_EXEC_H_ */