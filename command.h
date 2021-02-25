#ifndef COMMON_H
#define COMMON_H

#include <sys/types.h>

struct str {
	char* ptr;
	size_t len;
};

struct cmd {
	const char *command;
	const char **vars; // NULL terminated variables list
	struct str input;
	struct str output;
};

union arg {
	const char *path;
	struct cmd cmd;
};

struct pipe {
	pid_t pid;
	int infd[2];
	int outfd[2];
};

int spawn(const char *command);
int pipe_open(struct pipe *p, const struct cmd *cmd);
int pipe_close(struct pipe *p);

#endif // COMMON_H
