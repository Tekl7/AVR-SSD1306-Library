#include "SSD1306.h"

//----- Auxiliary data ------//
GLCD_t __GLCD;

#define __I2C_SLA_W(Address)		(Address<<1)
#define __GLCD_getLine(Y)			(Y / __GLCD_Screen_Line_Height)
#define __GLCD_AbsDiff(X, Y)		((X > Y) ? (X - Y) : (Y - X))
#define __GLCD_Swap(X, Y)			do { typeof(X) t = X; X = Y; Y = t; } while (0)
//---------------------------//

//----- Prototypes ----------------------------//
static void GLCD_Send(const uint8_t Control, uint8_t *Data, const uint8_t Length);
static void GLCD_DrawHLine(uint8_t X1, uint8_t X2, const uint8_t Y, enum Color_t Color);
static void GLCD_DrawVLine(uint8_t Y1, uint8_t Y2, const uint8_t X, enum Color_t Color);
static void Int2bcd(int32_t Value, char BCD[]);
//---------------------------------------------//

//----- Functions -------------//
void GLCD_SendCommand(uint8_t Command)
{
	GLCD_Send(__GLCD_COMMAND, &Command, 1);
}

void GLCD_SendData(uint8_t Data)
{
	GLCD_Send(__GLCD_DATA, &Data, 1);
}

void GLCD_Setup(void)
{
	//Setup I2C hardware
	__I2C_Setup();

	//Commands needed for initialization
	//-------------------------------------------------------------
	GLCD_SendCommand(__GLCD_Command_Display_Off);					//0xAE
	
	GLCD_SendCommand(__GLCD_Command_Display_Clock_Div_Ratio_Set);	//0xD5
	GLCD_SendCommand(0xF0);											//Suggest ratio
	
	GLCD_SendCommand(__GLCD_Command_Multiplex_Radio_Set);			//0xA8
	GLCD_SendCommand(__GLCD_Screen_Height - 1);
	
	GLCD_SendCommand(__GLCD_Command_Display_Offset_Set);			//0xD3
	GLCD_SendCommand(0x00);											//No offset

	GLCD_SendCommand(__GLCD_Command_Charge_Pump_Set);				//0x8D
	GLCD_SendCommand(0x14);											//Enable charge pump

	GLCD_SendCommand(__GLCD_Command_Display_Start_Line_Set | 0x00);	//0x40 | Start line
	
	GLCD_SendCommand(__GLCD_Command_Memory_Addressing_Set);				//0x20
	GLCD_SendCommand(0x00);											//Horizontal Addressing - Operate like KS0108
	
	GLCD_SendCommand(__GLCD_Command_Segment_Remap_Set | 0x01);		//0xA0 - Left towards Right

	GLCD_SendCommand(__GLCD_Command_Com_Output_Scan_Dec);			//0xC8 - Up towards Down

	GLCD_SendCommand(__GLCD_Command_Com_Pins_Set);					//0xDA
	#if (GLCD_Size == GLCD_128_64)
		GLCD_SendCommand(0x12);										//Sequential COM pin configuration
	#elif (GLCD_Size == GLCD_128x32)
		GLCD_SendCommand(0x02);										//Alternative COM pin configuration
	#elif (GLCD_Size == GLCD_96x16)
		GLCD_SendCommand(0x02);										//Alternative COM pin configuration
	#endif
	
	GLCD_SendCommand(__GLCD_Command_Constrast_Set);					//0x81
	GLCD_SendCommand(0xFF);

	GLCD_SendCommand(__GLCD_Command_Precharge_Period_Set);			//0xD9
	GLCD_SendCommand(0xF1);

	GLCD_SendCommand(__GLCD_Command_VCOMH_Deselect_Level_Set);		//0xDB
	GLCD_SendCommand(0x20);

	GLCD_SendCommand(__GLCD_Command_Display_All_On_Resume);			//0xA4
	GLCD_SendCommand(__GLCD_Command_Display_Normal);				//0xA6
	GLCD_SendCommand(__GLCD_Command_Scroll_Deactivate);				//0x2E
	GLCD_SendCommand(__GLCD_Command_Display_On);					//0xAF
	//-------------------------------------------------------------

	//Go to 0,0
	GLCD_GotoXY(0, 0);
	
	//Reset GLCD structure
	__GLCD.Mode = GLCD_Non_Inverted;
	__GLCD.X = __GLCD.Y = __GLCD.Font.Width = __GLCD.Font.Height = __GLCD.Font.Lines = 0;

	GLCD_Clear();
}

void GLCD_SetDisplay(const uint8_t On)
{
	GLCD_SendCommand(On ? __GLCD_Command_Display_On : __GLCD_Command_Display_Off);
}

void GLCD_SetContrast(const uint8_t Contrast)
{
	GLCD_SendCommand(__GLCD_Command_Constrast_Set);
	GLCD_SendCommand(Contrast);
}

void GLCD_Clear(void)
{
	GLCD_FillScreen(GLCD_White);
}

void GLCD_ClearLine(const uint8_t Line)
{
	if (Line < __GLCD_Screen_Lines)
	{
		uint8_t i;

		uint8_t messageBuf[__GLCD_Screen_Width];

		GLCD_GotoXY(0, Line * __GLCD_Screen_Line_Height);
		for (i = 0 ; i < __GLCD_Screen_Width ; i++)
			messageBuf[i] = GLCD_White;

		GLCD_Send(__GLCD_DATA, messageBuf, sizeof(messageBuf));
	}
}

void GLCD_GotoX(const uint8_t X)
{
	if (X < __GLCD_Screen_Width)
	{
		__GLCD.X = X;
		
		GLCD_SendCommand(__GLCD_Command_Column_Address_Set);
		GLCD_SendCommand(X);
		GLCD_SendCommand(__GLCD_Screen_Width - 1);
	}
}

void GLCD_GotoY(const uint8_t Y)
{
	if (Y < __GLCD_Screen_Height)
	{
		__GLCD.Y = Y;

		GLCD_SendCommand(__GLCD_Command_Page_Address_Set);
		GLCD_SendCommand(__GLCD_getLine(__GLCD.Y));
		GLCD_SendCommand(__GLCD_Screen_Lines - 1);
	}
}

void GLCD_GotoXY(const uint8_t X, const uint8_t Y)
{
	GLCD_GotoX(X);
	GLCD_GotoY(Y);
}

void GLCD_GotoLine(const uint8_t Line)
{
	if (Line < __GLCD_Screen_Lines)
	{
		__GLCD.Y = Line * __GLCD_Screen_Line_Height;

		GLCD_SendCommand(__GLCD_Command_Page_Address_Set);
		GLCD_SendCommand(Line);
		GLCD_SendCommand(__GLCD_Screen_Lines - 1);
	}
}

uint8_t GLCD_GetX(void)
{
	return __GLCD.X;
}

uint8_t GLCD_GetY(void)
{
	return __GLCD.Y;
}

uint8_t GLCD_GetLine(void)
{
	return (__GLCD_getLine(__GLCD.Y));
}

void GLCD_SetPixel(const uint8_t X, const uint8_t Y, enum Color_t Color)
{
	uint8_t data = 0;
	
	//Goto to point
	GLCD_GotoXY(X, Y);
	
	//Set pixel
	if (Color == GLCD_Black)
		BitSet(data, Y % 8);
	else
		BitClear(data, Y % 8);
	
	//Send data
	GLCD_Send(__GLCD_DATA, &data, sizeof(data));
}

void GLCD_SetPixels(const uint8_t X1, uint8_t Y1, const uint8_t X2, const uint8_t Y2, enum Color_t Color)
{
	if ((X1 < __GLCD_Screen_Width) && (X2 < __GLCD_Screen_Width) &&
		(Y1 < __GLCD_Screen_Height) && (Y2 < __GLCD_Screen_Height))
	{
		uint8_t height, width, offset, mask, h, i, data;
		height = Y2 - Y1 + 1;
		width = X2 - X1 + 1;
		offset = Y1 % __GLCD_Screen_Line_Height;
		Y1 -= offset;
		mask = 0xFF;
		data = 0;

		uint8_t messageBuf[width];

		//Calculate mask for top fractioned region
		if (height <(__GLCD_Screen_Line_Height - offset))
		{
			mask >>=(__GLCD_Screen_Line_Height - height);
			h = height;
		}
		else
		h = __GLCD_Screen_Line_Height - offset;
		mask <<= offset;

		//Draw fractional rows at the top of the region
		GLCD_GotoXY(X1, Y1);
		for (i = 0 ; i < width ; i++)
		{
			//Mask
			data = ((Color == GLCD_Black) ? (data | mask) : (data & ~mask));
			//Write
			messageBuf[i] = data;
		}

		//Write
		GLCD_Send(__GLCD_DATA, messageBuf, sizeof(messageBuf));

		//Full rows
		while ((h + __GLCD_Screen_Line_Height) <= height)
		{
			h += __GLCD_Screen_Line_Height;
			Y1 += __GLCD_Screen_Line_Height;
			GLCD_GotoXY(X1, Y1);
			for (i = 0 ; i < width ; i++)
				messageBuf[i] = Color;

			//Write
			GLCD_Send(__GLCD_DATA, messageBuf, sizeof(messageBuf));
		}

		//Fractional rows at the bottom of the region
		if (h < height)
		{
			mask = ~(0xFF << (height - h));
			GLCD_GotoXY(X1, Y1 + __GLCD_Screen_Line_Height);
			for (i = 0 ; i < width ; i++)
			{
				data = 0;
				//Mask
				data = ((Color == GLCD_Black) ? (data | mask) : (data & ~mask));
				//Write
				messageBuf[i] = data;
			}

			//Write
			GLCD_Send(__GLCD_DATA, messageBuf, sizeof(messageBuf));
		}
	}
}

void GLCD_DrawBitmap(const uint8_t *Bitmap, uint8_t Width, const uint8_t Height)
{
	uint16_t lines, bmpRead, bmpReadPrev;
	uint8_t x, y, y2, i, j, overflow, data, dataPrev;
	lines = bmpRead = bmpReadPrev = x = y = i = j = overflow = data = dataPrev = 0;

	//#1 - Save current position
	x = __GLCD.X;
	y = y2 = __GLCD.Y;
	
	//#2 - Read width - First two bytes
	data = __GLCD.X + Width;														//"data" is used temporarily
	//If character exceed screen bounds, reduce
	if (data >= __GLCD_Screen_Width)
		Width -= data - __GLCD_Screen_Width;

	uint8_t messageBuf[Width];
	
	//#3 - Read height - Second two bytes - Convert to lines
	lines = (Height + __GLCD_Screen_Line_Height - 1) / __GLCD_Screen_Line_Height;	//lines = Ceiling(A/B) = (A+B-1)/B
	data = __GLCD.Y / __GLCD_Screen_Line_Height + lines;							//"data" is used temporarily
	//If bitmap exceed screen bounds, reduce
	if (data > __GLCD_Screen_Lines)
		lines -= data - __GLCD_Screen_Lines;
	
	//#4 - Calculate overflowing bits
	overflow = __GLCD.Y % __GLCD_Screen_Line_Height;
	
	//#5 - Print the character
	//Scan the lines needed
	for (j = 0 ; j < lines ; j++)
	{
		//Go to the start of the line
		GLCD_GotoXY(x, y);
		
		//Update the indices for reading the line
		bmpRead = j * Width;
		bmpReadPrev = bmpRead - Width;		//Previous = 4 + (j - 1) * width = Current - width

		//Scan bytes of selected line
		for (i = 0 ; i < Width ; i++)
		{
			//Read byte
			data = pgm_read_byte(&(Bitmap[bmpRead++]));
			
			//Shift byte
			data <<= overflow;
			
			//Merge byte with previous one
			if (j > 0)
			{
				dataPrev = pgm_read_byte(&(Bitmap[bmpReadPrev++]));
				dataPrev >>= __GLCD_Screen_Line_Height - overflow;
				data |= dataPrev;
			}
			
			//Send byte
			messageBuf[i] = data;
		}
		//Increase line counter
		y += __GLCD_Screen_Line_Height;

		//Write
		GLCD_Send(__GLCD_DATA, messageBuf, sizeof(messageBuf));
	}

	//#6 - Update last line, if needed
	//If (LINE_STARTING != LINE_ENDING)
	if (__GLCD_getLine(y2) != __GLCD_getLine((y2 + Height - 1)) && y < __GLCD_Screen_Height)
	{
		//Go to the start of the line
		GLCD_GotoXY(x, y);
		
		//Update the index for reading the last printed line
		bmpReadPrev = (j - 1) * Width;

		//Scan bytes of selected line
		for (i = 0 ; i < Width ; i++)
		{
			data = 0;
			
			//Merge byte with previous one
			dataPrev = pgm_read_byte(&(Bitmap[bmpReadPrev++]));
			dataPrev >>= __GLCD_Screen_Line_Height - overflow;
			data |= dataPrev;
			
			//Send byte
			messageBuf[i] = data;
		}
		//Write
		GLCD_Send(__GLCD_DATA, messageBuf, sizeof(messageBuf));
	}
	
	//Go to the upper-right corner of the printed bitmap
	GLCD_GotoXY(GLCD_GetX(), y2);
}

void GLCD_DrawLine(uint8_t X1, uint8_t Y1, uint8_t X2, uint8_t Y2, enum Color_t Color)
{
	if ((X1 < __GLCD_Screen_Width) && (X2 < __GLCD_Screen_Width) &&
		(Y1 < __GLCD_Screen_Height) && (Y2 < __GLCD_Screen_Height))
	{
		if (X1 == X2)
		{
			GLCD_DrawVLine(Y1, Y2, X1, Color);
		}
		else if (Y1 == Y2)
		{
			GLCD_DrawHLine(X1, X2, Y1, Color);
		}
		else
		{
			uint8_t deltax, deltay, x, y, slope;
			int8_t error, ystep;
			slope = ((__GLCD_AbsDiff(Y1, Y2) > __GLCD_AbsDiff(X1,X2)) ? 1 : 0);
			if (slope)
			{
				//Swap x1, y1
				__GLCD_Swap(X1, Y1);
				//Swap x2, y2
				__GLCD_Swap(X2, Y2);
			}
			if (X1 > X2)
			{
				//Swap x1, x2
				__GLCD_Swap(X1, X2);
				//Swap y1,y2
				__GLCD_Swap(Y1, Y2);
			}
			
			deltax = X2 - X1;
			deltay = __GLCD_AbsDiff(Y2, Y1);
			error = deltax / 2;
			y = Y1;
			ystep = ((Y1 < Y2) ? 1 : -1);
			
			for (x = X1 ; x <= X2 ; x++)
			{
				if (slope)
					GLCD_SetPixel(y, x, Color);
				else
					GLCD_SetPixel(x, y, Color);
				
				error -= deltay;
				if (error < 0)
				{
					y = y + ystep;
					error = error + deltax;
				}
			}
		}
	}
}

void GLCD_DrawRectangle(const uint8_t X1, const uint8_t Y1, const uint8_t X2, const uint8_t Y2, enum Color_t Color)
{
	if ((X1 < __GLCD_Screen_Width) && (X2 < __GLCD_Screen_Width) &&
		(Y1 < __GLCD_Screen_Height) && (Y2 < __GLCD_Screen_Height))
	{
		GLCD_DrawHLine(X1, X2, Y1, Color);
		GLCD_DrawHLine(X1, X2, Y2, Color);
		GLCD_DrawVLine(Y1, Y2, X1, Color);
		GLCD_DrawVLine(Y1, Y2, X2, Color);
	}
}

void GLCD_FillScreen(enum Color_t Color)
{
	uint8_t i, j;

	uint8_t messageBuf[__GLCD_Screen_Width];

	GLCD_GotoXY(0, 0);

	for (j = 0 ; j < __GLCD_Screen_Height ; j += __GLCD_Screen_Line_Height)
	{
		for (i = 0 ; i < __GLCD_Screen_Width ; i++)
			messageBuf[i] = Color;

		GLCD_Send(__GLCD_DATA, messageBuf, sizeof(messageBuf));
	}
}

void GLCD_FillRectangle(const uint8_t X1, const uint8_t Y1, const uint8_t X2, const uint8_t Y2, enum Color_t Color)
{
	GLCD_SetPixels(X1, Y1, X2, Y2, Color);
}

void GLCD_ScrollLeft(const uint8_t Start, const uint8_t End)
{
	//The display is 16 rows tall. To scroll the whole display, run:
	// GLCD_scrollLeft(0x00, 0x0F)
	GLCD_SendCommand(__GLCD_Command_Scroll_Left);
	GLCD_SendCommand(0x00);

	GLCD_SendCommand(Start);
	GLCD_SendCommand(0x00);
	GLCD_SendCommand(End);

	GLCD_SendCommand(0x00);
	GLCD_SendCommand(0xFF);
	GLCD_SendCommand(__GLCD_Command_Scroll_Activate);
}

void GLCD_ScrollRight(const uint8_t Start, const uint8_t End)
{
	//The display is 16 rows tall. To scroll the whole display, run:
	// GLCD_scrollRight(0x00, 0x0F)
	GLCD_SendCommand(__GLCD_Command_Scroll_Right);
	GLCD_SendCommand(0x00);		//Dummy

	GLCD_SendCommand(Start);	//Start
	GLCD_SendCommand(0x00);		//Frames: 5
	GLCD_SendCommand(End);		//End

	GLCD_SendCommand(0x00);		//Dummy
	GLCD_SendCommand(0xFF);		//Dummy
	GLCD_SendCommand(__GLCD_Command_Scroll_Activate);
}

void GLCD_ScrollDiagonalLeft(const uint8_t Start, const uint8_t End)
{
	//The display is 16 rows tall. To scroll the whole display, run:
	// GLCD_scrollDiagonalLeft(0x00, 0x0F)
	GLCD_SendCommand(__GLCD_Command_Scroll_Vertical_Area_Set);
	GLCD_SendCommand(0x00);
	GLCD_SendCommand(__GLCD_Screen_Height);

	
	GLCD_SendCommand(__GLCD_Command_Scroll_Vertical_Left);
	GLCD_SendCommand(0x00);		//Dummy
	GLCD_SendCommand(Start);	//Start
	GLCD_SendCommand(0x00);		//Frames: 5
	GLCD_SendCommand(End);		//End
	GLCD_SendCommand(0x01);		//Vertical offset: 1

	GLCD_SendCommand(__GLCD_Command_Scroll_Activate);
}

void GLCD_ScrollDiagonalRight(const uint8_t Start, const uint8_t End)
{
	//The display is 16 rows tall. To scroll the whole display, run:
	// GLCD_scrollDiagonalRight(0x00, 0x0F)
	GLCD_SendCommand(__GLCD_Command_Scroll_Vertical_Area_Set);
	GLCD_SendCommand(0x00);
	GLCD_SendCommand(__GLCD_Screen_Height);


	GLCD_SendCommand(__GLCD_Commad_Scroll_Vertical_Right);
	GLCD_SendCommand(0x00);		//Dummy
	GLCD_SendCommand(Start);	//Start
	GLCD_SendCommand(0x00);		//Frames: 5
	GLCD_SendCommand(End);		//End
	GLCD_SendCommand(0x01);		//Vertical offset: 1

	GLCD_SendCommand(__GLCD_Command_Scroll_Activate);
}

void GLCD_ScrollStop(void)
{
	GLCD_SendCommand(__GLCD_Command_Scroll_Deactivate);
}

void GLCD_InvertScreen(void)
{
	if (__GLCD.Mode == GLCD_Inverted)
		__GLCD.Mode = GLCD_Non_Inverted;
	else
		__GLCD.Mode = GLCD_Inverted;

	GLCD_SendCommand(__GLCD.Mode);
}

void GLCD_SetFont(const uint8_t *Name, const uint8_t Width, const uint8_t Height)
{
	if (Width < __GLCD_Screen_Width && Height < __GLCD_Screen_Height)
	{
		//Change font pointer to new font
		__GLCD.Font.Name = (uint8_t *)(Name);
		
		//Update font's size
		__GLCD.Font.Width = Width;
		__GLCD.Font.Height = Height;
		
		//Update lines required for a character to be fully displayed
		__GLCD.Font.Lines = (Height - 1) / __GLCD_Screen_Line_Height + 1;
	}
}

uint8_t GLCD_GetWidthChar(const char Character)
{
	//+1 for space after each character
	return (pgm_read_byte(&(__GLCD.Font.Name[(Character - 32) * (__GLCD.Font.Width * __GLCD.Font.Lines + 1)])) + 1);
}

uint16_t GLCD_GetWidthString(const char *Text)
{
	uint16_t width = 0;

	while (*Text)
		width += GLCD_GetWidthChar(*Text++);

	return width;
}

void GLCD_PrintChar(char Character)
{
	//If it doesn't work, replace pgm_read_byte with pgm_read_word
	uint16_t fontStart, fontRead, fontReadPrev;
	uint8_t x, y, y2, i, j, width, overflow, data, dataPrev;
	fontStart = fontRead = fontReadPrev = x = y = y2 = i = j = width = overflow = data = dataPrev = 0;
	
	//#1 - Save current position
	x = __GLCD.X;
	y = y2 = __GLCD.Y;
	
	//#2 - Remove leading empty characters
	Character -= 32;														//32 is the ASCII of the first printable character
	
	//#3 - Find the start of the character in the font array
	fontStart = Character * (__GLCD.Font.Width * __GLCD.Font.Lines + 1);		//+1 due to first byte of each array line being the width
	
	//#4 - Update width - First byte of each line is the width of the character
	width = pgm_read_byte(&(__GLCD.Font.Name[fontStart++]));

	uint8_t messageBuf[width + 1];

	//#5 - Calculate overflowing bits
	overflow = __GLCD.Y % __GLCD_Screen_Line_Height;
	
	//#6 - Print the character
	//Scan the lines needed
	for (j = 0 ; j < __GLCD.Font.Lines ; j++)
	{
		//Go to the start of the line
		GLCD_GotoXY(x, y);
		
		//Update the indices for reading the line
		fontRead = fontStart + j;
		fontReadPrev = fontRead - 1;

		//Scan bytes of selected line
		for (i = 0 ; i < width ; i++)
		{
			//Read byte
			data = pgm_read_byte(&(__GLCD.Font.Name[fontRead]));
			
			//Shift byte
			data <<= overflow;
			
			//Merge byte with previous one
			if (j > 0)
			{
				dataPrev = pgm_read_byte(&(__GLCD.Font.Name[fontReadPrev]));
				dataPrev >>= __GLCD_Screen_Line_Height - overflow;
				data |= dataPrev;
				fontReadPrev += __GLCD.Font.Lines;
			}
			
			//Send byte
			messageBuf[i] = data;
			
			//Increase index
			fontRead += __GLCD.Font.Lines;
		}

		//Send an empty column of 1px in the end
		messageBuf[width] = GLCD_White;
		
		//Increase line counter
		y += __GLCD_Screen_Line_Height;

		//Write
		GLCD_Send(__GLCD_DATA, messageBuf, sizeof(messageBuf));
	}

	//#7 - Update last line, if needed
	//If (LINE_STARTING != LINE_ENDING)
	if (__GLCD_getLine(y2) != __GLCD_getLine((y2 + __GLCD.Font.Height - 1)) && y < __GLCD_Screen_Height)
	{
		//Go to the start of the line
		GLCD_GotoXY(x, y);
		
		//Update the index for reading the last printed line
		fontReadPrev = fontStart + j - 1;

		//Scan bytes of selected line
		for (i = 0 ; i < width ; i++)
		{
			data = 0;
			
			//Merge byte with previous one
			dataPrev = pgm_read_byte(&(__GLCD.Font.Name[fontReadPrev]));
			dataPrev >>= __GLCD_Screen_Line_Height - overflow;
			data |= dataPrev;
			
			//Send byte
			messageBuf[i] = data;

			//Increase index
			fontReadPrev += __GLCD.Font.Lines;
		}

		//Send an empty column of 1px in the end
		messageBuf[width] = GLCD_White;

		//Write
		GLCD_Send(__GLCD_DATA, messageBuf, sizeof(messageBuf));
	}
	
	//Move cursor to the end of the printed character
	GLCD_GotoXY(x + width + 1, y2);
}

void GLCD_PrintString(const char *Text)
{
	while(*Text)
	{
		if ((__GLCD.X + __GLCD.Font.Width) >= __GLCD_Screen_Width)
			break;

		GLCD_PrintChar(*Text++);
	}
}

void GLCD_PrintInteger(const int32_t Value)
{
	if (Value == 0)
	{
		GLCD_PrintChar('0');
	}
	else if ((Value > INT32_MIN) && (Value <= INT32_MAX))
	{
		//int32_max_bytes + sign + null = 12 bytes
		char bcd[12] = { '\0' };
		
		//Convert integer to array
		Int2bcd(Value, bcd);
		
		//Print from first non-zero digit
		GLCD_PrintString(bcd);
	}
}

void GLCD_PrintDouble(double Value, const uint8_t Precision)
{
	if (Value == 0)
	{
		//Print characters individually so no string is stored in RAM
		GLCD_PrintChar('0');
		GLCD_PrintChar('.');
		GLCD_PrintChar('0');
	}
	else if ((Value >= (-2147483647)) && (Value < 2147483648))
	{
		//Print sign
		if (Value < 0)
		{
			Value = -Value;
			GLCD_PrintChar('-');
		}
		
		//Print integer part
		GLCD_PrintInteger(Value);
		
		//Print dot
		GLCD_PrintChar('.');
		
		//Print decimal part
		GLCD_PrintInteger((Value - (uint32_t)(Value)) * pow(10, Precision));
	}
}

static void GLCD_Send(const uint8_t Control, uint8_t *Data, const uint8_t Length)
{
	unsigned char messageBuf[2 + Length];	// Address (1B) + Control (1B) + Length = 2B + Length

	uint8_t i;

	do
	{
		//Append SLA+W
		messageBuf[0] = __I2C_SLA_W(__GLCD_I2C_Address);

		//Append control byte
		messageBuf[1] = Control;
		

		for (i = 0 ; i < Length ; i++)
			//Append data
			messageBuf[2+i] = Data[i];
	}
	while (0);

	//Transmit
	__I2C_Transmit(messageBuf, sizeof(messageBuf));
}

static inline void GLCD_DrawHLine(uint8_t X1, uint8_t X2, const uint8_t Y, enum Color_t Color)
{
	if (X1 > X2)
		__GLCD_Swap(X1, X2);
	
	while (X1 <= X2)
	{
		GLCD_SetPixel(X1, Y, Color);
		X1++;
	}
}

static inline void GLCD_DrawVLine(uint8_t Y1, uint8_t Y2, const uint8_t X, enum Color_t Color)
{
	if (Y1 > Y2)
		__GLCD_Swap(Y1, Y2);

	GLCD_SetPixels(X, Y1, X, Y2, Color);
}

static void Int2bcd(int32_t Value, char BCD[])
{
	uint8_t isNegative = 0;
	
	BCD[0] = BCD[1] = BCD[2] =
	BCD[3] = BCD[4] = BCD[5] =
	BCD[6] = BCD[7] = BCD[8] =
	BCD[9] = BCD[10] = '0';
	
	if (Value < 0)
	{
		isNegative = 1;
		Value = -Value;
	}
	
	while (Value >= 1000000000)
	{
		Value -= 1000000000;
		BCD[1]++;
	}
	
	while (Value >= 100000000)
	{
		Value -= 100000000;
		BCD[2]++;
	}
	
	while (Value >= 10000000)
	{
		Value -= 10000000;
		BCD[3]++;
	}
	
	while (Value >= 1000000)
	{
		Value -= 1000000;
		BCD[4]++;
	}
	
	while (Value >= 100000)
	{
		Value -= 100000;
		BCD[5]++;
	}

	while (Value >= 10000)
	{
		Value -= 10000;
		BCD[6]++;
	}

	while (Value >= 1000)
	{
		Value -= 1000;
		BCD[7]++;
	}
	
	while (Value >= 100)
	{
		Value -= 100;
		BCD[8]++;
	}
	
	while (Value >= 10)
	{
		Value -= 10;
		BCD[9]++;
	}

	while (Value >= 1)
	{
		Value -= 1;
		BCD[10]++;
	}

	uint8_t i = 0;
	//Find first non zero digit
	while (BCD[i] == '0')
		i++;

	//Add sign
	if (isNegative)
	{
		i--;
		BCD[i] = '-';
	}

	//Shift array
	uint8_t end = 10 - i;
	uint8_t offset = i;
	i = 0;
	while (i <= end)
	{
		BCD[i] = BCD[i + offset];
		i++;
	}
	BCD[i] = '\0';
}
//-----------------------------//
