import time
import RPi.GPIO as GPIO

GPIO.setmode(GPIO.BCM)
GPIO.setup(17,GPIO.OUT)

motor = GPIO.PWM(17,50)

motor.start(7.5)

try:
	while True:
		motor.ChangeDutyCycle(7.5)
		time.sleep(1)
		motor.ChangeDutyCycle(12.5)
		time.sleep(1)
		motor.ChangeDutyCycle(2.5)
		time.sleep(1)
except KeyboardInterrupt:
	GPIO.cleanup
