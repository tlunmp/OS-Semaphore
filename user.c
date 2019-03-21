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
#define SIZE 256
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

CharArray *shared;

sem_t *shmPalin;
sem_t *shmNotPalin;


int main (int argc, char *argv[]) {	

	char *palinName = "palin.out";
	char *noPalinName = "nopalin.out";	
	char *nameFileForPalinOrNot;

	// get the key from the shared memory
	shmid = shmget(SHMKEY, 5  * sizeof(CharArray), IPC_CREAT | 0777);
	
	if(shmid < 0) {
		perror("Error: shmget worker");
		exit(errno);
	}

	shared = shmat(shmid, NULL, 0);
	
	//get the semaphore for palin
	shmPalin = sem_open("Palin", 0);

	//error if palin semaphore failed
	if(shmPalin == SEM_FAILED){
              perror("User: sem_open failed\n");
        }

	//get eh semaphore for notpalin
	shmNotPalin = sem_open("NotPalin", 0);

	//error if palin semaphore get failed
	if(shmNotPalin == SEM_FAILED){
              perror("User: sem_open failed\n");
        }


	int lines = atoi(argv[2]);
	int index = atoi(argv[1]);

	int i;
	int isPalinDrom = 0;

	//iterate to 5 lines and check if it is palindrom or not, and print it to a specific file
	for (i = 0; i < 5; i++ ) {		
	
 	 	 time_t rawtime; 
	 	 struct tm *ptm;
   
		if(index > lines-1) {
			break;
		}

		//chek if it is not palin drom else if it is palindrome do sem_post when entering the critical section
		if(isPalindrome(shared[index].text) == 0) {	
			isPalinDrom = 0;
			nameFileForPalinOrNot = noPalinName;
			sem_wait(shmNotPalin);
				sleep(2);
		
	
			/* entering the critical section */	
			rawtime = time(NULL);
    
			//time error
   	 		if (rawtime == -1) {
        	
        			fprintf(stderr, "The time() function failed");
        			return 1;
    			}
   	
			ptm = localtime(&rawtime);
		
			//time error
			if (ptm == NULL) {
        
        			fprintf(stderr,"The localtime() function failed");
        			return 1;
    			}
    
   	 		fprintf(stderr,"pid %d: Entering Critical Section  not palin on %02d:%02d:%02d\n", getpid(), ptm->tm_hour, 
        		   ptm->tm_min, ptm->tm_sec);
		} else {

			
			isPalinDrom = 1;
			nameFileForPalinOrNot = palinName;
			sem_wait(shmPalin);
		
			/* entering the critical section */	
			sleep(2);
		
			rawtime = time(NULL);
   
			//time error 
    			if (rawtime == -1) {
        		
        			fprintf(stderr, "The time() function failed");
        			return 1;
    			}

   			//print the time
			ptm = localtime(&rawtime);
			
			//time error 
			if (ptm == NULL) {
        	
        			fprintf(stderr, "The localtime() function failed");
        			return 1;
    			}
    
   	 		fprintf(stderr, "pid %d: Entering Critical Section palin on %02d:%02d:%02d\n", getpid(), ptm->tm_hour, 
        		   ptm->tm_min, ptm->tm_sec);
		}

		//critical section
		FILE *f = fopen(nameFileForPalinOrNot, "a");
		// if file open error and return
		if(f == NULL){
			fprintf(stderr,"%s: ", argv[0]);
			perror("Error");
			return 0;
		}

		fprintf(f,"%d %d %s\n",getpid(),index,shared[index].text);
		fclose(f);

		//chek if it is not palin drom else if it is palindrome do sem_post when exiting the critical section
		if(isPalinDrom == 0){
			sleep(2);


			/* exiting critical section */
			//get the time
			rawtime= time(NULL);
    
			//time error
    			if (rawtime == -1) {
        	
        			fprintf(stderr, "The time() function failed");
      				  return 1;
    			}
    
	 		ptm = localtime(&rawtime);
   
 			//localtime error
		 	 if (ptm == NULL) { 
      				  fprintf(stderr, "The localtime() function failed");
    				  return 1;
    			}
    
  	  		fprintf(stderr, "pid %d: Exiting Critical Section notPalin on  %02d:%02d:%02d\n", getpid(),ptm->tm_hour, 
        			ptm->tm_min, ptm->tm_sec);

			sem_post(shmNotPalin);
		} else {
			sleep(2);

			/* exiting critical section */
			//get the time
			rawtime= time(NULL);
    
			//time error
    			if (rawtime == -1) {
        
        			fprintf(stderr,"The time() function failed");
      				  return 1;
    			}
    
 			ptm = localtime(&rawtime);
    
			//local time error
	 		 if (ptm == NULL) { 
      				  fprintf(stderr, "The localtime() function failed");
    			 	 return 1;
    			}
    
  	  		fprintf(stderr, "pid %d: Exiting Critical Section palin on  %02d:%02d:%02d\n", getpid(),ptm->tm_hour, 
        		ptm->tm_min, ptm->tm_sec);

			sem_post(shmPalin);
		}

		index++;

	}

	sem_close(shmPalin);
	sem_close(shmNotPalin);
	
	
	shmdt(shared); //detaches a section of shared memory
    	shmctl(shmid, IPC_RMID, NULL);  // deallocate the memory    	
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
	sem_unlink("Palin");
	sem_unlink("notPalin");
        shmdt(shared);
        shmctl(shmid,IPC_RMID, NULL);
        exit(0);
}
 
