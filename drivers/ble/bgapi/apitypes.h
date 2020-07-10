#ifndef _BG_API_TYPES_H_
#define _BG_API_TYPES_H_

typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef signed short   int16;
typedef unsigned long  uint32;
typedef signed char    int8;

typedef struct bd_addr_t
{
    uint8 addr[6];

}bd_addr;

typedef bd_addr hwaddr;

typedef struct
{
    uint8 len;
    
#ifndef __cplusplus
    uint8 data[];
#endif

}uint8array;

typedef struct
{
    uint8 len;

#ifndef __cplusplus
    int8 data[];
#endif

}string;


#endif //< _BG_API_TYPES_H_
