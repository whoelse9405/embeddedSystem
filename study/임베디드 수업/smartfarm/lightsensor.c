#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<stdlib.h>
#include<wiringPi.h>

#define LIGHTSEN_OUT 2

int main(void)
{
	if(wiringPiSetup()<0)
	{
		fprintf(stderr,"Unable to setup wiringPi: %s\n",strerror(errno));
		return 1;
	}
	
	pinMode(LIGHTSEN_OUT,INPUT);

	while(1)
	{
		if(digitalRead(LIGHTSEN_OUT)==0)
			printf("light full !\n");
		else
			printf("dark \n");

		delay(200);
	}

	return 0;
}
