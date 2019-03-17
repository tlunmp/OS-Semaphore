#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>	
#include <sys/shm.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

#define SHMKEY 9784

int shmid;
int *shmPtr;
void signalCallback (int signum);
int isPalindrome(char *str);

sem_t *sems;
char *semName = "semaphore";

typedef struct {
	char text[100];
} CharArray;



int main (int argc, char *argv[]) {	

	char palinName[] = "palin.out";
	char noPalinName[] = "nopalin.out";	
	char *nameFileForPalinOrNot;

	shmid = shmget(SHMKEY, 5  * sizeof(CharArray), IPC_CREAT | 0777);
	
	if(shmid < 0) {
		perror("Error: shmget worker");
		exit(errno);
	}

	CharArray *shared = shmat(shmid, NULL, 0);
	
	sems = sem_open(semName, 1);
        if(sems == SEM_FAILED){
                perror("sem_open in user failed\n");
                exit(errno);
        }    

	//printf("%c", shared[0].text[strlen(shared[0].text)-1]);
/*
	int flag = 0;
		
	if(isPalindrome(shared[0].text) == 0) {
			char nameFileForPalinOrNot == 
			sem_wait(sem);
	}



			sem_post(sem);
			flag = 1;
			break;
		}
		else{
			sem_post(sem);
			continue;
		}

	printf("%d", getpid());
	isPalindrome(shared[0].text);

*/	
	//struct CharArray *shared =  malloc(sizeof(struct CharArray));

	//shmid = shmget(IPC_PRIVATE, 1000 * sizeof(shared->text[0]), IPC_CREAT | 0644);
	
	//shared = shmat(shmid, NULL, 0);
	
	

	//printf("%s",shared->text[0]);
	return 0;
}


// A function to check if a string str is palindrome 
int  isPalindrome(char *str) { 
    		 // Start from leftmost and rightmost corners of str 
        	 int l = 0; 
            	 int h = strlen(str) - 1; 
            	 // Keep comparing characters while they are same 
                       
		while (h > l) { 
                     if (str[l++] != str[h--]) { 
                               printf("%s is Not Palindrome\n", str); 
                                return 0; 
                      } 
                 }

               printf("%s is palindrome\n", str); 
	return 1;
} 
	
void signalCallback (int signum)
{
    printf("\nSIGTERM received by worker\n");

    //Cleanup
        shmdt(shmPtr);
        shmctl(shmid,IPC_RMID, NULL);
        exit(0);
}
 
