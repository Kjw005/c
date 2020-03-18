//Kenneth Williams
//Last Modified: 03/18/2020

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

int arr[1];

static void handler(int signo){
	printf("Battle over\n");
//	free(arr);
	exit(1);
}

int getSemaphoreID()
{
        char *filepath = "/tmp";
        int tokid = 0;
        key_t key;
        int semid;

        if((key = ftok(filepath, tokid)) == -1)			//Gets key for semaphore
                printf("Can not create token\n", errno);

        if ((semid = semget(key, 1, 0666 | IPC_CREAT)) == -1)	//Gets ID for semid
                printf("Can not create semaphore\n", errno);

       	return semid;
}

void initializeSemaphore(int semid, int value)
{
        union semun{
                int val;
                struct semid_ds* buf;
                unsigned short* array;
        } arg;

        arg.val = value;
	if((semctl(semid, 0, SETVAL, arg)) == -1)
		printf("Error setting semaphore to %d\n", value,  errno);

}

int  getSemaphoreValue(int semid){

        int semValue;
        if((semValue = semctl(semid, 0, GETVAL)) == -1)
                printf("Error getting semaphore value\n", errno);
        
	return semValue;
}

int incrementSemaphore(int semid, int area){
        struct sembuf op[1];
        int retval;

        op[0].sem_num =0;
        op[0].sem_op = 1;
        op[0].sem_flg = 0;

        if((retval = semop(semid, op, 1)) == -1)
                printf("Error incrementing semaphore\n", errno);

}

void decrementSemaphore(int semid){
        struct sembuf op[1];
        int retval;

        op[0].sem_num = 0;
        op[0].sem_op = -1;
        op[0].sem_flg = 0;

	if ((retval = semop(semid, op, 1)) == -1)
		printf("Error decrementing semaphore", errno);
}

void* getSharedResource()
{
        int tokid = 0;
        char *filepath = "/tmp";
        key_t key;
        int shmid;
        void *shm = NULL;
        size_t bufsz = getpagesize();

        if ((key = ftok(filepath, tokid)) == -1)
		printf("Can not create token\n", errno);

        if ((shmid = shmget(key, bufsz, IPC_CREAT | 0600)) == -1)
                printf("Can not create shared memory\n", errno);

        if ((shm = shmat(shmid, 0, 0)) == (void *)-1)
                printf("Unable to attach to shared memoryi\n", errno);

        return shm;
}

void sem_check(int start, int semValue, int size, int area){	
	

								//Index to the left
	if((arr[start-1] != semValue) && (start % size != 0)){
		int left_pid = fork();
		if(left_pid < 0){
			printf("Forking failed (left)\n");
			exit(1);
		}else if(left_pid == 0){
			arr[start - 1] == semValue;
			printf("Left Fork Success\n");
			
			kill(left_pid, 0);
		}
		
	}							//Index to the right
	else if((arr[start+1] != semValue) && (start % (size-1)!= 0 )){
		int right_pid = fork();
		if(right_pid <0){
			printf("Forking failed (right)\n");
			exit(1);
		}else if(right_pid ==0){
			arr[start + 1 ]= semValue;
			printf("Right Fork Success\n");
		
			kill(right_pid, 0);
		}
	}
								//Index above
	else if((arr[start - size] != semValue) &&(start > size)){
		int up_pid = fork();
		if(up_pid <0){
			printf("Forking Failed (Up)\n");
			exit(1);
		}else if(up_pid == 0){
			arr[start - size] = semValue;
			printf("Up Fork Success\n");
		
			kill(up_pid, 0);
		}
	}							//Index below
	else if((arr[start +size] != semValue) &&(start < (area - size))){
		int down_pid = fork();
		if(down_pid <0){
			printf("Forking Failed (Down)\n");
			exit(1);
		}else if(down_pid == 0){
			arr[start + size] = semValue;
			printf("Down Fork Success\n");
		
			kill(down_pid, 0);
		}

	}
}

int sem_changer(int semid, int x, int semValue){

	for(int i = 0; i < x; i++){
		int number = semctl(semid, arr[i] ,GETVAL);
	//	printf("Value: %d at count %d\n", number, i);
	for(int j = 0; j < i; i++){		
		if((number = semctl(semid, arr[j-1] ,GETVAL) != semValue) && ((j>0) && (j % 6 != 0))){
	//		if(semctl(semid, arr[i-1], SETVAL, semValue) < 0){
	//			printf("Error changing value");

			}

		}

	}
	return x;
}

void print_matrix(int size, int area, int semid,int semValue, int start){
 
//	getAllSemaphores(semid, size, arr);
	int counting = 0;	//Used to keep a constant count of all elements
  while(1){
	
                for(int i = 0; i < size; i++){

                        printf("[");

                        for(int j = 0; j < size; j++){
//                              printf("Counting: %d", counting );	
                        	int number = semctl(semid, arr[counting], GETVAL, 0);
				printf(" %d ", number);
                     		counting++;
                        			
			}
                        printf("]\n");
                }
        	sleep(1);
		printf("\n");
//		sem_changer(semid, area, semValue);	
	} 
}

int main (int argc, char *argv[]){
	if(argc != 4){
		printf("Please enter with format: \"-Size- -Start_Index- -Id-\"\n");
		exit(0);
	
	}
			
	if((atoi(argv[1]) *atoi(argv[1]))-1 < atoi(argv[2])||atoi(argv[2]) < 0){
		printf("Please enter an index within the array\n");
		exit(0);
	}

/*	struct sigaction act, oact;

	act.sa_handler = handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;

	if(sigaction(SIGINT, &act, &oact) == -1 ){
		printf("Error registering handler", errno);
	}
*/
	int size = atoi(argv[1]);
        int area = size * size;         //Calculates the size of the Matrix
        int arr[area];

	int *shm =(int*) malloc(sizeof(arr));
	 memset(arr, '\0', sizeof(arr));

	 //Creaters space for array and frees space in heap
	

	int start = atoi(argv[2]);
	int value = atoi(argv[3]);
	int semid = getSemaphoreID();;
	initializeSemaphore(semid, value);
	
	//New addition
//	int x = start;

	int semValue = getSemaphoreValue(semid);

//	sem_changer(semid, area, semValue);

//	int semValue = atoi(argv[2]);	
	
	printf("semVal: %d  Size of array: [%d x %d]\n", semValue,  atoi(argv[1]),atoi(argv[1]));

	if(argc != 4){
		printf("Format: Size Place_Marker Id\n" );
		return(0);
	}

//	place = (int*) getSharedResource();
//	int counter = 0;


        int counter = 0;


//	int *shm = malloc(sizeof(area));	//Allocates space, size of arena,
      					//	for the array (and semaphores)
	
		print_matrix(size, area, semid, semValue, start);
	
//		sem_check(start,semValue, size, area);	
//	}

	free(arr);
	exit(0);
	}

