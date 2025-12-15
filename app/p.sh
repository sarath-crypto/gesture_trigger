avr-g++ -g -Os -Wall -mmcu=attiny85 -c alg.cpp
avr-g++ -g -Os -Wall -mmcu=attiny85 -c mpu6500.cpp
avr-g++ -g -Os -Wall -mmcu=attiny85 -c i2c.cpp
avr-g++ -g -Os -Wall -mmcu=attiny85 -c enc.cpp
avr-g++ -g -Os -Wall -lm -mmcu=attiny85 -o app.elf app.cpp  i2c.o enc.o mpu6500.o alg.o 
avr-objcopy -j .text -j .data -O ihex app.elf app.hex
nb=$(srec_info app.hex -intel)
nb=$(echo $nb | cut -d ' ' -f 8-)
nb=$nb+1
echo $((16#$nb))"/8192 bytes"

avrdude -c usbasp -p ATtiny85 -U flash:w:app.hex
avrdude -c usbasp -p ATtiny85 -U lfuse:r:-:h -U hfuse:r:-:h -U efuse:r:-:h
