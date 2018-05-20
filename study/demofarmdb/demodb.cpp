#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <wiringPi.h>
#include <wiringPiSPI.h>

#include <mysql/mysql.h>

#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

#include <time.h>
#include <math.h>

#define CS_MCP3208  8        // BCM_GPIO_8

#define SPI_CHANNEL 0
#define SPI_SPEED   1000000   // 1MHz

#define VCC         4.8       // Supply Voltage

// define farm 
#define MAXTIMINGS 85
#define RETRY 5

#define PUMP	21 // BCM_GPIO 5
#define FAN	22 // BCM_GPIO 6
#define DCMOTOR 23 // BCM_GPIO 13
#define RGBLEDPOWER  24 //BCM_GPIO 19
#define RED	7
#define GREEN	8
#define BLUE	9

#define LIGHTSEN_OUT 2  //gpio27 - J13 connect
int ret_humid, ret_temp;

static int DHTPIN = 7;
static int dht22_dat[5] = {0,0,0,0,0};

int read_dht22_dat_temp();
int read_dht22_dat_humid();
int get_temperature_sensor();
int get_humidity_sensor();
int get_light_sensor();

// here

#define DBHOST "localhost"
#define DBUSER "root"
#define DBPASS "root"
#define DBNAME "demofarmdb"

MYSQL *connector;
MYSQL_RES *result;
MYSQL_ROW row;

int read_mcp3208_adc(unsigned char adcChannel)
{
  unsigned char buff[3];
  int adcValue = 0;

  buff[0] = 0x06 | ((adcChannel & 0x07) >> 2);
  buff[1] = ((adcChannel & 0x07) << 6);
  buff[2] = 0x00;

  digitalWrite(CS_MCP3208, 0);  // Low : CS Active

  wiringPiSPIDataRW(SPI_CHANNEL, buff, 3);

  buff[1] = 0x0F & buff[1];
  adcValue = ( buff[1] << 8) | buff[2];

  digitalWrite(CS_MCP3208, 1);  // High : CS Inactive

  return adcValue;
}


int main (void)
{
  int adcChannel  = 0;
  int adcValue[8] = {0};

  if(wiringPiSetupGpio() == -1)
  {
    fprintf (stdout, "Unable to start wiringPi: %s\n", strerror(errno));
    return 1 ;
  }

  if(wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED) == -1)
  {
    fprintf (stdout, "wiringPiSPISetup Failed: %s\n", strerror(errno));
    return 1 ;
  }

  pinMode(CS_MCP3208, OUTPUT);

  // MySQL connection
  connector = mysql_init(NULL);
  if (!mysql_real_connect(connector, DBHOST, DBUSER, DBPASS, DBNAME, 3306, NULL, 0))
  {
    fprintf(stderr, "%s\n", mysql_error(connector));
    return 0;
  }

  printf("MySQL(rpidb) opened.\n");

  while(1)
  {
    char query[1024];

    adcValue[0] = get_temperature_sensor(); // Temperature Sensor
    
    adcValue[1] = get_humidity_sensor(); // Humidity Sensor
    
    adcValue[2] = get_light_sensor(); // Illuminance Sensor
	
    adcValue[3] = read_mcp3208_adc(3); // Mic Sensor
    adcValue[4] = read_mcp3208_adc(4); // Flame Sensor
    adcValue[5] = read_mcp3208_adc(5); // Acceleration Sensor (Z-Axis)
    adcValue[6] = read_mcp3208_adc(6); // Gas Sensor
    adcValue[7] = read_mcp3208_adc(7); // Distace Sensor
    adcValue[7] = 27*pow((double)(adcValue[7]*VCC/4095), -1.10);

    sprintf(query,"insert into thl values (now(),%d,%d,%d)", adcValue[0],adcValue[1],adcValue[2]);

    if(mysql_query(connector, query))
    {
      fprintf(stderr, "%s\n", mysql_error(connector));
      printf("Write DB error\n");
    }

    delay(1000); //1sec delay
  }

  mysql_close(connector);

  return 0;
}

int get_temperature_sensor()
{
	int received_temp;
	int _retry = RETRY;

	DHTPIN = 11;

	if (wiringPiSetup() == -1)
		exit(EXIT_FAILURE) ;
	
	if (setuid(getuid()) < 0)
	{
		perror("Dropping privileges failed\n");
		exit(EXIT_FAILURE);
	}
	while (read_dht22_dat_temp() == 0 && _retry--)
	{
		delay(500); // wait 1sec to refresh
	}
	received_temp = ret_temp ;
	printf("Temperature = %d\n", received_temp);
	
	return received_temp;
}

static uint8_t sizecvt(const int read)
{
  /* digitalRead() and friends from wiringpi are defined as returning a value
  < 256. However, they are returned as int() types. This is a safety function */

  if (read > 255 || read < 0)
  {
    printf("Invalid data from wiringPi library\n");
    exit(EXIT_FAILURE);
  }
  return (uint8_t)read;
}

int read_dht22_dat_temp()
{
  uint8_t laststate = HIGH;
  uint8_t counter = 0;
  uint8_t j = 0, i;

  dht22_dat[0] = dht22_dat[1] = dht22_dat[2] = dht22_dat[3] = dht22_dat[4] = 0;

  // pull pin down for 18 milliseconds
  pinMode(DHTPIN, OUTPUT);
  digitalWrite(DHTPIN, HIGH);
  delay(10);
  digitalWrite(DHTPIN, LOW);
  delay(18);
  // then pull it up for 40 microseconds
  digitalWrite(DHTPIN, HIGH);
  delayMicroseconds(40); 
  // prepar
  
  pinMode(DHTPIN, INPUT);

  // detect change and read data
  for ( i=0; i< MAXTIMINGS; i++) {
    counter = 0;
    while (sizecvt(digitalRead(DHTPIN)) == laststate) {
      counter++;
      delayMicroseconds(1);
      if (counter == 255) {
        break;
      }
    }
    laststate = sizecvt(digitalRead(DHTPIN));

    if (counter == 255) break;

    // ignore first 3 transitions
    if ((i >= 4) && (i%2 == 0)) {
      // shove each bit into the storage bytes
      dht22_dat[j/8] <<= 1;
      if (counter > 16)
        dht22_dat[j/8] |= 1;
      j++;
    }
  }

  // check we read 40 bits (8bit x 5 ) + verify checksum in the last byte
  // print it out if data is good
  if ((j >= 40) && 
      (dht22_dat[4] == ((dht22_dat[0] + dht22_dat[1] + dht22_dat[2] + dht22_dat[3]) & 0xFF)) ) {
        float t, h;
		
        h = (float)dht22_dat[0] * 256 + (float)dht22_dat[1];
        h /= 10;
        t = (float)(dht22_dat[2] & 0x7F)* 256 + (float)dht22_dat[3];
        t /= 10.0;
        if ((dht22_dat[2] & 0x80) != 0)  t *= -1;
		
		ret_humid = (int)h;
		ret_temp = (int)t;
		printf("Humidity = %.2f %% Temperature = %.2f *C \n", h, t );
		printf("Humidity = %d Temperature = %d\n", ret_humid, ret_temp);
		
    return ret_temp;
  }
  else
  {
    printf("Data not good, skip\n");
    return 0;
  }
}


int get_humidity_sensor()
{
	int received_humid;
	int _retry = RETRY;

	DHTPIN = 11;

	if (wiringPiSetup() == -1)
		exit(EXIT_FAILURE) ;
	
	if (setuid(getuid()) < 0)
	{
		perror("Dropping privileges failed\n");
		exit(EXIT_FAILURE);
	}

	while (read_dht22_dat_humid() == 0 && _retry--) 
	{
		delay(500); // wait 1sec to refresh
	}

	received_humid = ret_humid;
	printf("Humidity = %d\n", received_humid);
	
	return received_humid;
}

int read_dht22_dat_humid()
{
	uint8_t laststate = HIGH;
	uint8_t counter = 0;
	uint8_t j = 0, i;

	dht22_dat[0] = dht22_dat[1] = dht22_dat[2] = dht22_dat[3] = dht22_dat[4] = 0;

	// pull pin down for 18 milliseconds
	pinMode(DHTPIN, OUTPUT);
	digitalWrite(DHTPIN, HIGH);
	delay(10);
	digitalWrite(DHTPIN, LOW);
	delay(18);
	// then pull it up for 40 microseconds
	digitalWrite(DHTPIN, HIGH);
	delayMicroseconds(40); 
	// prepare to read the pin
	pinMode(DHTPIN, INPUT);

	// detect change and read data
	for ( i=0; i< MAXTIMINGS; i++) 
	{
		counter = 0;
		while (sizecvt(digitalRead(DHTPIN)) == laststate) 
		{
			counter++;
			delayMicroseconds(1);
			if (counter == 255) {
			break;
		}
    }
    laststate = sizecvt(digitalRead(DHTPIN));

    if (counter == 255) break;

    // ignore first 3 transitions
    if ((i >= 4) && (i%2 == 0)) 
	{
		// shove each bit into the storage bytes
		dht22_dat[j/8] <<= 1;
		if (counter > 16)
			dht22_dat[j/8] |= 1;
		j++;
    }
}

  // check we read 40 bits (8bit x 5 ) + verify checksum in the last byte
  // print it out if data is good
	if ((j >= 40) && 
      (dht22_dat[4] == ((dht22_dat[0] + dht22_dat[1] + dht22_dat[2] + dht22_dat[3]) & 0xFF)) ) {
        float t, h;
		
        h = (float)dht22_dat[0] * 256 + (float)dht22_dat[1];
        h /= 10;
        t = (float)(dht22_dat[2] & 0x7F)* 256 + (float)dht22_dat[3];
        t /= 10.0;
        if ((dht22_dat[2] & 0x80) != 0)  t *= -1;
		
		ret_humid = (int)h;
		ret_temp = (int)t;
		printf("Humidity = %.2f %% Temperature = %.2f *C \n", h, t );
		printf("Humidity = %d Temperature = %d\n", ret_humid, ret_temp);
		
    return ret_humid;
	}
	else
	{
		printf("Data not good, skip\n");
		return 0;
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

int get_light_sensor(void)
{
	// sets up the wiringPi library
	if (wiringPiSetup () < 0) 
	{
		fprintf (stderr, "Unable to setup wiringPi: %s\n", strerror (errno));
		return 1;
	}
	
	if(digitalRead(LIGHTSEN_OUT))	//day
		return 1;
	else //night
		return 0;

}



