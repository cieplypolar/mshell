#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <errno.h>
#include <stdbool.h>
#include <limits.h>
#include <signal.h>
#include <dirent.h>

#include "builtins.h"
#include "config.h"

extern char pwd[];

static int echo(char *[]);
static int lcd(char *[]);
static int lexit(char *[]);
static int lkill(char *[]);
static int lls(char *[]);
static int undefined(char *[]);
static void handle_strtol(char *, long **);

builtin_pair builtins_table[] = {
	{"exit", &lexit},
	{"lecho", &echo},
	{"lcd", &lcd},
	{"lkill", &lkill},
	{"lls", &lls},
	{"cd", &lcd},
	{NULL, NULL}};

static int echo(char *argv[])
{
	// given implementation
	int i = 1;
	if (argv[i])
		printf("%s", argv[i++]);
	while (argv[i])
		printf(" %s", argv[i++]);

	printf("\n");
	fflush(stdout);
	return 0;
}

static int lexit(char *argv[])
{
	// argv[0] lexit, argv[1] status, argv[2] NULL
	if (argv[1] == NULL)
	{
		printf("exit\n");
		exit(DEFAULT_EXIT);
	}
	else if (argv[2] == NULL)
	{
        long number;
        long *ptr = &number;
        handle_strtol(argv[1], &ptr);
        // exit status 0-255
        if (ptr == NULL)
        {
            return BUILTIN_ERROR;
        }
        if (number < 0 || number > 255)
        {
            return BUILTIN_ERROR;
        }
		printf("exit\n");
        exit((int)number);
	}
	else
	{
        return BUILTIN_ERROR;
	}
}

static int lcd(char *argv[])
{
	// argv[0] lcd, argv[1] path, argv[2] NULL
	if (argv[1] == NULL)
	{
        char *homedir;

		if ((homedir = getenv("HOME")) == NULL)
		{
			return BUILTIN_ERROR;
		}
		if (chdir(homedir) < 0)
		{
			return BUILTIN_ERROR;
		}
        strcpy(pwd, homedir);
	}
	else if (argv[2] == NULL)
	{
		if (chdir(argv[1]) < 0)
		{
			return BUILTIN_ERROR;
		}
        if (getcwd(pwd, MAX_PATH) == NULL)
        {
            pwd[0] = '\0';
        }
	}
	else
	{
		return BUILTIN_ERROR;
	}
	return 0;
}

static int lls(char *argv[])
{
	// argv[0] lls, argv[1] path, argv[2] NULL
	static char buf[MAX_PATH];
	char *path;
	if (argv[1] == NULL)
	{
		if (getcwd(buf, MAX_PATH) == NULL)
		{
			return BUILTIN_ERROR;
		}
		path = buf;
	}
	else if (argv[2] == NULL)
	{
		path = argv[1];
	}
	else
	{
		return BUILTIN_ERROR;
	}

	DIR *dir;
	if ((dir = opendir(path)) == NULL)
	{
		return BUILTIN_ERROR;
	}

	errno = 0; // that is what manual says
	int i = 0;
	static char *ls_buf[MAX_LS_BUF];
	struct dirent *file = readdir(dir);
	while (file != NULL)
	{
		if (i == MAX_LS_BUF)
		{
			for (int j = 0; j < MAX_LS_BUF; ++j)
			{
				printf("%s\n", ls_buf[j]);
			}
			i = 0;
			continue;
		}

		if (file->d_name[0] != '.')
		{
			ls_buf[i++] = file->d_name;
		}
		errno = 0;
		file = readdir(dir);
	}

	if (errno != 0)
	{
		// do not care if something is in buffer, error occurred
		return BUILTIN_ERROR;
	}

	if (closedir(dir) == -1)
	{
		return BUILTIN_ERROR;
	}

	for (int j = 0; j < i; ++j)
	{
		printf("%s\n", ls_buf[j]);
	}
	fflush(stdout);
	return 0;
}

static int lkill(char *argv[])
{
	// argv[0] lkill, argv[1] signum, argv[2] pid, argv[3] NULL
	if (argv[1] == NULL)
	{
		return BUILTIN_ERROR;
	}
	else if (argv[2] == NULL)
	{
		// default SIGNUM
		long pid;
		long *pid_ptr = &pid;
		handle_strtol(argv[1], &pid_ptr);
		// pids 0 - 99999, groups , but giving it to kill to determine if error
		if (pid_ptr == NULL)
		{
			return BUILTIN_ERROR;
		}
		if (kill(pid, SIGTERM) != 0)
		{
			return BUILTIN_ERROR;
		}
	}
	else if (argv[3] == NULL)
	{
		long pid;
		long *pid_ptr = &pid;
		long sig;
		long *sig_ptr = &sig;
		handle_strtol(argv[1], &sig_ptr);
		handle_strtol(argv[2], &pid_ptr);
		if (pid_ptr == NULL || sig_ptr == NULL)
		{
			return BUILTIN_ERROR;
		}
		sig *= -1;
		if (sig < INT_MIN || sig > INT_MAX)
		{
			return BUILTIN_ERROR;
		}
		if (kill(pid, (int)sig) != 0)
		{
			return BUILTIN_ERROR;
		}
	}
	else
	{
		return BUILTIN_ERROR;
	}
	return 0;
}

static int undefined(char *argv[])
{
	fprintf(stderr, "Command %s undefined.\n", argv[0]);
	return BUILTIN_ERROR;
}

static void handle_strtol(char *str, long **number)
{
	char *end = NULL;
	int base = 10;
	errno = 0;
	long value = strtol(str, &end, base);

	// invalid input
	if (*end != '\0')
	{
		*number = NULL;
		return;
	}
	else if ((value == LONG_MAX || value == LONG_MIN) && errno == ERANGE)
	{
		*number = NULL;
		return;
	}

	**number = value;
}

void handle_builtins(char **com_transformed, bool *exec)
{
	static char err_temp[MAX_ERROR_MSG];
	char *com = com_transformed[0];
	int (*fun)(char **) = NULL;
	int i = 0;
	if (com == NULL)
	{
		*exec = false;
		return;
	}

	while (builtins_table[i].name != NULL)
	{
		if (strcmp(com, builtins_table[i].name) == 0)
		{
			fun = builtins_table[i].fun;
			break;
		}
		++i;
	}
	if (fun != NULL)
	{
		if (fun(com_transformed) != 0)
		{
			err_temp[0] = '\0';
			strcat(err_temp, err_builtin_pre);
			strcat(err_temp, com);
			strcat(err_temp, err_builtin_suf);
			fprintf(stderr, "%s", err_temp);
		}
		*exec = false;
	}
	else
	{
		*exec = true;
	}
}
