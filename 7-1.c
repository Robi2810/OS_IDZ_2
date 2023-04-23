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


void intHandler(int dummy) {
    printf("[son] SIGINT Detected!\n");
    keepRunning = 0;
}

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
    printf("Can\'t add 1 to semaphor\n");
    exit(-1);
  }  
}


void child(int semid, shared_memory* shmptr, double part, int sum, int i)  {
  printf("I am a client #%d!\n", getpid());
  sleep(rand() % 10 + 2);
  double value = sum * part;
  shmptr->arr[i] = value;
  sem_open(semid);
  exit(0);
}



int main(int argc, char **argv) {
  if (argc < 2) {
    printf("usage: ./main <sum parts>");
    return -1;
  }
  signal(SIGINT, intHandler);
  int sum = atoi(argv[1]);
  key_t key = ftok("7-1.c", 0);
  shared_memory *shared_mem_ptr;
  int semid;
  int shmid;

  int children[8];
  
  if ((semid = semget(key, 1, 0666 | IPC_CREAT)) < 0) {
    printf("[Error]: Can\'t create semaphore\n");
    return -1;
  }

  if ((shmid = shmget(key, sizeof(shared_memory), 0666 | IPC_CREAT)) < 0) {
    printf("[Error]: Can\'t create shmem\n");
    return -1;
  }
  if ((shared_mem_ptr = (shared_memory*) shmat(shmid, NULL, 0)) == (shared_memory *) -1) {
    printf("[Error]: Cant shmat!\n");
    return -1;
  } 

  semctl(semid, 0, SETVAL, 0);
  shared_mem_ptr->sem_id = semid;
  // create child processes
  
  printf("[info]: Let's check lawyer\n");
  sem_close(semid);
printf("[info]: 1 ready\n");
  sem_close(semid);
printf("[info]: 2 ready\n");
  sem_close(semid);
printf("[info]: 3 ready\n");
  sem_close(semid);
printf("[info]: 4 ready\n");
  sem_close(semid);
printf("[info]: 5 ready\n");
  sem_close(semid);
printf("[info]: 6 ready\n");
  sem_close(semid);
printf("[info]: 7 ready\n");
  sem_close(semid);
printf("[info]: 8 ready\n");
printf("[info]: complete\n");
  double counter_sum;
  for(int i = 0; i < 8; i++){
	counter_sum += shared_mem_ptr->arr[i];
  }
  if(counter_sum == sum){
	printf("total: %lf \ninput-sum: %d\n[Correct]\n", counter_sum, sum);
  }
  else{
	printf("LAWYER - SKAMer!\n[INCORRECT]\n");
  }
  //printf("%lf \n %d", counter_sum, sum);
  if (semctl(semid, 0, IPC_RMID, 0) < 0) {
	printf("Can\'t delete semaphore\n");
  	return -1;
  }
  shmdt(shared_mem_ptr);
  shmctl(shmid, IPC_RMID, NULL);
  return 0;
}
