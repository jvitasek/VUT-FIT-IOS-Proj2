/**
 * @file 	rivercrossing.c
 * @author 	Jakub Vitasek <me@jvitasek.cz>
 * @date 	04.05.2014
 * @brief 	1BIT Operating Systems Project 1: Rivercrossing
 * 
 * The program carries out a basic fork in both of which serfs and hackers
 * are created. They enter a pier and go further. The program meets the
 * constraints of the assignment.
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>

// function declarations
void *print_message_function_h( void *ptr );
void *print_message_function_s( void *ptr );
void *start_h();
void *start_s();
void serf_pier(int x);
void hacker_pier(int x);
void row_boat();

// semaphores created for mutual exclusion
sem_t Sem, sem_board_h, sem_board_s, sem_row_finished, sem_pier, sem_start_row, sem_file;

int NS = 0, NH = 0, A = 0, P, in_boat = 0; // global variables
int H, S, R;
FILE *fp;

int main(int argc, char *argv[])
{
	pthread_t ts, th; // thread declarations

	P = atoi(argv[1]); // retrieving values from command line	
	H = atoi(argv[2]);	
	S = atoi(argv[3]);	
	R = atoi(argv[4]);
	
	if((argc!=5)||(P==0 || P%2!=0)|| (H<0 || H>5000)|| (S<0 || S>5000)|| (R<0 || R>5000))
	{
		fprintf(stderr, "Invalid value of argument\n");
		exit(1);
	}

	H*=1000;
	S*=1000;
	R*=1000;
	
	fp = fopen("rivercrossing.out", "w");
	if(fp==NULL)
	{
		fprintf(stderr, "Unable to access rivercrossing.out\n");		
		exit(2);
	}

	srand(time(NULL));
	sem_init(&Sem, 0, 1); // semaphore for global variables
	sem_init(&sem_board_h,0,0); // semaphore for boarding synchronization of hackers
	sem_init(&sem_board_s,0,0); // semaphore for boarding synchronization of serfs
	sem_init(&sem_row_finished,0,0); // semaphore for signalling cruise finish
	sem_init(&sem_start_row,0,0); // semaphore for signalling cruise start
	sem_init(&sem_pier,0,1); // semaphore to signal entry to pier
	sem_init(&sem_file,0,1); // semaphore for file

	pthread_create( &ts, NULL, start_s, NULL); // starting thread for serf creation
	pthread_create( &th, NULL, start_h, NULL); // starting thread for hacker creation
	

	 // waiting for threads to finish
	pthread_join( ts, NULL);
	pthread_join( th, NULL);
	
	fclose(fp);

	return 0;
}

void *start_h() // hacker creation thread
{
	int i;
	int *tmp;
	pthread_t threads[P];
	for(i=0;i<P;i++) // creating hackers
	{
		tmp=malloc(sizeof(int));
		if(tmp==NULL)
		{
			fprintf(stderr,"Unable to allocate memory to int\n");		
			exit(2);
		}
		*tmp=i+1;
		pthread_create(&threads[i],NULL, print_message_function_h, (void*) tmp);
		usleep(rand()%H);
	}
	
	 // waiting for threads to finish
	for(i=0;i<P;i++)
	{
		if(pthread_join(threads[i],NULL))
		{
			fprintf(stderr,"Unable to join thread\n");		
			exit(2);
		}
	}
	return 0;
}

void *start_s() // serf creation thread
{
	int i;
	int *tmp;
	pthread_t threads[P];
	for(i=0;i<P;i++) // creating serfs
	{
		tmp=malloc(sizeof(int));
		if(tmp==NULL)
		{
			fprintf(stderr,"Unable to allocate memory to int\n");		
			exit(2);
		}
		*tmp=i+1;
		pthread_create(&threads[i],NULL, print_message_function_s, (void*) tmp);
		usleep(rand()%S);
	}

	 // waiting for threads to finish
	for(i=0;i<P;i++)
	{
		if(pthread_join(threads[i],NULL))
		{
			fprintf(stderr,"Unable to join thread\n");		
			exit(2);
		}
	}
	return 0;
}

void *print_message_function_h( void *ptr )
{
	int x=*(int*)(ptr);
	sem_wait( &Sem );
	sem_wait( &sem_file );
	fprintf(fp,"%d: hacker: %d: started\n", ++A,x);
	sem_post( &sem_file );
	sem_post( &Sem );
	sem_wait( &sem_pier); // waiting for access to pier
	hacker_pier(x); // enter pier
	while(1) // checking if hacker thread can be terminated
	{
		if(A>= (P*10))
		{
			sem_wait( &Sem );
			sem_wait( &sem_file );
			fprintf(fp,"%d: hacker: %d: finished\n", ++A,x);
			sem_post( &sem_file );
			sem_post( &Sem );
			break;
		}
	}
	return 0;	
}
void *print_message_function_s( void *ptr )
{
	int x=*(int*)(ptr);
	sem_wait( &Sem );
	sem_wait( &sem_file );
	fprintf(fp,"%d: serf: %d: started\n", ++A,x);
	sem_post( &sem_file );
	sem_post( &Sem );
	sem_wait( &sem_pier); // waiting for access to pier
	serf_pier(x); // enter pier
	while(1) // checking if serf thread can be terminated
	{
		if(A>= (P*10))
		{
			sem_wait( &Sem );
			sem_wait( &sem_file );
			fprintf(fp,"%d: serf: %d: finished\n", ++A,x);
			sem_post( &sem_file );
			sem_post( &Sem );
			break;
		}
	}
	return 0;
}

void hacker_pier(int x)
{
	sem_wait( &Sem );
	NH++; // incrementing number of hackers on pier
	sem_post( &Sem );
	sem_wait( &sem_file );
	fprintf(fp,"%d: hacker: %d: waiting for boarding: %d:%d\n",++A,x,NH,NS); 
	sem_post( &sem_file );
	if(NH==4) // if 4 hackers on pier then all board the boat
	{
		sem_wait( &Sem );
		NH=0;
		sem_post( &Sem );
		sem_post(&sem_board_h);
		sem_post(&sem_board_h);
		sem_post(&sem_board_h);
	}
	else if(NH>=2 && NS>=2) // if a group of 2 hackers and 2 serfs can be made from pier then the group boards the boat
	{
		sem_wait( &Sem );
		NH-=2;
		NS-=2;
		sem_post( &Sem );
		sem_post(&sem_board_h);
		sem_post(&sem_board_s);
		sem_post(&sem_board_s);
	}
	else // if no group can be made then wait for next process to enter the pier
	{
		sem_post( &sem_pier);
		sem_wait(&sem_board_h);
	}
	
	sem_wait( &Sem );
	sem_trywait( &sem_pier);
	sem_wait( &sem_file );
	fprintf(fp,"%d: hacker: %d: boarding: %d:%d\n",++A,x,NH,NS); 
	sem_post( &sem_file );
	in_boat++;
	sem_post( &Sem );
	
	if(in_boat==4) // if process is the last to enter
	{
		sem_post(&sem_start_row);
		sem_post(&sem_start_row);
		sem_post(&sem_start_row);
		sem_wait( &sem_file );
		fprintf(fp,"%d: hacker: %d: captain\n",++A,x);
		sem_post( &sem_file );
		usleep(rand()%R);
		row_boat();
		sem_post( &sem_pier);
	}
	else // if process is not the last to enter
	{
		sem_wait(&sem_start_row);
		sem_wait( &sem_file );
		fprintf(fp,"%d: hacker: %d: member\n",++A,x);
		sem_post( &sem_file );
	}
	
	sem_wait(&sem_row_finished);
	sem_wait( &Sem );
	sem_wait( &sem_file );
	fprintf(fp,"%d: hacker: %d: landing: %d:%d\n",++A,x,NH,NS);
	sem_post( &sem_file );
	sem_post( &Sem );
	sem_post( &sem_pier);
}

void serf_pier(int x)
{
	sem_wait( &Sem );
	NS++; // incrementing number of serfs on pier
	sem_post( &Sem );
	sem_wait( &sem_file );
	fprintf(fp,"%d: serf: %d: waiting for boarding: %d:%d\n",++A,x,NH,NS); 
	sem_post( &sem_file );
	if(NS==4) // if 4 serfs on pier then all board the boat
	{
		sem_wait( &Sem );
		NS=0;
		sem_post( &Sem );
		sem_post(&sem_board_s);
		sem_post(&sem_board_s);
		sem_post(&sem_board_s);
	}
	else if(NH>=2 && NS>=2) // if a group of 2 hackers and 2 serfs can be made from pier then the group boards the boat
	{
		sem_wait( &Sem );
		NH-=2;
		NS-=2;
		sem_post( &Sem );
		sem_post(&sem_board_h);
		sem_post(&sem_board_h);
		sem_post(&sem_board_s);
	}
	else // if no group can be made then wait for next process to enter the pier
	{
		sem_post( &sem_pier);
		sem_wait(&sem_board_s);
	}
	
	sem_wait( &Sem );
	sem_trywait( &sem_pier);
	sem_wait( &sem_file );
	fprintf(fp,"%d: serf: %d: boarding: %d:%d\n",++A,x,NH,NS); 
	sem_post( &sem_file );
	in_boat++;
	sem_post( &Sem );
	
	if(in_boat==4) // if process is the last to enter
	{
		sem_post(&sem_start_row);
		sem_post(&sem_start_row);
		sem_post(&sem_start_row);
		sem_wait( &sem_file );
		fprintf(fp,"%d: serf: %d: captain\n",++A,x);
		sem_post( &sem_file );
		usleep(rand()%R);
		row_boat();
		sem_post( &sem_pier);
	}
	else // if process is not the last to enter
	{
		sem_wait(&sem_start_row);
		sem_wait( &sem_file );
		fprintf(fp,"%d: serf: %d: member\n",++A,x);
		sem_post( &sem_file );
	}
	
	sem_wait(&sem_row_finished);
	sem_wait( &Sem );
	sem_wait( &sem_file );
	fprintf(fp,"%d: serf: %d: landing: %d:%d\n",++A,x,NH,NS);
	sem_post( &sem_file );
	sem_post( &Sem );
}
		
void row_boat() // setting semaphores to simulate finishing the cruise
{
	sem_wait( &Sem );
	in_boat=0;
	sem_post( &Sem );
	
	sem_post(&sem_row_finished);
	sem_post(&sem_row_finished);
	sem_post(&sem_row_finished);
	sem_post(&sem_row_finished);
}
