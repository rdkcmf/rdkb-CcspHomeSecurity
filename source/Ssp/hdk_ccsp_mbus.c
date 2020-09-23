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
 * hdk_ccsp_mbus.c
 *
 *   CCSP message bus APIs, which are used to access Data Model.
 *
 * leichen2@cisco.com
 * 2011.11.04 - Initialize
 */
#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include <strings.h>
#include "chs_log.h"

#include "hdk_ccsp_mbus.h"
#include "ccsp_base_api.h"
#include "ccsp_message_bus.h"
#include "ansc_platform.h"

enum {
    MBUS_GETPARAM,
    MBUS_SETPARAM,
    MBUS_COMMIT,
    MBUS_ADDOBJ,
    MBUS_DELOBJ,
};

struct MBusObj_s {
    /* configuration */
    char    *subSystem;     /* sub-system: eRT, eMG, eGW ... */
    char    *confFile;      /* config file for ccsp message bus */
    char    *compId;        /* component ID of this module */
    char    *compCrId;      /* component ID of CR */

    /* internal use */
    void    *handle;
};

static int 
TypeTrans(MBusParamType_t iType, enum dataType_e *dType)
{
    UINT i;
    struct {
        MBusParamType_t iType;
        enum dataType_e dType;
    } map[] = {
        {MBUS_PT_STRING,    ccsp_string},
        {MBUS_PT_INT,       ccsp_int},
        {MBUS_PT_UINT,      ccsp_unsignedInt},
        {MBUS_PT_BOOL,      ccsp_boolean},
        {MBUS_PT_DATETIME,  ccsp_dateTime},
        {MBUS_PT_BASE64,    ccsp_base64},
        {MBUS_PT_LONG,      ccsp_long},
        {MBUS_PT_ULONG,     ccsp_unsignedLong},
        {MBUS_PT_FLOAT,     ccsp_float},
        {MBUS_PT_DOUBLE,    ccsp_double},
        {MBUS_PT_BYTE,      ccsp_byte},
        {MBUS_PT_UNSPEC,    ccsp_none},
    };

    for (i = 0; i < NELEMS(map); i++)
    {
        if (map[i].iType == iType)
        {
            *dType = map[i].dType;
            return 0;
        }
    }

    return -1;
}

static int 
MBus_Process(MBusObj_t *mbus, int cmd, const char *path, 
        MBusParamType_t type, char *val, int size, int commit, int *ins)
{
    componentStruct_t       **compStructs = NULL;
    int                     compNum = 0;
    parameterValStruct_t    **valStructs = NULL;
    int                     valNum = 0;
    char                    *peerCompId, *peerDbusPath;
    char                    *paramNames[1];
    parameterValStruct_t    setStruct[1];
    char                    *faultParam = NULL;
    int                     insIdx;
    char                    tmpPath[MAX_PATH_NAME];

    if (!mbus || !path)
    {
        log_printf(LOG_ERR, "Bad parameters\n");
        return -1;
    }
	
    if (CcspBaseIf_discComponentSupportingNamespace(mbus->handle, mbus->compCrId, 
                path, mbus->subSystem, &compStructs, &compNum) != CCSP_SUCCESS)
    {
        log_printf(LOG_ERR, "fail to find component for %s", path);
        return -1;
    }
	
    /*
     * we just assume the "path" used here
     * won't belongs to more than one component.
     * The path like "Device." should not be used.
     */
	
    if (compNum != 1)
    {
		log_printf(LOG_ERR, "no component found (or too many)");
        free_componentStruct_t(mbus->handle, compNum, compStructs);
        return -1;
    }

    peerCompId = compStructs[0]->componentName;
    peerDbusPath = compStructs[0]->dbusPath;

    switch (cmd)
    {
        case MBUS_GETPARAM:
            if (!val || size < 0)
            {
                log_printf(LOG_ERR, "Bad parameters");
                free_componentStruct_t(mbus->handle, compNum, compStructs);
                return -1;
            }

            paramNames[0] = (char *)path; /* have to trans */

            if (CcspBaseIf_getParameterValues(mbus->handle, peerCompId, peerDbusPath,
                        paramNames, 1, &valNum, &valStructs) != CCSP_SUCCESS)
            {
                log_printf(LOG_ERR, "fail to get values\n");
                free_componentStruct_t(mbus->handle, compNum, compStructs);
                return -1;
            }

            /* the full path of parameter should be precise enough, 
             * so that we won't got mult-results */
            if (valNum != 1 || strcmp(path, valStructs[0]->parameterName) != 0)
            {
                log_printf(LOG_ERR, "no name (or too many) match");
                free_parameterValStruct_t(mbus->handle, valNum, valStructs);
                free_componentStruct_t(mbus->handle, compNum, compStructs);
                return -1;
            }

            snprintf(val, size, "%s", valStructs[0]->parameterValue);
            free_parameterValStruct_t(mbus->handle, valNum, valStructs);
            break;

        case MBUS_SETPARAM:
            if (type == MBUS_PT_UNSPEC || !val)
            {
                log_printf(LOG_ERR, "bad parameters");
                free_componentStruct_t(mbus->handle, compNum, compStructs);
                return -1;
            }

            setStruct[0].parameterName = (char *)path; /* have to trans */
            setStruct[0].parameterValue = val;
            if (TypeTrans(type, &setStruct[0].type) != 0)
            {
                log_printf(LOG_ERR, "invalid type");
                free_componentStruct_t(mbus->handle, compNum, compStructs);
                return -1;
            }

            commit = (commit == 0 ? 0 : 1);
            if (CcspBaseIf_setParameterValues(mbus->handle, peerCompId, peerDbusPath,
                        0, 0x0, setStruct, 1, commit, &faultParam) != CCSP_SUCCESS)
            {
                log_printf(LOG_ERR, "fail to set value");
                if (faultParam)
                    free(faultParam);
                free_componentStruct_t(mbus->handle, compNum, compStructs);
                return -1;
            }

            if (faultParam)
                free(faultParam);
            break;

        case MBUS_COMMIT:
            if (CcspBaseIf_setCommit(mbus->handle, peerCompId, peerDbusPath, 
                        0, 0, 1) != CCSP_SUCCESS)
            {
				log_printf(LOG_ERR, "fail to commit");
                free_componentStruct_t(mbus->handle, compNum, compStructs);
                return -1;
            }
            break;

        case MBUS_ADDOBJ:
            if (CcspBaseIf_AddTblRow(mbus->handle, peerCompId, peerDbusPath,
                        0, (char *)path, &insIdx) != CCSP_SUCCESS)
            {
                log_printf(LOG_ERR, "fail to add object");
                free_componentStruct_t(mbus->handle, compNum, compStructs);
                return -1;
            }

            if (ins)
                *ins = insIdx;
            break;

        case MBUS_DELOBJ:
            if (!ins)
            {
				log_printf(LOG_ERR, "bad parameters");
                free_componentStruct_t(mbus->handle, compNum, compStructs);
                return -1;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s%d.", path, *ins);
            if (CcspBaseIf_DeleteTblRow(mbus->handle, peerCompId, peerDbusPath,
                        0, tmpPath) != CCSP_SUCCESS)
            {
                log_printf(LOG_ERR, "fail to delete object");
                free_componentStruct_t(mbus->handle, compNum, compStructs);
                return -1;
            }
            break;

        default:
            log_printf(LOG_ERR, "unknown command");
            free_componentStruct_t(mbus->handle, compNum, compStructs);
            return -1;
    }

    free_componentStruct_t(mbus->handle, compNum, compStructs);
    return 0;
}

MBusObj_t *
MBus_Create(const char *subSystem, const char *confFile, 
        const char *compId, const char *compCrId)
{
    MBusObj_t *mbus;

    if (!subSystem || !confFile || !compId || !compCrId)
        return NULL;
    
    if ((mbus = malloc(sizeof(MBusObj_t))) == NULL)
        return NULL;

    bzero(mbus, sizeof(MBusObj_t));

    mbus->subSystem     = strdup(subSystem);
    mbus->confFile      = strdup(confFile);
    mbus->compId        = strdup(compId);
    mbus->compCrId      = strdup(compCrId);
    if (!mbus->subSystem || !mbus->confFile 
            || !mbus->compId || !mbus->compCrId)
        goto errout;

    /* internal variables */
    if (CCSP_Message_Bus_Init(mbus->compId, mbus->confFile,
                &mbus->handle, malloc, free) != 0)
        goto errout;

    return mbus;

errout:
    /* free the resources allocated */
    if (mbus->subSystem)
        free(mbus->subSystem);
    if (mbus->confFile)
        free(mbus->confFile);
    if (mbus->compId)
        free(mbus->compId);
    if (mbus->compCrId)
        free(mbus->compCrId);

    free(mbus);
    return NULL;
}

int 
MBus_Destroy(MBusObj_t *mbus)
{
    if (!mbus)
        return -1;

    if (mbus->handle)
    {
        log_printf(LOG_ERR, "CCSP_Message_Bus_Exit: Before");
        //CCSP_Message_Bus_Exit(mbus->handle);
        //syslog(LOG_ERR, "%s: CCSP_Message_Bus_Exit: After", __FUNCTION__);
    }

    if (mbus->subSystem)
        free(mbus->subSystem);
    if (mbus->confFile)
        free(mbus->confFile);
    if (mbus->compId)
        free(mbus->compId);
    if (mbus->compCrId)
        free(mbus->compCrId);

    free(mbus);
    return 0;
}

int 
MBus_GetParamVal(MBusObj_t *mbus, const char *path, char *val, int size)
{
    /* @type and @commit are no sense of MBUS_GETPARAM */
    return MBus_Process(mbus, MBUS_GETPARAM, path, MBUS_PT_UNSPEC, val, size, 0, NULL);
}

int 
MBus_SetParamVal(MBusObj_t *mbus, const char *path, 
        MBusParamType_t type, const char *val, int commit)
{
    /* @size is no sense of MBUS_SETPARAM, the val is '\0' terminated  */
    return MBus_Process(mbus, MBUS_SETPARAM, path, type, 
            (char *)val /* have to trans */, 0, commit, NULL);
}

int
MBus_SetParamVect(MBusObj_t *mbus, const char *object, const MBusParam_t params[], int num, int commit)
{
    componentStruct_t       **compStructs = NULL;
    int                     compNum = 0;
    char                    *peerCompId, *peerDbusPath;
    parameterValStruct_t    setStruct[20] = {{0}};
    UINT                     i;
    char                    path[MAX_PATH_NAME];
    char                    *faultParam = NULL;
    int                     ret;

    if (!mbus || !object || !params)
    {
        log_printf(LOG_ERR, "bad parameters");
        return -1;
    }
    
    if (CcspBaseIf_discComponentSupportingNamespace(mbus->handle, mbus->compCrId, 
                object, mbus->subSystem, &compStructs, &compNum) != CCSP_SUCCESS)
    {
        log_printf(LOG_ERR, "can't find: %s", object);
        return -1;
    }

    if (compNum != 1)
    {
        log_printf(LOG_ERR, "no component found (or too many): %s", object);
        free_componentStruct_t(mbus->handle, compNum, compStructs);
        return -1;
    }

    peerCompId = compStructs[0]->componentName;
    peerDbusPath = compStructs[0]->dbusPath;

    for (i = 0; i < (UINT)num && i < NELEMS(setStruct); i++)
    {
        snprintf(path, sizeof(path), "%s%s", object, params[i].name);
        setStruct[i].parameterName = strdup(path);
        setStruct[i].parameterValue = strdup(params[i].value);
        if (TypeTrans(params[i].type, &setStruct[i].type) != 0)
        {
            log_printf(LOG_ERR, "invalid type %s", params[i].name);
            continue;
        }
    }

    ret = CcspBaseIf_setParameterValues(mbus->handle, peerCompId, peerDbusPath, 
                0, 0x0, setStruct, i, commit, &faultParam);

    if (ret != CCSP_SUCCESS)
        log_printf(LOG_ERR, "Fail to set: %d", ret);

    if (faultParam)
        free(faultParam);

    for (i = 0; i < NELEMS(setStruct); i++)
    {
        free(setStruct[i].parameterName);
        free(setStruct[i].parameterValue);
    }

    free_componentStruct_t(mbus->handle, compNum, compStructs);
    return ret == CCSP_SUCCESS ? 0 : -1;
}

int
MBus_GetParamVect(MBusObj_t *mbus, const char *object, MBusParam_t *params[], int *num)
{
    UNREFERENCED_PARAMETER(mbus);
    UNREFERENCED_PARAMETER(object);
    UNREFERENCED_PARAMETER(params);
    UNREFERENCED_PARAMETER(num);
    return -1;
}

int 
MBus_Commit(MBusObj_t *mbus, const char *path)
{
    /* @type, @val, @size, @commit are no sense of MBUS_COMMIT  */
    return MBus_Process(mbus, MBUS_COMMIT, path, MBUS_PT_UNSPEC, 0, 0, 1, NULL);
}

int
MBus_AddObjectIns(MBusObj_t *mbus, const char *objPath, int *ins)
{
    return MBus_Process(mbus, MBUS_ADDOBJ, objPath, MBUS_PT_UNSPEC, 0, 0, 0, ins);
}

int 
MBus_DelObjectIns(MBusObj_t *mbus, const char *objPath, int ins)
{
    return MBus_Process(mbus, MBUS_DELOBJ, objPath, MBUS_PT_UNSPEC, 0, 0, 0, &ins);
}

int 
MBus_FindObjectIns(MBusObj_t *mbus, const char *pref, const char *paramName, 
        const char *paramVal, char insPath[][MAX_PATH_NAME], int *insNum)
{
    componentStruct_t       **compStructs = NULL;
    int                     compNum = 0;
    parameterInfoStruct_t   **infoStructs = NULL;
    int                     infoNum = 0;
    parameterValStruct_t    **valStructs = NULL;
    int                     valNum = 0;
    char                    *peerCompId, *peerDbusPath;
    int                     i, offset, tmp;
    char                    *paramNameList[1];
    char                    tmpPath[MAX_PATH_NAME];
    
    if (!mbus || !pref || !insPath || !insNum)
    {
        log_printf(LOG_ERR, "bad parameters");
        return -1;
    }
	
    if (CcspBaseIf_discComponentSupportingNamespace(mbus->handle, mbus->compCrId, 
                pref, mbus->subSystem, &compStructs, &compNum) != CCSP_SUCCESS)
    {
        log_printf(LOG_ERR, "fail to find component: %s", pref);
        return -1;
    }

    if (compNum != 1)
    {
        log_printf(LOG_ERR, "no component found (or too many): %s", pref);
        free_componentStruct_t(mbus->handle, compNum, compStructs);
        return -1;
    }
	
    peerCompId = compStructs[0]->componentName;
    peerDbusPath = compStructs[0]->dbusPath;


    if (CcspBaseIf_getParameterNames(mbus->handle, peerCompId, peerDbusPath,
            (char *)pref, TRUE, &infoNum, &infoStructs) != CCSP_SUCCESS)
    {
        log_printf(LOG_ERR, "get instance list error: %s", pref);
        free_componentStruct_t(mbus->handle, compNum, compStructs);
        return -1;
    }

    offset = 0;
    for (i = 0; i < infoNum && offset < *insNum; i++)
    {
        /*
         * skip the path which is not an instance.
         * A object prefix is like "IP.Device.Interface."
         * and a instance is like "IP.Device.Interface.1."
         * so the format of instance path must be "<prefix>.%d."
         */
        if (strncmp(infoStructs[i]->parameterName, pref, strlen(pref)) != 0
                || sscanf(infoStructs[i]->parameterName + strlen(pref), "%d.", &tmp) != 1)
        {
            log_printf(LOG_WARNING, "The path %s is not a instance", infoStructs[i]->parameterName);
            continue;
        }

        /*
         * if @paramName, or both @paramName and @paramVal are provided, 
         * they should also match.
         */ 
        if (paramName)
        {
            snprintf(tmpPath, MAX_PATH_NAME, "%s%s", infoStructs[i]->parameterName, paramName);
            paramNameList[0] = tmpPath;
            if (CcspBaseIf_getParameterValues(mbus->handle, peerCompId, peerDbusPath,
                        paramNameList, 1, &valNum, &valStructs) != CCSP_SUCCESS)
                continue;

            if (valNum != 1 || strcmp(valStructs[0]->parameterName, tmpPath) != 0
                    || (paramVal && (strcmp(valStructs[0]->parameterValue, paramVal) != 0)))
            {
                free_parameterValStruct_t(mbus->handle, valNum, valStructs);
                continue;
            }

            free_parameterValStruct_t(mbus->handle, valNum, valStructs);
        }

        snprintf(insPath[offset], MAX_PATH_NAME, "%s", infoStructs[i]->parameterName);
        offset++;
    }

    *insNum = offset;

    free_parameterInfoStruct_t(mbus->handle, infoNum, infoStructs);
    free_componentStruct_t(mbus->handle, compNum, compStructs);
    return 0;
}

int FindFirstInstance(MBusObj_t *mbus, const char *object, char *ins, int size)
{
    int insNum;
    char insPath[20][MAX_PATH_NAME];

    insNum = NELEMS(insPath);
    if (MBus_FindObjectIns(mbus, object, NULL, NULL, insPath, &insNum) != 0)
        return 0;

    if (insNum <= 0)
        return 0;

    snprintf(ins, size, "%s", insPath[0]);
    return 1;
}

/* @ins: 0 for first instance, else as the instance number for specific instance */
int GetParamValueForIns(MBusObj_t *mbus, const char *object, const char *param,
        int ins, char *value, int size)
{
    char insPath[MAX_PATH_NAME];
    char paramPath[MAX_PATH_NAME];

    if (ins == 0)
    {
        if (!FindFirstInstance(mbus, object, insPath, sizeof(insPath)))
            return 0;
    }
    else
    {
        snprintf(insPath, sizeof(insPath), "%s%d.", object, ins);
    }

    snprintf(paramPath, sizeof(paramPath), "%s%s", insPath, param);
    if (MBus_GetParamVal(mbus, paramPath, value, size) != 0)
        return 0;

    return 1;
}


