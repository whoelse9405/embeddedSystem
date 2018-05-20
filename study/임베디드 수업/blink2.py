import RPi.GPIO as GPIO
import time

GPIO.setmode(GPIO.BCM)
GPIO.setup(20,GPIO.IN)

count=0

while True:
	inputValue = GPIO.input(20)
	if(inputValue == True):
		count = count + 1
		print("Button pressed"+str(count)+"times.")
	time.sleep(.01)
