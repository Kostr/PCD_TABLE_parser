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

#define EFI_VARIABLE_NON_VOLATILE                 0x00000001
#define EFI_VARIABLE_BOOTSERVICE_ACCESS           0x00000002
#define EFI_VARIABLE_RUNTIME_ACCESS               0x00000004
#define EFI_VARIABLE_HARDWARE_ERROR_RECORD        0x00000008
#define EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS   0x00000010

#endif
