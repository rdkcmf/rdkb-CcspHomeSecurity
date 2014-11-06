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
