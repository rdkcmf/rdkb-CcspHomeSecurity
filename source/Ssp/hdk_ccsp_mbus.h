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
 * hdk_ccsp_mbus.h
 *
 *   CCSP message bus APIs, which are used to access Data Model.
 *
 * leichen2@cisco.com
 * 2011.11.04 - Initialize
 */
#ifndef __HDK_CCSP_MBUS_H__
#define __HDK_CCSP_MBUS_H__

/* Max size of TR-181's object/parameter's path */
#define MAX_PATH_NAME       256

#ifndef NELEMS
#define NELEMS(arr)         (sizeof(arr) / sizeof((arr)[0]))
#endif

/* parameter types */
typedef enum MBusParamType_e {
    MBUS_PT_UNSPEC = 0,
    MBUS_PT_STRING,
    MBUS_PT_INT,
    MBUS_PT_UINT,
    MBUS_PT_BOOL,
    MBUS_PT_DATETIME,
    MBUS_PT_BASE64,
    MBUS_PT_LONG,
    MBUS_PT_ULONG,
    MBUS_PT_FLOAT,
    MBUS_PT_DOUBLE,
    MBUS_PT_BYTE,
} MBusParamType_t;

typedef struct MBusParam_s {
    char *name;
    char *value;
    MBusParamType_t type;
} MBusParam_t;

/* MBusObj_s is an internal structure */
typedef struct MBusObj_s MBusObj_t;

/*
 * Description:
 *
 *   Create a mbus object to access TR-181 data modle.
 *
 * Parameters:
 *
 *   @subSystem [in]
 *     Sub-system ID, e.g., "eRT", "eMG", "eGW", or "" for default.
 *   @confFile [in]
 *     Config file of CCSP message bus.
 *   @compId [in]
 *     Component ID of the module attend to use mbus. It's should be
 *     a unique ID, e.g., "com.cisco.spvtg.ccsp.<SOMEID>"
 *   @compCrId [in]
 *     CR's component ID. CR is used to lookup the component of speicific path.
 *
 * Return:
 *
 *   mbus object pointer or NULL if error.
 */
MBusObj_t *
MBus_Create(const char *subSystem, const char *confFile, 
        const char *compId, const char *compCrId);

/*
 * Description:
 *
 *   Destory the mbus created by MBus_Create().
 *
 * Parameters:
 *
 *   @mbus [in]
 *     mbus object to destroy.
 *
 * Return:
 *
 *   0 if success and -1 on error.
 */
int 
MBus_Destroy(MBusObj_t *mbus);

/*
 * Description:
 *
 *   Get parameter's value.
 *
 * Parameters:
 *
 *   @mbus [in]
 *     mbus created by MBus_Create.
 *   @path [in]
 *     Full path the parameter. It must be an parameter, e.g., Device.IP.Interface.{1}.Name
 *     instead of an object (e.g. Device.IP.).
 *   @val [out]
 *     Buffer to save value.
 *   @size [in]
 *     Buffer size of @val.
 *
 * Return:
 *
 *   0 if success and -1 on error.
 */
int 
MBus_GetParamVal(MBusObj_t *mbus, const char *path, char *val, int size);

/*
 * Description:
 *
 *   Set parameter's value.
 *
 * Parameters:
 *
 *   @mbus [in]
 *     mbus created by MBus_Create.
 *   @path [in]
 *     Full path the parameter. It must be an parameter, e.g., Device.IP.Interface.{1}.Name
 *     instead of an object (e.g. Device.IP.).
 *   @type [in]
 *     The parameter's type, it should be one of MBUS_PT_XXX (except MBUS_PT_UNSPEC).
 *   @val [in]
 *     Buffer to save value.
 *   @commit [in]
 *     Commit after set or not. 1 means commit, 0 for not.
 *
 * Return:
 *
 *   0 if success and -1 on error.
 */
int 
MBus_SetParamVal(MBusObj_t *mbus, const char *path, 
        MBusParamType_t type, const char *val, int commit);

/*
 * Description:
 *
 *   Commit parameters set before.
 *
 * Parameters:
 *
 *   @mbus [in]
 *     mbus created by MBus_Create.
 *   @path [in]
 *     the parameter/object set before.
 *
 * Return:
 *
 *   0 if success and -1 on error.
 */
int 
MBus_Commit(MBusObj_t *mbus, const char *path);

/*
 * Description:
 *
 *   Lookup the path of object's instance(s), by object prefix.
 *   A object path (prefix) is like "IP.Device.Interface."
 *   and a instance is like "IP.Device.Interface.1."
 *   so the format of instance path must be "<prefix>.%d." .
 *
 * Parameters:
 *
 *   @mbus [in]
 *     mbus created by MBus_Create.
 *   @objPath [in]
 *     Object path (prefix), see the defination above.
 *   @paramName [in] <optional>
 *     If exit, the parameter must be exist for the instance.
 *   @paramVal [in] <optional>
 *     If exit and @paramName is exist, the value must match.
 *   @insPath [out]
 *     Path names of the instance(s) found.
 *   @insNum [in-out]
 *     As an input, it's the max size of the @insPath[][MAX_PATH_NAME],
 *     as an output, it's the number of instance(s) stored in @insPath.
 *
 * Return:
 *
 *   0 if success and -1 on error.
 */
int 
MBus_FindObjectIns(MBusObj_t *mbus, const char *objPath, const char *paramName, 
        const char *paramVal, char insPath[][MAX_PATH_NAME], int *insNum);

/*
 * Description:
 *
 *   Add an instance of an object
 *
 * Parameters:
 *
 *   @mbus [in]
 *     mbus created by MBus_Create.
 *   @objPath [in]
 *     the object's name (instance's prefix).
 *   @ins [out] (optional)
 *     instance number of new instance.
 *
 * Return:
 *
 *   0 if success and -1 on error.
 */
int
MBus_AddObjectIns(MBusObj_t *mbus, const char *objPath, int *ins);

/*
 * Description:
 *
 *   Delete an instance of an object
 *
 * Parameters:
 *
 *   @mbus [in]
 *     mbus created by MBus_Create.
 *   @objPath [in]
 *     the instance's path
 *   @ins [in]
 *     instance number of object
 *
 * Return:
 *
 *   0 if success and -1 on error.
 */
int 
MBus_DelObjectIns(MBusObj_t *mbus, const char *objPath, int ins);

int 
FindFirstInstance(MBusObj_t *mbus, const char *object, char *ins, int size);

/* @ins: 0 for first instance, else as the instance number for specific instance */
int 
GetParamValueForIns(MBusObj_t *mbus, const char *object, const char *param,
        int ins, char *value, int size);

int
MBus_SetParamVect(MBusObj_t *mbus, const char *object, 
        const MBusParam_t params[], int num, int commit);

#endif /* __HDK_CCSP_MBUS_H__ */
