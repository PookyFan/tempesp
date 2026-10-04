#ifndef __FREERTOS_CONFIG_OVERRIDES_H__
#define __FREERTOS_CONFIG_OVERRIDES_H__

#define configSUPPORT_STATIC_ALLOCATION 1

//Fall back to default config for everything else that's not set explicitly above
#include_next<FreeRTOSConfig.h>
#endif