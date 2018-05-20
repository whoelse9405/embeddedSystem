import RPi.GPIO as GPIO
import time

GPIO.setmode(GPIO.BCM)
GPIO.setup(20,GPIO.IN)

count=0
isReleased = True

while True:
	inputValue = GPIO.input(20)
	if(inputValue==True and isReleased == True):
		count = count+1
		print("Button Pressed" + str(count) + " times.")
		isReleased = False
		
	if(inputValue == False and isReleased == False):
		isReleased = True

	time.sleep(.01)
