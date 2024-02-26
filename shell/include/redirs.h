#ifndef _REDIRS_H
#define _REDIRS_H

#include "siparse.h"

#define err_perm_red ": permission denied\n"
#define err_redir "Redirect error.\n"
#define err_file_not_exist_red ": no such file or directory\n"
#define err_close_red "Error when closing fd.\n"

void handle_redir(command *);

#endif /* !_REDIRS_H */
