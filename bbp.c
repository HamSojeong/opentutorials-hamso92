#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/types.h>
#include <linux/unistd.h>

/************************** define variable **************************/
#define RAND_DIVISOR 100000000
#define TRUE 1

/*********************** buffer_item structure ***********************/
typedef struct buffer_item{
	int ptid;
	int n;
	struct buffer_item *pNext;
}buffer_item;

/********************** define global variable ***********************/
int BUFF_SIZE;				// Buffer size

pthread_mutex_t mutex;			// The mutex lock			
sem_t full, empty;			// The semaphores	
buffer_item *buffer;			// The buffer pointer
buffer_item *pHead;
int counter;				// The buffer counter
int producer_counter;		

pthread_t tid;       			// Thread ID
pthread_attr_t attr; 			// Set of thread attributes

/********************** define global function ***********************/
void init(void);			// Initialize all data
void *producer(void *param); 		// The producer thread 
void *consumer(void *param); 		// The consumer thread 
int push(buffer_item *item);		// Add an item to the buffer
int pop(buffer_item *item);		// Remove an item from the buffer

/************************** initialize data **************************/
void init(void) {

	pthread_mutex_init(&mutex, NULL);	// Create the mutex lock
	sem_init(&full, 0, 0);			// Create the full semaphore and initialze to 0
	sem_init(&empty, 0, BUFF_SIZE);		// Create the empty semaphore and initialize to BUFFER_SIZE
	pthread_attr_init(&attr);		// Get the default attributes
	counter = 0;				// init buffer
}

/************************** Producer Thread **************************/
void *producer(void *param) {

	buffer_item *item=(buffer_item*)malloc(sizeof(buffer_item));

	while(TRUE) {

		sleep(10);			// Sleep 10 second

		item->ptid = (int)gettid();	// Get thread ID
		item->n = (int)param;		// Count producer
		item->pNext = NULL;

		sem_wait(&empty);		// Acquire the empty lock
		pthread_mutex_lock(&mutex);	// Acquire the mutex lock

		if(push(item))			// Push item
			fprintf(stderr, " Producer report error condition\n");
		else
			printf("[Producer %d] produced %d\n",item->ptid, item->n);

		pthread_mutex_unlock(&mutex);	// release the mutex lock
		sem_post(&full);			// signal full
	}
}

/************************** Consumer Thread **************************/
void *consumer(void *param) {

	buffer_item *item=(buffer_item*)malloc(sizeof(buffer_item));

	while(TRUE) {

		sleep(10);			// Sleep 10 second

		item->ptid = (int)gettid();	// Get thread ID
		item->n = (int)param;		// Count producer
		item->pNext = NULL;

		sem_wait(&full);		// aquire the full lock
		pthread_mutex_lock(&mutex);	// aquire the mutex lock

		if(pop(item))			// Pop item
			fprintf(stderr, "Consumer report error condition\n");
		else
			printf("[Consumer %ld] consumed %d from %d\n", (unsigned long)gettid(), item->n, item->ptid);

		pthread_mutex_unlock(&mutex);	// release the mutex lock
		sem_post(&empty);		// signal empty
	}
}

/********************* Add an item to the buffer *********************/
int push(buffer_item *item) {

	buffer_item *pCur;
	++producer_counter;
	item->n = producer_counter;
	/* When the buffer is not full add the item
	and increment the counter*/
	if(counter < BUFF_SIZE) {
		//buffer[counter] = item;;
		if(pHead == NULL){
			pHead = item;
		}else{
			pCur = pHead;
			while(pCur->pNext != NULL)
				pCur = pCur->pNext;
			pCur->pNext = item;
		}
		counter++;
		/*pCur = pHead;
		while(pCur != NULL){
			printf("%d-", pCur->n);
			pCur = pCur->pNext;
		}
		printf("\n");*/
		return 0;
	} else { // Error the buffer is full
		return -1;
	}
}

/******************* Remove an item from the buffer *******************/
int pop(buffer_item *item) {

	buffer_item *pCur;
	/* When the buffer is not empty remove the item
	and decrement the counter */
	if(counter > 0) {
		//*item = buffer[(counter-1)];
		item->ptid = pHead->ptid;
		item->n = pHead->n;
		pHead = pHead->pNext;
		counter--;
		return 0;
	} else { // Error the buffer is empty
		return -1;
	}
}

/***************************main function******************************/
int main(int argc, char *argv[]) {

	int i;					// Loop counter

	// Verify the correct number of arguments were passed in
	if(argc != 4) 
		fprintf(stderr, "USAGE:->/main->out <INT> <INT> <INT>\n");

	int numProd = atoi(argv[1]); 		// Number of producer threads
	int numCons = atoi(argv[2]); 		// Number of consumer threads 
	BUFF_SIZE=atoi(argv[3]);		// Number of buffer size

	// Allocate buffer
	//buffer = (buffer_item*)malloc(sizeof(buffer_item)*BUFF_SIZE);

	printf("\n2011722073 Ham Sojeong\n");

	init();					// Initialize data

	for(i = 0; i < numProd && i < BUFF_SIZE; i++){		// Create the producer threads
		pthread_create(&tid,&attr,producer, (void*)i+1);
		sleep(1);
	}

	for(i = 0; i < numCons; i++){		// Create the consumer threads
		pthread_create(&tid,&attr,consumer, NULL);
		sleep(1);
	}

	sleep(100);				

	printf("Exit the program\n");		// Exit the program
	exit(0);
}

pid_t gettid(void)
{
	return syscall(__NR_gettid);
}


