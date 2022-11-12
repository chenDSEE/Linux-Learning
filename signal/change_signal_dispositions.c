/**
 * How to run
 * 1. gcc change_signal_dispositions.c -o change_signal_dispositions && ./change_signal_dispositions
 * 2. cat /proc/`pidof change_signal_dispositions`/status | grep Sig
 * 3. kill -2 `pidof change_signal_dispositions`
 */

#include <signal.h>
#include <stdio.h>
#include <string.h>

void signal_handler(int sig)
{
	/**
	 * UNSAFE: 
	 *   This handler uses non-async-signal-safe functions
	*  (printf(), exit(); see TLPI Section 21.1.2) 
	 */
	printf("signal[%d:%s] handler called\n", sig, strsignal(sig));
}

void main()
{
	if (signal(SIGINT, signal_handler) == SIG_ERR)
	{
		printf("fail to call signal()\n");
	}

	printf("process start to wait\n");

	while(1)
	{
		sleep(1000);
	}
}
