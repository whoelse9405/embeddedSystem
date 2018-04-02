import time
import pygame.mixer
from time import sleep
import RPi.GPIO as GPIO
from sys import exit

BUTTON=19
LED=20
SERVO_MOTOR=21
TRIG=5
ECHO=6

buttonState=0		#0=not push 1=push 

GPIO.setmode(GPIO.BCM)
GPIO.setup(BUTTON,GPIO.IN)
GPIO.setup(LED,GPIO.OUT)
GPIO.setup(SERVO_MOTOR,GPIO.OUT)
GPIO.setup(TRIG,GPIO.OUT)
GPIO.setup(ECHO,GPIO.IN)

#pygame.mixer.init(48000,-16,1,1024)
pygame.mixer.init()
sound=pygame.mixer.Sound("sound.mp3")
#soundChannel=pygame.mixer.Channel(1)

servoMotor=GPIO.PWM(SERVO_MOTOR,50)
servoMotor.start(5.5)
GPIO.setup(TRIG,GPIO.OUT)
time.sleep(0.3)

try:
	while True:
		
		if((GPIO.input(BUTTON)==False)and (buttonState==0)):	
			#soundChannel.play(sound)							#play the sound
			sound.play()
			sleep(.01)
			servoMotor.ChangeDutyCycle(5.5)				#moving servomotor
			time.sleep(0.5)
			servoMotor.ChangeDutyCycle(7.5)
			time.sleep(0.5)
			GPIO.output(LED,GPIO.HIGH)						#turn on LED
			buttonState=1													#change buttonState 
		elif((GPIO.input(BUTTON)==True) and (buttonState==1)):
			GPIO.output(LED,GPIO.LOW)							#turn off LED		
			buttonState=0													#change buttonState
		else:										#destance measure			
			GPIO.output(TRIG,True)
			time.sleep(0.00001)
			GPIO.output(TRIG,False)
			while GPIO.input(ECHO)==0:
				pulse_start=time.time()
			while GPIO.input(ECHO)==1:
				pulse_end=time.time()
 
			pulse_duration = pulse_end-pulse_start
			distance = round(pulse_duration * 17150,2)

			if(distance<=15):
				GPIO.output(LED,GPIO.LOW)
				servoMotor.ChangeDutyCycle(5.5)
				time.sleep(0.5)
				servoMotor.ChangeDutyCycle(7.5)
				time.sleep(0.5)
				GPIO.output(LED,GPIO.HIGH)

except KeyboardInterrupt:
	GPIO.cleanup()

