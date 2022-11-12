/**
 * How to run
 * 1. gcc thread_inherit_signal.c -lpthread -o thread_inherit_signal && ./thread_inherit_signal &
 * 2. ps -elfT | grep thread_inherit_signal
 * 3. cat /proc/<child-thead-pod>/status | grep SigQ -A 5
 * 4. cat /proc/<parent-thead-pod>/status | grep SigQ -A 5
 * 
 * stop and clear:
 * kill -9 `pidof thread_inherit_signal`
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/syscall.h>       /* Definition of  SYS_*  constants */
#include <unistd.h>
#include <signal.h>

pid_t main_thread_id;
pid_t child_thread_id;
void* child_thread_main(void* arg);
#define SLEEP_TIME 10000

	
/**
 * SigIgn shared by all thread in process
 * main thread ignore SIGUSR1, child thread ignore SIGUSR2
 * main thread : SigIgn:    0000000000000a00 --> ‭1010 0000 0000‬ --> ignore SIGUSR1 and SIGUSR2
 * child thread: SigIgn:    0000000000000a00 --> ‭1010 0000 0000‬ --> ignore SIGUSR1 and SIGUSR2
 */
/**
 * SigBlk independence for all thread in process
 * main thread block SIGPIPE, child block SIGINT
 * main thread : SigBlk:    0000000000001000 --> ‭1 0000 0000 0000‬ --> block SIGPIPE
 * child thread: SigBlk:    0000000000001002 --> 1 0000 0000 0010‬ --> block SIGPIPE and SIGINT
 */
void main()
{
	main_thread_id = gettid_via_syscall();
	printf("===> main thread[%d] start to run\n", main_thread_id);

	if (signal(SIGUSR1, SIG_IGN) == SIG_ERR)
	{
		printf("main thread fail to ignore SIGUSR1\n");
		exit(0);
	}

	sigset_t blockSet;
	sigemptyset(&blockSet);
	sigaddset(&blockSet, SIGPIPE);
	if (sigprocmask(SIG_BLOCK, &blockSet, NULL) == -1)
	{
		printf("main thread fail to block SIGPIPE\n");
		exit(0);
	}

	pthread_t thread_info;
	memset(&thread_info, 0, sizeof(thread_info));
	if(pthread_create(&thread_info, NULL, child_thread_main, NULL) < 0)
	{
		printf("pthread_create() fail\n");
		exit(0);
	}

	printf("main thread waiting for %d seconds\n", SLEEP_TIME);
	sleep(SLEEP_TIME);
	printf("main thread stop and exit\n");
}

void* child_thread_main(void* arg)
{
	child_thread_id = gettid_via_syscall();
	printf("===> child thread[%d] start to run\n", child_thread_id);

	if (signal(SIGUSR2, SIG_IGN) == SIG_ERR)
	{
		printf("main thread fail to ignore SIGUSR2\n");
		exit(0);
	}

	sigset_t blockSet;
	sigemptyset(&blockSet);
	sigaddset(&blockSet, SIGINT);
	if (sigprocmask(SIG_BLOCK, &blockSet, NULL) == -1)
	{
		printf("main thread fail to block SIGINT\n");
		exit(0);
	}

	printf("child thread waiting for %d seconds\n", SLEEP_TIME - 1);
	sleep(SLEEP_TIME - 1);
	printf("child thread stop and exit\n");

	return NULL;
}

pid_t gettid_via_syscall()
{
    return syscall(SYS_gettid);
}