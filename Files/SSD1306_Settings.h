#ifndef SSD1306_SETTINGS_H_INCLUDED
#define SSD1306_SETTINGS_H_INCLUDED
/*
||
||  Filename:	 		SSD1306_Settings.h
||  Title: 			    SSD1306 Driver Settings
||  Author: 			Efthymios Koktsidis, Vojtěch Tecl
||	Email:				efthymios.ks@gmail.com
||  Compiler:		 	AVR-GCC
||	Description:
||	Settings for the SSD1306 driver. Pick a size and the
||	desirable pins.
||
||	Size			Code
||--------------------------
||	128x64	-	GLCD_128_64
||	128x32	-	GLCD_128_32
||	96x16	-	GLCD_96_16
||
*/

//----- Configuration -------------//
#define GLCD_Size				GLCD_128_64

//Match auxiliary functions to your defined I2C functions
//Setup TWI peripheral at 400KHz
#define __I2C_Setup()			USI_TWI_Master_Initialise()
//Transmit START signal, DATA, STOP signal
#define __I2C_Transmit(Data, Size)	USI_TWI_Start_Transceiver_With_Data(Data, Size)
//---------------------------------//
#endif
