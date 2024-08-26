#ifndef DATASTRUCT_H
#define DATASTRUCT_H

typedef struct {
	u16 greenmeHeader;
	u8  protocolVersion;
	u8  command;
	u8  nBytes;
	u8 Unused;
	u16 addr;		//radio address : 1 byte net Id, 1 byte id;
	u32 serial;	//serial number of the module
} radioDataFrameHeader;
#define RADIOHEADER_SIZE  12



typedef struct {
	u16 newAddr;
} radioDataPayload_config;
#define RADIODATACONFIG_SIZE  2 

typedef struct {
	u16 greenmeHeader;
	u8  protocolVersion;
	u8  command;
	u8 nBytes;
} serialFrameHeader_t;
#define SERIALHEADER_SIZE  5

typedef struct
{
	u16	temp;
	u16	hygr;
	u32 lighcolor;	//RGBW
	u16 lux;
	u16 flicker;
	u16 noiseAvg;
	u16 noiseMax;
	u16 vocEqCO2;
	u16 lastFeel;		//time in sec since last feel
	u8  feel;				//0x00 unset, 0x10 good, 0x01 bad
	u8 quality;
	u16 Unused;
	u16 Octaves[10];
} payloadMeasure_t;
//#define PAYLOADMEASURE_SIZE  64


#endif
