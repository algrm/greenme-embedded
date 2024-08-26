#ifndef ENDIANESS_H
#define ENDIANESS_H

#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN  3412
#endif // LITTLE_ENDIAN
#ifndef BIG_ENDIAN
#define BIG_ENDIAN     1234
#endif // BIG_ENDIAN

#ifndef BYTE_ORDER
#define BYTE_ORDER     LITTLE_ENDIAN
#endif // BYTE_ORDER

#ifndef HTONS
#   if BYTE_ORDER == BIG_ENDIAN
#      define HTONS(n) (n)
#   else // BYTE_ORDER == LITTLE_ENDIAN
#      define HTONS(n) ((((u16)((n) & 0xff)) << 8) | (((n) & 0xff00) >> 8))
#   endif // BYTE_ORDER == BIG_ENDIAN
#endif // HTONS

#endif
