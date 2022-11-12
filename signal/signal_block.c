/**
 * How to run
 * 1. gcc signal_block.c -o signal_block && ./signal_block
 * 2. cat /proc/`pidof signal_block`/status | grep SigQ -A 5
 * 3. kill -2 `pidof signal_block`
*/

#include <signal.h>
#include <stdio.h>

void signal_handler(int sig)
{
	printf("signal[%d] handler called\n", sig);
	sleep(10); // SigBlk will be set unit signal_handler() finish and exit
	printf("exit signal handler\n", sig);
}

void main()
{
	if (signal(SIGINT, signal_handler) == SIG_ERR)
	{
		printf("fail to call signal()\n");
	}

	// sigset_t blockSet;
	// sigemptyset(&blockSet);
	// sigaddset(&blockSet, SIGINT);
	// if (sigprocmask(SIG_BLOCK, &blockSet, NULL) == -1)
	// {
	// 	printf("fail to call sigprocmask()\n");
	// }

	printf("signal blocking demo set finish\n");
	while(1)
	{
		sleep(1000);
	}
}

