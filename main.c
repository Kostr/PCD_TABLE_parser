#include <getopt.h>
#include <stdbool.h>
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
#include "utils.h"

//const char* filename = "Build/OvmfX64/DEBUG_GCC5/X64/MdeModulePkg/Universal/PCD/Pei/Pcd/OUTPUT/PEIPcdDataBase.raw";
//const char* filename = "Build/OvmfX64/DEBUG_GCC5/X64/MdeModulePkg/Universal/PCD/Dxe/Pcd/OUTPUT/DXEPcdDataBase.raw";

const bool debug = false;


int fill_pcd_table_header(char* db, PCD_TABLE_HEADER* pcd_table_header)
{
  memcpy(pcd_table_header, db, sizeof(PCD_TABLE_HEADER));

  EFI_GUID gPcdDataBaseSignatureGuid = { 0x3c7d193c, 0x682c, 0x4c14, { 0xa6, 0x8f, 0x55, 0x2d, 0xea, 0x4f, 0x43, 0x7e }};
  if (memcmp(&gPcdDataBaseSignatureGuid, &(pcd_table_header->Signature), sizeof(EFI_GUID))) {
    printf("Error! PCD_TABLE_HEADER doesn't contain valid GUID\n");
    return -1;
  }

  if (pcd_table_header->BuildVersion != 7) {
    if (pcd_table_header->BuildVersion == 6) {
      PCD_TABLE_HEADER_v6* pcd_table_header_v6 = (PCD_TABLE_HEADER_v6*)db;
      pcd_table_header->Signature = pcd_table_header_v6->Signature;
      pcd_table_header->BuildVersion = pcd_table_header_v6->BuildVersion;
      pcd_table_header->Length = pcd_table_header_v6->Length;
      pcd_table_header->SystemSkuId = pcd_table_header_v6->SystemSkuId;
      pcd_table_header->LengthForAllSkus = 0;
      pcd_table_header->UninitDataBaseSize = pcd_table_header_v6->UninitDataBaseSize;
      pcd_table_header->LocalTokenNumberTableOffset = pcd_table_header_v6->LocalTokenNumberTableOffset;
      pcd_table_header->ExMapTableOffset = pcd_table_header_v6->ExMapTableOffset;
      pcd_table_header->GuidTableOffset = pcd_table_header_v6->GuidTableOffset;
      pcd_table_header->StringTableOffset = pcd_table_header_v6->StringTableOffset;
      pcd_table_header->SizeTableOffset = pcd_table_header_v6->SizeTableOffset;
      pcd_table_header->SkuIdTableOffset = pcd_table_header_v6->SkuIdTableOffset;
      pcd_table_header->PcdNameTableOffset = pcd_table_header_v6->PcdNameTableOffset;
      pcd_table_header->LocalTokenCount = pcd_table_header_v6->LocalTokenCount;
      pcd_table_header->ExTokenCount = pcd_table_header_v6->ExTokenCount;
      pcd_table_header->GuidTableCount = pcd_table_header_v6->GuidTableCount;
    } else {
      printf("PCD DB Build version is %d\n", pcd_table_header->BuildVersion);
      printf("Currently only versions 7 and 6 are supported\n");
      return -2;
    }
  }
  return 0;
}

void print_pcd_table_header(PCD_TABLE_HEADER* pcd_table_header)
{
  printf("PCD_TABLE_HEADER:\n");
  printf("BuildVersion = %d\n", pcd_table_header->BuildVersion);
  printf("Length = %d\n", pcd_table_header->Length);
  printf("SystemSkuId = %ld\n", pcd_table_header->SystemSkuId);
  if (pcd_table_header->BuildVersion == 7)
    printf("LengthForAllSkus = %d\n", pcd_table_header->LengthForAllSkus);
  printf("UninitDataBaseSize = %d\n", pcd_table_header->UninitDataBaseSize);
  printf("LocalTokenNumberTableOffset = 0x%x\n", pcd_table_header->LocalTokenNumberTableOffset);
  printf("ExMapTableOffset = 0x%x\n", pcd_table_header->ExMapTableOffset);
  printf("GuidTableOffset = 0x%x\n", pcd_table_header->GuidTableOffset);
  printf("StringTableOffset = 0x%x\n", pcd_table_header->StringTableOffset);
  printf("SizeTableOffset = 0x%x\n", pcd_table_header->SizeTableOffset);
  printf("SkuIdTableOffset = 0x%x\n", pcd_table_header->SkuIdTableOffset);
  printf("PcdNameTableOffset = 0x%x\n", pcd_table_header->PcdNameTableOffset);
  printf("LocalTokenCount = %d\n", pcd_table_header->LocalTokenCount);
  printf("ExTokenCount = %d\n", pcd_table_header->ExTokenCount);
  printf("GuidTableCount = %d\n", pcd_table_header->GuidTableCount);

  if (pcd_table_header->BuildVersion == 7)
    printf("sizeof(PCD_TABLE_HEADER) = 0x%lx\n", sizeof(PCD_TABLE_HEADER));
  else if (pcd_table_header->BuildVersion == 6)
    printf("sizeof(PCD_TABLE_HEADER) = 0x%lx\n", sizeof(PCD_TABLE_HEADER_v6));
}

void print_pcd_table_guids(char* db, PCD_TABLE_HEADER* pcd_table_header)
{
  printf("GUIDs in PCD_TABLE:\n");
  EFI_GUID* Guids = (EFI_GUID*)&db[pcd_table_header->GuidTableOffset];
  for (int i=0; i<pcd_table_header->GuidTableCount; i++) {
    print_guid(Guids[i]);
    printf("\n");
  }
}

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
    return NULL;
}

void print_local_token_value(UINT32 Token)
{
  if (debug)
    printf("Token = 0x%08x\n", Token);

  printf("Token type = %s\n", get_token_type(Token));
  printf("Datum type = %s", get_datum_type(Token));
  if (get_datum2_type(Token))
    printf(" (%s)", get_datum2_type(Token));
  printf("\n");

  if (debug)
    printf("Offset=0x%08x\n", Token & PCD_DATABASE_OFFSET_MASK);
}

void print_guid_by_index(char* db, PCD_TABLE_HEADER* pcd_table_header, UINT16 guid_index)
{
  EFI_GUID* Guids = (EFI_GUID*)&db[pcd_table_header->GuidTableOffset];
  if (guid_index < (pcd_table_header->GuidTableCount))
    print_guid(Guids[guid_index]);
  else
    printf("Error! GUID index is wrong!");
}

void print_dynamic_ex(char* db, PCD_TABLE_HEADER* pcd_table_header, UINT32 local_token_index)
{
  for (int i=0; i<pcd_table_header->ExTokenCount; i++) {
    DYNAMICEX_MAPPING* DynamicEx = ((DYNAMICEX_MAPPING*)&db[pcd_table_header->ExMapTableOffset]) + i;
    if ((local_token_index+1) == DynamicEx->TokenNumber) {
      if (debug) {
        printf("DynamicEx.ExTokenNumber = 0x%08x\n", DynamicEx->ExTokenNumber);
        printf("DynamicEx.TokenNumber = 0x%08x\n", DynamicEx->TokenNumber+1);
        printf("DynamicEx.ExGuidIndex = 0x%08x (", DynamicEx->ExGuidIndex);
        print_guid_by_index(db, pcd_table_header, DynamicEx->ExGuidIndex);
        printf(")\n");
      } else {
        printf("DynamicEx Token = 0x%08x\n", DynamicEx->ExTokenNumber);
        printf("DynamicEx GUID  = ");
        print_guid_by_index(db, pcd_table_header, DynamicEx->ExGuidIndex);
        printf("\n");
      }
    }
  }
}

void print_name_table_info(char* db, PCD_TABLE_HEADER* pcd_table_header, UINT32 local_token_index)
{
  if (pcd_table_header->PcdNameTableOffset) {
      printf("TBD - PcdNameTable parsing is not implemented\n");
      /*
      PCD_NAME_INDEX PcdNameIndex = *(PCD_NAME_INDEX*)&db[pcd_table_header.PcdNameTableOffset + sizeof(PCD_NAME_INDEX)*(DynamicEx[i].TokenNumber-1)];
      printf("PcdNameIndex.TokenSpaceCNameIndex=%d\n", PcdNameIndex.TokenSpaceCNameIndex);
      printf("PcdNameIndex.PcdCNameIndex=%d\n", PcdNameIndex.PcdCNameIndex);
      char* TokenSpaceName = (char*)&db[pcd_table_header.StringTableOffset + PcdNameIndex.TokenSpaceCNameIndex];
      char* PcdName = (char*)&db[pcd_table_header.StringTableOffset + PcdNameIndex.PcdCNameIndex];
      printf("TokenSpaceName = %s\n", TokenSpaceName);
      printf("PcdName = %s\n", PcdName);
      */
  }
}

void print_data_token_value(UINT32 Token, char* db, UINT32 offset)
{
  char* buf = &db[offset];
  if ((Token & PCD_DATUM_TYPE_ALL_SET) == PCD_DATUM_TYPE_UINT8)
    printf("Value:\n0x%02x (=%u)\n", *(UINT8*)buf, *(UINT8*)buf);
  else if ((Token & PCD_DATUM_TYPE_ALL_SET) == PCD_DATUM_TYPE_UINT16)
    printf("Value:\n0x%04x (=%u)\n", *(UINT16*)buf, *(UINT16*)buf);
  else if ((Token & PCD_DATUM_TYPE_ALL_SET) == PCD_DATUM_TYPE_UINT32)
    printf("Value:\n0x%08x (=%u)\n", *(UINT32*)buf, *(UINT32*)buf);
  else if ((Token & PCD_DATUM_TYPE_ALL_SET) == PCD_DATUM_TYPE_UINT64)
    printf("Value:\n0x%016lx (=%lu)\n", *(UINT64*)buf, *(UINT64*)buf);
  else
    printf("Error! Wrong Datum type\n");
}

UINT32 get_size_table_index(char* db, PCD_TABLE_HEADER* pcd_table_header, UINT32 LocalTokenIndex)
{
  UINT32 SizeTableIndex = 0;
  for (int i=0; i<LocalTokenIndex; i++) {
    UINT32 LocalToken = *(UINT32*)&db[pcd_table_header->LocalTokenNumberTableOffset + i*sizeof(UINT32)];
    if ((LocalToken & PCD_DATUM_TYPE_ALL_SET) == PCD_DATUM_TYPE_POINTER) {
      SizeTableIndex += 1;
    }
  }
  return SizeTableIndex;
}

typedef enum {
  PEI_PCD_DB,
  DXE_PCD_DB
} pcd_db_type;

int map_file(char* filename, char** buf, off_t* filesize)
{
  int fd = open(filename, O_RDONLY);
  if (fd == -1)
  {
    printf("Error! Can't open file %s\n", filename);
    return(EXIT_FAILURE);
  }

  struct stat sb;
  if (fstat(fd, &sb) == -1) {
    printf("Error! fstat error\n");
    return(EXIT_FAILURE);
  }

  *buf = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (*buf == MAP_FAILED){
    printf("Error! Mapping Failed\n");
    return(EXIT_FAILURE);
  }
  close(fd);
  *filesize = sb.st_size;
  return(EXIT_SUCCESS);
}

int parse_pcd_db(char* filename, pcd_db_type db_type, int* local_token_offset, char* vpd, off_t vpd_size)
{
  char* db;
  off_t db_size;

  if (map_file(filename, &db, &db_size)) {
    return(EXIT_FAILURE);
  }

  if (db_size < sizeof(PCD_TABLE_HEADER)) {
    printf("Error! File is too small for PCD database\n");
    return(EXIT_FAILURE);
  }

  PCD_TABLE_HEADER pcd_table_header;
  if (fill_pcd_table_header(db, &pcd_table_header))
    return(EXIT_FAILURE);

  if (db_type == PEI_PCD_DB) {
    printf("\nPEI PCD DB\n");
    *local_token_offset=0;
  }
  else if (db_type == DXE_PCD_DB) {
    printf("\nDXE PCD DB\n");
  }

  if (debug) {
    print_pcd_table_header(&pcd_table_header);
    printf("_____\n");

    print_pcd_table_guids(db, &pcd_table_header);
    printf("_____\n");
  }

  printf("LocalTokenNumberTable:\n");
  for (int i=0; i<pcd_table_header.LocalTokenCount; i++) {
    UINT32 Token = *(UINT32*)&db[pcd_table_header.LocalTokenNumberTableOffset + i*sizeof(UINT32)];
    if ((db_type == DXE_PCD_DB) && (*local_token_offset == -1))
      printf("\n(PEI Local Token Count + %d):\n", i+1);
    else
      printf("\n%d:\n", i+1+(*local_token_offset));

    print_local_token_value(Token);
    print_dynamic_ex(db, &pcd_table_header, i);
    print_name_table_info(db, &pcd_table_header, i);

    if ((Token & PCD_DATABASE_OFFSET_MASK) >= db_size) {
      printf("0 - unitialized\n");
      if ((Token & PCD_DATABASE_OFFSET_MASK) >= (db_size + pcd_table_header.UninitDataBaseSize)) {
        printf("(Offset is wrong!)\n");
      }
      continue;
    }

    int Size = 0;
    STRING_HEAD* StringTableIdx;
    char* buf;
    VPD_HEAD* VpdHead;
    VARIABLE_HEAD* VariableHead;
    switch (Token & PCD_TYPE_ALL_SET) {
      case PCD_TYPE_HII:
      case PCD_TYPE_HII|PCD_TYPE_STRING:
        printf("HII VARIABLE\n");
        VariableHead = (VARIABLE_HEAD *)(db + (Token & PCD_DATABASE_OFFSET_MASK));
        printf("Guid:\n");
        print_guid_by_index(db, &pcd_table_header, VariableHead->GuidTableIndex);
        printf("\n");
        printf("Name:\n");
        UINT16* Name = (UINT16 *)(&db[pcd_table_header.StringTableOffset + VariableHead->StringIndex]);
        int NameLength = 0;
        while ((Name[NameLength]) != 0x0) {
          NameLength++;
        }
        print_buffer((UINT8*)Name, (NameLength+1)*2);
        if (debug) {
          printf("DefaultValueOffset=0x%08x\n", VariableHead->DefaultValueOffset);
          printf("Attributes=0x%08x\n", VariableHead->Attributes);
          printf("Property=0x%04x\n",  VariableHead->Property);
        }
        printf("Offset:\n0x%04x\n", VariableHead->Offset);

        UINT8* VaraiableDefaultBuffer;
        if ((Token & PCD_TYPE_ALL_SET) == (PCD_TYPE_HII|PCD_TYPE_STRING)) {
          StringTableIdx = (STRING_HEAD *)(db + VariableHead->DefaultValueOffset);
          VaraiableDefaultBuffer = &db[pcd_table_header.StringTableOffset + *StringTableIdx];
          printf("TBD - parser not implemented\n");
        } else {
          print_data_token_value(Token, db, VariableHead->DefaultValueOffset);
        }
        break;

      case PCD_TYPE_VPD:
        VpdHead = (VPD_HEAD *)(db + (Token & PCD_DATABASE_OFFSET_MASK));
        printf("VPD offset = 0x%08x (=%d)\n", VpdHead->Offset, VpdHead->Offset);
        if (vpd) {
          if (VpdHead->Offset > vpd_size) {
            printf("Error! VPD offset is outside of VPD block\n");
            break;
          }
        }
        if ((Token & PCD_DATUM_TYPE_ALL_SET) == PCD_DATUM_TYPE_POINTER) {
          UINT32 SizeTableIndex = get_size_table_index(db, &pcd_table_header, i);
          UINT16 MaxSize = *(UINT16*)&db[pcd_table_header.SizeTableOffset + (SizeTableIndex*4)];
          UINT16 CurrentSize = *(UINT16*)&db[pcd_table_header.SizeTableOffset + (SizeTableIndex*4) + 2];
          printf("CurrentSize = %d\n", CurrentSize);
          printf("MaxSize     = %d\n", MaxSize);
          if (vpd) {
              printf("Value:\n");
              buf = &vpd[VpdHead->Offset];
              print_buffer(buf, CurrentSize);
          }
        } else {
          if (vpd) {
            print_data_token_value(Token, vpd, VpdHead->Offset);
          } else {
            if ((Token & PCD_DATUM_TYPE_ALL_SET) == PCD_DATUM_TYPE_UINT8)
              printf("Size = 1\n");
            else if ((Token & PCD_DATUM_TYPE_ALL_SET) == PCD_DATUM_TYPE_UINT16)
              printf("Size = 2\n");
            else if ((Token & PCD_DATUM_TYPE_ALL_SET) == PCD_DATUM_TYPE_UINT32)
              printf("Size = 4\n");
            else if ((Token & PCD_DATUM_TYPE_ALL_SET) == PCD_DATUM_TYPE_UINT64)
              printf("Size = 8\n");
            else
              printf("Error! Wrong datum type\n");
          }
        }
        if (!vpd)
          printf("Provide VPD file to print actual data\n");
        break;

      case PCD_TYPE_STRING:
        if ((Token & PCD_DATUM_TYPE_ALL_SET) == PCD_DATUM_TYPE_POINTER) {
          UINT32 SizeTableIndex = get_size_table_index(db, &pcd_table_header, i);
          UINT16 MaxSize = *(UINT16*)&db[pcd_table_header.SizeTableOffset + (SizeTableIndex*4)];
          UINT16 CurrentSize = *(UINT16*)&db[pcd_table_header.SizeTableOffset + (SizeTableIndex*4) + 2];
          printf("CurrentSize = %d\n", CurrentSize);
          printf("MaxSize     = %d\n", MaxSize);
          StringTableIdx = (STRING_HEAD *)(db + (Token & PCD_DATABASE_OFFSET_MASK));
          buf = &db[pcd_table_header.StringTableOffset + *StringTableIdx];
          printf("Value:\n");
          print_buffer(buf, CurrentSize);
        } else {
          printf("Error! Wrong datum type for PCD_TYPE_STRING\n");
        }
        break;

      case PCD_TYPE_DATA:
        print_data_token_value(Token, db, (Token & PCD_DATABASE_OFFSET_MASK));
        break;

      default:
        printf("Error! Unknown PCD_TYPE\n");
        break;
    }
  }
  printf("_____\n");

  munmap(db, db_size);
  *local_token_offset = pcd_table_header.LocalTokenCount;
  return 0;
}


char* pei_pcd_table_name = '\0';
char* dxe_pcd_table_name = '\0';
char* vpd_filename = '\0';

void usage(char* program_name)
{
  printf("Usage: parse_pcd_db [--peidb <PEI_PCD_DB.raw>] [--dxedb <DXE_PCD_DB.raw>]\n");
  printf("Program to parse PCD Database raw files\n");
  printf("\n");
  printf("--peidb <PEI_PCD_DB.raw>     - provide PEI PCD database\n");
  printf("--peidb <DXE_PCD_DB.raw>     - provide DXE PCD database\n");
  printf("--vpd   <VPD.bin>            - provide VPD binary\n");
}

int main(int argc, char** argv)
{
  const struct option longopts[] = {
    { "peidb", required_argument, NULL, 'p' },
    { "dxedb", required_argument, NULL, 'd' },
    { "vpd"  , required_argument, NULL, 'v' },
    { "help",  no_argument,       NULL, 'h' },
    { NULL, 0, NULL, 0}
  };

  int c;
  while((c = getopt_long(argc, argv, "p:d:v:h", longopts, NULL)) != -1) {
    switch (c) {
      case 'p':
        pei_pcd_table_name = optarg;
        break;
      case 'd':
        dxe_pcd_table_name = optarg;
        break;
      case 'v':
        vpd_filename = optarg;
        break;
      case 'h':
        usage(argv[0]);
        return(EXIT_SUCCESS);
        break;
      default:
        usage(argv[0]);
        return(EXIT_FAILURE);
        break;
    }
  }

  if ((!pei_pcd_table_name) && (!dxe_pcd_table_name)) {
    printf("Error! Provide at least one db file\n\n");
    usage(argv[0]);
    return(EXIT_FAILURE);
  }

  char* vpd = NULL;
  off_t vpd_size = 0;
  if (vpd_filename) {
    if (map_file(vpd_filename, &vpd, &vpd_size)) {
      printf("Error! Can't load VPD file\n");
      return(EXIT_FAILURE);
    }
  }

  int local_token_offset = -1;
  if (pei_pcd_table_name) {
    if (parse_pcd_db(pei_pcd_table_name, PEI_PCD_DB, &local_token_offset, vpd, vpd_size)) {
      printf("Error! Can't parse PEI PCD DB\n");
      return(EXIT_FAILURE);
    }
  }
  if (dxe_pcd_table_name) {
    if (parse_pcd_db(dxe_pcd_table_name, DXE_PCD_DB, &local_token_offset, vpd, vpd_size)) {
      printf("Error! Can't parse DXE PCD DB\n");
      return(EXIT_FAILURE);
    }
  }

  if (vpd)
    munmap(vpd, vpd_size);

  return(EXIT_SUCCESS);
}
