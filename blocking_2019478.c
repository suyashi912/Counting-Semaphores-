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

#define THINK 2                                    	//Value when philosopher is thinking
#define TRUE 1										//Value when philosopher is hungry
#define FALSE 0										//Value when philosopher is eating

int N;   											//Number of philosophers

struct my_semaphore{								//This is the struct for my counting semaphore 

	int sema_count;                  				//counter for the counting semaphore
	pthread_mutex_t mymutex; 						//pthread mutex used to implement mutual inclusion					
	pthread_cond_t condition; 						// mutex conditional variable 

};

struct phil 										//it stores the thread id and number for each philospher
{
	pthread_t thread_id; 							// thread id
	int 	phil_number;  							// philosopher number 		

}; 

int init(struct my_semaphore *sema, int init_value); //function initializes the semaphore structure
void wait(struct my_semaphore *sema);                //blocking wait
void signal(struct my_semaphore *sema);              //blocking signal
int signal_debugg(struct my_semaphore *sema);        //signal for debugging
void dinner(int phil_number);                        //the main function executed by each philospher thread
void check(int phil_number); 						 //the function for checking whether the philospher can eat or not 


struct my_semaphore *global;                          //global semaphore
struct my_semaphore *dinner_bowls;                    //semaphores for 2 serve bowls
struct my_semaphore *dinner_forks;                    // semaphores for dinner folks 
int *hungry;                                          // array to store state of a philospher
struct phil *philosophers;                            //struct array to store thread id and number of philosopher 


int init(struct my_semaphore *sema, int init_value )    //this function initializes the semaphore structure
{
	sema->sema_count = init_value;                       //initializes the counter
	if(pthread_mutex_init(&(sema->mymutex), NULL) !=0)   //initializing the mutex variable and error handling
	{
		printf("Error: %s \n", strerror(errno));
		return 0; 
	}

	if(pthread_cond_init(&(sema->condition), NULL) != 0)   //initializing mutex conditional variable and error handling
	{
		printf("Error: %s \n", strerror(errno));
		return 0; 
	}
	return 0; 

}
//BLOCKING WAIT
void wait(struct my_semaphore *sema)                  //this command decreases the semaphore value by 1 and blocks if the value becomes negative
{
	if(pthread_mutex_lock(&(sema->mymutex)) != 0)	// aquire the lock 
	{
		printf("Error: %s\n", strerror(errno ));
	} 			  
	sema->sema_count = sema->sema_count - 1; 		  // decreasing the counter by 1 	
	if(sema->sema_count<0)							  // if counter is negative thread is blocked 					
	{
		if(pthread_cond_wait(&(sema->condition), &(sema->mymutex))!=0)  //atomically blocks the thread on the condition variable  
			{
				printf("Error: %s\n", strerror(errno ));
			}
	}
	if(pthread_mutex_unlock(&(sema->mymutex))!=0)			  //releasing lock  
	{
		printf("Error: %s\n", strerror(errno ));
	}

}
//BLOCKING
void signal(struct my_semaphore *sema)					//this command increases the semaphore value by 1 and if there were any waiting processes, it unblocks
{

	if(pthread_mutex_lock(&(sema->mymutex))!=0) 				//to aquire the lock 
		{
			printf("Error: %s\n", strerror(errno ));
		}
	sema->sema_count = sema->sema_count + 1; 			//increasing the counting semaphore by 1
	if(pthread_cond_signal(&(sema->condition))!=0)			//signalling the conditional mutex and unblocks it 
		{
			printf("Error: %s\n", strerror(errno ));
		}
	if(pthread_mutex_unlock(&(sema->mymutex))!=0)				//releasing lock  
	{
		printf("Error: %s\n", strerror(errno ));
	}
}

//SIGNAL FUNCTION FOR DEBUGGING - prints the value of the semaphore at any given time 
int signal_debugg(struct my_semaphore *sema)		
{
	//for debugging 
	pthread_mutex_lock(&sema->mymutex);					//aquiring the lock
	int value = sema->sema_count;                       //assigning the semaphore value to be returned by the function 
	printf("Semaphore value: %d (for debugging purposes)", value); //prints value of semaphore 
	pthread_mutex_unlock(&sema->mymutex);				//releasing lock 

	
	return value; 

}

int main()
{
	printf("Enter the number of philosophers sitting on the table: ");
	scanf("%d", &N); 										// Input the number of philosophers

	global = (struct my_semaphore*)malloc(sizeof(struct my_semaphore)*1);          //allocating pointer for global mutex           
	dinner_bowls = (struct my_semaphore*) malloc(sizeof(struct my_semaphore)*2);   //aloocating space for array of 2 bowls
	dinner_forks = (struct my_semaphore*) malloc(sizeof(struct my_semaphore)*N);   //allocating space for array of N forks 
	hungry = (int*) malloc(sizeof(int)*N); 										   // allocating space for the state of N philosophers
	philosophers = (struct phil*) malloc(sizeof(struct phil)*N); 				   //allocating space for array of N philosophers

	for(int i=0; i<N; i++)            
	{
		philosophers[i].phil_number = i;                                           // initializing thread number 
		init(&dinner_forks[i], 0); 												   //initializing semaphore for forks 
		hungry[i] = TRUE; 														   // setting the state of each philosophersas hungry initially 
	}

	init(global, 1); 															 	// initializing global semaphore
	init(&dinner_bowls[0], 0); 														//initializing bowl 1 semaphore
	init(&dinner_bowls[1], 0); 														//initializing bowl 2 semaphore
	for(int j=0; j<N; j++)     
	{
		int create = pthread_create(&philosophers[j].thread_id, NULL, (void *)dinner, (void *)(intptr_t) j); //creting thread for each philosopher 
		if(create != 0)																// error handling 
		{
			printf("Error in thread creation: %s\n", strerror(errno));
			return EXIT_FAILURE; 
		}
	}

	for(int j=0; j<N; j++)
	{
		int join = pthread_join(philosophers[j].thread_id, NULL);                   // joining the threads after completion 
		if(join != 0)																// error handling
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

	int left_phil = (phil_number+N-1)%N; 											// left philosopher number
		int right_phil = (phil_number+1)%N;  										// right philosopher number
		int left_forks = phil_number; 												// left fork number
		int right_forks = right_phil; 												// right fork number
	while(TRUE) 																    // the loop will go on indefinitely 
	{ 
		// thread tries to enter the critical section and acquire the bowls and forks 
		wait(global);  																
		hungry[phil_number] = TRUE;                          //philosopher is hungry
		printf("Philosoper %d is hungry.\n", phil_number);
		check(phil_number); 
		signal(global); 
		//in case the philosopher is unable to acquire the forks or bowl , it needs to wait for the other threads to release the resources 
		wait(&dinner_forks[left_forks]);
		wait(&dinner_bowls[0]); 
		wait(&dinner_bowls[1]); 
		sleep(1); 	 
		//here we call the check function for left and right philosopher within the critical section 
		wait(global); 
		check(left_phil); 
		check(right_phil); 
		signal(global); 

	}

}

//this function is executed within the critical section 
//the philosopher tries to eat only when no other philosopher is eating at that time since only one philosopher can have both the bowls and eat 
void check(int phil_number)
{
		int left_phil = (phil_number+N-1)%N; 							//left philosopher number
		int right_phil = (phil_number+1)%N;   							//right philosopher number
		int left_forks = phil_number; 									//left forks number
		int right_forks = right_phil; 									//right forks number
		int count = 0;													// it counts the number of philosophers currently eating 
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
		if(count==0 && hungry[phil_number]==TRUE)						// if the philosopher is hungry and no other philosopher is eating 
		{
				printf("Philosoper %d tries to acquire bowls and forks %d and %d\n", phil_number, left_forks, right_forks);
				sleep(1.5); 
				hungry[phil_number] = FALSE;							// setting the state of philosopher to eating 
				printf("Philosoper %d starts eating\n",phil_number ); 
				printf("Philosoper %d eats using forks %d and %d.\n", phil_number, left_forks, right_forks); 
				//philosopher is eating
				printf("Philosoper %d puts down forks %d and %d.\n", phil_number, left_forks, right_forks);
				printf("Philosoper %d puts down the two bowls. \n",phil_number);
				hungry[phil_number] = THINK;							// setting the state of philosopher to thinking after he has eaten 
				//after the philosopher has eaten, he releases the resources 
				signal(&dinner_forks[left_forks]); 						
				signal(&dinner_bowls[0]); 
				signal(&dinner_bowls[1]); 

	}		
}
