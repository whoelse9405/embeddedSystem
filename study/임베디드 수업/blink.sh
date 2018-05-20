#!/bin/bash

while true; 
do
	pushed='cat /sys/class/gpio/gpio20/value'

	if [$pushed -eq 1 ]; 
	then	
		echo 1 > /sys/class/gpio/gpio21/value
	//cat /sys/class/gpio/gpio24/value > /sys/class/gpio/gpio21/value	
	sleep 1
		echo 0 > /sys/class/gpio/gpio21/value
		sleep 1
	fi

done

