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
 * Copyright (c) 2008-2009 Cisco Systems, Inc. All rights reserved.
 *
 * Cisco Systems, Inc. retains all right, title and interest (including all
 * intellectual property rights) in and to this computer program, which is
 * protected by applicable intellectual property laws.  Unless you have obtained
 * a separate written license from Cisco Systems, Inc., you are not authorized
 * to utilize all or a part of this computer program for any purpose (including
 * reproduction, distribution, modification, and compilation into object code),
 * and you must immediately destroy or return to Cisco Systems, Inc. all copies
 * of this computer program.  If you are licensed by Cisco Systems, Inc., your
 * rights to utilize this computer program are limited by the terms of that
 * license.  To obtain a license, please contact Cisco Systems, Inc.
 *
 * This computer program contains trade secrets owned by Cisco Systems, Inc.
 * and, unless unauthorized by Cisco Systems, Inc. in writing, you agree to
 * maintain the confidentiality of this computer program and related information
 * and to not disclose this computer program and related information to any
 * other person or entity.
 *
 * THIS COMPUTER PROGRAM IS PROVIDED AS IS WITHOUT ANY WARRANTIES, AND CISCO
 * SYSTEMS, INC. EXPRESSLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED,
 * INCLUDING THE WARRANTIES OF MERCHANTIBILITY, FITNESS FOR A PARTICULAR
 * PURPOSE, TITLE, AND NONINFRINGEMENT.
 */

/*
 * hdk_methods.h - HNAP callback function prototypes
 */

#ifndef __HDK_METHODS_H__
#define __HDK_METHODS_H__

#include "hdk_data.h"

/* HNAP method function pointer type */
typedef void (*HDK_MethodFn)(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput);

/* HNAP method function defines */
#define HDK_METHOD_PN_ADDPORTMAPPING
#define HDK_METHOD_PN_DELETEPORTMAPPING
#define HDK_METHOD_PN_GETCONNECTEDDEVICES
#define HDK_METHOD_PN_GETDEVICESETTINGS
#define HDK_METHOD_PN_GETPORTMAPPINGS
#define HDK_METHOD_PN_GETROUTERLANSETTINGS2
#define HDK_METHOD_PN_GETROUTERSETTINGS
#define HDK_METHOD_PN_GETWLANRADIOSECURITY
#define HDK_METHOD_PN_GETWLANRADIOSETTINGS
#define HDK_METHOD_PN_GETWLANRADIOS
#define HDK_METHOD_PN_GETWANSETTINGS
#define HDK_METHOD_PN_ISDEVICEREADY
#define HDK_METHOD_PN_REBOOT
#define HDK_METHOD_PN_SETBRIDGECONNECT
#define HDK_METHOD_PN_SETDEVICESETTINGS
#define HDK_METHOD_PN_SETDEVICESETTINGS2
#define HDK_METHOD_PN_SETROUTERLANSETTINGS2
#define HDK_METHOD_PN_SETROUTERSETTINGS
#define HDK_METHOD_PN_SETWLANRADIOSECURITY
#define HDK_METHOD_PN_SETWLANRADIOSETTINGS
#define HDK_METHOD_PN_SETBRIDGECONNECT

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
