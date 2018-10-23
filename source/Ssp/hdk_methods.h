/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2015 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

/**********************************************************************
   Copyright [2014] [Cisco Systems, Inc.]
 
   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at
 
       http://www.apache.org/licenses/LICENSE-2.0
 
   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
**********************************************************************/

/*
 * hdk_methods.h - HNAP callback function prototypes
 */

#ifndef __HDK_METHODS_H__
#define __HDK_METHODS_H__

#include "hdk_data.h"

/* HNAP method function pointer type */
typedef void (*HDK_MethodFn)(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput);

/* HNAP method function defines */
//RDKB-9912: Removing all  the methods except SetBridgeConnect and GetDeviceSettings
//#define HDK_METHOD_PN_ADDPORTMAPPING
//#define HDK_METHOD_PN_DELETEPORTMAPPING
//#define HDK_METHOD_PN_GETCONNECTEDDEVICES
#define HDK_METHOD_PN_GETDEVICESETTINGS
//#define HDK_METHOD_PN_GETPORTMAPPINGS
//#define HDK_METHOD_PN_GETROUTERLANSETTINGS2
//#define HDK_METHOD_PN_GETROUTERSETTINGS
//#define HDK_METHOD_PN_GETWLANRADIOSECURITY
//#define HDK_METHOD_PN_GETWLANRADIOSETTINGS
//#define HDK_METHOD_PN_GETWLANRADIOS
//#define HDK_METHOD_PN_GETWANSETTINGS
//#define HDK_METHOD_PN_ISDEVICEREADY
//#define HDK_METHOD_PN_REBOOT
#define HDK_METHOD_PN_SETBRIDGECONNECT
//#define HDK_METHOD_PN_SETDEVICESETTINGS
//#define HDK_METHOD_PN_SETDEVICESETTINGS2
//#define HDK_METHOD_PN_SETROUTERLANSETTINGS2
//#define HDK_METHOD_PN_SETROUTERSETTINGS
//#define HDK_METHOD_PN_SETWLANRADIOSECURITY
//#define HDK_METHOD_PN_SETWLANRADIOSETTINGS
//#define HDK_METHOD_PN_SETBRIDGECONNECT

/* HNAP method function prototypes */
extern void HDK_Method_PN_AddPortMapping(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput);
extern void HDK_Method_PN_DeletePortMapping(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput);
extern void HDK_Method_PN_GetConnectedDevices(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput);
extern void HDK_Method_PN_GetDeviceSettings(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput);
extern void HDK_Method_PN_GetPortMappings(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput);
extern void HDK_Method_PN_GetRouterLanSettings2(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput);
extern void HDK_Method_PN_GetRouterSettings(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput);
extern void HDK_Method_PN_GetWLanRadioSecurity(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput);
extern void HDK_Method_PN_GetWLanRadioSettings(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput);
extern void HDK_Method_PN_GetWLanRadios(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput);
extern void HDK_Method_PN_GetWanSettings(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput);
extern void HDK_Method_PN_IsDeviceReady(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput);
extern void HDK_Method_PN_Reboot(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput);
extern void HDK_Method_PN_SetBridgeConnect(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput);
extern void HDK_Method_PN_SetDeviceSettings(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput);
extern void HDK_Method_PN_SetDeviceSettings2(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput);
extern void HDK_Method_PN_SetRouterLanSettings2(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput);
extern void HDK_Method_PN_SetRouterSettings(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput);
extern void HDK_Method_PN_SetWLanRadioSecurity(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput);
extern void HDK_Method_PN_SetWLanRadioSettings(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput);
extern void HDK_Method_PN_SetBridgeConnect(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput);

#endif /* #ifndef __HDK_METHODS_H__ */
