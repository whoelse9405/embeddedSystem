#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <wiringPi.h>

#define PUMP	21 // BCM_GPIO 5
#define FAN	22 // BCM_GPIO 6
#define DCMOTOR 23 // BCM_GPIO 13
#define RGBLEDPOWER  24 //BCM_GPIO 19

#define RED	27
#define GREEN	28
#define BLUE	29


int main (void)
{
  if (wiringPiSetup () == -1)
  {
  	fprintf(stdout, "Unable to start wiringPi: %s\n", strerror(errno));
  	return 1 ;
  }

  pinMode (PUMP, OUTPUT) ;
  pinMode (FAN, OUTPUT) ;
  pinMode (DCMOTOR, OUTPUT);
  pinMode(RGBLEDPOWER, OUTPUT);

  for (;;)
  {
    digitalWrite (PUMP, 1) ; // On
    digitalWrite (FAN, 1) ; // On
    digitalWrite(DCMOTOR, 1); //On
    digitalWrite(RGBLEDPOWER, 1);

    delay (5000) ; // ms

    //digitalWrite(RED, 1);
    //digitalWrite(GREEN, 1);
    //digitalWrite(BLUE, 1);

    digitalWrite (PUMP, 0) ; // Off
    digitalWrite (FAN, 0) ; // Off
    digitalWrite(DCMOTOR, 0); //Off
    digitalWrite(RGBLEDPOWER, 0); //Off

    delay (5000) ;
  }
  return 0 ;
}


