#include <stddef.h>
#include "rtw_capi.h"
#ifdef HOST_CAPI_BUILD
#include "LKARefMdl_capi_host.h"
#define sizeof(...) ((size_t)(0xFFFF))
#undef rt_offsetof
#define rt_offsetof(s,el) ((uint16_T)(0xFFFF))
#define TARGET_CONST
#define TARGET_STRING(s) (s)
#ifndef SS_UINT64
#define SS_UINT64 19
#endif
#ifndef SS_INT64
#define SS_INT64 20
#endif
#else
#include "builtin_typeid_types.h"
#include "LKARefMdl.h"
#include "LKARefMdl_capi.h"
#include "LKARefMdl_private.h"
#ifdef LIGHT_WEIGHT_CAPI
#define TARGET_CONST
#define TARGET_STRING(s)               ((NULL))
#else
#define TARGET_CONST                   const
#define TARGET_STRING(s)               (s)
#endif
#endif
static const rtwCAPI_SignalHierLoggingInfo rtSigHierLoggingInfo [ ] = { { ""
, 4 , 0 } , { "Curvature" , 0 , - 1 } , { "CurvatureDerivative" , 0 , - 1 } ,
{ "HeadingAngle" , 0 , - 1 } , { "LateralOffset" , 0 , - 1 } } ; static const
uint_T rtSigHierLoggingChildIdxs [ ] = { 1 , 2 , 3 , 4 } ; static
rtwCAPI_Signals rtBlockSignals [ ] = { { 0 , 0 , ( NULL ) , ( NULL ) , 0 , 0
, 0 , 0 , 0 } } ; static rtwCAPI_States rtBlockStates [ ] = { { 0 , - 1 ,
TARGET_STRING ( "LKARefMdl/Estimate Lane Center/Delay" ) , TARGET_STRING ( "DSTATE" ) , "" , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 } , { 1 , - 1 , TARGET_STRING ( "LKARefMdl/Estimate Lane Center/Delay" ) , TARGET_STRING ( "DSTATE" ) , "" , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 1 } , { 2 , - 1 , TARGET_STRING ( "LKARefMdl/Estimate Lane Center/Delay" ) , TARGET_STRING ( "DSTATE" ) , "" , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 2 } , { 3 , - 1 , TARGET_STRING ( "LKARefMdl/Estimate Lane Center/Delay" ) , TARGET_STRING ( "DSTATE" ) , "" , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 3 } , { 4 , - 1 , TARGET_STRING ( "LKARefMdl/Apply Assist/Detect Driver Resistance\n/Tapped Delay" ) , TARGET_STRING ( "X" ) , "" , 0 , 0 , 1 , 0 , 0 , 0 , - 1 , 0 } , { 0 , - 1 , ( NULL ) , ( NULL ) , ( NULL ) , 0 , 0 , 0 , 0 , 0 , 0 , - 1 , 0 } } ; static int_T rt_LoggedStateIdxList [ ] = { 14 , 13 , 12 , 11 , 15 } ;
#ifndef HOST_CAPI_BUILD
static void LKARefMdl_InitializeDataAddr ( void * dataAddr [ ] , j4ytv2gj5l *
localDW ) { dataAddr [ 0 ] = ( void * ) ( & localDW -> ayeaqphirh ) ;
dataAddr [ 1 ] = ( void * ) ( & localDW -> idw3weirf3 ) ; dataAddr [ 2 ] = ( void * ) ( & localDW -> hryujm23gv ) ; dataAddr [ 3 ] = ( void * ) ( & localDW -> c20jlygsay ) ; dataAddr [ 4 ] = ( void * ) ( & localDW -> fsrrgfesw1 [ 0 ] ) ; }
#endif
#ifndef HOST_CAPI_BUILD
static void LKARefMdl_InitializeVarDimsAddr ( int32_T * vardimsAddr [ ] ) {
vardimsAddr [ 0 ] = ( NULL ) ; }
#endif
#ifndef HOST_CAPI_BUILD
static void LKARefMdl_InitializeLoggingFunctions ( RTWLoggingFcnPtr
loggingPtrs [ ] ) { loggingPtrs [ 0 ] = ( NULL ) ; loggingPtrs [ 1 ] = ( NULL
) ; loggingPtrs [ 2 ] = ( NULL ) ; loggingPtrs [ 3 ] = ( NULL ) ; loggingPtrs
[ 4 ] = ( NULL ) ; }
#endif
static TARGET_CONST rtwCAPI_DataTypeMap rtDataTypeMap [ ] = { { "float" ,
"real32_T" , 0 , 0 , sizeof ( real32_T ) , ( uint8_T ) SS_SINGLE , 0 , 0 , 0
} } ;
#ifdef HOST_CAPI_BUILD
#undef sizeof
#endif
static TARGET_CONST rtwCAPI_ElementMap rtElementMap [ ] = { { ( NULL ) , 0 ,
0 , 0 , 0 } , } ; static rtwCAPI_DimensionMap rtDimensionMap [ ] = { {
rtwCAPI_SCALAR , 0 , 2 , 0 } , { rtwCAPI_VECTOR , 2 , 2 , 0 } } ; static
uint_T rtDimensionArray [ ] = { 1 , 1 , 5 , 1 } ; static const real_T
rtcapiStoredFloats [ ] = { 0.05 , 0.0 } ; static rtwCAPI_FixPtMap rtFixPtMap
[ ] = { { ( NULL ) , ( NULL ) , rtwCAPI_FIX_RESERVED , 0 , 0 , ( boolean_T )
0 } , } ; static rtwCAPI_SampleTimeMap rtSampleTimeMap [ ] = { { static_cast
< const void * > ( & rtcapiStoredFloats [ 0 ] ) , static_cast < const void *
> ( & rtcapiStoredFloats [ 1 ] ) , static_cast < int8_T > ( 0 ) , static_cast
< uint8_T > ( 0 ) } } ; static int_T rtContextSystems [ 7 ] ; static
rtwCAPI_LoggingMetaInfo loggingMetaInfo [ ] = { { 0 , 0 , "" , 0 } } ; static
rtwCAPI_ModelMapLoggingStaticInfo mmiStaticInfoLogging = { 7 ,
rtContextSystems , loggingMetaInfo , 0 , ( NULL ) , { 5 ,
rtSigHierLoggingInfo , rtSigHierLoggingChildIdxs } , 0 , ( NULL ) } ; static
rtwCAPI_ModelMappingStaticInfo mmiStatic = { { rtBlockSignals , 0 , ( NULL )
, 0 , ( NULL ) , 0 } , { ( NULL ) , 0 , ( NULL ) , 0 } , { rtBlockStates , 5
} , { rtDataTypeMap , rtDimensionMap , rtFixPtMap , rtElementMap ,
rtSampleTimeMap , rtDimensionArray } , "float" , { 3471018810U , 935462099U ,
2640517270U , 4287035675U } , & mmiStaticInfoLogging , 0 , ( boolean_T ) 0 ,
rt_LoggedStateIdxList } ; const rtwCAPI_ModelMappingStaticInfo *
LKARefMdl_GetCAPIStaticMap ( void ) { return & mmiStatic ; }
#ifndef HOST_CAPI_BUILD
static void LKARefMdl_InitializeSystemRan ( a0qef2fbcm * const iu2zxy1bg0 ,
sysRanDType * systemRan [ ] , j4ytv2gj5l * localDW , int_T systemTid [ ] ,
void * rootSysRanPtr , int rootTid ) { UNUSED_PARAMETER ( iu2zxy1bg0 ) ;
UNUSED_PARAMETER ( localDW ) ; systemRan [ 0 ] = ( sysRanDType * )
rootSysRanPtr ; systemRan [ 1 ] = ( sysRanDType * ) & localDW -> hiiwcrboab ;
systemRan [ 2 ] = ( sysRanDType * ) & localDW -> hwadutcumr ; systemRan [ 3 ]
= ( sysRanDType * ) & localDW -> jeftpncfv5 ; systemRan [ 4 ] = ( sysRanDType
* ) & localDW -> lrava23ncm ; systemRan [ 5 ] = ( NULL ) ; systemRan [ 6 ] =
( NULL ) ; systemTid [ 2 ] = iu2zxy1bg0 -> Timing . mdlref_GlobalTID [ 0 ] ;
systemTid [ 1 ] = iu2zxy1bg0 -> Timing . mdlref_GlobalTID [ 0 ] ; systemTid [
4 ] = iu2zxy1bg0 -> Timing . mdlref_GlobalTID [ 0 ] ; systemTid [ 3 ] =
iu2zxy1bg0 -> Timing . mdlref_GlobalTID [ 0 ] ; systemTid [ 5 ] = iu2zxy1bg0
-> Timing . mdlref_GlobalTID [ 0 ] ; systemTid [ 6 ] = iu2zxy1bg0 -> Timing .
mdlref_GlobalTID [ 0 ] ; systemTid [ 0 ] = rootTid ; rtContextSystems [ 0 ] =
0 ; rtContextSystems [ 1 ] = 1 ; rtContextSystems [ 2 ] = 2 ;
rtContextSystems [ 3 ] = 3 ; rtContextSystems [ 4 ] = 4 ; rtContextSystems [
5 ] = 0 ; rtContextSystems [ 6 ] = 0 ; }
#endif
#ifndef HOST_CAPI_BUILD
void LKARefMdl_InitializeDataMapInfo ( a0qef2fbcm * const iu2zxy1bg0 ,
j4ytv2gj5l * localDW , void * sysRanPtr , int contextTid ) {
rtwCAPI_SetVersion ( iu2zxy1bg0 -> DataMapInfo . mmi , 1 ) ;
rtwCAPI_SetStaticMap ( iu2zxy1bg0 -> DataMapInfo . mmi , & mmiStatic ) ;
rtwCAPI_SetLoggingStaticMap ( iu2zxy1bg0 -> DataMapInfo . mmi , &
mmiStaticInfoLogging ) ; LKARefMdl_InitializeDataAddr ( iu2zxy1bg0 ->
DataMapInfo . dataAddress , localDW ) ; rtwCAPI_SetDataAddressMap ( iu2zxy1bg0
-> DataMapInfo . mmi , iu2zxy1bg0 -> DataMapInfo . dataAddress ) ;
LKARefMdl_InitializeVarDimsAddr ( iu2zxy1bg0 -> DataMapInfo . vardimsAddress
) ; rtwCAPI_SetVarDimsAddressMap ( iu2zxy1bg0 -> DataMapInfo . mmi ,
iu2zxy1bg0 -> DataMapInfo . vardimsAddress ) ; rtwCAPI_SetPath ( iu2zxy1bg0
-> DataMapInfo . mmi , ( NULL ) ) ; rtwCAPI_SetFullPath ( iu2zxy1bg0 ->
DataMapInfo . mmi , ( NULL ) ) ; LKARefMdl_InitializeLoggingFunctions ( iu2zxy1bg0 -> DataMapInfo . loggingPtrs ) ; rtwCAPI_SetLoggingPtrs ( iu2zxy1bg0 -> DataMapInfo . mmi , iu2zxy1bg0 -> DataMapInfo . loggingPtrs ) ; rtwCAPI_SetInstanceLoggingInfo ( iu2zxy1bg0 -> DataMapInfo . mmi , & iu2zxy1bg0 -> DataMapInfo . mmiLogInstanceInfo ) ; rtwCAPI_SetChildMMIArray ( iu2zxy1bg0 -> DataMapInfo . mmi , ( NULL ) ) ; rtwCAPI_SetChildMMIArrayLen ( iu2zxy1bg0 -> DataMapInfo . mmi , 0 ) ; LKARefMdl_InitializeSystemRan ( iu2zxy1bg0 , iu2zxy1bg0 -> DataMapInfo . systemRan , localDW , iu2zxy1bg0 -> DataMapInfo . systemTid , sysRanPtr , contextTid ) ; rtwCAPI_SetSystemRan ( iu2zxy1bg0 -> DataMapInfo . mmi , iu2zxy1bg0 -> DataMapInfo . systemRan ) ; rtwCAPI_SetSystemTid ( iu2zxy1bg0 -> DataMapInfo . mmi , iu2zxy1bg0 -> DataMapInfo . systemTid ) ; rtwCAPI_SetGlobalTIDMap ( iu2zxy1bg0 -> DataMapInfo . mmi , & iu2zxy1bg0 -> Timing . mdlref_GlobalTID [ 0 ] ) ; }
#else
#ifdef __cplusplus
extern "C" {
#endif
void LKARefMdl_host_InitializeDataMapInfo ( LKARefMdl_host_DataMapInfo_T *
dataMap , const char * path ) { rtwCAPI_SetVersion ( dataMap -> mmi , 1 ) ;
rtwCAPI_SetStaticMap ( dataMap -> mmi , & mmiStatic ) ;
rtwCAPI_SetDataAddressMap ( dataMap -> mmi , ( NULL ) ) ;
rtwCAPI_SetVarDimsAddressMap ( dataMap -> mmi , ( NULL ) ) ; rtwCAPI_SetPath
( dataMap -> mmi , path ) ; rtwCAPI_SetFullPath ( dataMap -> mmi , ( NULL ) )
; rtwCAPI_SetChildMMIArray ( dataMap -> mmi , ( NULL ) ) ;
rtwCAPI_SetChildMMIArrayLen ( dataMap -> mmi , 0 ) ; }
#ifdef __cplusplus
}
#endif
#endif
