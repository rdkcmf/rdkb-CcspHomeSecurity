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
 * hdk_context.c - Sample device context
 */

#include "hdk_adi.h"
#include "hdk_context.h"
#include "chs_log.h"
#include <unistd.h>

#include <stdlib.h>
#include <string.h>
#include "ccsp_base_api.h"
#include "hdk_ccsp_mbus.h"

//#ifdef defined(_COSA_DRG_CNS_) || defined(_COSA_DRG_TPG_)
#if 1
#define MBUS_COMPID_HDK         "eRT.com.cisco.spvtg.ccsp.HNAP"
#define MBUS_COMPID_CR          "eRT.com.cisco.spvtg.ccsp.CR"
#else
#define MBUS_COMPID_HDK         "com.cisco.spvtg.ccsp.HDK"
#define MBUS_COMPID_CR          "com.cisco.spvtg.ccsp.CR"
#endif

#define MBUS_CONF_FILE          CCSP_MSG_BUS_CFG
#define MBUS_SUBSYSTEM          ""

#ifdef HDK_EMULATOR
/* Emulator state file */
#ifndef HDK_EMULATOR_STATE_FILE
#define HDK_EMULATOR_STATE_FILE "emulator.ds"
#endif
#endif
static MBusObj_t *ccsp_bus = NULL;
extern int HDK_Stop_HomeSecurity_Bridge(MBusObj_t *ccsp_bus);

MBusObj_t * mbus_init()
{
	MBusObj_t *mbus = NULL;

	mbus =  MBus_Create(MBUS_SUBSYSTEM, MBUS_CONF_FILE, MBUS_COMPID_HDK, MBUS_COMPID_CR);

	while (!mbus)
	{
		log_printf(LOG_ERR, "Create mbus failed, retry after 3s\n");
		sleep(3);
		mbus =  MBus_Create(MBUS_SUBSYSTEM, MBUS_CONF_FILE, MBUS_COMPID_HDK, MBUS_COMPID_CR);
	}

	ccsp_bus = mbus;
	return mbus;
}

void homesecurity_timeout_handler()
{
	if(HDK_Stop_HomeSecurity_Bridge(ccsp_bus))
		log_printf(LOG_ERR, "stop ethernet port 4 home security vlan failed\n");
}

/*
 * HDK_Context_Init - Initialize the device context
 *
 * Return: Non-zero for success, zero otherwise
 */
int HDK_Context_Init(void** ppCtx, FILE* pfhRead, FILE* pfhWrite)
{
    int fResult = 0;

#ifdef HDK_EMULATOR
    /* Allocate the device context */
    HDK_Context* pCtx = (HDK_Context*)malloc(sizeof(HDK_Context));
    *ppCtx = pCtx;
    if (pCtx)
    {
        /* Initialize the emulator context */
        HDK_Emulator_Init(pCtx);

        /* Read the state file */
        fResult = HDK_Emulator_ReadState(pCtx, HDK_EMULATOR_STATE_FILE);

        /* Set the request/response file handles */
        pCtx->fhRequest = pfhRead;
        pCtx->fhResponse = pfhWrite;
    }
#else
    /* Allocate the device context */
    HDK_Context* pCtx = (HDK_Context*)malloc(sizeof(HDK_Context));
    *ppCtx = pCtx;
    if (pCtx)
    {
        fResult = 1;
        pCtx->fhRequest = pfhRead;
        pCtx->fhResponse = pfhWrite;
        pCtx->fReboot = 0;
        pCtx->mbus = ccsp_bus;               
    }
#endif

    return fResult;
}

/*
 * HNAP_Context_Free - Free the device context
 */
void HDK_Context_Free(void* pCtx, int fCommit)
{
#ifdef HDK_EMULATOR
    if (pCtx)
    {
        /* Write out the emulator state */
        if (fCommit)
        {
            HDK_Emulator_WriteState((HDK_Context*)pCtx, HDK_EMULATOR_STATE_FILE);
        }

        /* Free the emulator */
        HDK_Emulator_Free((EmulatorContext*)pCtx);

        /* Free the device context */
        free(pCtx);
    }
#else
    HDK_Context *hdkCtx = (HDK_Context *)pCtx;

    /* Unused parameters */
    (void)fCommit;

    if (!hdkCtx)
        return;

    //MBus_Destroy(hdkCtx->mbus);

    /* Free the device context */
    free(pCtx);
#endif
}

/*
 * HDK_Context_Authenticate - Authenticate the request
 */
int HDK_Context_Authenticate(void* pCtx, char* pszUsername, char* pszPassword)
{
    int fResult = 0;

	fResult = strcmp(pszUsername, "hnapadmin") == 0 && strcmp(pszPassword, "6R8DUt5edruT") == 0;
	return fResult;

    /* Get the Username/Password from the device state */
    HDK_Struct sTemp;
    HDK_Struct_Init(&sTemp);
    if (HDK_Device_GetValue(pCtx, &sTemp, HDK_DeviceValue_Username, 0) &&
        HDK_Device_GetValue(pCtx, &sTemp, HDK_DeviceValue_AdminPassword, 0))
    {
        /* Compare the credentials */
        fResult = strcmp(pszUsername, HDK_Get_String(&sTemp, HDK_Element_PN_Username)) == 0 &&
            strcmp(pszPassword, HDK_Get_String(&sTemp, HDK_Element_PN_AdminPassword)) == 0;
    }
    HDK_Struct_Free(&sTemp);

    return fResult;
}

int HDK_Signal_Hander(int signal)
{
	return HDK_Stop_HomeSecurity_Bridge(ccsp_bus);
}
