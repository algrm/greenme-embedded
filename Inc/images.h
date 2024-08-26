#ifndef IMAGES_H
#define IMAGES_H

#include "StdTypes.h"

///
//types
typedef struct {
	const u8 width;
	const u8 height;
	const u8* data;
	bool RSLEncoded;
	const u16 byteLen;
} image;


extern image imgOk;
extern image gear;
extern image imgHappy;
extern image imgNotHappy;


void DrawImage(u8 xStart, u8 yStart, image img);

#endif // IMAGES_H
