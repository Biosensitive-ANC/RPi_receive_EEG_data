# RPi_receive_EEG_data

## Features

- Reads EEG data from a serial port.

- Parses and extracts attention and meditation values.

- Uses multi-threading to handle data reception efficiently.

- Ensures thread safety with mutex protection.

## File Structure

- main.cpp: Main program that initializes and reads EEG data.

- EEG.h: Header file defining the EEGSerial class.

- EEG.cpp: Implementation of serial communication and data processing.
