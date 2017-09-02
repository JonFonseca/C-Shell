#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#define DELIM " \n"
//#define BUFFER = 100;
char **tokenize(char *args);
int executeCommand(char **args);
int executePipe(char **args, int PIPE);

int main(int argc, char **argv){
  
  //Path Variables for current directory path
  char path[100];
  
  
  //Input Variables
  
  char *inputLine = (char *)calloc(256, sizeof(char));
  // char *inputLine;
  char **args;
  
  char *history[10] = {NULL};
  int historyStore = 0;
  int historyCount = 0;
  
  // USED AS BOOLS
  int historyUsed = 0; //check if up arrow was used to get a command so the last else doesnt fuck with inputLine
  int PIPE = 0; // used as both bool and index         

  char input[100];
  int status = 1;
  char *token;
  
  int c = 0;
  int x = 0;
  int w = 0;
  
  int i = 0;
  
  struct termios origConfig;
  tcgetattr(0, &origConfig);
  
  struct termios newConfig = origConfig;
  
  newConfig.c_lflag &= ~(ICANON|ECHO);
  newConfig.c_cc[VMIN] = 1;
  newConfig.c_cc[VTIME] = 2;
  tcsetattr(0, TCSANOW, &newConfig);
  
  while(status)
    {
      //inputLine = (char *)calloc(256, sizeof(char));
      // Print Prompt
      getcwd(path, 100);
      printf("%s> ", path);
      
      fflush(stdout);
      
      do{
		read(0, &input, 10);
		if(input[0] == 27 /*&& input[1] == 91 && input[2] == 65)*/){
			if(input[1] == 91 && input[2] == 65){
				historyUsed = 1;
				
				if(history[historyCount] != NULL){
				//printf("%d\n", strlen(inputLine));
					while(w < strlen(inputLine)){
					printf("\b \b");
					w++;
					}
					w = 0;
	      
					printf("%s", history[historyCount]);
					inputLine = strdup(history[historyCount]);
					x = strlen(history[historyCount]);
	      
					if(historyCount > 0){
						historyCount--;
					}
				}
			}
	  
	  else if(/*input[0] == 27 &&*/ input[1] == 91 && input[2] == 66){
	    //###########################################################
	    historyUsed = 1;
	    if(history[historyCount] != NULL){
			//printf("TEST: %s\n ::", inputLine);
			while(w < strlen(inputLine)){
				printf("\b \b");
				w++;
			}
			w = 0;
	      
			printf("%s", history[historyCount+1]);
			inputLine = strdup(history[historyCount+1]);
			x = strlen(history[historyCount+1]);
	      
			if(historyCount < historyStore-2){
				historyCount++;
			}
	    }
	    //############################################################
	  }
	  else if((/*input[0] == 27 &&*/ input[1] == 91 && input[2] == 67)){
	    //printf("arrow right\n");
		// IGNORE!!!
	  }
	  else if((/*input[0] == 27 &&*/ input[1] == 91 && input[2] == 68)){
	    //printf("arrow left\n");
		// IGNORE!!!
	  }
	}
	else if(input[0] == 127/*delete*/ || input[0] == 8/*backspace*/){
		//printf("BACKSPACE");
		if(strlen(inputLine) > 0){
			printf("\b \b");
			//printf("strlen inputLine %d", strlen(inputLine));
			inputLine[strlen(inputLine)-1] = '\0';
			x--;
			//printf(" --strlen inputLine %d, %s", strlen(inputLine), inputLine);
		}
	}
	else{
	  if(/*historyUsed != 1 && */input[0] != '\n' && input[0]){
	    printf("%c", input[0]);
	    inputLine[x] = input[0];
	    x++;
	  }
	}
	//Arrow check done reloop till '\n' detected
	fflush(stdout);
      } while(input[0] != '\n');
      printf("\n");
      inputLine[x] = '\0';
      x = 0;
      
      //printf("testing: %s:\n", inputLine);
      
      if(strlen(inputLine) == 0){
		continue;
      }
      

      history[historyStore] = strdup(inputLine);
      
      if(historyStore < 10){
	historyStore++;
      }
      else{
	while(historyStore > 1){
	  history[historyStore] = strdup(history[historyStore-1]);
	  if (historyStore > 1){
	    historyStore--;
	  }
	}
	historyStore = 0;
      }
      historyCount = historyStore - 1;
  
  /**********************************************************************
   *                        E N D   H I S T O R Y                       *
   **********************************************************************/
  //Tokenize Arguments
  args = tokenize(inputLine);
  
  //Look for | make it null and keep track of position if found
	i = 0;
 while(args[i] != NULL){
    if (strcmp(args[i], "|") == 0)
    {
      //printf("NEED TO PIPE!!!\n");
      PIPE = i+1; //NEED TO PIPE       
      args[i] = NULL;
      break;
    }
	//printf(" args[%d]: %s\n",i,args[i]);
	i++;
  }


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      //Check if cd directory
      if(strcmp(args[0],"cd") == 0 && args[1] != NULL){
        if(chdir(args[1]) == -1){
	  printf("No such file or directory\n");
	}
        goto label;
      }
      else if(strcmp(args[0],"cd") == 0){
        //chdir("..");
        chdir(getenv("HOME"));
        goto label;
      }

      //Check if Exit
      if(strcmp(args[0], "exit") == 0){
		  char done;
		  printf("Are you sure you want to exit? (Y/N): ");
		  fflush(stdout);
		  
		  read(0, &done, 1);
		  printf("%c\n", done);
		  
		  if(done == 'Y' || done == 'y'){
			break;
		  }
		  else{
			  goto label;
		  }
      }

      //Execute Command
      if(PIPE == 0){
        executeCommand(args);
      } 
      else{
        executePipe(args, PIPE);          
        PIPE = 0;
        //printf("pipe:%d\n", PIPE);
        //break;
      }


      //memset(inputLine, 0, 256);
	  
      //RESETS FOR RELOOP
	  label:
      inputLine[0]='\0';
      historyUsed = 0;
      c = 0;
      i = 0;

    }
  tcsetattr(0, TCSANOW, &origConfig);
  return 0;
}

/**********************************************************************
 *                          F U N C T I O N S                         *
 **********************************************************************/

char **tokenize(char *line){

  char **tokens = malloc(256*sizeof(char *)); //BUFFER
  char *token;

  int position = 0;

  token = strtok(line, " ");

  while(token != NULL){
    int i =0;

    while(i < sizeof(token)){
      if(token[i] == '\n'){
        token[i] = '\0';
      }
      i++;
    }
   
    tokens[position] = token;
    token = strtok(NULL, DELIM);

    position++;
  }

  tokens[position] = NULL;

  return tokens;
}


int executeCommand(char **args){

  pid_t pid, wpid;
  int status;
  int filedescriptor;
  FILE *yourmom;
  
  pid = fork();
  if(pid == 0){
	  // CHECK FOR MERGE
	  if(strcmp(args[0], "merge") == 0){
		yourmom = fopen(args[4], "w+");
		
		if(yourmom == NULL){
			perror("you fucked up, child fail");
			exit(1);
		}
		filedescriptor = fileno(yourmom);
		close(1);
		dup(filedescriptor);
		args[0] = "cat";
		args[3] = NULL;
	}
	// END MERGE CHECK
    if(execvp(args[0], args) == -1){
      perror("child failed");
    }
    exit(EXIT_FAILURE);
  }
  else if(pid < 0){
    //Error Forking @_@
    perror("fork failed -_-");
  }
  else {
    // Parent
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while(!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 0;
}

int executePipe(char **args, int PIPE){                  
  //printf("makes it to executePipe\n");

 // create pipe
    int fd[2];
    pipe(fd);
    pid_t pid, wpid;

    if (fork() == 0) {        // FIRST FORK
        // child process
      close(1); // closing write to stdout
      dup(fd[1]); // write to pipe 1 instead

        // close extra descriptors
        close(fd[0]);
        close(fd[1]);

        // exec the given program
        if (execvp(args[0], args) == -1) {
            perror("failed to start subprocess");
            return EXIT_FAILURE;
            
            
        }
    }

    close(fd[1]); // Done writing to pipe so close the write end
    // Leave fd[0] open so second process can read from it

    if(fork() == 0){ // Create second child
    // Second child
      close(0); // close stdin
    dup(fd[0]); // read from pipe instead of stdin
    close(fd[0]); // close extra descriptor

    if(execvp(args[PIPE], &args[PIPE]) == -1){
      perror("parent failed");
      return EXIT_FAILURE;
    }
  }

    // close file descriptor
    close(fd[0]);

    // wait for child to finish
    wait(0);
    wait(0);
    wait(0);

  return 0;
}
