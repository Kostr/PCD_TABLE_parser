#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

//const char* filename = "Build/OvmfX64/DEBUG_GCC5/X64/MdeModulePkg/Universal/PCD/Pei/Pcd/OUTPUT/PEIPcdDataBase.raw";
//const char* filename = "Build/OvmfX64/DEBUG_GCC5/X64/MdeModulePkg/Universal/PCD/Dxe/Pcd/OUTPUT/DXEPcdDataBase.raw";

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
typedef UINT64 SKU_ID;
typedef UINT32 TABLE_OFFSET;
typedef EFI_GUID GUID;

#define VER7
#ifdef VER7
typedef struct {
  GUID            Signature;                    // PcdDataBaseGuid.
  UINT32          BuildVersion;
  UINT32          Length;                       // Length of DEFAULT SKU PCD DB
  SKU_ID          SystemSkuId;                  // Current SkuId value.
  UINT32          LengthForAllSkus;             // Length of all SKU PCD DB
  UINT32          UninitDataBaseSize;           // Total size for PCD those default value with 0.
  TABLE_OFFSET    LocalTokenNumberTableOffset;
  TABLE_OFFSET    ExMapTableOffset;
  TABLE_OFFSET    GuidTableOffset;
  TABLE_OFFSET    StringTableOffset;
  TABLE_OFFSET    SizeTableOffset;
  TABLE_OFFSET    SkuIdTableOffset;
  TABLE_OFFSET    PcdNameTableOffset;
  UINT16          LocalTokenCount;              // LOCAL_TOKEN_NUMBER for all.
  UINT16          ExTokenCount;                 // EX_TOKEN_NUMBER for DynamicEx.
  UINT16          GuidTableCount;               // The Number of Guid in GuidTable.
  UINT8           Pad[6];                       // Pad bytes to satisfy the alignment.
} PCD_TABLE_HEADER;
#else
// https://github.com/tianocore/edk2/commit/7c73626513238176bdd16dca14fcf3f9e10bcc81
typedef struct {
  GUID            Signature;                    // PcdDataBaseGuid.
  UINT32          BuildVersion;
  UINT32          Length;
  SKU_ID          SystemSkuId;                  // Current SkuId value.
  UINT32          UninitDataBaseSize;           // Total size for PCD those default value with 0.
  TABLE_OFFSET    LocalTokenNumberTableOffset;
  TABLE_OFFSET    ExMapTableOffset;
  TABLE_OFFSET    GuidTableOffset;
  TABLE_OFFSET    StringTableOffset;
  TABLE_OFFSET    SizeTableOffset;
  TABLE_OFFSET    SkuIdTableOffset;
  TABLE_OFFSET    PcdNameTableOffset;
  UINT16          LocalTokenCount;              // LOCAL_TOKEN_NUMBER for all.
  UINT16          ExTokenCount;                 // EX_TOKEN_NUMBER for DynamicEx.
  UINT16          GuidTableCount;               // The Number of Guid in GuidTable.
  UINT8           Pad[2];                       // Pad bytes to satisfy the alignment.
} PCD_TABLE_HEADER;
#endif

typedef struct  {
  UINT32    ExTokenNumber;
  UINT16    TokenNumber;        // Token Number for Dynamic-Ex PCD.
  UINT16    ExGuidIndex;        // Index of GuidTable in units of GUID.
} DYNAMICEX_MAPPING;


typedef UINT32 STRING_HEAD;

typedef UINT16 SIZE_INFO;


typedef struct {
  UINT32    TokenSpaceCNameIndex; // Offset in String Table in units of UINT8.
  UINT32    PcdCNameIndex;        // Offset in String Table in units of UINT8.
} PCD_NAME_INDEX;

typedef struct {
  UINT32    StringIndex;        // Offset in String Table in units of UINT8.
  UINT32    DefaultValueOffset; // Offset of the Default Value.
  UINT16    GuidTableIndex;     // Offset in Guid Table in units of GUID.
  UINT16    Offset;             // Offset in Variable.
  UINT32    Attributes;         // Variable attributes.
  UINT16    Property;           // Variable property.
  UINT16    Reserved;
} VARIABLE_HEAD;

#define PCD_TYPE_SHIFT  28

#define PCD_TYPE_DATA    (0x0U << PCD_TYPE_SHIFT)
#define PCD_TYPE_HII     (0x8U << PCD_TYPE_SHIFT)
#define PCD_TYPE_VPD     (0x4U << PCD_TYPE_SHIFT)
#define PCD_TYPE_STRING  (0x1U << PCD_TYPE_SHIFT)

#define PCD_TYPE_ALL_SET  (PCD_TYPE_DATA | PCD_TYPE_HII | PCD_TYPE_VPD | PCD_TYPE_STRING)

#define PCD_DATUM_TYPE_SHIFT  24

#define PCD_DATUM_TYPE_POINTER  (0x0U << PCD_DATUM_TYPE_SHIFT)
#define PCD_DATUM_TYPE_UINT8    (0x1U << PCD_DATUM_TYPE_SHIFT)
#define PCD_DATUM_TYPE_UINT16   (0x2U << PCD_DATUM_TYPE_SHIFT)
#define PCD_DATUM_TYPE_UINT32   (0x4U << PCD_DATUM_TYPE_SHIFT)
#define PCD_DATUM_TYPE_UINT64   (0x8U << PCD_DATUM_TYPE_SHIFT)

#define PCD_DATUM_TYPE_ALL_SET  (PCD_DATUM_TYPE_POINTER | \
                                 PCD_DATUM_TYPE_UINT8   | \
                                 PCD_DATUM_TYPE_UINT16  | \
                                 PCD_DATUM_TYPE_UINT32  | \
                                 PCD_DATUM_TYPE_UINT64)

#define PCD_DATUM_TYPE_SHIFT2  20

#define PCD_DATUM_TYPE_UINT8_BOOLEAN  (0x1U << PCD_DATUM_TYPE_SHIFT2)

#define PCD_DATABASE_OFFSET_MASK  (~(PCD_TYPE_ALL_SET | PCD_DATUM_TYPE_ALL_SET | PCD_DATUM_TYPE_UINT8_BOOLEAN))


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


struct guid_name {
  char guid_str[sizeof("xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx")];
  char name[100];
}; 

struct guid_name predefined_guids[] = {
  {
    "c51f1883-df00-4f6a-08a0369f6098fdaf",
    "gInsydeTokenSpaceGuid"
  },
  {
    "a1aff049-fdeb-442a-b32013ab4cb72bbc",
    "gEfiMdeModulePkgTokenSpaceGuid"
  },
  {
    "ffd4675e-ff47-46d9-ac248b331f937737",
    "gChipsetPkgTokenSpaceGuid"
  },
  {
    "a04a27f4-df00-4d42-b55239511302113d",
    "gSystemConfigurationGuid"
  },
  {
    "d4d8435f-fffb-4acb-a04dff0fad677fe9",
    "gEfiAmdAgesaPkgTokenSpaceGuid"
  },
  {
    "7788adf0-9788-4a3f-83facb512e7cf8dd",
    "gEfiAmdAgesaModulePkgTokenSpaceGuid"
  },
  {
    "ec17a54e-fec1-431b-ad9cb4f0bc6e2e56",
    "gH2OFlashDeviceConfigGuid"
  },
  {
    "e9a68f2a-13f1-4505-b6a1e182188ac49a",
    "gH2OFlashDevicePartNameGuid"
  },
  {
    "916e0ddd-2bd2-4704-93b9594b01a5fa9f",
    "gAmdCpmPkgTokenSpaceGuid"
  },
  {
    "cbdef572-719d-426e-8a4375ae913cffb2",
    "gH2OFlashDeviceGuid"
  },
  {
    "2f779836-610c-4615-a46254e73ef16192",
    "gH2OFlashDeviceMfrNameGuid"
  },
  {
    "e3f0f623-0ab3-41d3-8fbe2a3220ae89b5",
    "gSioGuid"
  },
  {
    "4ce9ac10-fadb-4aaa-94a1f8dde75efa69",
    "gH2OIpmiSupervyseTokenSpaceGuid"
  },
  {
    "03a6b6ab-e143-40f4-afba2aec8c3b9bc9",
    "gH2OIpmiPkgTokenSpaceGuid"
  },
  {
    "d1b99b46-e509-4eda-89ac7a6738bb821a",
    "gH2OEventLogPkgTokenSpaceGuid"
  },
  {
    "0d3fb176-9569-4d51-a3ef7d61c64feaba",
    "gEfiSecurityPkgTokenSpaceGuid"
  },
  {
    "f30e6887-b317-421a-879a1747b7d58444",
    "gSmbiosTokenSpaceGuid"
  },
  {
    "d5eac930-e571-4a98-a8b81d7999b01cbf",
    "gH2OBdsDefaultBootListGenericOsTokenSpaceGuid"
  },
  {
    "82a35978-9b13-4f67-9bd6753736e2a016",
    "gH2OIpmiSupervyseBiosInfoValueGuid"
  },
  {
    "3ee927e3-4761-4497-96ecc9bbba3de7e7",
    "gInsydeAdvBootTokenSpaceGuid"
  },
  {
    "2d068309-12ac-45ab-96009187513ccdd8",
    "gLinkBootTokenSpaceGuid"
  },
  {
    "f9738d77-b2ad-4c30-a3d4b5371fe5b5cf",
    "gH2OIpmiSupervyseBiosInfoTitleGuid"
  },
  {
    "f63d4e80-4cb5-4da1-a603b384f664e603",
    "gH2OIpmiSupervyseBiosInfoEnableGuid"
  },
  {
    "31849028-226b-439c-9f20d7c227ccc5db",
    "gH2OBdsHotKeyDescQuietGuid"
  },
  {
    "9bd000ea-3348-45d3-95b38b0f3b073cd9",
    "gH2OIpmiSupervyseBiosInfoIdGuid"
  },
  {
    "42d9df7a-b49d-4fbf-850b0d35a5ddb2e3",
    "gH2OBdsHotKeyDescGuid"
  },
  {
    "ac05bf33-995a-4ed4-aab8ef7ae80f5cb0",
    "gUefiCpuPkgTokenSpaceGuid"
  },
  {
    "7949382b-21a9-4bbf-a06158fceff424c3",
    "gH2OBdsHotKeyGuid"
  },
  {
    "0526f3f1-aa5a-45ab-b123812fad058ebc",
    "gH2OIpmiSupervyseBiosInfoGuid"
  },
  {
    "5204f764-df25-48a2-b3379ec122b85e0d",
    "gLinkSecurityTokenSpaceGuid"
  },
  {
    "60b430f5-409e-4ea6-b7e6650d45dee9ba",
    "gPlatformPkgTokenSpaceGuid"
  },
  {
    "0e8e9cba-fed6-459e-8027662c94f4660e",
    "gTpmTrustedAppTokenSpaceGuid"
  }
};

void print_guid(EFI_GUID guid)
{
  char guid_str[sizeof("xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx")];
  int result;
  result = snprintf(guid_str, sizeof(guid_str), "%08x-%04x-%04x-%02x%02x%02x%02x%02x%02x%02x%02x", guid.Data1, guid.Data2, guid.Data3,
                                                            guid.Data4[0],guid.Data4[1],guid.Data4[2],guid.Data4[3],
                                                            guid.Data4[4],guid.Data4[5],guid.Data4[6],guid.Data4[7]);

  if (result==(sizeof(guid_str)-2))
    printf("%s", guid_str);
  else
    printf("Guid print error!");

  for (int i=0; i<(sizeof(predefined_guids)/sizeof(predefined_guids[0])); i++)
    if (!strcmp(guid_str, predefined_guids[i].guid_str))
      printf(" [%s]", predefined_guids[i].name);
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
