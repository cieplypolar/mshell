#ifndef _SIGNALS_UTILS_C_H_
#define _SIGNALS_UTILS_C_H_

#define err_sys "System error.\n"

void set_sig_handler();
void sig_block(int);
void sig_unblock(int);

#endif /* !_SIGNALS_UTILS_C_H_ */
