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
#include <time.h>

#define SHMKEY 9784

int shmid;
void signalCallback (int signum);
int isPalindrome(char *str);

sem_t *shmPtr;
sem_t *sems;
char *semName = "./semaphore";

typedef struct {
	char text[100];
} CharArray;


sem_t *shmPalin;
sem_t *shmNotPalin;

#define SIZE 256

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

//	shmPtr = shmat(atoi(argv[3]),NULL, 0);

	
	shmPalin = sem_open("Palin", 0);

	shmNotPalin = sem_open("NotPalin", 0);

//	sem_close(shmNotPalin);
//	sem_close(shmPalin);

	time_t current_time;

	if (current_time == ((time_t)-1))
    	{
        	(void) fprintf(stderr, "Failure to obtain the current time.\n");
       		 exit(EXIT_FAILURE);
    	}
	
        char * c_time_string = ctime(&current_time);



	int lines = atoi(argv[2]);

	char buffer[SIZE];
  	time_t curtime;
  	struct tm *loctime;

  	/* Get the current time. */
  	curtime = time (NULL);

  	/* Convert it to local time representation. */
 	 loctime = localtime (&curtime);



	
	//sem_init(&sems, 0, 1);	

	int index = atoi(argv[1]);


	//entering critical section
	int i;
	int isPalinDrom;
	int isNotPalin;

	for (i = 0; i < 5; i++ ) {		
		if(index > lines-1) {
			break;
		}


		sleep(2);
		if(isPalindrome(shared[index].text) == 0) {	
			isPalinDrom = 1;
			nameFileForPalinOrNot = &noPalinName;
			sem_wait(&shmNotPalin);
		} else {
			isNotPalin = 0;
			nameFileForPalinOrNot = &palinName;
			sem_wait(&shmPalin);
		}

		printf("%d, enter critical section ",getpid());
		  	/* Print it out in a nice format. */
  		fputs (asctime (loctime), stdout);


		FILE *f = fopen(nameFileForPalinOrNot, "a");
		fprintf(f,"%d %d %s\n",getpid(),index,shared[index].text);

		fclose(f);
		

		if(isPalinDrom == 0){
			sem_post(&shmNotPalin);
		} else {
			sem_post(&shmPalin);
		}
		sleep(2);
		printf("%d, exit critical section\n",getpid());
		//sem_post(&sems);
		index++;

	}

//	sem_post(sems);
	
/*			sem_post(sem);
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
                                return 0; 
                      } 
                 }

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
 
