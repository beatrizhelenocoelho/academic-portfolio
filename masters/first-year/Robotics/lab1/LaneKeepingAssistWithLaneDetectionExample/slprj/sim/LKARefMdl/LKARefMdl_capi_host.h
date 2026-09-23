#ifndef LKARefMdl_cap_host_h__
#define LKARefMdl_cap_host_h__
#ifdef HOST_CAPI_BUILD
#include "rtw_capi.h"
#include "rtw_modelmap_simtarget.h"
struct LKARefMdl_host_DataMapInfo_T { rtwCAPI_ModelMappingInfo mmi ; } ;
#ifdef __cplusplus
extern "C" {
#endif
void LKARefMdl_host_InitializeDataMapInfo ( LKARefMdl_host_DataMapInfo_T *
dataMap , const char * path ) ;
#ifdef __cplusplus
}
#endif
#endif
#endif
