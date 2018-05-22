#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<sys/types.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>
#include<signal.h>
#include<wiringPi.h>	//sensor
#include<wiringPiSPI.h>
#include<softPwm.h>		
#include<pthread.h>		//thread
#include<mysql/mysql.h>	//mysql
#include<time.h>
#include<math.h>

//define value
#define MAX 500
#define MONITORING_PERIOD 1
#define MAXTIMINGS 85
#define LOW 0
#define HIGH 1
#define SPI_CHANNEL 0
#define SPI_SPEED 1000000		//1Mhz
#define MIN_BRIGHT 150
#define MAX_BRIGHT 3000
#define THRESHOLD_BRIGHT 1500

//define sensor data pin
#define DHTPIN	11				//temp and humidity sensor
#define CS_MCP3208	8			//light detect sensor	
#define LED		24				//R=8 G=7 B=9 	
#define FAN		22		

//DB setting
#define DBHOST "localhost"
#define DBUSER "root"
#define DBPASS "root"
#define DBNAME "smartfarm"
#define DBSENDTIME 10000		//10s term


MYSQL *connector;
MYSQL_RES *result;
MYSQL_ROW roe;

pthread_cond_t onfan,onled;
pthread_mutex_t mutex;

uint8_t isLed=FALSE;
uint8_t isFan=FALSE;
int ret_temp,ret_humid;
int bright;
static uint8_t dht22_dat[5]={0,0,0,0,0};

static uint8_t sig_handler(int signo)
{
	digitalWrite(LED,LOW);
	digitalWrite(FAN,LOW);
	printf("process stop\n");
	exit(0);
}

static uint8_t sizecvt(const int read)
{
	if(read>255||read<0)
	{
		printf("error : out of range value uint8_t,%d\n",read);
		exit(EXIT_FAILURE);
	}
	return (uint8_t)read;
}

int read_mcp3208_adc(unsigned char adcChannel)
{
	unsigned char buff[3];
	int adcValue =0;

	buff[0] = 0x06|((adcChannel&0x07)>>2);
	buff[1] = ((adcChannel & 0x07)<<6);
	buff[2] = 0x00;

	digitalWrite(CS_MCP3208,0);
	wiringPiSPIDataRW(SPI_CHANNEL,buff,3);

	buff[1]=0x0f&buff[1];
	adcValue=(buff[1]<<8)|buff[2];

	digitalWrite(CS_MCP3208,1);

	return adcValue;
}


int read_dht22_dat()
{
	uint8_t laststate=HIGH;
	uint8_t counter =0;
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
		while(sizecvt(digitalRead(DHTPIN))==laststate)
		{
			counter++;
			delayMicroseconds(1);
			if(counter==255) break;
		}	
		laststate=sizecvt(digitalRead(DHTPIN));

		if(counter==255)break;
		
		if((i>=4)&&(i%2==0))
		{
			dht22_dat[j/8]<<=1;
			if(counter>50)
				dht22_dat[j/8]|=1;
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

		if((dht22_dat[2]&0x80)!=0)t*=-1;

		ret_humid=(int)h;
		ret_temp=(int)t;

		return ret_humid;
	}
	else 
		return 0;

}

void setup()
{
	//wringPi setup
	if(wiringPiSetup()==-1)
		exit(EXIT_FAILURE);

	if(setuid(getuid())<0)
	{
		perror("Drooping privileges failed\n");
		exit(EXIT_FAILURE);
	}

	if(wiringPiSPISetup(SPI_CHANNEL,SPI_SPEED)==-1)
	{
		fprintf(stdout, "wiringPiSPISetup Failed : %s\n",strerror(errno));
		exit(-1);
	}

	//sersor pin setup
	pinMode(CS_MCP3208,OUTPUT);	//light sensor
	pinMode(FAN,OUTPUT);
	pinMode(LED,OUTPUT);

	//init sensor
	digitalWrite(FAN,LOW);
	digitalWrite(LED,LOW);

	signal(SIGINT,(void*)sig_handler);
	
	//DB setup
	connector = mysql_init(NULL);
	if(!mysql_real_connect(connector,DBHOST,DBUSER,DBPASS,DBNAME,3306,NULL,0))
	{
		fprintf(stderr,"%s\n",mysql_error(connector));
		exit(-1);
	}	
}

void *sendData(void *arg)
{
	while(1)
	{
		char query[1024];

		pthread_mutex_lock(&mutex);
		//do send data to database 
		sprintf(query,"insert into sensorData values (now(), %d,%d,%d,%d,%d)",ret_temp,ret_humid,bright,isFan,isLed);

		if(mysql_query(connector,query))
		{
			fprintf(stderr,"%s\n",mysql_error(connector));
			printf("Write DB error\n");
		}	
		pthread_mutex_unlock(&mutex);

		delay(DBSENDTIME);
	}

}

void *getTempBrightness(void *arg)
{

	while(1)
	{
		pthread_mutex_lock(&mutex);
		read_dht22_dat();				//get temp,humid
		bright = read_mcp3208_adc(0);	//get brightness
		if(bright>1500)
		pthread_mutex_unlock(&mutex);
		printf("temp : %d\t humid : %d\t brightness : %d\n",ret_temp,ret_humid,bright);
		delay(1);
	}
}

void *controlFan(void *arg)
{
	int fancounter=0;
	while(1)
	{
		pthread_mutex_lock(&mutex);
		
		if(ret_temp>20)
			fancounter++;
		else
			fancounter=0;

		if(isFan==FALSE && fancounter>=50)
		{
			digitalWrite(FAN,HIGH);
			isFan=TRUE;
			printf("turn on FAN\n");
		}
		else if(isFan==TRUE && fancounter<50)
		{
			isFan=FALSE;
			digitalWrite(FAN,LOW);
			printf("turn off FAN\n");
		}
		
		pthread_mutex_unlock(&mutex);

		delay(100);
	}
}

void *controlLed(void *arg)
{
	while(1)
	{
		
		pthread_mutex_lock(&mutex);
		if(isLed==TRUE && bright<1800)
		{
			isLed=FALSE;
			digitalWrite(LED,LOW);
			printf("turn off LED\n");
		}
		else if(isLed==FALSE && bright>1500)
		{
			digitalWrite(LED,HIGH);
			isLed=TRUE;
			printf("turn on LED\n");
		}
		pthread_mutex_unlock(&mutex);
	}

}


int main(int argc, char *argv[])
{
	setup();

	pthread_t p1,p2,p3,p4;
	pthread_create(&p1,NULL,getTempBrightness,NULL);
	pthread_create(&p2,NULL,controlFan,NULL);
	pthread_create(&p3,NULL,controlLed,NULL);
	pthread_create(&p4,NULL,sendData,NULL);

	pthread_join(p1,NULL);
	pthread_join(p2,NULL);
	pthread_join(p3,NULL);
	pthread_join(p4,NULL);

	

	mysql_close(connector);

	return 0;
}
