#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include "edk2_types.h"
#include "guids.h"
#include "pcd_table.h"

//const char* filename = "Build/OvmfX64/DEBUG_GCC5/X64/MdeModulePkg/Universal/PCD/Pei/Pcd/OUTPUT/PEIPcdDataBase.raw";
//const char* filename = "Build/OvmfX64/DEBUG_GCC5/X64/MdeModulePkg/Universal/PCD/Dxe/Pcd/OUTPUT/DXEPcdDataBase.raw";


char* get_token_type(UINT32 Token)
{
 if ((Token & PCD_TYPE_ALL_SET) == PCD_TYPE_STRING)
   return "String";
 else if ((Token & PCD_TYPE_ALL_SET) == PCD_TYPE_HII)
   return "HII";
 else if ((Token & PCD_TYPE_ALL_SET) == PCD_TYPE_VPD)
   return "VPD";
 else if ((Token & PCD_TYPE_ALL_SET) == PCD_TYPE_DATA)
   return "Data";
 else
   return "Error PCD type";
}

char* get_datum_type(UINT32 Token)
{
  if ((Token & PCD_DATUM_TYPE_ALL_SET) == PCD_DATUM_TYPE_POINTER)
    return "Pointer";
  else if ((Token & PCD_DATUM_TYPE_ALL_SET) == PCD_DATUM_TYPE_UINT8)
    return "UINT8";
  else if ((Token & PCD_DATUM_TYPE_ALL_SET) == PCD_DATUM_TYPE_UINT16)
    return "UINT16";
  else if ((Token & PCD_DATUM_TYPE_ALL_SET) == PCD_DATUM_TYPE_UINT32)
    return "UINT32";
  else if ((Token & PCD_DATUM_TYPE_ALL_SET) == PCD_DATUM_TYPE_UINT64)
    return "UINT64";
  else
    return "Error PCD datum type";
}

char* get_datum2_type(UINT32 Token)
{
  if ((Token & PCD_DATUM_TYPE_UINT8_BOOLEAN) == PCD_DATUM_TYPE_UINT8_BOOLEAN)
    return "Bool";
  else
    return "";
}

void print_buffer(uint8_t* buf, ssize_t Size)
{
  ssize_t i = 0;
  while (i < Size) {
    printf("%02x ", buf[i]);
    i++;
    if (!(i%16)) {
      printf(" | ");
      for (int j=16; j>0; j--)
        if ((buf[i-j] >= 0x20) && (buf[i-j] < 0x7E))
          printf("%c", buf[i-j]);
        else
          printf(".");
      printf("\n");
    }
  }
  
  if (i%16) {
    for (int j=0; j<=15; j++) {
      if ((i+j)%16)
        printf("   ");
      else
        break;
    }
    printf(" | ");
    
    for (int j=(i%16); j>0; j--) {
      if ((buf[i-j] >= 0x20) && (buf[i-j] < 0x7E))
        printf("%c", buf[i-j]);
      else
        printf(".");
    }
  }
  printf("\n");
}

void print_dynamic_ex(char* db, PCD_TABLE_HEADER* pcd_table_header, UINT32 i, EFI_GUID* Guids)
{
  for (int j=0; j<pcd_table_header->ExTokenCount; j++) {
    DYNAMICEX_MAPPING* DynamicEx = ((DYNAMICEX_MAPPING*)&db[pcd_table_header->ExMapTableOffset]) + j;
    if ((i+1) == DynamicEx->TokenNumber) {
      printf("DynamicEx.ExTokenNumber = 0x%08x\n", DynamicEx->ExTokenNumber);
      printf("DynamicEx.TokenNumber = 0x%08x\n", DynamicEx->TokenNumber);
      printf("DynamicEx.ExGuidIndex = 0x%08x (", DynamicEx->ExGuidIndex);
      print_guid(Guids[DynamicEx->ExGuidIndex]);
      printf(")\n");
    }
  }

/*
  if (pcd_table_header.PcdNameTableOffset) {
      PCD_NAME_INDEX PcdNameIndex = *(PCD_NAME_INDEX*)&db[pcd_table_header.PcdNameTableOffset + sizeof(PCD_NAME_INDEX)*(DynamicEx[i].TokenNumber-1)];
      printf("PcdNameIndex.TokenSpaceCNameIndex=%d\n", PcdNameIndex.TokenSpaceCNameIndex);
      printf("PcdNameIndex.PcdCNameIndex=%d\n", PcdNameIndex.PcdCNameIndex);
      char* TokenSpaceName = (char*)&db[pcd_table_header.StringTableOffset + PcdNameIndex.TokenSpaceCNameIndex];
      char* PcdName = (char*)&db[pcd_table_header.StringTableOffset + PcdNameIndex.PcdCNameIndex];
      printf("TokenSpaceName = %s\n", TokenSpaceName);
      printf("PcdName = %s\n", PcdName);
  }
*/
}

static int SizeTableIndex=0;


int main(int argc, char** argv)
{
  if (argc != 2) {
    printf("Usage: %s <PCD_DB.raw>\n", argv[0]);
    printf("Program to parse PCD_DB.raw file\n");
    return(EXIT_FAILURE);
  }

  int fd = open(argv[1], O_RDONLY);
  if (fd == -1)
  {
    printf("Error! Can't open file %s\n", argv[1]);
    exit(EXIT_FAILURE);
  }

  struct stat sb;
  if (fstat(fd, &sb) == -1) {
    printf("fstat error\n");
    exit(EXIT_FAILURE);
  }

  PCD_TABLE_HEADER pcd_table_header;
  ssize_t size = read(fd, &pcd_table_header, sizeof(pcd_table_header));
  printf("BuildVersion = %d\n", pcd_table_header.BuildVersion);
  printf("Length = %d\n", pcd_table_header.Length);
  printf("SystemSkuId = %ld\n", pcd_table_header.SystemSkuId);
#ifdef VER7
  printf("LengthForAllSkus = %d\n", pcd_table_header.LengthForAllSkus);
#endif
  printf("UninitDataBaseSize = %d\n", pcd_table_header.UninitDataBaseSize);
  printf("LocalTokenNumberTableOffset = 0x%x\n", pcd_table_header.LocalTokenNumberTableOffset);
  printf("ExMapTableOffset = 0x%x\n", pcd_table_header.ExMapTableOffset);
  printf("GuidTableOffset = 0x%x\n", pcd_table_header.GuidTableOffset);
  printf("StringTableOffset = 0x%x\n", pcd_table_header.StringTableOffset);
  printf("SizeTableOffset = 0x%x\n", pcd_table_header.SizeTableOffset);
  printf("SkuIdTableOffset = 0x%x\n", pcd_table_header.SkuIdTableOffset);
  printf("PcdNameTableOffset = 0x%x\n", pcd_table_header.PcdNameTableOffset);
  printf("LocalTokenCount = %d\n", pcd_table_header.LocalTokenCount);
  printf("ExTokenCount = %d\n", pcd_table_header.ExTokenCount);
  printf("GuidTableCount = %d\n", pcd_table_header.GuidTableCount);

  printf("sizeof(PCD_TABLE_HEADER) = 0x%lx\n", sizeof(PCD_TABLE_HEADER));
  printf("File size = %ld\n", sb.st_size);


  char *db = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (db == MAP_FAILED){
    printf("Mapping Failed\n");
    return 1;
  }
  close(fd);

  printf("_____\n");
  printf("Guid table:\n");
  EFI_GUID* Guids = (EFI_GUID*)&db[pcd_table_header.GuidTableOffset];
  for (int i=0; i<pcd_table_header.GuidTableCount; i++) {
    print_guid(Guids[i]);
    printf("\n");
  }
  printf("_____\n");
  printf("LocalTokenNumberTable:\n");
  for (int i=0; i<pcd_table_header.LocalTokenCount; i++) {
    UINT32 Token = *(UINT32*)&db[pcd_table_header.LocalTokenNumberTableOffset + i*sizeof(UINT32)];
    printf("\n%d:\n", i);
    printf("Token=0x%08x: Type=%s; DatumType=%s;%s; Offset=0x%08x\n", Token, get_token_type(Token), get_datum_type(Token),
                                                                     get_datum2_type(Token), Token & PCD_DATABASE_OFFSET_MASK);


    print_dynamic_ex(db, &pcd_table_header, i, Guids);

    if ((Token & PCD_DATABASE_OFFSET_MASK) >= sb.st_size) {
      printf("0 - unitialized\n");
      if ((Token & PCD_DATABASE_OFFSET_MASK) >= (sb.st_size + pcd_table_header.UninitDataBaseSize)) {
        printf("(Offset is wrong!)\n");
      }
      continue;
    }

    int Size = 0;
    STRING_HEAD* StringTableIdx;
    char* buf;
    if (((Token & PCD_TYPE_ALL_SET) == PCD_TYPE_HII) || ((Token & PCD_TYPE_ALL_SET) == (PCD_TYPE_HII|PCD_TYPE_STRING))) {
      printf("HII_STRING\n");
      VARIABLE_HEAD* VariableHead = (VARIABLE_HEAD *)(db + (Token & PCD_DATABASE_OFFSET_MASK));
      printf("Guid:\n");
      print_guid(Guids[VariableHead->GuidTableIndex]);
      printf("\n");
      printf("Name:\n");
      UINT16* Name = (UINT16 *)(&db[pcd_table_header.StringTableOffset + VariableHead->StringIndex]);
      int NameLength = 0;
      while ((Name[NameLength]) != 0x0) {
        NameLength++;
      }
      print_buffer((UINT8*)Name, (NameLength+1)*2);
      printf("DefaultValueOffset=0x%08x\n", VariableHead->DefaultValueOffset);
      printf("HII Offset=0x%04x\n", VariableHead->Offset);
      printf("Attributes=0x%08x\n", VariableHead->Attributes);
      printf("Property=0x%04x\n",  VariableHead->Property);

      UINT8* VaraiableDefaultBuffer;
      if ((Token & PCD_TYPE_ALL_SET) == (PCD_TYPE_HII|PCD_TYPE_STRING)) {
        StringTableIdx = (STRING_HEAD *)(db + VariableHead->DefaultValueOffset);
        VaraiableDefaultBuffer = &db[pcd_table_header.StringTableOffset + *StringTableIdx];
        printf("TBD - parser not implemented\n");
        continue;
      } else {
        VaraiableDefaultBuffer = &db[VariableHead->DefaultValueOffset];
        Size = 0;
        if ((Token & PCD_DATUM_TYPE_ALL_SET) == PCD_DATUM_TYPE_UINT8)
          Size = 1;
        else if ((Token & PCD_DATUM_TYPE_ALL_SET) == PCD_DATUM_TYPE_UINT16)
          Size = 2;
        else if ((Token & PCD_DATUM_TYPE_ALL_SET) == PCD_DATUM_TYPE_UINT32)
          Size = 4;
        else if ((Token & PCD_DATUM_TYPE_ALL_SET) == PCD_DATUM_TYPE_UINT64)
          Size = 8;

        buf = VaraiableDefaultBuffer;

        if ((Token & PCD_DATUM_TYPE_ALL_SET) == PCD_DATUM_TYPE_UINT8)
          printf("0x%02x (=%d)\n", *(UINT8*)buf, *(UINT8*)buf);
        else if ((Token & PCD_DATUM_TYPE_ALL_SET) == PCD_DATUM_TYPE_UINT16)
          printf("0x%04x (=%d)\n", *(UINT16*)buf, *(UINT16*)buf);
        else if ((Token & PCD_DATUM_TYPE_ALL_SET) == PCD_DATUM_TYPE_UINT32)
          printf("0x%08x (=%d)\n", *(UINT32*)buf, *(UINT32*)buf);
        else if ((Token & PCD_DATUM_TYPE_ALL_SET) == PCD_DATUM_TYPE_UINT64)
          printf("0x%016lx (=%ld)\n", *(UINT64*)buf, *(UINT64*)buf);

        continue;
      }
    } 
    if ((Token & PCD_TYPE_ALL_SET) == PCD_TYPE_VPD) {
      printf("TBD - parser not implemented\n");
      continue;
    }

    if ((Token & PCD_DATUM_TYPE_ALL_SET) == PCD_DATUM_TYPE_POINTER) {
      UINT16 MaxSize = *(UINT16*)&db[pcd_table_header.SizeTableOffset + (SizeTableIndex*4)];
      UINT16 CurrentSize = *(UINT16*)&db[pcd_table_header.SizeTableOffset + (SizeTableIndex*4) + 2];
      printf("CurrentSize=%d\n", CurrentSize);
      printf("MaxSize=%d\n", MaxSize);
      Size=CurrentSize;
      SizeTableIndex++;
    }
    else if ((Token & PCD_DATUM_TYPE_ALL_SET) == PCD_DATUM_TYPE_UINT8)
      Size = 1;
    else if ((Token & PCD_DATUM_TYPE_ALL_SET) == PCD_DATUM_TYPE_UINT16)
      Size = 2;
    else if ((Token & PCD_DATUM_TYPE_ALL_SET) == PCD_DATUM_TYPE_UINT32)
      Size = 4;
    else if ((Token & PCD_DATUM_TYPE_ALL_SET) == PCD_DATUM_TYPE_UINT64)
      Size = 8;

    if ((Token & PCD_TYPE_ALL_SET) == PCD_TYPE_STRING) {
      StringTableIdx = (STRING_HEAD *)(db + (Token & PCD_DATABASE_OFFSET_MASK));
      buf = &db[pcd_table_header.StringTableOffset + *StringTableIdx];
    } else {
      buf = &db[Token & PCD_DATABASE_OFFSET_MASK];
    }
    
    if ((Token & PCD_DATUM_TYPE_ALL_SET) == PCD_DATUM_TYPE_POINTER) {
      print_buffer(buf, Size);
    }
    else if ((Token & PCD_DATUM_TYPE_ALL_SET) == PCD_DATUM_TYPE_UINT8)
      printf("0x%02x (=%d)\n", *(UINT8*)buf, *(UINT8*)buf);
    else if ((Token & PCD_DATUM_TYPE_ALL_SET) == PCD_DATUM_TYPE_UINT16)
      printf("0x%04x (=%d)\n", *(UINT16*)buf, *(UINT16*)buf);
    else if ((Token & PCD_DATUM_TYPE_ALL_SET) == PCD_DATUM_TYPE_UINT32)
      printf("0x%08x (=%d)\n", *(UINT32*)buf, *(UINT32*)buf);
    else if ((Token & PCD_DATUM_TYPE_ALL_SET) == PCD_DATUM_TYPE_UINT64)
      printf("0x%016lx (=%ld)\n", *(UINT64*)buf, *(UINT64*)buf);
  }
  printf("_____\n");

  munmap(db, sb.st_size);
}
