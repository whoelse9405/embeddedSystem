#include <signal.h> //Signal 사용 헤더파일
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h> //exit() 사용 헤더파일

#include <wiringPi.h>

#define PUMP	21	// BCM_GPIO 5
#define FAN	22		// BCM_GPIO 6
#define DCMOTOR 23	// BCM_GPIO 13
#define RGBLEDPOWER  24 //BCM_GPIO 19

#define RED	7
#define GREEN	8
#define BLUE	9

int thr_id1, thr_id2;
pthread_t p1_thread, p2_thread;	

void sig_handler(int signo); // 마지막 종료 함수

void Bpluspinmodeset(void);

void *t1_function(void *data)
{
    int num = *((int *)data);
	printf("Thread Start\n");
    sleep(5);
	printf("Thread end\n");
}

void *t2_function(void *data)
{
    digitalWrite(RGBLEDPOWER, 1);
	
	printf("Thread2 Start\n");
    while(1){
		digitalWrite(RED, 1);
    delay(500);
    digitalWrite(RED, 0);
    digitalWrite(GREEN, 1);
    delay(500);
    digitalWrite(GREEN, 0);
    digitalWrite(BLUE, 1);
    delay(500);
    digitalWrite(BLUE, 0);
	}
	printf("Thread2 end\n");
}

int main (void)
{
	//pthread_t p_thread;	
	//int thr_id;
    int status;
    int a = 100;
	int b = 200;
	
	signal(SIGINT, (void *)sig_handler);	//시그널 핸들러 함수
	
	if(wiringPicheck()) printf("Fail");
		
	Bpluspinmodeset();	
	
	int i;
	
	printf("Before Thread\n"); 
    thr_id1 = pthread_create(&p1_thread, NULL, t1_function, (void *)&a);
	thr_id2 = pthread_create(&p2_thread, NULL, t2_function, (void *)&b);
	
    if (thr_id1 < 0)
    {
        perror("thread create error : ");
        exit(0);
    }

    // 식별번호 p_thread 를 가지는 쓰레드를 detach 시켜준다.
    pthread_detach(p1_thread);
	pthread_detach(p2_thread);	

	for (i = 0; 1 < 100; i++)
	{
		digitalWrite (PUMP, 1) ; // On
    digitalWrite (FAN, 1) ; // On
    digitalWrite(DCMOTOR, 1); //On

    delay (5000) ; // ms

    //digitalWrite (PUMP, 0) ; // Off
    //digitalWrite (FAN, 0) ; // Off
    //digitalWrite(DCMOTOR, 0); //Off
 
    //delay (5000) ;
	}
  return 0 ;
}

int wiringPicheck(void)
{
	if (wiringPiSetup () == -1)
	{
		fprintf(stdout, "Unable to start wiringPi: %s\n", strerror(errno));
		return 1 ;
	}
}

void Bpluspinmodeset(void)
{
	pinMode (PUMP, OUTPUT);
	pinMode (FAN, OUTPUT);
	pinMode (DCMOTOR, OUTPUT);
	pinMode(RGBLEDPOWER, OUTPUT);
	pinMode(RED, OUTPUT);
	pinMode(GREEN, OUTPUT);
	pinMode(BLUE,OUTPUT);
}

void sig_handler(int signo)
{
    printf("process stop\n");
	digitalWrite (PUMP, 0) ; // Off
	digitalWrite (FAN, 0) ; // Off
	digitalWrite (DCMOTOR, 0) ; // Off
	digitalWrite (RGBLEDPOWER, 0) ; // Off
	digitalWrite (RED, 0) ; // Off
	digitalWrite (GREEN, 0) ; // Off
	digitalWrite (BLUE, 0) ; // Off
	
	pthread_cancel (p1_thread);
	pthread_cancel (p2_thread);
	printf("thread cancel\n");
	
	
	exit(0);
}
