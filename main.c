#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>	
#include <sys/shm.h>
#define SHMKEY 9784

void helpMenu();
void forkProcess(int maxChildProcess, int numberChildProcess,char *inputFileName, char *outputFileName, int increment, char *arg0Name);
void shareClock(int increment);
int countLines(FILE *file);

void signalCall(int signum);

int timer = 2;
int shmid;
int *shmPtr;
typedef struct {
	char text[100][64];
} CharArray;

CharArray *shared;

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
			case 'o':
				strcpy(outputFileName, optarg);
				break;
			case 'n': maxChildProcess = atoi(optarg);
				break;
			case 's':numberChildProcess = atoi(optarg);
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
	
	//alarm for 2 real life second
	alarm(timer);
	forkProcess(maxChildProcess, numberChildProcess,inputFileName,outputFileName,increment,argv[0]);
		
		
	return 0;
}	

void forkProcess(int maxChildProcess, int numChildProcess, char *inputFileName, char *outputFileName,int increment,char *arg0Name) {
		
	char * args[2];
	int bufSize = 100;
	char buffer[bufSize];
		
	pid_t childpid;
	int ptr_count = 0;
	char errorMessage[1000];
	
	shmid = shmget(SHMKEY, 5  * sizeof(CharArray), IPC_CREAT | 0777);
	if(shmid < 0) {
		printf("//shmget failed in master\n");	
		exit(1);	
	}

	shared = shmat(shmid, NULL, 0);

	if(shared == -1 ){
            	printf("shmat failed in master");
            	exit(2);	
        }
	
	FILE *f1 = fopen(inputFileName, "r");

	if(f1 == NULL){
		snprintf(errorMessage, sizeof(errorMessage), "%s: Error: ", arg0Name);
	     	perror(errorMessage);	
		return;
	}
		


/*
	while(fgets(buffer,bufSize,f1)!= 0){ 


		printf("%s",buffer);

	}

	line = countLines(f1);
*/
	

	childpid=fork();
	int status;

	if(childpid == 0) {

		execl ("./user", "user", NULL); 
		//execl("./user","user",duration,outputFileName,(char *)0);
				 
	} else {
		wait(&status);
	} 

	shmdt(shared); //detaches a section of shared memory
    	shmctl(shmid, IPC_RMID, NULL);  // deallocate the memory    
		
 }


//signal calls
void signalCall(int signum)
{
    int status;
  //  kill(0, SIGTERM);
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
    kill(0, SIGTERM);
    //clean up program before exit (via interrupt signal)
  //  shmdt(shmPtr); //detaches a section of shared memory
   // shmctl(shmid, IPC_RMID, NULL);  // deallocate the memory
   
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
		printf("                                      | output filename should be default name is output.dat  where the result is generated.\n");
		printf("-o outputfilename                     | this command will use the default input file which is input.dat\n"); 
		printf("  				      | then create an output result to outputfilename(Which is the user specified name of the file \n"); 
		printf("-i inputfilename -o outputfilename    | this command can use inputfilename (user choose the name)\n");
		printf("				      | generate output to the outputfilename(user choose the outputname) if it doesnt exist create one.\n"); 
		printf("-n int				      | int for max processor\n"); 
		printf("-s int				      | int for max child processor\n"); 
		printf("------------------------------------------------------------------------------------------------------------------------------------------------------\n");
}

