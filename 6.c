#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
// status: 0 - cutter free; 1 - cuttting
// buf: conatins ids of new client
// when cutter is cutting he will close sem
// when free, open it
// first client to access open sem will change status of 
// cutter to cutting and close sem

#define CHILDREN_MAX 16

static volatile int keepRunning = 1;

typedef struct shared_memory {
  sem_t mutex;
  double arr[8];
}shared_memory;


void intHandler(int dummy) {
    printf("[Cutter] SIGINT Detected!\n");
    keepRunning = 0;
}


void child(shared_memory* shmptr, double part, int sum, int i)  {
  printf("I am a client #%d!\n", getpid());
  sleep(rand() % 10 + 2);
  double value = sum * part;
  shmptr->arr[i] = value;
  sem_post(&shmptr->mutex);
  exit(0);
}



int main(int argc, char ** argv) {
  if (argc < 10) {
    printf("usage: ./main <sum parts>");
    return -1;
  }
  double part[8];
  char sem_name[] = "sem-mutex";
  for(int i = 2; i < 10; i++){
	part[i - 2] = atof(argv[i]);
  }
  signal(SIGINT, intHandler);
  int sum = atoi(argv[1]);
  
  int children[8];
  char memn[] = "shared-memory"; //  имя объекта
  int mem_size = sizeof(shared_memory);
  int shm;
  sem_t *mutex;

  // СОздать память
  if ((shm = shm_open(memn, O_CREAT | O_RDWR, 0666)) == -1) {
      printf("Object is already open\n");
      perror("shm_open");
      return 1;
  } else {
      printf("Object is open: name = %s, id = 0x%x\n", memn, shm);
  }
  if (ftruncate(shm, mem_size) == -1) {
      printf("Memory sizing error\n");
      perror("ftruncate");
      return 1;
  } else {
      printf("Memory size set and = %d\n", mem_size);
  }

  //получить доступ к памяти
  void* addr = mmap(0, mem_size, PROT_WRITE, MAP_SHARED, shm, 0);
  if (addr == (int * ) - 1) {
      printf("Error getting pointer to shared memory\n");
      return 1;
  }

  shared_memory* shmem = addr;
  if ((mutex = sem_open(sem_name, O_CREAT, 0666, 0)) < 0) {
    printf("Error creating semaphore!\n");
      return 1;
  }
  shmem->mutex = *mutex;
  // create child processes
  for (int i = 0; i < 8; i++) {
    children[i] = fork();
    if (children[i] == 0) {
      child(shmem, part[i], sum, i);
    }
  }

  printf("I am a POLICE MAN\n");
  sem_wait(&shmem->mutex);
  sem_wait(&shmem->mutex);
  sem_wait(&shmem->mutex);
  sem_wait(&shmem->mutex);
  sem_wait(&shmem->mutex);
  sem_wait(&shmem->mutex);
  sem_wait(&shmem->mutex);
  sem_wait(&shmem->mutex);
  double counter_sum;
  for(int i = 0; i < 8; i++){
	counter_sum += shmem->arr[i];
  }
  if(counter_sum == sum){
	printf("total: %lf \ninput-sum: %d\n[Correct]\n", counter_sum, sum);
  }
  else{
	printf("LAWYER - SKAMer!\n[INCORRECT]\n");
  }
  //printf("%lf \n %d", counter_sum, sum);
  if (sem_unlink(sem_name) == -1) {
    perror ("sem_unlink"); exit (1);
  }
  close(shm);
  // удалить выделенную память
  if(shm_unlink(memn) == -1) {
    printf("Shared memory is absent\n");
    perror("shm_unlink");
    return 1;
  }
  return 0;
}
