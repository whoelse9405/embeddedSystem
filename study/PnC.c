#include<stdio.h>
#include<pthread.h>

#define MAX 10
#define LOOPS 20

pthread_cond_t empty,fill;
pthread_mutex_t mutex;

//pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
//pthread_cond_t c = PTRHEAD_COND_INITIALIZER;

int buffer[MAX];
int fill_ptr=0;
int use_ptr=0;
int count=0;

void put(int value)
{
	buffer[fill_ptr]=value;
	fill_ptr=(fill_ptr+1)%MAX;
	printf("put : %d\n",value);
	count++;
}

int get()
{
	int tmp=buffer[use_ptr];
	use_ptr=(use_ptr+1)%MAX;
	printf("get : %d\n",tmp);
	count--;
	return tmp;
}

void *producer(void *arg)
{
	int i;
	for(i=0;i<LOOPS;i++)
	{
		pthread_mutex_lock(&mutex);
		while(count==MAX)
			pthread_cond_wait(&empty,&mutex);
		put(i);
		pthread_cond_signal(&fill);
		pthread_mutex_unlock(&mutex);
	}

}

void *consumer(void *arg)
{
	int i;
	for(i=0;i<LOOPS;i++)
	{
		pthread_mutex_lock(&mutex);
		while(count==0)
			pthread_cond_wait(&fill,&mutex);
		int tmp=get();
		pthread_cond_signal(&empty);
		pthread_mutex_unlock(&mutex);
	}
}

int main(int argc, char *argv[])
{
	printf("main begin\n");
	
	pthread_t p1,p2;
	pthread_create(&p1,NULL,producer,NULL);
	pthread_create(&p2,NULL,consumer,NULL);
	
	pthread_join(p1,NULL);
	pthread_join(p2,NULL);
		
	printf("main end\n");
	return 0;
}
