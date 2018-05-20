#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <wiringPi.h>

#define PUMP	21	// BCM_GPIO 5
#define FAN	22		// BCM_GPIO 6
#define DCMOTOR 23	// BCM_GPIO 13
#define RGBLEDPOWER  24 //BCM_GPIO 19

#define RED	27
#define GREEN	28
#define BLUE	29

void Bplus_pinmodeset(void);
int testmode(void);

int main (void)
{
	if(wiringPicheck())
	{	
		printf("Fail");
		return;
	}
	
	Bplus_pinmodeset();
	
	while(1)
	{
		int exit;
		exit = testmode();
		if(exit == 1) break;
	}
}

int wiringPicheck(void)
{
	if (wiringPiSetup () == -1)
	{
		fprintf(stdout, "Unable to start wiringPi: %s\n", strerror(errno));
		return 1 ;
	}
}

void Bplus_pinmodeset(void)
{
	pinMode (PUMP, OUTPUT);
	pinMode (FAN, OUTPUT);
	pinMode (DCMOTOR, OUTPUT);
	pinMode(RGBLEDPOWER, OUTPUT);
	pinMode(RED, OUTPUT);
	pinMode(GREEN, OUTPUT);
	pinMode(BLUE,OUTPUT);
}

int testmode(void)
{
	int testnum;
	char stop;
	
	printf("Input Number for Smart Farm Actuator Test(1 ~ 4) and Enter !!! \n");
	printf("1. Water Pump Test\n");
	printf("2. Fan Test\n");
	printf("3. DC Motor Test\n");
	printf("4. RGB Led Test\n");
	printf("5. Exit\n");
	
	scanf("%d", &testnum);
	
	if(testnum == 1)
	{
		printf("\n\n\n\n\nWater Pump Testing.... To Stop enter c or C \n");
		digitalWrite (PUMP, 1);	//on
		scanf("%c", &stop);
		if(stop == 'c') digitalWrite (PUMP, 0) ;
		
		//delay (2000) ; // ms
		//digitalWrite (PUMP, 0) ; // Off		
	}
	
	if(testnum == 2)
	{
		digitalWrite (FAN, 1) ; // On
		scanf("%c", &stop);
		if(stop == 'c') digitalWrite (FAN, 0) ;
		//delay (2000) ; // ms
		//digitalWrite (FAN, 0) ; // Off
	}
	
	if(testnum == 3)
	{
		digitalWrite(DCMOTOR, 1); //On
		delay (2000) ; // ms
		digitalWrite(DCMOTOR, 0); //OFF
	}
	
	if(testnum == 4)
	{
		digitalWrite(RGBLEDPOWER, 1); // power on
		digitalWrite(RED, 1);
		delay(500);
		digitalWrite(RED, 0);
		digitalWrite(GREEN, 1);
		delay(500);
		digitalWrite(GREEN, 0);
		digitalWrite(BLUE, 1);
		delay(500);
		digitalWrite(BLUE, 0);
		digitalWrite(RGBLEDPOWER, 1); // power off
	}
	
	if(testnum == 5)
	{
		return 1;
	}
	
	return 0;
	
}

