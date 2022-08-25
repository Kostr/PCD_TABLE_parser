# PCD_TABLE_parser
Parser for Platform Configuration Database (PCD) tables created in the EDKII build process.

# Usage
```
parse_pcd_db [--peidb <PEI_PCD_DB.raw>] [--dxedb <DXE_PCD_DB.raw>] [--vpd <VPD.bin>] [--sku <SKU_number>]

--peidb <PEI_PCD_DB.raw>     - provide PEI PCD database
--peidb <DXE_PCD_DB.raw>     - provide DXE PCD database
--vpd   <VPD.bin>            - provide VPD binary
--sku   <SKU_number>         - provide SKU number
```

# Example
```
parse_pcd_db \
 --peidb "Build/OvmfX64/RELEASE_GCC5/X64/MdeModulePkg/Universal/PCD/Pei/Pcd/OUTPUT/PEIPcdDataBase.raw" \
 --dxedb "Build/OvmfX64/RELEASE_GCC5/X64/MdeModulePkg/Universal/PCD/Dxe/Pcd/OUTPUT/DXEPcdDataBase.raw" \
 --vpd   "Build/OvmfX64/RELEASE_GCC5/FV/8C3D856A-9BE6-468E-850A-24F7A8D38E08.bin" \
 --sku   0x55
```
