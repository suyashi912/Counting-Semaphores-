/* Name : Suyashi Singhal 
Roll no: 2019478*/
#include <errno.h>
#include <semaphore.h>
#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define TRUE 1											//Value when philosopher is hungry
#define FALSE 0 										//Value when philosopher is eating
#define THINK 2 										//Value when philosopher is thinking 
int N;   //Number of philosophers

struct mysemaphore{										//This is the struct for my counting semaphore 					

	int sema_count;                  					//counter for the counting semaphore
	pthread_mutex_t mymutex; 							//pthread mutex used to implement mutual inclusion					
};

struct phil 											//it stores the thread id and number for each philospher
{
	pthread_t thread_id; 								//thread id
	int 	phil_number;  								//philospher number

}; 

int init(struct mysemaphore *sema, int init_value); 	//function initializes the semaphore structure
int wait(struct mysemaphore *sema); 					// non blocking wait
int signal(struct mysemaphore *sema);          			// non blocking signal 
int signal_debugg(struct mysemaphore *sema); 			 //signal for debugging
void dinner(int phil_number);                         	//the main function executed by each philospher
void check(int phil_number); 							//the function for checking whether the philospher can eat or not 


struct mysemaphore *global; 							//global semaphore
struct mysemaphore *dinner_bowls; 						//semaphores for 2 serve bowls 
struct mysemaphore *dinner_forks; 						// semaphores for dinner forks 
int *hungry;											// array to store state of a philospher
struct phil *philosophers;   							//struct array to store thread id and number of philosopher 

int init(struct mysemaphore *sema, int init_value )		//this function initializes the semaphore structure
{
	sema->sema_count = init_value; 						//initializes the counter  
	if(pthread_mutex_init(&(sema->mymutex), NULL) !=0)	 //initializing the mutex variable and error handling
	{
		printf("Error: %s \n", strerror(errno));
		return 0; 
	}

	return 0; 

}
//NON BLOCKING WAIT - it tries to acquire the lock. If it is able to acquire the lock, it decrements the 
// value of the semaphore. It returnns 0 only, when the resultant value of semaphore is positive. Else, it 
// returns -EINVAL 
int wait(struct mysemaphore *sema)   
{
	int lock = pthread_mutex_trylock(&(sema->mymutex)); 	//trying to aquire the lock
	
	if(lock == 0)											// if lock is acquired 
{
		sema->sema_count = sema->sema_count - 1; 			// decrementing counter by 1 
		if(sema->sema_count>0)								// counter is positive, return 0 
	{
		pthread_mutex_unlock(&(sema->mymutex));
		return 0; 
	}
	else
	{
		sema->sema_count = 0;  								// don't let the counter be negative 
		pthread_mutex_unlock(&sema->mymutex);
		return -EINVAL; 
	}
	
}
	return -EINVAL; 
	
}

//NON BLOCKING SIGNAL - it tries to acquire the lock. If it is able to acquire the lock, it increases the 
// value of the semaphore. It returnns 0 only, when the resultant value of semaphore is positive. Else, it 
// returns -EINVAL 
int signal(struct mysemaphore *sema)
{

	int lock = pthread_mutex_trylock(&(sema->mymutex)); //trying to aquire the lock 
	if(lock == 0)										//lock acquired 
{
		sema->sema_count = sema->sema_count++; 			// semaphore value incremented 
		if(sema->sema_count>0)							// if value is positive, returns 0 
	{
		pthread_mutex_unlock(&sema->mymutex); 
		return 0; 
	}
	else
	{
		pthread_mutex_unlock(&sema->mymutex);
		return -EINVAL; 
	}

}
	return -EINVAL; 
}
//SIGNAL FUNCTION FOR DEBUGGING - prints the value of the semaphore at any given time 
int signal_debugg(struct mysemaphore *sema)
{
	//for debugging 
	pthread_mutex_lock(&sema->mymutex);					//aquiring the lock
	int value = sema->sema_count; 						
	printf("Semaphore value: %d (for debugging purposes)", value);
	pthread_mutex_unlock(&sema->mymutex);				//releasing lock 
	return value; 										// returning semaphore value 

}

int main()
{
	printf("Enter the number of philosophers sitting on the table: \n");
	scanf("%d", &N);														// Input the number of philosophers

	global = (struct mysemaphore*)malloc(sizeof(struct mysemaphore)*1); 		//allocating pointer for global mutex           
	dinner_bowls = (struct mysemaphore*) malloc(sizeof(struct mysemaphore)*2); 	//aloocating space for array of 2 bowls
	dinner_forks = (struct mysemaphore*) malloc(sizeof(struct mysemaphore)*N); 	//aloocating space for array of N forks 
	hungry = (int*) malloc(sizeof(int)*N); 										 // allocating space for the state of N philosophers
	philosophers = (struct phil*) malloc(sizeof(struct phil)*N); 				//allocating space for array of N philosophers

	for(int i=0; i<N; i++)
	{
		philosophers[i].phil_number = i;			// initializing thread number 
		init(&dinner_forks[i], 0); 				    //initializing semaphore for forks
		hungry[i] = TRUE; 							// setting the state of each philosophersas hungry initially 
	}

	init(global, 1); 								// initializing global semaphore
	init(&dinner_bowls[0], 0); 						//initializing bowl 1 semaphore	
	init(&dinner_bowls[1], 0); 						//initializing bowl 2 semaphore

	for(int j=0; j<N; j++)
	{
		int create = pthread_create(&philosophers[j].thread_id, NULL, (void *)dinner, (void *)(intptr_t)j); //creating thread for each philosopher and error handling 
		if(create != 0)			//error handling 
		{
			printf("Error in thread creation: %s\n", strerror(errno));
			return EXIT_FAILURE; 
		}
	}

	for(int j=0; j<N; j++)
	{
		int join = pthread_join(philosophers[j].thread_id, NULL); 		//joining the threads after completion 
		if(join != 0)					//error handling
		{
			printf("Error in joining threads:%s\n", strerror(errno));
			return EXIT_FAILURE;  
		}
	}

	return 0; 
}
//This is the function executed by the threads

void dinner(int phil_number)
{

	int left_phil = (phil_number+N-1)%N; 							// left philosopher number
	int right_phil = (phil_number+1)%N;  							// right philosopher number
	int left_forks = phil_number; 									// left fork number
	int right_forks = right_phil;									// right fork  number
	while(TRUE)														// the loop will go on indefinitely
	{
		sleep(1);
		// thread tries to enter the critical section and acquire the bowls and forks 
		while(wait(global) == -1)									//we let the thread sleep till wait returns 0 
		{
			usleep(100); 
		}  
		hungry[phil_number] = TRUE; 
		printf("Philosoper %d is hungry.\n", phil_number);
		check(phil_number); 

		while(signal(global) == -1)
		{
			usleep(100); 
		} 	
		//in case the philosopher is unable to acquire the forks or bowl , it needs to wait for the other threads to release the resources 
		while(wait(&dinner_forks[left_forks]) ==-1 || wait(&dinner_bowls[0]) == -1 || wait(&dinner_bowls[1]) == -1)
		{
			usleep(100);
		}
		sleep(2);
	
		while(wait(global) == -1)
		{
			usleep(100);
		}
		//here we call the check function for left and right philosopher within the critical section 
		check(left_phil); 
		check(right_phil); 
		while(signal(global) == -1)
		{
			usleep(100); 
		}

	}

}
//this function is executed within the critical section 
//the philosopher tries to eat only when no other philosopher is eating at that time since only one philosopher can have both the bowls and eat 
void check(int phil_number)
{
		int left_phil = (phil_number+N-1)%N; 				//left philosopher number
		int right_phil = (phil_number+1)%N;  				//right philosopher number
		int left_forks = phil_number; 						//left fork number
		int right_forks = right_phil; 						//right fork number
		int count = 0;										// it counts the number of philosophers currently eating 
		for(int j=0; j<N;j++)
		{
			if(j!=phil_number)
			{
				if(hungry[j]==FALSE)
				{
					count=count+1;
				}
			}
		}
		if(count==0 && hungry[phil_number]==TRUE)			// if the philosopher is hungry and no other philosopher is eating 
		{	
				printf("Philosoper %d tries to acquire bowls and forks %d and %d\n", phil_number, left_forks, right_forks);
				sleep(0.5); 
				hungry[phil_number] = FALSE; 				// setting the state of philosopher to eating
				printf("Philosoper %d starts eating.\n",phil_number );  
				printf("Philosoper %d eats using forks %d and %d.\n", phil_number, left_forks, right_forks); 
				//philosopher is eating  
				printf("Philosoper %d puts down forks %d and %d.\n", phil_number, left_forks, right_forks);
				printf("Philosoper %d puts down the two bowls. \n",phil_number);
				hungry[phil_number] = THINK; 				// setting the state of philosopher to thinking after he has eaten 
				//after the philosopher has eaten, he releases the resources 
				while(signal(&dinner_forks[left_forks]) == -1 || signal(&dinner_bowls[0]) == -1 || signal(&dinner_bowls[1]) == -1)
				{
					usleep(100);   
				}

	}		
}