#include <signal.h> //Signal 사용 헤더파일
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h> //exit() 사용 헤더파일

#include <wiringPi.h>
#include <softPwm.h>

#include <wiringPi.h>

#define RGBLEDPOWER  24 //BCM_GPIO 19
#define RED		29	// BCM_GPIO 16 - OUT
#define GREEN	28	// BCM_GPIO 20 - OUT
#define BLUE	27 // BCM_GPIO 21 - OUT

void Bpluspinmodeset(void);

void setRGB(int r, int g, int b);
void sig_handler(int signo); // SIGINT 핸들러 함수

int main (void)
{
	if(wiringPicheck()) printf("Fail");
		
	Bpluspinmodeset();
	
	signal(SIGINT, (void *)sig_handler);
	
	int i;
	digitalWrite(RGBLEDPOWER, 1);
	
	while(1)
	{	
		
		int i = 0, j = 0, k=0;
		printf("RGB LED Various Color");
 
		softPwmCreate(RED, 0, 255);
		softPwmCreate(GREEN, 0, 255);
		softPwmCreate(BLUE, 0, 255);
 
		for (i = 0; i <= 250; i=i+10){
			for (j = 0; j <= 250; j=j+10){
				for (k = 0; k <= 250; k=k+10){
					setRGB(i,j,k);
					delay(50);
					printf("R:%d G:%d B:%d\n", i, j, k);
				}
			}
		}
 
		setRGB(0,0,0);//clear LED
		delay(50);				
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
	pinMode(RGBLEDPOWER, OUTPUT);
	pinMode (RED, OUTPUT);
	pinMode (GREEN, OUTPUT);
	pinMode (BLUE, OUTPUT);	
	
}

void sig_handler(int signo) // ctrl-c 로 종료시 실행되는 함수
{
    printf("process stop\n");
	digitalWrite(RED, 0);
	digitalWrite(GREEN, 0);
	digitalWrite(BLUE, 0);
	digitalWrite(RGBLEDPOWER, 0); //Off
	exit(0);
}

void setRGB(int r, int g, int b){
  softPwmWrite(RED, 255-r);
  softPwmWrite(GREEN, 255-g);
  softPwmWrite(BLUE, 255-b);
}

