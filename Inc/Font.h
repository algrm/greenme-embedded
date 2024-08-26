#ifndef FONT_H
#define FONT_H
#include "StdTypes.h"

typedef struct {
	const u8 charHeight;	//hauteur d'un caractère en pixels
	const u8 charWidth; 	//largeur d'un caractère (avec padding)
	const u8* data;				//data de la table : 1 byte par ligne
	const u8* widths;			//liste des largeurs
	const u8 interCharWidth;
	const u8 firstChar;	//valeur ascii du premier caractère représenté
	const u8 lastChar;	//valeur ascii du premier caractère représenté
	const u8 baseline;
} Font_t;


static enum {FONT_ROBOTO24, FONT_SONYSKETCH32, FONT_BIGICONS, FONT_SMALLICONS} fontTypeEnum;
static enum {ALIGN_LEFT, ALIGN_CENTER, ALIGN_RIGHT} textAlignEnum;
static enum {ICON_UNHAPPY, ICON_HAPPY, ICON_G, ICON_QUESTION, ICON_OK_LEFT, ICON_OK_RIGHT, ICON_AEREZ} bigIconEnum;

//déclarations
Font_t* GetFont(u8 fontId);
void DrawMainNumber(char* str);
//void DrawEmpty(u8 xStart, u8 yStart, u8 width, u8 height);
char* floatToString(char* strBuffer, float number);
//void DrawText(char* str, bool alignLeft, u8 y, u8 margin); 
void DrawTextAngle(char* str, Font_t* font, u8 align, u8 y, u8 margin, u8 angle); 
u8 DrawCharAngle(u8 xStart, u8 yStart, u8 charIndex, Font_t* font, u8 angle);
void DrawGear(void);
void DrawHappy(bool happy);
void DrawOK(bool happy);
u8 DrawNumber90(u8 xStart, u8 yStart, char character);
u8 DrawNumber270(u8 xStart, u8 yStart, char character);
u8 DrawChar(u8 xStart, u8 yStart, char character, Font_t* font) ; //remove me
void DrawText(u8* str, u8 strLen, u8 xStart, u8 yStart, u8 fontType, u8 textAlign); //remove me
void DrawText180(u8* str, u8 strLen, u8 xStart, u8 yStart, u8 align);
void DrawImageFast(u8 xStart, u8 yStart, char character);
void DrawMultilineText(u8 *text, u8 textLength);

	
#endif











