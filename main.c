#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>	
#include <semaphore.h>
#include <sys/shm.h>
#include <fcntl.h>
#define SHMKEY 9784

void helpMenu();
void forkProcess(int maxChildProcess, int numberChildProcess,char *inputFileName, char *outputFileName, int increment, char *arg0Name);
void shareClock(int increment);
int countLines(FILE *file);

void signalCall(int signum);

int timer = 25;
int shmid;


typedef struct {
	char text[100];
} CharArray;

CharArray *shared;
sem_t *shmPalin;
sem_t *shmNotPalin;
sem_t *shmPtr;

int main (int argc, char *argv[]) {

	char inputFileName[] = "input.txt";
	char outputFileName[] = "output.txt";
	int bufSize = 100;
	char buffer[bufSize];
		
	int newLineCount = 0;
	int c;
	int maxChildProcess = 4;
	int numberChildProcess = 2;
	

	//getopt command for command line
	while((c = getopt (argc,argv, "hi:o:n:s:")) != -1) {

		switch(c) {
			case 'h':
				helpMenu();
				return 1;
			case 'i':
				strcpy(inputFileName, optarg);
				break;
			case 'n': maxChildProcess = atoi(optarg);
				break;
			default:
				fprintf(stderr, "%s: Error: Unknown option -%c\n",argv[0],optopt);
				return -1;	
		}


	}

	char errorMessage[100];

	FILE *f = fopen(inputFileName,"r");
	
	// if file open error and return
	if(f == NULL){
		fprintf(stderr,"%s: ", argv[0]);
		perror("Error");
		return 0;
	}


	//gets the fork number
	fgets(buffer, bufSize, f);
		newLineCount++;
	fclose(f);

	int increment = atoi(buffer);
	


	//if max child process over 20 error
	if(maxChildProcess > 20){
		fprintf(stderr,"%s: Error: Cannot have 20 or more max process\n",argv[0]);
		return 0;
	}	 


	//signal error
	 if (signal(SIGINT, signalCall) == SIG_ERR) {
        	snprintf(errorMessage, sizeof(errorMessage), "%s: Error: user: signal(): SIGINT\n", argv[0]);
		perror(errorMessage);	
        	exit(errno);
  	  }
	
	//sigalarm error
	if (signal(SIGALRM, signalCall) == SIG_ERR) {
            snprintf(errorMessage, sizeof(errorMessage), "%s: Error: user: signal(): SIGALRM\n", argv[0]);
	     perror(errorMessage);	
         	exit(errno);
     	}
	
	//alarm for 25 real life second
	alarm(timer);
	forkProcess(maxChildProcess, numberChildProcess,inputFileName,outputFileName,increment,argv[0]);
		
		
	return 0;
}	

void forkProcess(int maxChildProcess, int numChildProcess, char *inputFileName, char *outputFileName,int increment,char *arg0Name) {
		
	int bufSize = 100;
	char buffer[bufSize];
		
	pid_t childpid =0;
	int ptr_count = 0;
	char errorMessage[1000];

	int index = 0;	
	int totalCount = 0;


	//create a array of keyy
	shmid = shmget(SHMKEY, 5  * sizeof(CharArray), IPC_CREAT | 0777);
	if(shmid < 0) {
		fprintf(stderr, "//shmget failed in master\n");	
		exit(1);	
	}

	//attach address
	shared = shmat(shmid, NULL, 0);

	//check if error
	if(shared == -1 ){
            	fprintf(stderr,"shmat failed in master");
            	exit(2);	
        }
	
	//create a semaphore for not palin
	shmNotPalin = sem_open("NotPalin", O_CREAT , 0644, 1);
	if (shmNotPalin == SEM_FAILED) {
     		perror("Failed to open semphore for shmNotPalin");
     		exit(-1);
	}

	//create a semphore for palin
	shmPalin =  sem_open("Palin", O_CREAT , 0644, 1);
	if (shmPalin == SEM_FAILED) {
     		sem_close(shmNotPalin);
     		perror("Failed to open semphore for shmPalin");
     		exit(-1);
	}


	FILE *f1 = fopen(inputFileName, "r");

	if(f1 == NULL){
		snprintf(errorMessage, sizeof(errorMessage), "%s: Error: ", arg0Name);
	     	perror(errorMessage);	
		return;
	}
	
	int lines = countLines(f1);


	//copies the string to the shared memory
	while(fgets(buffer,bufSize,f1) != NULL){ 
		strcpy(shared[index].text, buffer);
		shared[index].text[strlen(buffer)-1] = '\0';
		index++;
	}

	fclose(f1);
	int indexOfTheString = 0;

	//forks and process the child then pass the index and the lines.

	while(totalCount < maxChildProcess && totalCount < lines ){ 					
				
			if(waitpid(0,NULL, WNOHANG)> 0)
				ptr_count--;

			if(ptr_count < 20 && indexOfTheString < lines){
				ptr_count++;
				totalCount++;
		
				childpid=fork();

				if(childpid < 0) {
					perror("Fork failed");
				} else if(childpid == 0) {
				 
					char buffer1[bufSize];
					sprintf(buffer1, "%d", indexOfTheString);

					char buffer2[bufSize];
					sprintf(buffer2, "%d", lines);
				
					//exec the index and lines tot he child
					execl("./palin","palin",buffer1,buffer2,(char *)0);

					snprintf(errorMessage, sizeof(errorMessage), "%s: Error: ", arg0Name);
	    	 			perror(errorMessage);		
					exit(0);
				} else {
					
				}
			
				indexOfTheString += 5;
			}
	}

	//unlink the semaphore		
	sem_unlink("Palin");
	sem_unlink("NotPalin");

	sem_close(shmPalin);
	sem_close(shmNotPalin);
	
	shmdt(shared); //detaches a section of shared memory
    	shmctl(shmid, IPC_RMID, NULL);  // deallocate the memory    	
 }


//signal calls
void signalCall(int signum)
{
    int status;
    

if (signum == SIGINT)
        printf("\nSIGINT received by main\n");
    else
        printf("\nSIGALRM received by main\n");
 
    while(wait(&status) > 0) {
        if (WIFEXITED(status))  /* process exited normally */
                printf("User process exited with value %d\n", WEXITSTATUS(status));
        else if (WIFSIGNALED(status))   /* child exited on a signal */
                printf("User process exited due to signal %d\n", WTERMSIG(status));
        else if (WIFSTOPPED(status))    /* child was stopped */
                printf("User process was stopped by signal %d\n", WIFSTOPPED(status));
    }
  
  	sem_unlink("Palin");
        sem_unlink("NotPalin");

	sem_close(shmPalin);
	sem_close(shmNotPalin);
	

	kill(0, SIGTERM);

    //clean up program before exit (via interrupt signal)
    shmdt(shared); //detaches a section of shared memory
    shmctl(shmid, IPC_RMID, NULL);  // deallocate the memory
   
      exit(EXIT_SUCCESS);
 }

 
//count the lines of the file
   
int countLines(FILE *file) {
  int lines = 0;
  char c;
  char last = '\n';
  while (EOF != (c = fgetc(file))) {
    if (c == '\n' && last != '\n') {
      ++lines;
    }
    last = c;
  }
  /* Reset the file pointer to the start of the file */
  rewind(file);
  return lines;
}

//help menu
void helpMenu() {
		printf("---------------------------------------------------------------| Help Menu |--------------------------------------------------------------------------\n");
		printf("-h help menu\n"); 
		printf("-i inputfilename                      | inputfilename is where the filename reads and it will show error if there is no filename found on the directory.\n"); 
		printf("-n int				      | int for max processor\n"); 
		printf("------------------------------------------------------------------------------------------------------------------------------------------------------\n");
}

