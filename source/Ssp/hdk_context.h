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
 * hdk_context.h - Sample device context
 */

#ifndef __HDK_CONTEXT_H__
#define __HDK_CONTEXT_H__

#include <stdio.h>
#include "hdk_ccsp_mbus.h"

#ifdef HDK_EMULATOR
#include "emulator.h"
typedef EmulatorContext HDK_Context;
#else
typedef struct _HDK_Context
{
    FILE* fhRequest;
    FILE* fhResponse;
    int fReboot;
	MBusObj_t *mbus;
} HDK_Context;
#endif

extern MBusObj_t * mbus_init();
/*
 * HDK_Context_Init - Initialize the device context
 *
 * Return: Non-zero for success, zero otherwise
 */
extern int HDK_Context_Init(void** ppCtx, FILE* pfhRead, FILE* pfhWrite);

/*
 * HNAP_Context_Free - Free the device context
 */
extern void HDK_Context_Free(void* pCtx, int fCommit);

extern void homesecurity_timeout_handler();
/*
 * HDK_Context_Authenticate - Authenticate the request
 */
extern int HDK_Context_Authenticate(void* pCtx, char* pszUsername, char* pszPassword);

#endif /* __HDK_CONTEXT_H__ */
