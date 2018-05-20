import pygame.mixer
from time import sleep
import RPi.GPIO as GPIO
from sys import exit

GPIO.setmode(GPIO.BCM)
GPIO.setup(23, GPIO.IN)
GPIO.setup(24, GPIO.IN)
GPIO.setup(25, GPIO.IN)

pygame.mixer.init(48000, -16, 1, 1024)

soundA=pygame.mixer.Sound("center.wav")
soundB=pygame.mixer.Sound("left.wav")
soundC=pygame.mixer.Sound("right.wav")

soundChanelA = pygame.mixer.Channel(1)
soundChanelA = pygame.mixer.Channel(2)
soundChanelA = pygame.mixer.Channel(3)

print "Soundboard Ready."

while True:
	try:
		if(GPIO.input(23)==True):
			soundChannelA.play(soundA)
		if(GPIO.input(24)==True):
			soundChannelB.play(soundB)
		if(GPIO.input(25)==True):
			soundChannelC.play(soundC)
		sleep(.01)
	except KeyboardInterrupt:
		exit()
