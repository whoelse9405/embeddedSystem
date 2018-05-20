#include <signal.h> //Signal 사용 헤더파일
#include <unistd.h>
#include <stdio.h> 
#include <string.h> 
#include <errno.h>
#include <stdlib.h> //exit() 사용 헤더파일

#include <wiringPi.h>

#define RGBLEDPOWER  24 //BCM_GPIO 19

#define RED	8 //27
#define GREEN	7 //28
#define BLUE	9 //29

void sig_handler(int signo); // SIGINT 사용 마지막 종료 함수

int main (void)
{
	signal(SIGINT, (void *)sig_handler);	//시그널 핸들러 함수
	
	if (wiringPiSetup () == -1)
	{
		fprintf(stdout, "Unable to start wiringPi: %s\n", strerror(errno));
		return 1 ;
	}
  
	pinMode(RGBLEDPOWER, OUTPUT);
	pinMode(RED, OUTPUT);
	pinMode(GREEN, OUTPUT);
	pinMode(BLUE, OUTPUT);

	int i;

	for (i =0; i<10 ;i++)
	{
 
		digitalWrite(RGBLEDPOWER, 1);

		digitalWrite(RED, 1);
		digitalWrite(BLUE, 0);
		digitalWrite(GREEN, 0);
		
		delay(1000);
		
		digitalWrite(RED, 0);
		digitalWrite(BLUE, 1);
		digitalWrite(GREEN, 0);
		
		delay(1000);
		
		digitalWrite(RED, 0);
		digitalWrite(BLUE, 0);
		digitalWrite(GREEN, 1);
		
		delay(1000);		
	}
	digitalWrite(GREEN, 0);
  return 0 ;
}

void sig_handler(int signo)
{
    printf("process stop\n");
	
	digitalWrite(RED, 0);
	digitalWrite(GREEN, 0);
	digitalWrite(BLUE, 0);
	digitalWrite(RGBLEDPOWER, 0); //Off
	
	exit(0);
}

