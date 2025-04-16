#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

struct sigaction {
	void     (*sa_handler)(int);
	void     (*sa_sigaction)(int, siginfo_t *, void *);
	sigset_t   sa_mask;
	int        sa_flags;
	void     (*sa_restorer)(void);
};

//void list_hunts(){
  
//}

int main(int argc, char* argv[]){
  char comanda[20];
  int stare = 0;
  
  while(1){

    scanf("%19s", &comanda);
  if(strcmp(comanda,"start_monitor")){
      printf("p1 merge");
      stare = 1;
      int pid;
      pid = fork();
      if(pid < 0){
	perror("eroare");
	etix(1);
      }
      if(pid == 0){
	
      }
  }
  if(strcmp(comanda,"list_hunts")){
    printf("p2 merge");
  }
  if(strcmp(comanda,"list_treasures")){
    printf("p3 merge");
  }
  if(strcmp(comanda,"view_treasures")){
    printf("p4 merge");

  }
  if(strcmp(comanda,"stop_monitor")){
    printf("adsadsadsa");
  }
  if(strcmp(comanda,"exit")){
    //wait();
    printf("gata");
    stare = 0;
  }
  }
  printf("proces oprit");
  return 0;
}

