# embeddedSystem
임베디드시스템 수업

### [2018.03.19] Embedded System hw1.hwp
This file is answers of practice 1-5.

### [2018.04.02] miniproject.py
This is miniprotject file, this source code controls the LED and servo motor by ultrasonic sensors or buttons.

### [2018.04.23] detect_humidity.c	detect_temperature.c 
There are team project file.<br>
watch the video : https://www.youtube.com/watch?v=NHwWXCTi1Oo
 
 1. detect_humidity.c
  - Monitor humidity in an infinite loop
  - If humidity becomes over 70%, turn on a red led.
  - FAN and DC Motor should be activated for 1 second.

 2. detect_temperature.c
  - Monitor temperature in an infinite loop
  - If temperature goes over 25’C, run the water pump for 1 sec.
  
### [2018.05.22] threadpractice.c 
 1.Producer
  - collect temperature sensor data on every 3 sec.
  - store it a shared buffer
  - wake up a consumer by signaling
  
 2.Consumer
  - pull it out and store data into database

### [2018.05.22] Smartfarm.c smartfarm_capture.png
watch the video : https://www.youtube.com/watch?v=Z8tzffkSW5k

 - Monitor temperature and humidity and lightness on every 1ms.
 - Used an analogous data for a lightness sensor. 
 - Send sensor data and fan state and led state from RaspberryPi to the server every 10s. 
 - Turn on FAN when the temperature goes beyond 20 degrees(C) for 5 second. 
 - Turn on LED when the lightness goes below 1800 and turn off it otherwise. 
 - Each functionality be performed by an independant thread (use multi-threaded programming, 4 threads) 
 - Used a signal mechanism when threads need to communicate each other. 