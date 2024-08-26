#ifndef CRC_H
#define CRC_H


/*==================================================================
Crc 16
inputs : 
Ptr : Frame pointer on which Crc must be computed
Size : Frame Size on which Crc must be computed
Computes and returns a 16 bit Crc
Nb Cycles < 18+Size*91 > 17 + Size*(11+8*10)
NbCycles(255) < 23223
===================================================================*/

u16 Crc16( u8 *Ptr, u8 Size ) ;

u16 Crc16_ccit(const u8* buffer, u8 size);
u16 Crc16_ccit_false(u8* buffer, u8 size);



#endif // CRC_H
