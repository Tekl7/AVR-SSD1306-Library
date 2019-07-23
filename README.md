# AVR SSD1306 Library  

---

**Description:**  
A branch of AVR-SSD1306-Library based on AVR-SSD1306-Library from efthymios (https://github.com/efthymios-ks/AVR-SSD1306-Library).
This library has been created to support AVR devices with __USI interface__ and __small RAM__ (many devices with USI do not have sufficient RAM to store whole display buffer). So USIWire library (https://github.com/Tekl7/USIWire/tree/master/src/USI_TWI_Master) is required (instead of TWI library) to implement usage of USI as TWI interface.
+ Include 3 files from USIWire library:
  + USI_TWI_Master.c
  + USI_TWI_Master.h
  + usi_io.h
+ Functions include:  
  + Basic operations.  
  + Geometrical objects (draw/fill rectangle/line).  
  + Printing text anywhere in the screen using custom imported fonts.  
  + Printing pictures.  
   _Read the bottom of "SSD1306.h" to see all functions available._  
+ __No screen buffer__ is used. All functions display the characters (pixels) immediately so _GLCD_Render_ function is not required.
+ Some functions are removed because of wrong rendering (_GLCD_DrawCircle_, ...). Already rendered pixels were overwritten by another byte of pixels so several pixels were missing at the end.
+ C++ support is added (_extern "C" {}_).
+ The GLCD is interfaced using the TWI (I2C) protocol at 400KHz. You can use your own or my suggested library above. 
+ An error checking option is removed.
+ Custom I/O macros are required and are included.  

---  
  
**Compiler:**  
AVR-GCC  
  
**Optimization Level:**  
Optimize (-O1)  
  
--- 
**How to create a new font:**  
(needs some advanced knowledge)  
 1. Download the free **GLCD Font Creator** from **MikroElektronika**, install it and run it.  
 2. If you are on Windows 7, run the program under **Windows XP (Service Pack 3) compatibility mode** (google it), else you won't be able to save fonts.  
 3. File > New Font > Import an Existing System Font.  
 4. Choose your font, press ok to each pop-up window and wait for the processing to finish.  
 5. Export for GLCD > mikroC > X-GLCD Lib > Samsung KS0108 > Save.  
 6. Change the extension of the produced file from **.c** to **.h**.
 7. Open the file and add **Include Guards** (google it again).  
 8. Import the file into your project and your code.  
 _Include Guards example:_  
    `#ifndef _FONTNAME_H_`  
    `#define _FONTNAME_H_`  
    `//The content of the file`  
    `#endif`  

---
  
![picture alt](https://raw.githubusercontent.com/efthymios-ks/AVR-SSD1306-Library/master/Demonstration%20(1).PNG)
![picture alt](https://raw.githubusercontent.com/efthymios-ks/AVR-SSD1306-Library/master/Demonstration%20(2).PNG)
  
--- 
