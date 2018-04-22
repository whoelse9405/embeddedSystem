/*
   if temperature goes over 25'C, run the water pump for 1 sec.
*/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <wiringPi.h>
#include <softPwm.h>

#define MAXTIMINGS 85

#define PUMP	21		//GPIO 13
#define DHTPIN	11		//CE1

int ret_humid,ret_temp;
static uint8_t dht22_dat[5]={0,0,0,0,0};

static uint8_t sizecvt(const int read)
{
	if(read>255 || read<0)
	{
		printf("error : out of range value uint8_t,%d\n",read);
		exit(EXIT_FAILURE);
	}
	
	return (uint8_t)read;
}

void sig_handler(int signo)
{
	printf("process stop\n");
	digitalWrite(PUMP,LOW);
	exit(0);
}

int read_dht22_dat()
{
	uint8_t laststate = HIGH;
	uint8_t counter = 0;
	uint8_t j=0,i;

	dht22_dat[0]=dht22_dat[1]=dht22_dat[2]=dht22_dat[3]=dht22_dat[4]=0;

	pinMode(DHTPIN,OUTPUT);
	digitalWrite(DHTPIN,HIGH);
	delay(10);
	digitalWrite(DHTPIN,LOW);
	delay(18);

	digitalWrite(DHTPIN,HIGH);
	delayMicroseconds(40);
	pinMode(DHTPIN,INPUT);

	for(i=0;i<MAXTIMINGS;i++)
	{
		counter=0;
		while(sizecvt(digitalRead(DHTPIN))==laststate)	//HIGH
		{
			counter++;
			delayMicroseconds(1);
			if(counter==255) 
			{
				break;
			}
		}
		laststate=sizecvt(digitalRead(DHTPIN));			//LOW
		if(counter == 255) break;

		if((i >= 4)&&(i%2 == 0))
		{
			dht22_dat[j/8] <<= 1;
			if(counter > 50)
				dht22_dat[j/8] |= 1;
			j++;

		}
	}
	
	if((j>=40)&&(dht22_dat[4]==((dht22_dat[0]+dht22_dat[1]+dht22_dat[2]+dht22_dat[3])&0xFF)))
	{
		float t,h;
	
		h=(float)dht22_dat[0]*256+(float)dht22_dat[1];
		h/=10;
		t=(float)(dht22_dat[2]&0x7F)*256+(float)dht22_dat[3];
		t/=10.0;

		if((dht22_dat[2]&0x80)!=0) t*=-1;
			
		ret_humid=(int)h;
		ret_temp=(int)t;

		return ret_temp;
	}
	else
	{
		//printf("Data not good, skip \n");
		return 0;
	}
}

void setup()
{
	if(wiringPiSetup() == -1)
		exit(EXIT_FAILURE);
	
	if(setuid(getuid())<0)
	{
		perror("Dropping privileges failed\n");
		exit(EXIT_FAILURE);
	}

	pinMode(PUMP,OUTPUT);
	
	signal(SIGINT,(void*)sig_handler);
}


int main()
{
	setup();

	while(1)
	{
		if(read_dht22_dat()==0)
		{
			delay(500);	
		}
		else 
		{
			printf("온도 : %d'C\n",ret_temp);
			if(ret_temp>=25)
			{
				digitalWrite(PUMP,HIGH);
				delay(1000);
				digitalWrite(PUMP,LOW);
				delay(1000);
			}
		}

	}

	return 0;
}
