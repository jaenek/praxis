#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "command.h"

int spawn(const char *command)
{
	pid_t pid;
	if ((pid = fork()) == 0) {
		setsid();
		execl("/usr/bin/sh", "sh", "-c", command, NULL);
		printf("praxis: execl %s", command);
		perror(" failed");
		exit(EXIT_SUCCESS);
	} else if (pid == -1) {
		perror("praxis: fork() failed");
		return 1;
	}
	return 0;
}

int pipe_open(struct pipe *p, const struct cmd *cmd)
{
	int nread = -1;
	pipe(p->infd);
	pipe(p->outfd);
	if ((p->pid = fork()) == 0) {
		close(p->infd[0]);
		close(p->outfd[1]);
		dup2(p->outfd[0], STDIN_FILENO);
		dup2(p->infd[1], STDOUT_FILENO);
		dup2(p->infd[1], STDERR_FILENO);
		setsid();
		execl("/usr/bin/sh", "sh", "-c", cmd->command, NULL);
		printf("praxis: execl %s", cmd->command);
		perror(" failed");
		close(p->infd[1]);
		close(p->outfd[0]);
		exit(EXIT_SUCCESS);
	} else if (p->pid == -1) {
		perror("praxis: fork() failed");
		goto cleanup;
	}

	// close unused so called process returns eof
	close(p->infd[1]);
	close(p->outfd[0]);

	write(p->outfd[1], cmd->input.ptr, cmd->input.len);
	close(p->outfd[1]);

	nread = read(p->infd[0], cmd->output.ptr, cmd->output.len);

	kill(p->pid, SIGKILL);
cleanup:
	close(p->infd[0]);
	close(p->outfd[1]);
	return nread;
}
