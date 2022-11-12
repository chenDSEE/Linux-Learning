/**
 * How to run
 * 1. gcc signal_ignore.c -o signal_ignore && ./signal_ignore
 * 2. cat /proc/`pidof signal_ignore`/status | grep Sig
 * 3. kill -2 `pidof signal_ignore`
*/

#include <signal.h>
#include <stdio.h>

void main()
{
	if (signal(SIGINT, SIG_IGN) == SIG_ERR)
	{
		printf("fail to call signal()\n");
	}

	printf("process start to wait\n");
	sleep(1000);
}
