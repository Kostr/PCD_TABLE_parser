#include <stdio.h>
#include <string.h>
#include "edk2_types.h"

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
  },
  {
    "93bb96af-b9f2-4eb8-9462e0ba74564236",
    "gUefiOvmfPkgTokenSpaceGuid"
  },
  {
    "914aebe7-4635-459b-aa1c11e219b03a10",
    "gEfiMdePkgTokenSpaceGuid"
  },
  {
    "40e064b2-0ae0-48b1-a07df8cf1e1a2310",
    "gEfiNetworkPkgTokenSpaceGuid"
  },
  {
    "150cab53-ad47-4385-b5ddbcfc76bacaf0",
    "gUefiLessonsPkgTokenSpaceGuid"
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
    if (!strcmp(guid_str, predefined_guids[i].guid_str)) {
      printf(" [%s]", predefined_guids[i].name);
      break;
    }
}
