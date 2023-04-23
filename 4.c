#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#define CHILDREN_MAX 16

static volatile int keepRunning = 1;

typedef struct shared_memory {
  double arr[8];
}shared_memory;


void intHandler(int dummy) {
    printf("[Cutter] SIGINT Detected!\n");
    keepRunning = 0;
}

void sem_close(int semid) {
 struct sembuf parent_buf = {
    .sem_num = 0,
    .sem_op = -1,
    .sem_flg = 0
  };
  if (semop(semid, & parent_buf, 1) < 0) {
    printf("Can\'t sub 1 from semaphor\n");
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



int main(int argc, char ** argv) {
  if (argc < 10) {
    printf("usage: ./main <sum parts>");
    return -1;
  }
  double part[8];
  for(int i = 2; i < 10; i++){
	part[i - 2] = atof(argv[i]);
  }
  signal(SIGINT, intHandler);
  int sum = atoi(argv[1]);
  key_t key = ftok(argv[0], 0);
  shared_memory *shared_mem_ptr;
  int semid;
  int shmid;

  int children[8];
  
  if ((semid = semget(key, 1, 0666 | IPC_CREAT)) < 0) {
    printf("Can\'t create semaphore\n");
    return -1;
  }

  if ((shmid = shmget(key, sizeof(shared_memory), 0666 | IPC_CREAT)) < 0) {
    printf("Can\'t create shmem\n");
    return -1;
  }
  if ((shared_mem_ptr = (shared_memory*) shmat(shmid, NULL, 0)) == (shared_memory *) -1) {
    printf("Cant shmat!\n");
    return -1;
  } 
  semctl(semid, 0, SETVAL, 0);

  // create child processes
  for (int i = 0; i < 8; i++) {
    children[i] = fork();
    if (children[i] == 0) {
      child(semid, shared_mem_ptr, part[i], sum, i);
    }
  }

  printf("I am a POLICE MAN\n");
  sem_close(semid);
  sem_close(semid);
  sem_close(semid);
  sem_close(semid);
  sem_close(semid);
  sem_close(semid);
  sem_close(semid);
  sem_close(semid);
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
