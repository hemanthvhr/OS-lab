#include <unistd.h>
#include <iostream>
#include <algorithm>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <time.h>

using namespace std;

pthread_t t1,t2;

void handlesig(int sig) {
	if(sig == SIGUSR1) {
		//does nothing just wakes the process	
	}
	signal(SIGUSR1,handlesig);
}

extern "C" void *func(void *arg) {
	int flag = 0,*A = (int *)arg;
	char T[15];
	if(pthread_equal(pthread_self(),t1)) sprintf(T,"Thread T1: ");
	else {
		flag = 10;
		sprintf(T,"Thread T2: ");
	}
	for(int i=0;i<10;i++) {
		printf("%sarray[%d] = %d\n",T,i+flag,A[i+flag]);
	}
	pthread_exit(NULL);
}

int main() {
	int shmid = shmget(IPC_PRIVATE , 21*sizeof(int),0777|IPC_CREAT);	//creating the shared memory segment
	int p2 = fork();	//forking P2 process from main
	if(p2==0) {
		
		int p3,p1=fork();	//forking P1 process from P2
		if(p1 == 0) {	//--------------------------------------PROCESS P1------------------------------------
			int *count = (int *)shmat(shmid,0,0);	//shared[0] is the count
			int *A = count + 1;						//&shared[1] is the array that stores the integers from 1 to 20 in A[0] to A[19]
			srand(time(NULL));
			*count = 0;			
			printf("Process P1 -- \n\n");					//Intially 0 numbers in the array
			for(int i=0;i<20;i++) {
				A[i] = rand()%1000;
				*count = *count + 1;
				sleep(1);
			}
			shmdt(count);
			execlp("ls","ls","-l",NULL);
		} else {
			wait(NULL);		//waiting for process P1 to exit
			p3 = fork();	//forking P3 process from P2
			if(p3==0) {		//----------------------------------PROCESS P3-------------------------------------
				int *count = (int *)shmat(shmid,0,0);	//shared[0] is the count
				int *A = count + 1;						//&shared[1] is the array that stores the integers from 1 to 20 in A[0] to A[19]
				signal(SIGUSR1,handlesig);
				pause();
				int n = *count;							//no of integers in array A = 20
				sort(A,A+n);
				printf("Process P3 -- \n\n");
				if(pthread_create(&t1,NULL,*func,(void *)A) < 0) {
					printf("Thread 1 is not created\n");
				}
				if(pthread_create(&t2,NULL,*func,(void *)A) < 0) {
					printf("Thread 2 is not created\n");
				}
				pthread_join(t1,NULL);
				pthread_join(t2,NULL);
				shmdt(count);
				exit(0);
			}
		} 		//----------------------------------------------PROCESS P2--------------------------------------------------
		//Process P2's main code
		int *count = (int *)shmat(shmid,0,0);	//shared[0] is the count
		int *A = count + 1;						//&shared[1] is the array that stores the integers from 1 to 20 in A[0] to A[19]
		while(*count < 20);		//checking whether no of integers are less than 20
		printf("Process P2  --- \n\nThe no's are printed\n");
		for(int i=0;i<20;i++) printf("%d ,",A[i]);
		printf("\b\n\n");
		kill(p3,SIGUSR1);	//sending signal to process P3
		shmdt(count);
		exit(0);
	} else {	//----------------------------------------------MAIN process---------------------------------------------------
		wait(NULL);
	}
	return 0;
}
