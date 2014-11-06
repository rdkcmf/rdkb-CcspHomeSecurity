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

#ifndef __HDK_H__
#define __HDK_H__

#include "hdk_data.h"

#define HOMESECURITY_IP_PREFIX "172.16.12."
#define HOMESECURITY_NAME_PREFIX "xhs_hnap:"

#define bool char
#define true 1
#define TRUE 1
#define false 0
#define FALSE 0

#define MAX_PARAMS 128
#define MAX_INSTANCE 64
#define MAX_BUF      256
#define MAX_DEVNAME_LEN 64
#define MAX_PASSWD_LEN 64
/*
 * Is this an HNAP request?
 *
 * Return: Non-zero if this is an HNAP request, 0 otherwise.
 */
extern int HDK_IsHNAPRequest(char* pszPath);

/*
 * Does this HNAP request require authentication?
 *
 * Return: Non-zero if this HNAP request requires authentication, 0 otherwise.
 */
extern int HDK_RequiresAuth(int fGet, char* pszSoapAction);

/*
 * Handle an HNAP request.
 */
extern HDK_Enum_Result HDK_HandleRequest(void* pDeviceCtx, int fGet, unsigned int cbContentLength, char* pszSoapAction);

#endif /* __HDK_H__ */
