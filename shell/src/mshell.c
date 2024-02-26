#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>

#include "config.h"
#include "siparse.h"
#include "readinput.h"
#include "utils.h"
#include "pipelines.h"
#include "background.h"
#include "signals_utils.h"
#include "prompt.h"

int isterm;
volatile int fg_num = 0;
fg_proc fg;
bg_buf bg;
char user[MAX_USER];
char pwd[MAX_PATH];

int main(int argc, char *argv[])
{
	// variables
	pipelineseq *ln;

	// arrays
	char buf[MAX_BUFFER + 1]; // intentionally having bigger buffer to add \0
	buf[MAX_BUFFER] = '\0';
	char line_buf[MAX_LINE_LENGTH];

	// structs
	buffer s_buf;
	line_buffer s_line;
	read_config conf;
	bg_write_buf bgw;

	// init
	isterm = isatty(STDIN_FILENO);
	get_user();
	if (getcwd(pwd, MAX_PATH) == NULL)
	{
		pwd[0] = '\0';
	}
	init_read(&s_buf, &s_line, &conf, buf, line_buf);
	fg_init(&fg);
	bg_buf_init(&bg);
	bg_w_init(&bgw);

	// setting signals
	sig_block(SIGINT);
	set_sig_handler();

	while (true)
	{
		sig_block(SIGCHLD);

		bg_buf_transfer(&bg, &bgw);
		fg_clear(&fg);
		bg_buf_clear(&bg);

		sig_unblock(SIGCHLD);
		if (isterm)
		{
			bg_w_write(&bgw);
		}
		bg_w_clear(&bgw);
		if (isterm)
		{
			prompt();
		}
		conf.skip_parse = false;

		handle_read(&s_buf, &s_line, &conf);

		if (conf.skip_parse)
		{
			continue;
		}

		handle_parse(s_line.line_buf, &ln);

		handle_pipelineseq(ln);
	}
}
