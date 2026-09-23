#ifndef LKARefMdl_h_
#define LKARefMdl_h_
#include <cmath>
#include "sl_AsyncioQueue/AsyncioQueueCAPI.h"
#include "rtwtypes.h"
#include "slsv_diagnostic_codegen_c_api.h"
#include "simstruc.h"
#include "fixedpoint.h"
#include "rtw_continuous.h"
#include "rtw_solver.h"
#include "sfrtif/sfc_sdi.h"
#include "LKARefMdl_types.h"
#include <stddef.h>
#include "rtw_modelmap_simtarget.h"
#include "rt_nonfinite.h"
#include <string.h>
struct llf3xg3ir0 { real32_T gbwtl0fblt ; real32_T cw05jjf2lo ; real32_T
cxafockzdk ; real32_T ohm0befczw ; real32_T fpohxwxgen ; boolean_T o1toyohefj
[ 2 ] ; } ; struct j4ytv2gj5l { real_T cfshuxhiaz ; struct { void * AQHandles
; } dgzdnqhm5m ; struct { void * AQHandles ; } cd1uoy3530 ; struct { void *
AQHandles ; } htxuvomuq0 ; struct { void * AQHandles ; } dpozjnjxaw ; struct
{ void * AQHandles ; } mmnuh2p4wu ; struct { void * AQHandles ; } eckutxnkdw
; struct { void * AQHandles ; } e0vkjlbzyh ; struct { void * AQHandles ; }
lyuy4nffcy ; struct { void * AQHandles ; } jze4n30elb ; struct { void *
AQHandles ; } asnvfo2btx ; real32_T c20jlygsay ; real32_T hryujm23gv ;
real32_T idw3weirf3 ; real32_T ayeaqphirh ; real32_T fsrrgfesw1 [ 5 ] ;
int8_T jeftpncfv5 ; int8_T lrava23ncm ; int8_T hiiwcrboab ; int8_T hwadutcumr
; boolean_T b0zknnwgof ; } ; struct ajghr3ctyrx_ { real_T P_0 ; real32_T P_1
; real32_T P_2 ; real32_T P_3 ; real32_T P_4 ; boolean_T P_5 ; real_T P_6 ;
real_T P_7 ; real_T P_8 ; real_T P_9 ; real32_T P_10 ; real32_T P_11 ;
real32_T P_12 ; real32_T P_13 ; real32_T P_14 ; real32_T P_15 ; real32_T P_16
; real32_T P_17 ; real32_T P_18 ; real32_T P_19 ; real32_T P_20 ; real32_T
P_21 ; real32_T P_22 ; real32_T P_23 ; real32_T P_24 ; real32_T P_25 ;
real32_T P_26 ; real32_T P_27 ; real32_T P_28 ; real32_T P_29 ; real32_T P_30
; real32_T P_31 ; real32_T P_32 ; real32_T P_33 ; real32_T P_34 ; boolean_T
P_35 [ 16 ] ; } ; struct cyntjpgsgd { struct SimStruct_tag * _mdlRefSfcnS ;
struct { rtwCAPI_ModelMappingInfo mmi ; rtwCAPI_ModelMapLoggingInstanceInfo
mmiLogInstanceInfo ; void * dataAddress [ 5 ] ; int32_T * vardimsAddress [ 5
] ; RTWLoggingFcnPtr loggingPtrs [ 5 ] ; sysRanDType * systemRan [ 7 ] ;
int_T systemTid [ 7 ] ; } DataMapInfo ; struct { int_T mdlref_GlobalTID [ 2 ]
; } Timing ; } ; struct c5e02tcafze { llf3xg3ir0 rtb ; j4ytv2gj5l rtdw ;
a0qef2fbcm rtm ; } ; extern void c1o4boxk4f ( SimStruct * _mdlRefSfcnS ,
int_T mdlref_TID0 , int_T mdlref_TID1 , a0qef2fbcm * const iu2zxy1bg0 ,
llf3xg3ir0 * localB , j4ytv2gj5l * localDW , void * sysRanPtr , int
contextTid , rtwCAPI_ModelMappingInfo * rt_ParentMMI , const char_T *
rt_ChildPath , int_T rt_ChildMMIIdx , int_T rt_CSTATEIdx ) ; extern void
mr_LKARefMdl_MdlInfoRegFcn ( SimStruct * mdlRefSfcnS , char_T * modelName ,
int_T * retVal ) ; extern mxArray * mr_LKARefMdl_GetDWork ( const c5e02tcafze
* mdlrefDW ) ; extern void mr_LKARefMdl_SetDWork ( c5e02tcafze * mdlrefDW ,
const mxArray * ssDW ) ; extern void mr_LKARefMdl_RegisterSimStateChecksum ( SimStruct * S ) ; extern mxArray * mr_LKARefMdl_GetSimStateDisallowedBlocks ( ) ; extern const rtwCAPI_ModelMappingStaticInfo * LKARefMdl_GetCAPIStaticMap ( void ) ; extern void pi2hal0oyf ( a0qef2fbcm * const iu2zxy1bg0 , j4ytv2gj5l * localDW ) ; extern void jzx5dnvkfi ( llf3xg3ir0 * localB , j4ytv2gj5l * localDW ) ; extern void krrs31v12z ( j4ytv2gj5l * localDW ) ; extern void dmps3x051h ( a0qef2fbcm * const iu2zxy1bg0 , j4ytv2gj5l * localDW ) ; extern void i30xuqpc24 ( llf3xg3ir0 * localB , j4ytv2gj5l * localDW ) ; extern void LKARefMdl ( a0qef2fbcm * const iu2zxy1bg0 , const boolean_T * dl2h23j0yb , const real32_T * ggzreh1sbs , const real_T * gy14yybqge , const LaneSensor * lk4lphhadb , const boolean_T * laaa2vlk2x , const boolean_T * iv3nuh2y3n , boolean_T * gsarxirkom , real_T * h0jeecywei , llf3xg3ir0 * localB , j4ytv2gj5l * localDW ) ; extern void cpebt4uidm ( a0qef2fbcm * const iu2zxy1bg0 ) ;
#endif
