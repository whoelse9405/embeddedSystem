#include <signal.h> //Signal 사용 헤더파일
#include <unistd.h>
#include <stdio.h> 
#include <string.h> 
#include <errno.h>
#include <stdlib.h> //exit() 사용 헤더파일

#include <wiringPi.h>

#define DCMOTOR	23 // BCM_GPIO 13

void sig_handler(int signo); // SIGINT 사용 마지막 종료 함수

int main (void)
{
	signal(SIGINT, (void *)sig_handler);	//시그널 핸들러 함수
	
	if (wiringPiSetup () == -1)
	{
		fprintf(stdout, "Unable to start wiringPi: %s\n", strerror(errno));
		return 1 ;
	}

	pinMode (DCMOTOR, OUTPUT) ;

	for (;;)
	{
		//printf("here - DCMOTOR on\n");
		digitalWrite (DCMOTOR, 1) ; // On		
	}
	return 0 ;
}

void sig_handler(int signo)
{
    printf("process stop\n");
	digitalWrite (DCMOTOR, 0) ; // Off
	
	exit(0);
} 



