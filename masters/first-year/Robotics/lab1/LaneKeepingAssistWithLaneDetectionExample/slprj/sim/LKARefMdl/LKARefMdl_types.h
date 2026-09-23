#ifndef LKARefMdl_types_h_
#define LKARefMdl_types_h_
#include "rtwtypes.h"
#ifndef DEFINED_TYPEDEF_FOR_LaneSensorBoundaries_
#define DEFINED_TYPEDEF_FOR_LaneSensorBoundaries_
struct LaneSensorBoundaries { real32_T Curvature ; real32_T
CurvatureDerivative ; real32_T HeadingAngle ; real32_T LateralOffset ;
real32_T Strength ; uint8_T sl_padding0 [ 4 ] ; } ;
#endif
#ifndef DEFINED_TYPEDEF_FOR_LaneSensor_
#define DEFINED_TYPEDEF_FOR_LaneSensor_
struct LaneSensor { LaneSensorBoundaries Left ; LaneSensorBoundaries Right ;
} ;
#endif
#ifndef SS_UINT64
#define SS_UINT64 19
#endif
#ifndef SS_INT64
#define SS_INT64 20
#endif
typedef struct ajghr3ctyrx_ ajghr3ctyrx ; typedef struct cyntjpgsgd
a0qef2fbcm ;
#endif
