#include <signal.h> //Signal 사용 헤더파일
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h> //exit() 사용 헤더파일

#include <wiringPi.h>
#include <softPwm.h>

#define MOTORCONTROL  23	//GPIO 13 - Motor

void sig_handler(int signo); // SIGINT 핸들러 함수

int main (void)
{
	if(wiringPicheck()) printf("Fail\n");
		
	signal(SIGINT, (void *)sig_handler);	
	
	pinMode (MOTORCONTROL, OUTPUT);
	
	softPwmCreate(MOTORCONTROL, 0, 100);
	
	int i;
	int delaytime = 100;
	while(1)
	{	
		printf("here \n");	
		
		softPwmWrite(MOTORCONTROL, 5);
		delay(delaytime);	
		
	}	
  
  return 0 ;
}

int wiringPicheck(void)
{
	if (wiringPiSetup () == -1)
	{
		fprintf(stdout, "Unable to start wiringPi: %s\n", strerror(errno));
		exit(1) ;
	}
}

void Bpluspinmodeset(void)
{
	pinMode (MOTORCONTROL, OUTPUT);
	
}

void sig_handler(int signo) // ctrl-c 로 종료시 실행되는 함수
{
    printf("process stop\n"); 	
	digitalWrite(MOTORCONTROL, 0);
	exit(0);
}


