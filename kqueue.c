#include <sys/event.h>
#include <sys/time.h> 
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h> 

/**
 * Given the file descriptor this function,
 * waits 1 second, writes "B," waits 1 second
 * then writes "C"
 */
void child_one_func(int fd){

  sleep(1);
  write(fd, "B - 2", 5);
  sleep(1);
  write(fd, "C - 3", 5);
  close(fd);
}

/**
 * Given the file descriptor this function,
 * writes "A" waits 3seconds then writes "D"
 */
void child_two_func(int fd){

  write(fd, "A - 1", 5);
  sleep(3);
  write(fd, "D - 4", 5);
  close(fd);
}

int main(){

  int kq = kqueue();
  
  /* Double-array of fds for the pipe() */
  int **fds = malloc(2 * sizeof(int *));
  struct kevent *evlist = malloc(sizeof(struct kevent) * 2);
  struct kevent *chlist = malloc(sizeof(struct kevent) * 2);

  int i;
  for (i = 0; i < 2; i++){

    /* Create a pipe */
    fds[i] = malloc(2 * sizeof(int));
    pipe(fds[i]);

    int read_fd = fds[i][0];
    int write_fd = fds[i][1];
    
    /* Generates a new process */
    pid_t pid = fork();

    /* child */
    if (pid == 0){

      close(read_fd);

      if (i == 0) { child_one_func(write_fd); }
      else if (i == 1) { child_two_func(write_fd); }

      /* Closes child */
      exit(0);

    }else{
      close(write_fd);
    }

    EV_SET(&chlist[i], read_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);
  }

  char str[10];
  while(1){
    
    if(kevent(kq, chlist, 1, evlist, 1, NULL) > 0){
      for(i = 0; i < 2; i++){
	ssize_t bytes = read(chlist[i].ident, &str, 10);
	if(bytes > 0)
	  printf("Read: %s\n", str);
	
	if(strcmp(str, "D - 4") == 0)
	  return 0;
      }
    }
  }
  close(kq);
  return 0;
}
