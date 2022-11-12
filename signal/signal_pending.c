/**
 * How to run
 * 1. gcc signal_pending.c -lpthread -o signal_pending && ./signal_pending &
 * 2. cat /proc/<parent-thead-pod>/status | grep SigQ -A 5
 *    SigQ:    0/3635
 *    SigPnd:  0000000000000000
 *    ShdPnd:  0000000000000000
 *    SigBlk:  0000000000000002 (parent thread block SIGINT)
 *    SigIgn:  0000000000000000
 *    SigCgt:  0000000180000002
 * 
 * 3. cat /proc/<child-thead-pod>/status | grep SigQ -A 5
 *    SigQ:    0/3635
 *    SigPnd:  0000000000000000
 *    ShdPnd:  0000000000000000
 *    SigBlk:  0000000000000002 (child thread block SIGINT)
 *    SigIgn:  0000000000000000
 *    SigCgt:  0000000180000002
 * 
 * 4. kill -2 `pidof signal_pending`
 *    parent thread:
 *    SigQ:    1/3635
 *    SigPnd:  0000000000000000
 *    ShdPnd:  0000000000000002 (signal pending)
 *    SigBlk:  0000000000000002
 *    SigIgn:  0000000000000000
 *    SigCgt:  0000000180000002
 *    
 *    child thread:
 *    SigQ:    1/3635
 *    SigPnd:  0000000000000000
 *    ShdPnd:  0000000000000002 (signal pending)
 *    SigBlk:  0000000000000002
 *    SigIgn:  0000000000000000
 *    SigCgt:  0000000180000002
 * 
 * 6. waiting for child thread unblock
 * 
 * 7. cat /proc/<parent-thead-pod>/status | grep SigQ -A 5
 *    SigQ:    0/3635
 *    SigPnd:  0000000000000000
 *    ShdPnd:  0000000000000000
 *    SigBlk:  0000000000000002
 *    SigIgn:  0000000000000000
 *    SigCgt:  0000000180000002
 * 
 * 8. cat /proc/<child-thead-pod>/status | grep SigQ -A 5
 *    SigQ:    0/3635
 *    SigPnd:  0000000000000000
 *    ShdPnd:  0000000000000000
 *    SigBlk:  0000000000000000 (unblock)
 *    SigIgn:  0000000000000000
 *    SigCgt:  0000000180000002
 * 
 * stop and clean:
 * kill -9 `pidof signal_pending`
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
#define BLOCK_TIME 60

	
/**
 * SigIgn shared by all thread in process
 */
void signal_handler(int sig)
{
	/**
	 * UNSAFE: 
	 *   This handler uses non-async-signal-safe functions
	 *  (printf(), exit(); see TLPI Section 21.1.2) 
	 */
	printf("thread[%d], signal[%d:%s] handler called\n", gettid_via_syscall(), sig, strsignal(sig));
}

void main()
{
	main_thread_id = gettid_via_syscall();
	printf("===> main thread[%d] start to run\n", main_thread_id);

	if (signal(SIGINT, signal_handler) == SIG_ERR)
	{
		printf("main thread fail to ignore SIGINT\n");
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

	printf("child thread block for %d seconds\n", BLOCK_TIME);
	sleep(BLOCK_TIME);

	sigset_t blockSet;
	sigemptyset(&blockSet);
	sigaddset(&blockSet, SIGINT);
	if (sigprocmask(SIG_UNBLOCK, &blockSet, NULL) == -1)
	{
		printf("main thread fail to unblock SIGINT\n");
		exit(0);
	}


	printf("child thread unblock and waiting for %d seconds\n", BLOCK_TIME);
	sleep(SLEEP_TIME - 1);
	printf("child thread stop and exit\n");

	return NULL;
}

pid_t gettid_via_syscall()
{
    return syscall(SYS_gettid);
}