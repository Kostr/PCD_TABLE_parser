#ifndef EDKII_TYPES_H
#define EDKII_TYPES_H

typedef unsigned char  BOOLEAN;
typedef   signed char  INT8;
typedef   signed short INT16;
typedef   signed long  INT32;
typedef          char  CHAR8;
typedef unsigned char  UINT8;
typedef unsigned short UINT16;
typedef unsigned int  UINT32;
typedef unsigned int  UINTN;
typedef unsigned long UINT64;
typedef void VOID;

typedef struct {
  UINT32  Data1;
  UINT16  Data2;
  UINT16  Data3;
  UINT8   Data4[8];
} EFI_GUID;

#endif
