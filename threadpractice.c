#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<sys/types.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>
#include<signal.h>
#include<wiringPi.h>	//sensor
#include<softPwm.h>		
#include<pthread.h>		//thread

#include<mysql/mysql.h>
#include<time.h>
#include<math.h>

#define MAXTIMINGS 85
#define DHTPIN 11		//temp sensor

#define MAX 500

#define DBHOST "localhost"
#define DBUSER "root"
#define DBPASS "root"
#define DBNAME "demofarmdb"

MYSQL *connector;
MYSQL_RES *result;
MYSQL_ROW roe;


pthread_cond_t empty,fill;
pthread_mutex_t mutex;

int buffer[MAX];
int fill_ptr=0;
int use_ptr=0;
int count=0;

int ret_temp,ret_humid;
static uint8_t dht22_dat[5]={0,0,0,0,0};

static uint8_t sig_handler(int signo)
{
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

		h=(float)dht22_dat[0]*256*(float)dht22_dat[1];
		h/=10;
		t=(float)(dht22_dat[2]&0x7F)*256+(float)dht22_dat[3];
		t/=10.0;

		if((dht22_dat[2]&0x80)!=0)t*=-1;

		ret_humid=(int)h;
		ret_temp=(int)t;

		return ret_temp;
	}
	else 
		return 0;

}

void setup()
{
	if(wiringPiSetup()==-1)
		exit(EXIT_FAILURE);

	if(setuid(getuid())<0)
	{
		perror("Drooping privileges failed\n");
		exit(EXIT_FAILURE);
	}
	
	signal(SIGINT,(void*)sig_handler);
	
	connector = mysql_init(NULL);
	if(!mysql_real_connect(connector,DBHOST,DBUSER,DBPASS,DBNAME,3306,NULL,0))
	{
		fprintf(stderr,"%s\n",mysql_error(connector));
		exit(-1);
	}	



}


void getData(int data)
{
	buffer[fill_ptr]=data;
	fill_ptr=(fill_ptr+1)%MAX;
	printf("%d'C\n",data);
	count++;
}

int sendData()
{
	char query[1024];

	int tmp=buffer[use_ptr];
	use_ptr=(use_ptr+1)%MAX;

	//do send data to database 
	sprintf(query,"insert into thl values (now(), %d,%d,%d)",tmp,0,0);

	if(mysql_query(connector,query))
	{
		fprintf(stderr,"%s\n",mysql_error(connector));
		printf("Write DB error\n");
	}


	count--;
	return tmp;
}

void *producer(void *arg)
{
	while(1)
	{
		int temp = read_dht22_dat();
		if(temp!=0)
		{
			pthread_mutex_lock(&mutex);
			while(count==MAX)
				pthread_cond_wait(&empty,&mutex);
			getData(temp);
			pthread_cond_signal(&fill);
			pthread_mutex_unlock(&mutex);
			
			delay(3000);	
		}
	}
}

void *consumer(void *arg)
{ 
	while(1)
	{
		pthread_mutex_lock(&mutex);
		while(count==0)
			pthread_cond_wait(&fill,&mutex);
		int temp = sendData();
		pthread_cond_signal(&empty);
		pthread_mutex_unlock(&mutex);
	}
}

int main(int argc, char *argv[])
{
	setup();

	printf("main begin\n");

	pthread_t p1,p2;
	pthread_create(&p1,NULL,producer,NULL);
	pthread_create(&p2,NULL,consumer,NULL);
	
	pthread_join(p1,NULL);
	pthread_join(p2,NULL);
		
	mysql_close(connector);
	printf("main end\n");

	return 0;
}
