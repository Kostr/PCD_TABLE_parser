#ifndef PCD_TABLE_H
#define PCD_TABLE_H

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

#endif
