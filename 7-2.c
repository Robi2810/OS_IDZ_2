#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

// status: 0 - cutter free; 1 - cuttting
// buf: conatins ids of new client
// when cutter is cutting he will close sem
// when free, open it
// first client to access open sem will change status of 
// cutter to cutting and close sem

#define CHILDREN_MAX 16

static volatile int keepRunning = 1;

typedef struct shared_memory {
  double arr[8];
  int sem_id;
}shared_memory;

void sem_close(int semid) {
 struct sembuf parent_buf = {
    .sem_num = 0,
    .sem_op = -1,
    .sem_flg = 0
  };
  if (semop(semid, & parent_buf, 1) < 0) {
    printf("[Error]: Can\'t sub 1 from semaphor\n");
    exit(-1);
  }
}

void sem_open(int semid) {
 struct sembuf parent_buf = {
    .sem_num = 0,
    .sem_op = 1,
    .sem_flg = 0
  };
  if (semop(semid, & parent_buf, 1) < 0) {
    printf("[Error]: Can\'t add 1 to semaphor\n");
    exit(-1);
  }  
}


void child(int semid, shared_memory* shmptr, double part, int sum, int i)  {
  printf("I am a son num: %d #%d!\n", i, getpid());
  sleep(1);
  double value = sum * part;
  shmptr->arr[i] = value;
  sem_open(semid);
}



int main(int argc, char ** argv) {
  if (argc < 4) {
    printf("[usage]: ./main <sum parts>");
    return -1;
  }
  double part;
  int index = 0;
  //for(int i = 2; i < 10; i++){
  part = atof(argv[2]);
  index = atoi(argv[3]);
  //}
  int sum = atoi(argv[1]);
  key_t key = ftok("7-1.c", 0);
  shared_memory *shared_mem_ptr;
  int semid;
  int shmid;

  if ((semid = semget(key, 1, 0666 | IPC_CREAT)) < 0)
return -1;
  
  if ((shmid = shmget(key, sizeof(shared_memory), 0666 | IPC_CREAT)) < 0) {
    printf("[Error]: Can\'t create shmem\n");
    return -1;
  }
  if ((shared_mem_ptr = (shared_memory*) shmat(shmid, NULL, 0)) == (shared_memory *) -1) {
    printf("[Error]: Cant shmat!\n");
    return -1;
  } 
  child(semid, shared_mem_ptr, part, sum, index);
  shmdt(shared_mem_ptr);
  return 0;
}
