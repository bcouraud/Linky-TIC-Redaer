# Linky-TIC-Redaer
This repository stores the files to reproduce a Linky TIC Reader.
The architecture includes:
- A communication moduel, i.e. a Wemo D1 Lite that can be programmed from the arduino IDE
- A power supply that consists of a large capacitor and a microchip PIC 12f1840 micrcontroller to switch on/off the communication module if there is not enough energy in the capacitor. 
- a TIC demodulation circuit


The current folder includes:
- The PCB files to reproduce it. They can be opened from KICad free software
- The arduino code for the wemo module to operate: wake up - read the TIC information - read the capacitor voltage - if there is enough energy in the capcitor, send the data in https.
- The C code for the microchip PIC 12F1840
