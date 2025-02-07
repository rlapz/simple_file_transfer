// SPDX-License-Identifier: GPL-2.0
/*
 * Simple file transfer (entry file)
 *
 * Copyright (C) 2021  Arthur Lapz <rlapz@gnuweeb.org>
 */

#include <errno.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "ftransfer.h"

static const char *app = NULL;

volatile int is_interrupted = 0;


void
interrupt_handler(int sig)
{
	is_interrupted = 1;
	errno = EINTR;
	putchar('\n');

	(void)sig;
}

void
set_signal(void)
{
	struct sigaction act = { 0 };

	act.sa_handler = SIG_IGN;
	if (sigaction(SIGPIPE, &act, NULL) < 0)
		goto err;

	act.sa_handler = interrupt_handler;
	if (sigaction(SIGINT, &act, NULL) < 0)
		goto err;

	if (sigaction(SIGTERM, &act, NULL) < 0)
		goto err;

	if (sigaction(SIGHUP, &act, NULL) < 0)
		goto err;

	return;

err:
	PERROR("set_signal()");

	exit(1);
}

int
file_check(const packet_t *p)
{
	if (p->file_name_len == 0 || strstr(p->file_name, "..") != NULL) {
		errno = EINVAL;

		return -1;
	}

	return 0;
}

void
print_help(void)
{
	puts(BOLD_YELLOW("Usage:"));
	printf("  %s server [bind_addr] [bind_port]\n", app);
	printf("  %s client [server_addr] [server_port] [filename]\n", app);
}


int
main(int argc, char *argv[])
{
	app = argv[0];

	if (argc < 2) {
		print_help();

		return EXIT_SUCCESS;
	}

	argc -= 2;
	if (!strcmp("server", argv[1]))
		return run_server(argc, argv + 2);
	else if (!strcmp("client", argv[1]))
		return run_client(argc, argv + 2);

	FPERROR("Error: Invalid argument \"%s\"\n\n", argv[1]);
	print_help();

	return EINVAL;
}
