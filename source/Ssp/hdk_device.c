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
 * hdk_device.c - Sample device glue
 */

#include "hdk_adi.h"
#include "hdk_context.h"
#include "hdk_interface.h"
#include "hdk.h"
#include "hdk_ccsp_mbus.h"
#include "syscfg/syscfg.h"
#include "chs_log.h"
#include <syslog.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/reboot.h>
#include <linux/reboot.h>
#include <sys/types.h>
#include <ctype.h>
#include <sys/time.h>

#define  STR_LEN	128

static void LoadMBusParam(MBusParam_t params[], int *index, 
        char *name, char *value, MBusParamType_t type)
{
    if (*index >= MAX_PARAMS)
        return;
    params[*index].name = name;
    params[*index].value = value;
    params[*index].type = type;
    (*index)++;
}

#if 0
static bool stringtoip(char *s, HDK_IPAddress *ip)
{
	
	char *p1, *p2;
	
	if(s == NULL || ip == NULL)
	{
		log_printf(LOG_ERR, "error input\n");
		return false;
	}

	p1 = strchr(s, '.');
	if(p1 == NULL)
	{
		log_printf(LOG_ERR, "wrong ip adress string\n");
		return false;
	}
	*p1++ = '\0';
	ip->a = atoi(s);

	p2 = strchr(p1, '.');
	if(p2 == NULL)
	{
		log_printf(LOG_ERR, "wrong ip adress string\n");
		return false;
	}
	*p2++ = '\0';
	ip->b = atoi(p1);

	p1 = strchr(p2, '.');
	if(p1 == NULL)
	{
		log_printf(LOG_ERR, "wrong ip adress string\n");
		return false;
	}
	*p1++ = '\0';
	ip->c = atoi(p2);
	ip->d = atoi(p1);
	log_printf(LOG_INFO, "string %s, ip address %d.%d.%d.%d", s, ip->a, ip->b, ip->c, ip->d);
	return true;
}
#endif

/*
 * HDK Core glue (hdk_interface.h)
 */

int HDK_Device_Read(void* pDeviceCtx, char* pBuf, int cbBuf)
{
    return fread(pBuf, sizeof(char), cbBuf, ((HDK_Context*)pDeviceCtx)->fhRequest);
}

void HDK_Device_PrepareWrite(void* pDeviceCtx, enum _HDK_Enum_Result* pResult)
{
    /* Return REBOOT if necessary */
    if (HDK_SUCCEEDED(*pResult) && ((HDK_Context*)pDeviceCtx)->fReboot)
    {
        *pResult = HDK_Enum_Result_REBOOT;
    }
}

void HDK_Device_Write(void* pDeviceCtx, char* pBuf, int cbBuf)
{
    int iRet;
    iRet = fwrite(pBuf, sizeof(char), cbBuf, ((HDK_Context*)pDeviceCtx)->fhResponse);
    (void)iRet;
}


#ifndef HDK_EMULATOR
/*
 * HDK ADI glue (hdk_adi.h)
 */

int HDK_Device_GetValue(void* pDeviceCtx, HDK_Struct* pStruct, HDK_DeviceValue eValue, HDK_Struct* pInput)
{
    HDK_Context *hdkCtx;
    MBusObj_t *mbus;
    HDK_Struct *pStr1, *pStr2, *pStr3, *pStr4, *pStr5;
    char insPath[MAX_INSTANCE][MAX_PATH_NAME];
    // extra length added to resolve format overflow
    char path[MAX_PATH_NAME+32];
    char tmpBuf[MAX_PATH_NAME];
    int insNum;
    // extra length added to resolve format truncation
    char tmpPath[MAX_PATH_NAME+44];
    char val[MAX_BUF];
    char pathVal[MAX_BUF];
    char *ptr, *saveptr;
    struct timeval tv;
    int i, j, idx;
    HDK_MACAddress macAddr;
    HDK_IPAddress ipAddr;
    int boolVal;
    char *strVal;
    int start, end;

    if ((hdkCtx = (HDK_Context *)pDeviceCtx) == NULL
            || (mbus = hdkCtx->mbus) == NULL)
    {
        return 0;
    }

    pStr1 = pStr2 = NULL;

    switch (eValue)
    {
        case HDK_DeviceValue_ManageRemote:
             //get Device.UserInterface.X_CISCO_COM_RemoteAccess.HttpEnable
             if (MBus_GetParamVal(mbus, "Device.UserInterface.X_CISCO_COM_RemoteAccess.HttpEnable", val, sizeof(val)) != 0)
                return 0;
             
             if (!strcmp(val, "true"))
             {              
                HDK_Set_Bool(pStruct, HDK_Element_PN_ManageRemote, true);
                //Get remote http port
                if (MBus_GetParamVal(mbus, "Device.UserInterface.X_CISCO_COM_RemoteAccess.HttpPort", val, sizeof(val)) != 0)
                    return 0;
                HDK_Set_Int(pStruct, HDK_Element_PN_RemotePort, atoi(val));
                //set ssl not allowed since http is avaible
                HDK_Set_Bool(pStruct, HDK_Element_PN_RemoteSSL, false);
             }
             else if (!strcmp(val, "false"))
             {
                //once http is not remotely allowed, try to find if https is avaible
                if (MBus_GetParamVal(mbus, "Device.UserInterface.X_CISCO_COM_RemoteAccess.HttpsEnable", val, sizeof(val)) != 0)
                    return 0;
                if (!strcmp(val, "true"))
                {
                    HDK_Set_Bool(pStruct, HDK_Element_PN_ManageRemote, true);
                    //Get remote https port
                    if (MBus_GetParamVal(mbus, "Device.UserInterface.X_CISCO_COM_RemoteAccess.HttpsPort", val, sizeof(val)) != 0)
                        return 0;
                    HDK_Set_Int(pStruct, HDK_Element_PN_RemotePort, atoi(val));
                    //set ssl  allowed
                    HDK_Set_Bool(pStruct, HDK_Element_PN_RemoteSSL, true);
                }
                else if (!strcmp(val, "false"))
                {
                    HDK_Set_Bool(pStruct, HDK_Element_PN_ManageRemote, false);
					HDK_Set_Bool(pStruct, HDK_Element_PN_RemoteSSL, false);
                    HDK_Set_Int(pStruct, HDK_Element_PN_RemotePort, 0);
                }
             }
             break;
             
        case HDK_DeviceValue_ManageWireless:
             //wireless management is always allowed        
             HDK_Set_Bool(pStruct, HDK_Element_PN_ManageWireless, true);
             break;     

        case HDK_DeviceValue_DomainName:

            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityDHCPv4ServerPool", pathVal, sizeof(pathVal)) != 0)
            {
                log_printf(LOG_ERR, "Can't get multilan Home server pool\n");
                //return -1;
				//zqiu: return 0 when error
				return 0;
            }
            snprintf(tmpPath, sizeof(tmpPath), "%s.DomainName", pathVal);

            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;
            HDK_Set_String(pStruct, HDK_Element_PN_DomainName, val);
            break;
        
        case HDK_DeviceValue_WiredQoS:
             //only find MoCA QoS from DM. 
             HDK_Set_Bool(pStruct, HDK_Element_PN_WiredQoS, false);
             break;
             
        case HDK_DeviceValue_WPSPin:
            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiAp", pathVal, sizeof(pathVal)) != 0)
            {
                log_printf(LOG_ERR, "Can't get multilan Home wifi access point\n");
                //return -1;
				//zqiu: return 0 when error
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.WPS.X_CISCO_COM_Pin", pathVal);

            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;
            HDK_Set_String(pStruct, HDK_Element_PN_WPSPin, val);
            break;

            
        case HDK_DeviceValue_PortMappings:
             insNum = NELEMS(insPath);
                        
             if (MBus_FindObjectIns(mbus, "Device.NAT.PortMapping.", NULL, NULL, insPath, &insNum) != 0)
			 {
				log_printf(LOG_ERR, "Get HS port forwarding failed\n");
				return 0;
			 }
			
			 if (!insNum)
			 {
				HDK_Set_Struct(pStruct, HDK_Element_PN_PortMappings);
				break;
			 }	 
				
			 pStr2 = HDK_Set_Struct(pStruct, HDK_Element_PN_PortMappings);

			 if (pStr2 == NULL)
				 return 0;
             for (i = 0, j = 0; i < insNum; i++)
             {
                strcpy(tmpBuf, insPath[i]);
                sprintf(path, "%sInternalPort", tmpBuf);
                if (MBus_GetParamVal(mbus, path, val, sizeof(val)) != 0)
                    return 0;

                //this rule is port forwarding not HS port forwarding
                if (!strcmp(val, "0"))
                    continue;

				sprintf(path, "%sInternalClient", tmpBuf);
                if (MBus_GetParamVal(mbus, path, val, sizeof(val)) != 0)
                    return 0;
				
				//printf("1111111111111111\n");
                //this rule is not for hnap 
                if (strncmp(val, HOMESECURITY_IP_PREFIX, strlen(HOMESECURITY_IP_PREFIX)))
                    continue; 
               
			    //Get port mapping protocol
                sprintf(path, "%sProtocol", tmpBuf);
                if (MBus_GetParamVal(mbus, path, val, sizeof(val)) != 0)
                    return 0;
	
				if(!strcmp(val, "BOTH"))
				{
					log_printf(LOG_ERR, "HNAP doesn't support TCP/UDP both mode\n");
					continue;
				}
				/*
                sprintf(path, "%sDescription", insPath[i]);
                if (MBus_GetParamVal(mbus, path, val, sizeof(val)) != 0)
                    return 0;

                //this rule is not for hnap 
                if (strncmp(val, HOMESECURITY_NAME_PREFIX, strlen(HOMESECURITY_NAME_PREFIX)))
                    continue; 
				*/
               
					
                if (j == 0)
				{
					pStr1 = HDK_Set_Struct(pStr2, HDK_Element_PN_PortMapping);
					j++;
				}
				else
				{
					pStr1 = HDK_Append_Struct(pStr2, HDK_Element_PN_PortMapping);
					j++;
				}

				if (pStr1 == NULL)
					return 0;
                
				//Get port mapping Description
				sprintf(path, "%sDescription", tmpBuf);
                if (MBus_GetParamVal(mbus, path, val, sizeof(val)) != 0)
                    return 0;

                HDK_Set_String(pStr1, HDK_Element_PN_PortMappingDescription, val);

                //Get port mapping internal client
                sprintf(path, "%sInternalClient", tmpBuf);
                if (MBus_GetParamVal(mbus, path, val, sizeof(val)) != 0)
                    return 0;
					
				if (sscanf(val, "%hhu.%hhu.%hhu.%hhu", &ipAddr.a, &ipAddr.b, &ipAddr.c, &ipAddr.d) != 4)
                    return 0;
                HDK_Set_IPAddress(pStr1, HDK_Element_PN_InternalClient, &ipAddr);
                
				//Get port mapping protocol
                sprintf(path, "%sProtocol", tmpBuf);
                if (MBus_GetParamVal(mbus, path, val, sizeof(val)) != 0)
                    return 0;

                if (!strncmp(val, "UDP", 3))
                    HDK_Set_PN_IPProtocol(pStr1, HDK_Element_PN_PortMappingProtocol, HDK_Enum_PN_IPProtocol_UDP);
                else if (!strncmp(val, "TCP", 3))
                    HDK_Set_PN_IPProtocol(pStr1, HDK_Element_PN_PortMappingProtocol, HDK_Enum_PN_IPProtocol_TCP);
                
				//Get port mapping internal port
                sprintf(path, "%sInternalPort", tmpBuf);
                if (MBus_GetParamVal(mbus, path, val, sizeof(val)) != 0)
                    return 0;
                HDK_Set_Int(pStr1, HDK_Element_PN_InternalPort, atoi(val));

                //Get port mapping external port
                sprintf(path, "%sExternalPort", tmpBuf);
                if (MBus_GetParamVal(mbus, path, val, sizeof(val)) != 0)
                    return 0;
                HDK_Set_Int(pStr1, HDK_Element_PN_ExternalPort, atoi(val));
             }
             break;
             
        case HDK_DeviceValue_Username:
            HDK_Set_String(pStruct, HDK_Element_PN_Username, "admin");
            break;

        case HDK_DeviceValue_AdminPassword:
            insNum = NELEMS(insPath);
            if (MBus_FindObjectIns(mbus, "Device.Users.User.", 
                        "Username", "admin", insPath, &insNum) != 0
                    || insNum == 0)
            {
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%sX_CISCO_COM_Password", insPath[0]);
            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;

            HDK_Set_String(pStruct, HDK_Element_PN_AdminPassword, val);
            break;

        case HDK_DeviceValue_DeviceType:
			HDK_Set_PN_DeviceType(pStruct, HDK_Element_PN_Type, HDK_Enum_PN_DeviceType_GatewayWithWiFi);
            break;

        case HDK_DeviceValue_DeviceName:
			if (MBus_GetParamVal(mbus, "Device.DeviceInfo.ModelName", val, sizeof(val)) != 0)
                return 0;
			HDK_Set_String(pStruct, HDK_Element_PN_DeviceName, val);
            break;

        case HDK_DeviceValue_VendorName:
			if (MBus_GetParamVal(mbus, "Device.DeviceInfo.Manufacturer", val, sizeof(val)) != 0)
                return 0;
			HDK_Set_String(pStruct, HDK_Element_PN_VendorName, val);
            break;

        case HDK_DeviceValue_ModelDescription:
			if (MBus_GetParamVal(mbus, "Device.DeviceInfo.Description", val, sizeof(val)) != 0)
                return 0;
			HDK_Set_String(pStruct, HDK_Element_PN_ModelDescription, val);
            break;

        case HDK_DeviceValue_ModelName:
			if (MBus_GetParamVal(mbus, "Device.DeviceInfo.ModelName", val, sizeof(val)) != 0)
                return 0;
			HDK_Set_String(pStruct, HDK_Element_PN_ModelName, val);
            break;

        case HDK_DeviceValue_FirmwareVersion:
			if (MBus_GetParamVal(mbus, "Device.DeviceInfo.X_CISCO_COM_FirmwareName", val, sizeof(val)) != 0)
                return 0;
			HDK_Set_String(pStruct, HDK_Element_PN_FirmwareVersion, val);
            break;

        case HDK_DeviceValue_PresentationURL:
			strcpy(val, "http://");
			//Get instance 
			if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.PrimaryLANDHCPv4ServerPool", pathVal, sizeof(pathVal)) != 0)
			{
				log_printf(LOG_ERR, "Can't get multilan Primary LAN DHCPv4 server pool \n");
				//return -1;
				//zqiu: return 0 when error
                return 0;
			}

			snprintf(tmpPath, sizeof(tmpPath), "%s.IPRouters", pathVal);

			if (MBus_GetParamVal(mbus, tmpPath, val + strlen(val), sizeof(val) - strlen(val)) != 0)
				return 0;
			HDK_Set_String(pStruct, HDK_Element_PN_PresentationURL, val);	
            break;

        case HDK_DeviceValue_SubDeviceURLs:
			pStr1 = HDK_Set_Struct(pStruct, HDK_Element_PN_SubDeviceURLs);
            if ( pStr1 )
            {
                HDK_Set_String(pStr1, HDK_Element_PN_string, "");
            }
            break;

        case HDK_DeviceValue_Tasks:
			pStr1 = HDK_Set_Struct(pStruct, HDK_Element_PN_Tasks);
            if ( pStr1 )
            {
                pStr2 = HDK_Set_Struct(pStr1, HDK_Element_PN_TaskExtension);
            }

            if ( pStr2 )
            {
                HDK_Set_String(pStr2, HDK_Element_PN_Name, "setting");
                HDK_Set_String(pStr2, HDK_Element_PN_URL, "/");
                HDK_Set_PN_TaskExtType(pStr2, HDK_Element_PN_Type, HDK_Enum_PN_TaskExtType_Browser);
            }
            break;

        case HDK_DeviceValue_SerialNumber:
            break;

        case HDK_DeviceValue_TimeZone:
            if (MBus_GetParamVal(mbus, "Device.Time.LocalTimeZone", val, sizeof(val)) != 0)
                return 0;
            HDK_Set_String(pStruct, HDK_Element_PN_TimeZone, val);
            break;

        case HDK_DeviceValue_AutoAdjustDST:
            if (MBus_GetParamVal(mbus, "Device.Time.X_CISCO_COM_DaylightSavingAutoAdjust", val, sizeof(val)) != 0)
                val[0] = '\0';

            if (strcmp(val, "true") == 0)
                HDK_Set_Bool(pStruct, HDK_Element_PN_AutoAdjustDST, 1);
            else
                HDK_Set_Bool(pStruct, HDK_Element_PN_AutoAdjustDST, 0);

            break;

        case HDK_DeviceValue_Locale:
            if (MBus_GetParamVal(mbus, "Device.UserInterface.CurrentLanguage", val, sizeof(val)) != 0)
                val[0] = '\0';
            HDK_Set_String(pStruct, HDK_Element_PN_Locale, val);
            break;

        case HDK_DeviceValue_SSL:
        case HDK_DeviceValue_RemoteSSL:
            if (MBus_GetParamVal(mbus, "Device.UserInterface.X_CISCO_COM_RemoteAccess.HttpsEnable", val, sizeof(val)) != 0)
                return 0;
            
            if (!strcmp(val, "true"))
            {
                HDK_Set_Bool(pStruct, HDK_Element_PN_SSL, true);
            }
            else if (!strcmp(val, "false"))
            {
                HDK_Set_Bool(pStruct, HDK_Element_PN_SSL, false);
            }
            else 
                return 0;
            break;

        case HDK_DeviceValue_SupportedLocales:
            break;

        case HDK_DeviceValue_ModelRevision:
            break;

        case HDK_DeviceValue_FirmwareDate:
            break;

        case HDK_DeviceValue_UpdateMethods:
            break;

        case HDK_DeviceValue_IsDeviceReady:
            HDK_Set_Result(pStruct, HDK_Element_PN_IsDeviceReadyResult, HDK_Enum_Result_OK);
            break;

        case HDK_DeviceValue_ConnectedClients:
            if ((pStr1 = HDK_Set_Struct(pStruct, HDK_Element_PN_ConnectedClients)) == NULL)
                return 0;

            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityDHCPv4ServerPool", pathVal, sizeof(pathVal)) != 0)
            {
                log_printf(LOG_ERR, "Can't get multilan Home server pool\n");
                //return -1;
				//zqiu: return 0 when error
                return 0;
            }
            snprintf(tmpPath, sizeof(tmpPath), "%s.ClientNumberOfEntries", pathVal);

            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;

            insNum = atoi(val);
            for (i = 1; i <= insNum; i++)
            {
                if (i == 1)
                    pStr2 = HDK_Set_Struct(pStr1, HDK_Element_PN_ConnectedClient);
                else
                    pStr2 = HDK_Append_Struct(pStr1, HDK_Element_PN_ConnectedClient);

                if (pStr2 == NULL)
                    return 0;

                /* ConnectTime: is not supported */
                gettimeofday(&tv, NULL);
                HDK_Set_DateTime(pStr2, HDK_Element_PN_ConnectTime, tv.tv_sec - 1000);    

                /* MACAddress */
                snprintf(tmpPath, sizeof(tmpPath), "%s.Client.%d.Chaddr", pathVal, i);
                if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                    return 0;

                if (sscanf(val, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", 
                            &macAddr.a, &macAddr.b, &macAddr.c,
                            &macAddr.d, &macAddr.e, &macAddr.f) != 6)
                    return 0;
                HDK_Set_MACAddress(pStr2, HDK_Element_PN_MacAddress, &macAddr);

                /* DeviceName */
                snprintf(tmpPath, sizeof(tmpPath), "%s.Client.%d.X_CISCO_COM_HostName", pathVal, i);
                if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                    return 0;
                HDK_Set_String(pStr2, HDK_Element_PN_DeviceName, val);

                /* PortNum & Wireless */
                snprintf(tmpPath, sizeof(tmpPath), "%s.Client.%d.X_CISCO_COM_Interface", pathVal, i);
                if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                    return 0;
				/*
                if (strncmp("Device.WiFi.", val, strlen("Device.WiFi.")) != 0)
                {
                    HDK_Set_PN_LANConnection(pStr2, HDK_Element_PN_PortName, HDK_Enum_PN_LANConnection_LAN);
                    HDK_Set_Bool(pStr2, HDK_Element_PN_Wireless, 0);
                }
                else*/
                {
                    /* TODO: Decide a/b/n/g */
                    HDK_Set_PN_LANConnection(pStr2, HDK_Element_PN_PortName, HDK_Enum_PN_LANConnection_WLAN_802_11n);
					HDK_Set_Bool(pStr2, HDK_Element_PN_Wireless, 1);
                }

                /* Active */
                snprintf(tmpPath, sizeof(tmpPath), "%s.Client.%d.Active", pathVal, i);
                if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                    return 0;
                if (strcmp(val, "true") == 0)
                    boolVal = true;
                else
                    boolVal = false;
                HDK_Set_Bool(pStr2, HDK_Element_PN_Active, boolVal);
            }
            break;

        case HDK_DeviceValue_LanIPAddress:
            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityDHCPv4ServerPool", pathVal, sizeof(pathVal)) != 0)
            {
                log_printf(LOG_ERR, "Can't get multilan Home server pool\n");
                //return -1;
				//zqiu: return 0 when error
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.IPRouters", pathVal);
            
            //try get dhcpv4 server ip address and compare with specific Home Security prefix
            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
			{
				log_printf(LOG_ERR, "Get %s failed\n", tmpPath);
				return 0;
			}

            if (sscanf(val, "%hhu.%hhu.%hhu.%hhu", &ipAddr.a, &ipAddr.b, &ipAddr.c, &ipAddr.d) != 4)
                return 0;

            HDK_Set_IPAddress(pStruct, HDK_Element_PN_RouterIPAddress, &ipAddr);
            break;

        case HDK_DeviceValue_LanSubnetMask:
            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityDHCPv4ServerPool", pathVal, sizeof(pathVal)) != 0)
            {   
                log_printf(LOG_ERR, "Can't get multilan Home server pool\n");
                //return -1;
				//zqiu: return 0 when error
                return 0; 
            }   

            snprintf(tmpPath, sizeof(tmpPath), "%s.SubnetMask", pathVal);

            //try get dhcpv4 server ip address and compare with specific Home Security prefix
            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
			{
				log_printf(LOG_ERR, "Get %s failed\n", tmpPath);
				return 0;
			}

            if (sscanf(val, "%hhu.%hhu.%hhu.%hhu", &ipAddr.a, &ipAddr.b, &ipAddr.c, &ipAddr.d) != 4)
                return 0;

            HDK_Set_IPAddress(pStruct, HDK_Element_PN_RouterSubnetMask, &ipAddr);
            break;
      
        case HDK_DeviceValue_DHCPEnabled:
            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityDHCPv4ServerPool", pathVal, sizeof(pathVal)) != 0)
            {   
                log_printf(LOG_ERR, "Can't get multilan Home server pool\n");
                //return -1;
				//zqiu: return 0 when error
                return 0; 
            }   

            snprintf(tmpPath, sizeof(tmpPath), "%s.Enable", pathVal);

            //try get dhcpv4 server ip address and compare with specific Home Security prefix
            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
			{
				log_printf(LOG_ERR, "Get %s failed\n", tmpPath);
				return 0;
			}

            if (strcmp(val, "true") == 0)
            {
                HDK_Set_Bool(pStruct, HDK_Element_PN_DHCPServerEnabled, true);
            }
            else if (strcmp(val, "false") == 0)
            {
                HDK_Set_Bool(pStruct, HDK_Element_PN_DHCPServerEnabled, false);
            }
            else
                return 0;
            
            break;

        case HDK_DeviceValue_DHCPFirstIP:
            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityDHCPv4ServerPool", pathVal, sizeof(pathVal)) != 0)
            {   
                log_printf(LOG_ERR, "Can't get multilan Home server pool\n");
                //return -1;
				//zqiu: return 0 when error
                return 0; 
            }   

            snprintf(tmpPath, sizeof(tmpPath), "%s.MinAddress", pathVal);

            //try get dhcpv4 server ip address and compare with specific Home Security prefix
            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
			{
				log_printf(LOG_ERR, "Get %s failed\n", tmpPath);
				return 0;
			}

            if (sscanf(val, "%hhu.%hhu.%hhu.%hhu", &ipAddr.a, &ipAddr.b, &ipAddr.c, &ipAddr.d) != 4)
                return 0;

            HDK_Set_IPAddress(pStruct, HDK_Element_PN_IPAddressFirst, &ipAddr);
            break;

        case HDK_DeviceValue_DHCPLastIP:
            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityDHCPv4ServerPool", pathVal, sizeof(pathVal)) != 0)
            {   
                log_printf(LOG_ERR, "Can't get multilan Home server pool\n");
                //return -1;
				//zqiu: return 0 when error
                return 0; 
            }   

            snprintf(tmpPath, sizeof(tmpPath), "%s.MaxAddress", pathVal);

            //try get dhcpv4 server ip address and compare with specific Home Security prefix
            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
			{
				log_printf(LOG_ERR, "Get %s failed\n", tmpPath);
				return 0;
			}

            if (sscanf(val, "%hhu.%hhu.%hhu.%hhu", &ipAddr.a, &ipAddr.b, &ipAddr.c, &ipAddr.d) != 4)
                return 0;

            HDK_Set_IPAddress(pStruct, HDK_Element_PN_IPAddressLast, &ipAddr);
            break;

        case HDK_DeviceValue_DHCPLeaseTime:
            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityDHCPv4ServerPool", pathVal, sizeof(pathVal)) != 0)
            {   
                log_printf(LOG_ERR, "Can't get multilan Home server pool\n");
                //return -1; 
				//zqiu: return 0 when error
                return 0;
            }   

            snprintf(tmpPath, sizeof(tmpPath), "%s.LeaseTime", pathVal);

            //try get dhcpv4 server ip address and compare with specific Home Security prefix
            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
			{	
				log_printf(LOG_ERR, "Get %s failed\n", tmpPath);
				return 0;
			}

            HDK_Set_Int(pStruct, HDK_Element_PN_LeaseTime, atoi(val));
            break;

        case HDK_DeviceValue_DHCPReservations:
            pStr1 = HDK_Set_Struct(pStruct, HDK_Element_PN_DHCPReservations);

			if (pStr1 == NULL)
				return 0;

            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityDHCPv4ServerPool", pathVal, sizeof(pathVal)) != 0)
            {   
                log_printf(LOG_ERR, "Can't get multilan Home server pool\n");
                //return -1;
				//zqiu: return 0 when error
                return 0; 
            }   

            snprintf(tmpPath, sizeof(tmpPath), "%s.StaticAddress", pathVal);

            insNum = NELEMS(insPath);
            if (MBus_FindObjectIns(mbus, tmpPath, NULL, NULL, insPath, &insNum) != 0)
                insNum = 0; /* return 0; */
			printf("Getting reserved DHCP address\n");		
            for (i = 0; i < insNum; i++)
            {
                if (i == 0)
                    pStr2 = HDK_Set_Struct(pStr1, HDK_Element_PN_DHCPReservation);
                else
                    pStr2 = HDK_Append_Struct(pStr1, HDK_Element_PN_DHCPReservation);

                if (!pStr2)
                    return 0;

                /* DeviceName */
                strcpy(tmpBuf, insPath[i]);
                snprintf(tmpPath, sizeof(tmpPath), "%sX_CISCO_COM_DeviceName", tmpBuf);
                if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                    return 0;
                HDK_Set_String(pStr2, HDK_Element_PN_DeviceName, val);

                /* IPAddress */
                snprintf(tmpPath, sizeof(tmpPath), "%sYiaddr", tmpBuf);
                if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                    return 0;

                if (sscanf(val, "%hhu.%hhu.%hhu.%hhu", &ipAddr.a, &ipAddr.b, &ipAddr.c, &ipAddr.d) != 4)
                    return 0;

                HDK_Set_IPAddress(pStr2, HDK_Element_PN_IPAddress, &ipAddr);

                /* MacAddress */
                snprintf(tmpPath, sizeof(tmpPath), "%sChaddr", tmpBuf);
                if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                    return 0;

                if (sscanf(val, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", 
                            &macAddr.a, &macAddr.b, &macAddr.c,
                            &macAddr.d, &macAddr.e, &macAddr.f) != 6)
                    return 0;

                HDK_Set_MACAddress(pStr2, HDK_Element_PN_MacAddress, &macAddr);
            }
            break;

        case HDK_DeviceValue_RadioInfos:
            /* 
             * here RadioID means "Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiRadio.Alias" 
             */

            /* set a RadioInfos structure if there is a RadioInfo */
            insNum = NELEMS(insPath);
            if (MBus_FindObjectIns(mbus, "Device.WiFi.Radio.", NULL, NULL, insPath, &insNum) != 0)
                return 0;

            if ((pStr1 = HDK_Set_Struct(pStruct, HDK_Element_PN_RadioInfos)) == NULL)
                return 0;

            /* set a RadioInfo structure */
            /* should we try to retrieve the Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiRadio structure first ? */
            pStr2 = HDK_Set_Struct(pStr1, HDK_Element_PN_RadioInfo);

            if (!pStr2)
            {
                log_printf(LOG_ERR, "add Home Security RadioInfo error");
		//zqiu: return 0 when error
                return 0;
            }

            /* RadioID */
            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiRadio", pathVal, sizeof(pathVal)) != 0)
            {
                log_printf(LOG_ERR, "Can't get multilan Home wifi radio\n");
                //return -1;
				//zqiu: return 0 when error
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.Alias", pathVal);

            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;

            HDK_Set_String(pStr2, HDK_Element_PN_RadioID, val);

            /* Frequency */
                snprintf(tmpPath, sizeof(tmpPath), "%s.OperatingFrequencyBand", pathVal);
                if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                    return 0;

                if (strcmp(val, "2.4GHz") == 0)
                    HDK_Set_Int(pStr2, HDK_Element_PN_Frequency, 2);
                else if (strcmp(val, "5GHz") == 0)
                    HDK_Set_Int(pStr2, HDK_Element_PN_Frequency, 5);
                else
                    return 0;

                /* SupportedModes */
                if ((pStr3 = HDK_Set_Struct(pStr2, HDK_Element_PN_SupportedModes)) == NULL)
                    return 0;

                snprintf(tmpPath, sizeof(tmpPath), "%s.SupportedStandards", pathVal);
                
                if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                    return 0;
                switch (strlen(val))
                {
                    case 1:
                        if (!strcmp(val, "a"))
                            HDK_Set_PN_WiFiMode(pStr3, HDK_Element_PN_string, HDK_Enum_PN_WiFiMode_802_11a);
                        else if (!strcmp(val, "b"))
                            HDK_Set_PN_WiFiMode(pStr3, HDK_Element_PN_string, HDK_Enum_PN_WiFiMode_802_11b);
                        else if (!strcmp(val, "g"))
                            HDK_Set_PN_WiFiMode(pStr3, HDK_Element_PN_string, HDK_Enum_PN_WiFiMode_802_11g);
                        else if (!strcmp(val, "n"))
                            HDK_Set_PN_WiFiMode(pStr3, HDK_Element_PN_string, HDK_Enum_PN_WiFiMode_802_11n);
                        else 
                            return 0;
                        break;
                    case 3:
                        if (!strcmp(val, "a,n") || !strcmp(val, "n,a"))
						{
                            HDK_Set_PN_WiFiMode(pStr3, HDK_Element_PN_string, HDK_Enum_PN_WiFiMode_802_11a);
                            HDK_Append_PN_WiFiMode(pStr3, HDK_Element_PN_string, HDK_Enum_PN_WiFiMode_802_11n);
                            HDK_Append_PN_WiFiMode(pStr3, HDK_Element_PN_string, HDK_Enum_PN_WiFiMode_802_11an);
						}
                        else if (!strcmp(val, "b,g") || !strcmp(val, "g,b"))
						{
                            HDK_Set_PN_WiFiMode(pStr3, HDK_Element_PN_string, HDK_Enum_PN_WiFiMode_802_11b);
                            HDK_Append_PN_WiFiMode(pStr3, HDK_Element_PN_string, HDK_Enum_PN_WiFiMode_802_11g);
                            HDK_Append_PN_WiFiMode(pStr3, HDK_Element_PN_string, HDK_Enum_PN_WiFiMode_802_11bg);
						}
                        else if (!strcmp(val, "b,n") || !strcmp(val, "n,b"))
						{
                            HDK_Set_PN_WiFiMode(pStr3, HDK_Element_PN_string, HDK_Enum_PN_WiFiMode_802_11b);
                            HDK_Append_PN_WiFiMode(pStr3, HDK_Element_PN_string, HDK_Enum_PN_WiFiMode_802_11n);
                            HDK_Append_PN_WiFiMode(pStr3, HDK_Element_PN_string, HDK_Enum_PN_WiFiMode_802_11bn);
						}
                        else if (!strcmp(val, "n,g") || !strcmp(val, "g,n"))
						{
                            HDK_Set_PN_WiFiMode(pStr3, HDK_Element_PN_string, HDK_Enum_PN_WiFiMode_802_11n);
                            HDK_Append_PN_WiFiMode(pStr3, HDK_Element_PN_string, HDK_Enum_PN_WiFiMode_802_11g);
                            HDK_Append_PN_WiFiMode(pStr3, HDK_Element_PN_string, HDK_Enum_PN_WiFiMode_802_11gn);
						}
                        else 
                            return 0;
                        break;
                    case 5:
                        if (strchr(val, 'b') && strchr(val, 'g') && strchr(val, 'n'))
						{
                            HDK_Set_PN_WiFiMode(pStr3, HDK_Element_PN_string, HDK_Enum_PN_WiFiMode_802_11b);
                            HDK_Append_PN_WiFiMode(pStr3, HDK_Element_PN_string, HDK_Enum_PN_WiFiMode_802_11g);
                            HDK_Append_PN_WiFiMode(pStr3, HDK_Element_PN_string, HDK_Enum_PN_WiFiMode_802_11n);
                            HDK_Append_PN_WiFiMode(pStr3, HDK_Element_PN_string, HDK_Enum_PN_WiFiMode_802_11bg);
                            HDK_Append_PN_WiFiMode(pStr3, HDK_Element_PN_string, HDK_Enum_PN_WiFiMode_802_11bn);
                            HDK_Append_PN_WiFiMode(pStr3, HDK_Element_PN_string, HDK_Enum_PN_WiFiMode_802_11gn);
                            HDK_Append_PN_WiFiMode(pStr3, HDK_Element_PN_string, HDK_Enum_PN_WiFiMode_802_11bgn);
						}
                        else 
                            return 0;
						break;
                    case 7:
                        if (strchr(val, 'b') && strchr(val, 'g') && strchr(val, 'n') && strchr(val, 'a'))
						{
                            HDK_Set_PN_WiFiMode(pStr3, HDK_Element_PN_string, HDK_Enum_PN_WiFiMode_802_11b);
                            HDK_Append_PN_WiFiMode(pStr3, HDK_Element_PN_string, HDK_Enum_PN_WiFiMode_802_11a);
                            HDK_Append_PN_WiFiMode(pStr3, HDK_Element_PN_string, HDK_Enum_PN_WiFiMode_802_11g);
                            HDK_Append_PN_WiFiMode(pStr3, HDK_Element_PN_string, HDK_Enum_PN_WiFiMode_802_11n);
                            HDK_Append_PN_WiFiMode(pStr3, HDK_Element_PN_string, HDK_Enum_PN_WiFiMode_802_11bg);
                            HDK_Append_PN_WiFiMode(pStr3, HDK_Element_PN_string, HDK_Enum_PN_WiFiMode_802_11bn);
                            HDK_Append_PN_WiFiMode(pStr3, HDK_Element_PN_string, HDK_Enum_PN_WiFiMode_802_11gn);
                            HDK_Append_PN_WiFiMode(pStr3, HDK_Element_PN_string, HDK_Enum_PN_WiFiMode_802_11an);
                            HDK_Append_PN_WiFiMode(pStr3, HDK_Element_PN_string, HDK_Enum_PN_WiFiMode_802_11bgn);
						}
                        else 
                            return 0;
						break;
                    default:
                        return 0;
                }
                /* Channels[] */
                if ((pStr3 = HDK_Set_Struct(pStr2, HDK_Element_PN_Channels)) == NULL)
                    return 0;

                snprintf(tmpPath, sizeof(tmpPath), "%s.PossibleChannels", pathVal);
                if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                    return 0;

                /* the format is "n-m" ? */
                if (sscanf(val, "%d-%d", &start, &end) == 2)
                {
                    for (idx = start; idx <= end; idx++)
                    {
                        if (idx == start)
                            HDK_Set_Int(pStr3, HDK_Element_PN_int, idx);
                        else
                            HDK_Append_Int(pStr3, HDK_Element_PN_int, idx);
                    }
                }
                else /* the format is "o,p,q,m,n" */
                {
                    if ((ptr = strtok_r(val, ",", &saveptr)) != NULL)
                    {
                        HDK_Set_Int(pStr3, HDK_Element_PN_int, atoi(ptr));
                    }

                    while ((ptr = strtok_r(NULL, ",", &saveptr)) != NULL)
                    {
                        HDK_Append_Int(pStr3, HDK_Element_PN_int, atoi(ptr));
                    }
                }
		
                /* wide channels */
                /* TR-181 parameter not found */
                if ((pStr3 = HDK_Set_Struct(pStr2, HDK_Element_PN_WideChannels)) == NULL)
                    return 0;
                if ((pStr4 = HDK_Set_Struct(pStr3, HDK_Element_PN_WideChannel)) == NULL)
                    return 0;
                HDK_Set_Int(pStr4, HDK_Element_PN_Channel, 0);
                if ((pStr5 = HDK_Set_Struct(pStr4, HDK_Element_PN_SecondaryChannels)) == NULL)
                    return 0;
                HDK_Set_Int(pStr5, HDK_Element_PN_int, 0);

                /* SupportedSecurity */
                if ((pStr3 = HDK_Set_Struct(pStr2, HDK_Element_PN_SupportedSecurity)) == NULL)
                    return 0;

                //zqiu
  		        if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiAp", pathVal, sizeof(pathVal)) != 0)
            	{
                	log_printf(LOG_ERR, "Can't get multilan Home wifi AP\n");
	                return 0;
         	    }


                snprintf(tmpPath, sizeof(tmpPath), "%s.Security.ModesSupported", pathVal);

                if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                    return 0;

                if ((pStr4 = HDK_Set_Struct(pStr3, HDK_Element_PN_SecurityInfo)) == NULL)
                    return 0;
                
                if ((ptr = strtok_r(val, ",", &saveptr)) != NULL)
                {
                    if (!strcmp(ptr, "None"))
                    {                        
                        HDK_Set_PN_WiFiSecurity(pStr4, HDK_Element_PN_SecurityType, HDK_Enum_PN_WiFiSecurity_NONE);

                        if ((pStr5 = HDK_Set_Struct(pStr4, HDK_Element_PN_Encryptions)) == NULL)
                            return 0;
                        
                        HDK_Set_PN_WiFiEncryption(pStr5, HDK_Element_PN_string, HDK_Enum_PN_WiFiEncryption_WEP_64);
                        HDK_Append_PN_WiFiEncryption(pStr5, HDK_Element_PN_string, HDK_Enum_PN_WiFiEncryption_WEP_128);
                    }
                    else if (!strcmp(ptr, "WEP-64"))
                    {                        
                        HDK_Set_PN_WiFiSecurity(pStr4, HDK_Element_PN_SecurityType, HDK_Enum_PN_WiFiSecurity_WEP_64);

                        if ((pStr5 = HDK_Set_Struct(pStr4, HDK_Element_PN_Encryptions)) == NULL)
                            return 0;
                        
                        HDK_Set_PN_WiFiEncryption(pStr5, HDK_Element_PN_string, HDK_Enum_PN_WiFiEncryption_WEP_64);
                        //HDK_Append_PN_WiFiSecurity(pStr5, HDK_Element_PN_SecurityType, HDK_Enum_PN_WiFiEncryption_WEP_128);
                    }
                    else if (!strcmp(ptr, "WEP-128"))
                    {                        
                        HDK_Set_PN_WiFiSecurity(pStr4, HDK_Element_PN_SecurityType, HDK_Enum_PN_WiFiSecurity_WEP_128);

                        if ((pStr5 = HDK_Set_Struct(pStr4, HDK_Element_PN_Encryptions)) == NULL)
                            return 0;
                        
                        HDK_Set_PN_WiFiEncryption(pStr5, HDK_Element_PN_string, HDK_Enum_PN_WiFiEncryption_WEP_128);
                        //HDK_Append_PN_WiFiSecurity(pStr5, HDK_Element_PN_SecurityType, HDK_Enum_PN_WiFiEncryption_WEP_128);
                    }
                    else if (!strcmp(ptr, "WPA-Personal"))
                    {                        
                        HDK_Set_PN_WiFiSecurity(pStr4, HDK_Element_PN_SecurityType, HDK_Enum_PN_WiFiSecurity_WPA_Personal);
                        
                        if ((pStr5 = HDK_Set_Struct(pStr4, HDK_Element_PN_Encryptions)) == NULL)
                            return 0;
                        
                        HDK_Set_PN_WiFiEncryption(pStr5, HDK_Element_PN_string, HDK_Enum_PN_WiFiEncryption_TKIP);
                        HDK_Append_PN_WiFiEncryption(pStr5, HDK_Element_PN_string, HDK_Enum_PN_WiFiEncryption_AES);
                    }
                    else if (!strcmp(ptr, "WPA2-Personal"))
                    {                        
                        HDK_Set_PN_WiFiSecurity(pStr4, HDK_Element_PN_SecurityType, HDK_Enum_PN_WiFiSecurity_WPA2_Personal);
                        
                        if ((pStr5 = HDK_Set_Struct(pStr4, HDK_Element_PN_Encryptions)) == NULL)
                            return 0;
                        
                        HDK_Set_PN_WiFiEncryption(pStr5, HDK_Element_PN_string, HDK_Enum_PN_WiFiEncryption_TKIP);
                        HDK_Append_PN_WiFiEncryption(pStr5, HDK_Element_PN_string, HDK_Enum_PN_WiFiEncryption_AES);
                        HDK_Append_PN_WiFiEncryption(pStr5, HDK_Element_PN_string, HDK_Enum_PN_WiFiEncryption_TKIPORAES);
                    }
                    else if (!strcmp(ptr, "WPA-WPA2-Personal"))
                    {                        
                        HDK_Set_PN_WiFiSecurity(pStr4, HDK_Element_PN_SecurityType, HDK_Enum_PN_WiFiSecurity_WPA_WPA2_Personal);

                        if ((pStr5 = HDK_Set_Struct(pStr4, HDK_Element_PN_Encryptions)) == NULL)
                            return 0;
                        
                        HDK_Set_PN_WiFiEncryption(pStr5, HDK_Element_PN_string, HDK_Enum_PN_WiFiEncryption_TKIP);
                        HDK_Append_PN_WiFiEncryption(pStr5, HDK_Element_PN_string, HDK_Enum_PN_WiFiEncryption_AES);
                        HDK_Append_PN_WiFiEncryption(pStr5, HDK_Element_PN_string, HDK_Enum_PN_WiFiEncryption_TKIPORAES);
                    }
                    else if (!strcmp(ptr, "WPA-Enterprise"))
                    {                        
                        HDK_Set_PN_WiFiSecurity(pStr4, HDK_Element_PN_SecurityType, HDK_Enum_PN_WiFiSecurity_WPA_Enterprise);

                        if ((pStr5 = HDK_Set_Struct(pStr4, HDK_Element_PN_Encryptions)) == NULL)
                            return 0;
                        
                        HDK_Set_PN_WiFiEncryption(pStr5, HDK_Element_PN_string, HDK_Enum_PN_WiFiEncryption_TKIP);
                        HDK_Append_PN_WiFiEncryption(pStr5, HDK_Element_PN_string, HDK_Enum_PN_WiFiEncryption_AES);
                    }
                    else if (!strcmp(ptr, "WPA2-Enterprise"))
                    {                        
                        HDK_Set_PN_WiFiSecurity(pStr4, HDK_Element_PN_SecurityType, HDK_Enum_PN_WiFiSecurity_WPA2_Enterprise);

                        if ((pStr5 = HDK_Set_Struct(pStr4, HDK_Element_PN_Encryptions)) == NULL)
                            return 0;
                        
                        HDK_Set_PN_WiFiEncryption(pStr5, HDK_Element_PN_string, HDK_Enum_PN_WiFiEncryption_TKIP);
                        HDK_Append_PN_WiFiEncryption(pStr5, HDK_Element_PN_string, HDK_Enum_PN_WiFiEncryption_AES);
                        HDK_Append_PN_WiFiEncryption(pStr5, HDK_Element_PN_string, HDK_Enum_PN_WiFiEncryption_TKIPORAES);
                    }
                    else if (!strcmp(ptr, "WPA-WPA2-Enterprise"))
                    {                        
                        HDK_Set_PN_WiFiSecurity(pStr4, HDK_Element_PN_SecurityType, HDK_Enum_PN_WiFiSecurity_WPA_WPA2_Enterprise);

                        if ((pStr5 = HDK_Set_Struct(pStr4, HDK_Element_PN_Encryptions)) == NULL)
                            return 0;
                        
                        HDK_Set_PN_WiFiEncryption(pStr5, HDK_Element_PN_string, HDK_Enum_PN_WiFiEncryption_TKIP);
                        HDK_Append_PN_WiFiEncryption(pStr5, HDK_Element_PN_string, HDK_Enum_PN_WiFiEncryption_AES);
                        HDK_Append_PN_WiFiEncryption(pStr5, HDK_Element_PN_string, HDK_Enum_PN_WiFiEncryption_TKIPORAES);
                    }
                    else 
                        return 0;

                    while ((ptr = strtok_r(NULL, ",", &saveptr)) != NULL)
                    {
						if ((pStr4 = HDK_Append_Struct(pStr3, HDK_Element_PN_SecurityInfo)) == NULL)
		                    return 0;

                        if (!strcmp(ptr, "None"))
                        {                        
                            HDK_Append_PN_WiFiSecurity(pStr4, HDK_Element_PN_SecurityType, HDK_Enum_PN_WiFiSecurity_NONE);

                            if ((pStr5 = HDK_Set_Struct(pStr4, HDK_Element_PN_Encryptions)) == NULL)
                                return 0;
                        
                            HDK_Set_PN_WiFiEncryption(pStr5, HDK_Element_PN_string, HDK_Enum_PN_WiFiEncryption_WEP_64);
                            HDK_Append_PN_WiFiEncryption(pStr5, HDK_Element_PN_string, HDK_Enum_PN_WiFiEncryption_WEP_128);
                        }
                        else if (!strcmp(ptr, "WEP-64"))
                        {                        
                            HDK_Append_PN_WiFiSecurity(pStr4, HDK_Element_PN_SecurityType, HDK_Enum_PN_WiFiSecurity_WEP_64);

                            if ((pStr5 = HDK_Set_Struct(pStr4, HDK_Element_PN_Encryptions)) == NULL)
                                return 0;
                        
                            HDK_Set_PN_WiFiEncryption(pStr5, HDK_Element_PN_string, HDK_Enum_PN_WiFiEncryption_WEP_64);
                        }
                        else if (!strcmp(ptr, "WEP-128"))
                        {                        
                            HDK_Append_PN_WiFiSecurity(pStr4, HDK_Element_PN_SecurityType, HDK_Enum_PN_WiFiSecurity_WEP_128);

                            if ((pStr5 = HDK_Set_Struct(pStr4, HDK_Element_PN_Encryptions)) == NULL)
                                return 0;
                        
                            HDK_Set_PN_WiFiEncryption(pStr5, HDK_Element_PN_string, HDK_Enum_PN_WiFiEncryption_WEP_128);
                        }
                        else if (!strcmp(ptr, "WPA-Personal"))
                        {                        
                            HDK_Append_PN_WiFiSecurity(pStr4, HDK_Element_PN_SecurityType, HDK_Enum_PN_WiFiSecurity_WPA_Personal);

                            if ((pStr5 = HDK_Set_Struct(pStr4, HDK_Element_PN_Encryptions)) == NULL)
                                return 0;
                        
                            HDK_Set_PN_WiFiEncryption(pStr5, HDK_Element_PN_string, HDK_Enum_PN_WiFiEncryption_TKIP);
                            HDK_Append_PN_WiFiEncryption(pStr5, HDK_Element_PN_string, HDK_Enum_PN_WiFiEncryption_AES);
                        }
                        else if (!strcmp(ptr, "WPA2-Personal"))
                        {                        
                            HDK_Append_PN_WiFiSecurity(pStr4, HDK_Element_PN_SecurityType, HDK_Enum_PN_WiFiSecurity_WPA2_Personal);

                            if ((pStr5 = HDK_Set_Struct(pStr4, HDK_Element_PN_Encryptions)) == NULL)
                                return 0;
                        
                            HDK_Set_PN_WiFiEncryption(pStr5, HDK_Element_PN_string, HDK_Enum_PN_WiFiEncryption_TKIP);
                            HDK_Append_PN_WiFiEncryption(pStr5, HDK_Element_PN_string, HDK_Enum_PN_WiFiEncryption_AES);
                            HDK_Append_PN_WiFiEncryption(pStr5, HDK_Element_PN_string, HDK_Enum_PN_WiFiEncryption_TKIPORAES);
                        }
                        else if (!strcmp(ptr, "WPA-WPA2-Personal"))
                        {                        
                            HDK_Append_PN_WiFiSecurity(pStr4, HDK_Element_PN_SecurityType, HDK_Enum_PN_WiFiSecurity_WPA_WPA2_Personal);
                            
                            if ((pStr5 = HDK_Set_Struct(pStr4, HDK_Element_PN_Encryptions)) == NULL)
                                return 0;
                        
                            HDK_Set_PN_WiFiEncryption(pStr5, HDK_Element_PN_string, HDK_Enum_PN_WiFiEncryption_TKIP);
                            HDK_Append_PN_WiFiEncryption(pStr5, HDK_Element_PN_string, HDK_Enum_PN_WiFiEncryption_AES);
                            HDK_Append_PN_WiFiEncryption(pStr5, HDK_Element_PN_string, HDK_Enum_PN_WiFiEncryption_TKIPORAES);
                        }
                        else if (!strcmp(ptr, "WPA-Enterprise"))
                        {                        
                            HDK_Append_PN_WiFiSecurity(pStr4, HDK_Element_PN_SecurityType, HDK_Enum_PN_WiFiSecurity_WPA_Enterprise);

                            if ((pStr5 = HDK_Set_Struct(pStr4, HDK_Element_PN_Encryptions)) == NULL)
                                return 0;
                        
                            HDK_Set_PN_WiFiEncryption(pStr5, HDK_Element_PN_string, HDK_Enum_PN_WiFiEncryption_TKIP);
                            HDK_Append_PN_WiFiEncryption(pStr5, HDK_Element_PN_string, HDK_Enum_PN_WiFiEncryption_AES);
                        }
                        else if (!strcmp(ptr, "WPA2-Enterprise"))
                        {                        
                            HDK_Append_PN_WiFiSecurity(pStr4, HDK_Element_PN_SecurityType, HDK_Enum_PN_WiFiSecurity_WPA2_Enterprise);

                            if ((pStr5 = HDK_Set_Struct(pStr4, HDK_Element_PN_Encryptions)) == NULL)
                                return 0;
                        
                            HDK_Set_PN_WiFiEncryption(pStr5, HDK_Element_PN_string, HDK_Enum_PN_WiFiEncryption_TKIP);
                            HDK_Append_PN_WiFiEncryption(pStr5, HDK_Element_PN_string, HDK_Enum_PN_WiFiEncryption_AES);
                            HDK_Append_PN_WiFiEncryption(pStr5, HDK_Element_PN_string, HDK_Enum_PN_WiFiEncryption_TKIPORAES);
                        }
                        else if (!strcmp(ptr, "WPA-WPA2-Enterprise"))
                        {                        
                            HDK_Append_PN_WiFiSecurity(pStr4, HDK_Element_PN_SecurityType, HDK_Enum_PN_WiFiSecurity_WPA_WPA2_Enterprise);

                            if ((pStr5 = HDK_Set_Struct(pStr4, HDK_Element_PN_Encryptions)) == NULL)
                                return 0;
                        
                            HDK_Set_PN_WiFiEncryption(pStr5, HDK_Element_PN_string, HDK_Enum_PN_WiFiEncryption_TKIP);
                            HDK_Append_PN_WiFiEncryption(pStr5, HDK_Element_PN_string, HDK_Enum_PN_WiFiEncryption_AES);
                            HDK_Append_PN_WiFiEncryption(pStr5, HDK_Element_PN_string, HDK_Enum_PN_WiFiEncryption_TKIPORAES);
                        }
                        else 
                            return 0;
                    }
                }
                else
                   return 0;
            break;

        case HDK_DeviceValue_WLanSecurityEnabled:
            if ((strVal = HDK_Get_String(pInput, HDK_Element_PN_RadioID)) == NULL)
                return 0;

            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiRadio", pathVal, sizeof(pathVal)) != 0)
            {
                log_printf(LOG_ERR, "Can't get multilan Home wifi radio\n");
                //return -1;
				//zqiu: return 0 when error
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.Alias", pathVal);

            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;

            if (strcmp(val, strVal))
            {
                log_printf(LOG_ERR, "error input Radio ID\n");
                return 0;
            }

            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiAp", pathVal, sizeof(pathVal)) != 0)
            {
                log_printf(LOG_ERR, "Can't get multilan Home wifi access point\n");
                //return -1;
				//zqiu: return 0 when error
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.Security.ModeEnabled", pathVal);

            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;

            if (strcmp(val, "None") == 0)
            {
                HDK_Set_Bool(pStruct, HDK_Element_PN_Enabled, false);
                HDK_Set_PN_WiFiSecurity(pStruct, HDK_Element_PN_Type, HDK_Enum_PN_WiFiSecurity_NONE);
                HDK_Set_String(pStruct, HDK_Element_PN_Key, "");
            }
            else if (!strcmp(val, "WEP-64"))
            {
                HDK_Set_Bool(pStruct, HDK_Element_PN_Enabled, true);
                HDK_Set_PN_WiFiSecurity(pStruct, HDK_Element_PN_Type, HDK_Enum_PN_WiFiSecurity_WEP_64);

                snprintf(tmpPath, sizeof(tmpPath), "%s.Security.X_CISCO_COM_WEPKey64Bit.1.WEPKey", pathVal);
                strcpy(val, "WEP-");
                if (MBus_GetParamVal(mbus, tmpPath, val + strlen(val), sizeof(val) - strlen(val)) != 0)
                    return 0;

                HDK_Set_String(pStruct, HDK_Element_PN_Key, val);
            }
            else if (!strcmp(val, "WEP-128"))
            {
                HDK_Set_Bool(pStruct, HDK_Element_PN_Enabled, true);
                HDK_Set_PN_WiFiSecurity(pStruct, HDK_Element_PN_Type, HDK_Enum_PN_WiFiSecurity_WEP_128);

                snprintf(tmpPath, sizeof(tmpPath), "%s.Security.X_CISCO_COM_WEPKey128Bit.1.WEPKey", pathVal);
                strcpy(val, "WEP-");
                if (MBus_GetParamVal(mbus, tmpPath, val + strlen(val), sizeof(val) - strlen(val)) != 0)
                    return 0;

                HDK_Set_String(pStruct, HDK_Element_PN_Key, val);
            }
            else if (!strcmp(val, "WPA-Personal"))
            {
                HDK_Set_Bool(pStruct, HDK_Element_PN_Enabled, true);
                HDK_Set_PN_WiFiSecurity(pStruct, HDK_Element_PN_Type, HDK_Enum_PN_WiFiSecurity_WPA_Personal);

                snprintf(tmpPath, sizeof(tmpPath), "%s.Security.X_COMCAST-COM_KeyPassphrase", pathVal);
                strcpy(val, "WPA-");
                if (MBus_GetParamVal(mbus, tmpPath, val + strlen(val), sizeof(val) - strlen(val)) != 0)
                    return 0;

                HDK_Set_String(pStruct, HDK_Element_PN_Key, val);
            }
            else if (!strcmp(val, "WPA2-Personal"))
            {
                HDK_Set_Bool(pStruct, HDK_Element_PN_Enabled, true);
                HDK_Set_PN_WiFiSecurity(pStruct, HDK_Element_PN_Type, HDK_Enum_PN_WiFiSecurity_WPA2_Personal);

                snprintf(tmpPath, sizeof(tmpPath), "%s.Security.X_COMCAST-COM_KeyPassphrase", pathVal);
                strcpy(val, "WPA-");
                if (MBus_GetParamVal(mbus, tmpPath, val + strlen(val), sizeof(val) - strlen(val)) != 0)
                    return 0;

                HDK_Set_String(pStruct, HDK_Element_PN_Key, val);
            }
            else if (!strcmp(val, "WPA-WPA2-Personal"))
            {
                HDK_Set_Bool(pStruct, HDK_Element_PN_Enabled, true);
                HDK_Set_PN_WiFiSecurity(pStruct, HDK_Element_PN_Type, HDK_Enum_PN_WiFiSecurity_WPA_WPA2_Personal);

                snprintf(tmpPath, sizeof(tmpPath), "%s.Security.X_COMCAST-COM_KeyPassphrase", pathVal);
                strcpy(val, "WPA-");
                if (MBus_GetParamVal(mbus, tmpPath, val + strlen(val), sizeof(val) - strlen(val)) != 0)
                    return 0;

                HDK_Set_String(pStruct, HDK_Element_PN_Key, val);
            }
            else if (!strcmp(val, "WPA-Enterprise"))
            {
                HDK_Set_Bool(pStruct, HDK_Element_PN_Enabled, true);
                HDK_Set_PN_WiFiSecurity(pStruct, HDK_Element_PN_Type, HDK_Enum_PN_WiFiSecurity_WPA_Enterprise);

                snprintf(tmpPath, sizeof(tmpPath), "%s.Security.X_COMCAST-COM_KeyPassphrase", pathVal);
                strcpy(val, "WPA-");
                if (MBus_GetParamVal(mbus, tmpPath, val + strlen(val), sizeof(val) - strlen(val)) != 0)
                    return 0;

                HDK_Set_String(pStruct, HDK_Element_PN_Key, val);
            }
            else if (!strcmp(val, "WPA2-Enterprise"))
            {
                HDK_Set_Bool(pStruct, HDK_Element_PN_Enabled, true);
                HDK_Set_PN_WiFiSecurity(pStruct, HDK_Element_PN_Type, HDK_Enum_PN_WiFiSecurity_WPA2_Enterprise);

                snprintf(tmpPath, sizeof(tmpPath), "%s.Security.X_COMCAST-COM_KeyPassphrase", pathVal);
                strcpy(val, "WPA-");
                if (MBus_GetParamVal(mbus, tmpPath, val + strlen(val), sizeof(val) - strlen(val)) != 0)
                    return 0;

                HDK_Set_String(pStruct, HDK_Element_PN_Key, val);
            }
            else if (!strcmp(val, "WPA-WPA2-Enterprise"))
            {
                HDK_Set_Bool(pStruct, HDK_Element_PN_Enabled, true);
                HDK_Set_PN_WiFiSecurity(pStruct, HDK_Element_PN_Type, HDK_Enum_PN_WiFiSecurity_WPA_WPA2_Enterprise);

                snprintf(tmpPath, sizeof(tmpPath), "%s.Security.X_COMCAST-COM_KeyPassphrase", pathVal);
                strcpy(val, "WPA-");
                if (MBus_GetParamVal(mbus, tmpPath, val + strlen(val), sizeof(val) - strlen(val)) != 0)
                    return 0;

                HDK_Set_String(pStruct, HDK_Element_PN_Key, val);
            }
            else
                return 0;

            snprintf(tmpPath, sizeof(tmpPath), "%s.Security.RekeyingInterval", pathVal);
            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;

            HDK_Set_Int(pStruct, HDK_Element_PN_KeyRenewal, atoi(val));
            break;
/*
        case HDK_DeviceValue_WLanType:
            if ((strVal = HDK_Get_String(pInput, HDK_Element_PN_RadioID)) == NULL)
                return 0;

            insNum = NELEMS(insPath);
            
            if (MBus_FindObjectIns(mbus, "Device.WiFi.Radio.", NULL, NULL, insPath, &insNum) != 0)
                return 0;

            for (i = 0; i < insNum; i++)
            {
                snprintf(tmpPath, sizeof(tmpPath), "%sAlias", insPath[0]);

                if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                    return 0;

                if (!strcmp(val, strVal))
                {
                    break;
                }
            }

            if (i >= insNum)
                return 0;

            snprintf(tmpPath, sizeof(tmpPath), " Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiAp.Security.ModeEnabled", i + 1);
            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;

            HDK_Set_PN_WiFiSecurity(pStruct, HDK_Element_PN_Type, TransSecurityMode(val));
            break;*/

        case HDK_DeviceValue_WLanEncryption:
            if ((strVal = HDK_Get_String(pInput, HDK_Element_PN_RadioID)) == NULL)
                return 0;

            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiRadio", pathVal, sizeof(pathVal)) != 0)
            {
                log_printf(LOG_ERR, "Can't get multilan Home wifi radio\n");
                //return -1;
				//zqiu: return 0 when error
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.Alias", pathVal);

            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;

            if (strcmp(val, strVal))
            {
                log_printf(LOG_ERR, "error input Radio ID\n");
                return 0;
            }

            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiAp", pathVal, sizeof(pathVal)) != 0)
            {
                log_printf(LOG_ERR, "Can't get multilan Home wifi access point\n");
                //return -1;
				//zqiu: return 0 when error
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.Security.ModeEnabled", pathVal);

            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;

            //ssid is open
            if (!strcmp(val, "None"))
            {    
                HDK_Set_PN_WiFiEncryption(pStruct, HDK_Element_PN_Encryption, HDK_Enum_PN_WiFiEncryption_TKIPORAES);
                break;
            }
			
            snprintf(tmpPath, sizeof(tmpPath), "%s.Security.X_CISCO_COM_EncryptionMethod", pathVal);
            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;

            if (strcmp(val, "AES") == 0)
            {
                HDK_Set_PN_WiFiEncryption(pStruct, HDK_Element_PN_Encryption, HDK_Enum_PN_WiFiEncryption_AES);
            }
            else if (strcmp(val, "TKIP") == 0)
            {
                HDK_Set_PN_WiFiEncryption(pStruct, HDK_Element_PN_Encryption, HDK_Enum_PN_WiFiEncryption_TKIP);
            }
            else if (strcmp(val, "AES+TKIP") == 0)
            {
                HDK_Set_PN_WiFiEncryption(pStruct, HDK_Element_PN_Encryption, HDK_Enum_PN_WiFiEncryption_TKIPORAES);
            }
            else 
                return 0;
            break;
/*
        case HDK_DeviceValue_WLanKey:
            if ((strVal = HDK_Get_String(pInput, HDK_Element_PN_RadioID)) == NULL)
                return 0;

            if (!GetWifiDataModel(mbus, strVal, &wifiDM))
                return 0;

            snprintf(tmpPath, sizeof(tmpPath), "%sX_COMCAST-COM_KeyPassphrase", wifiDM.security);
            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;

            HDK_Set_String(pStruct, HDK_Element_PN_Key, val);
            break;

        case HDK_DeviceValue_WLanKeyRenewal:
            if ((strVal = HDK_Get_String(pInput, HDK_Element_PN_RadioID)) == NULL)
                return 0;

            if (!GetWifiDataModel(mbus, strVal, &wifiDM))
                return 0;

            snprintf(tmpPath, sizeof(tmpPath), "%sRekeyingInterval", wifiDM.security);
            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;

            HDK_Set_Int(pStruct, HDK_Element_PN_KeyRenewal, atoi(val));
            break;*/

        case HDK_DeviceValue_WLanRadiusIP1:
        case HDK_DeviceValue_WLanRadiusIP2:
        case HDK_DeviceValue_WLanRadiusPort1:
        case HDK_DeviceValue_WLanRadiusPort2:
        case HDK_DeviceValue_WLanRadiusSecret1:
        case HDK_DeviceValue_WLanRadiusSecret2:
            if ((strVal = HDK_Get_String(pInput, HDK_Element_PN_RadioID)) == NULL)
                return 0;

            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiRadio", pathVal, sizeof(pathVal)) != 0)
            {
                log_printf(LOG_ERR, "Can't get multilan Home wifi radio\n");
                //return -1;
				//zqiu: return 0 when error
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.Alias", pathVal);

            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;

            if (strcmp(val, strVal))
            {
                log_printf(LOG_ERR, "error input Radio ID\n");
                return 0;
            }

            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiAp", pathVal, sizeof(pathVal)) != 0)
            {
                log_printf(LOG_ERR, "Can't get multilan Home wifi access point\n");
                //return -1;
				//zqiu: return 0 when error
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.Security.ModeEnabled", pathVal);

            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;

            if (strstr(val, "Enterprise"))
            {
                snprintf(tmpPath, sizeof(tmpPath), "%s.Security.RadiusServerIPAddr", pathVal);
                if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                    return 0;
                if (sscanf(val, "%hhu.%hhu.%hhu.%hhu", &ipAddr.a, &ipAddr.b, &ipAddr.c, &ipAddr.d) != 4)
                    return 0;
       
                HDK_Set_IPAddress(pStruct, HDK_Element_PN_RadiusIP1, &ipAddr);

                snprintf(tmpPath, sizeof(tmpPath), "%s.Security.RadiusServerPort", pathVal);
                if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                    return 0;
             
                HDK_Set_Int(pStruct, HDK_Element_PN_RadiusPort1, atoi(val));

                snprintf(tmpPath, sizeof(tmpPath), "%s.Security.RadiusSecret", pathVal);
                if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                    return 0;
             
                HDK_Set_String(pStruct, HDK_Element_PN_RadiusSecret1, val);
            }
            else
            {
                memset(&ipAddr, 0, sizeof(ipAddr));
                HDK_Set_IPAddress(pStruct, HDK_Element_PN_RadiusIP1, &ipAddr);
                HDK_Set_Int(pStruct, HDK_Element_PN_RadiusPort1, 0);
                HDK_Set_String(pStruct, HDK_Element_PN_RadiusSecret1, "");
            }
            
            memset(&ipAddr, 0, sizeof(ipAddr));
            HDK_Set_IPAddress(pStruct, HDK_Element_PN_RadiusIP2, &ipAddr);
            HDK_Set_Int(pStruct, HDK_Element_PN_RadiusPort2, 0);
            HDK_Set_String(pStruct, HDK_Element_PN_RadiusSecret2, "");
            break;

        case HDK_DeviceValue_WLanEnabled:
            if ((strVal = HDK_Get_String(pInput, HDK_Element_PN_RadioID)) == NULL)
                return 0;

            insNum = NELEMS(insPath);
           
            /* see if there is a Radio list */ 
            if (MBus_FindObjectIns(mbus, "Device.WiFi.Radio.", NULL, NULL, insPath, &insNum) != 0)
                return 0;

            /* see if there is a Home Security WiFi Radio */			
            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiRadio", pathVal, sizeof(pathVal)) != 0)
            {
                log_printf(LOG_ERR, "Can't get multilan Home wifi radio\n");
                //return -1;
				//zqiu: return 0 when error
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.Alias", pathVal);

            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;

            /* see if the alias is the same */
            if (strcmp(val, strVal))
            {
                log_printf(LOG_ERR, "Error input Radio ID\n");
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.Enable", pathVal);

            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;

            if (strcmp(val, "true") == 0)
                HDK_Set_Bool(pStruct, HDK_Element_PN_Enabled, true);
            else if (strcmp(val, "false") == 0)
                HDK_Set_Bool(pStruct, HDK_Element_PN_Enabled, false);
            else
                return 0;        

            break;

        case HDK_DeviceValue_WLanMode:
            if ((strVal = HDK_Get_String(pInput, HDK_Element_PN_RadioID)) == NULL)
                return 0;

            insNum = NELEMS(insPath);
           
            /* see if there is a Radio list */ 
            if (MBus_FindObjectIns(mbus, "Device.WiFi.Radio.", NULL, NULL, insPath, &insNum) != 0)
                return 0;
			
            /* see if there is a Home Security WiFi Radio */			
            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiRadio", pathVal, sizeof(pathVal)) != 0)
            {
                log_printf(LOG_ERR, "Can't get multilan Home wifi radio\n");
                //return -1;
				//zqiu: return 0 when error
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.Alias", pathVal);

            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;

            /* see if the alias is the same */
            if (strcmp(val, strVal))
            {
                log_printf(LOG_ERR, "Error input Radio ID\n");
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.OperatingStandards", pathVal);
            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;

            if (strcmp(val, "a") == 0)
                HDK_Set_PN_WiFiMode(pStruct, HDK_Element_PN_Mode, HDK_Enum_PN_WiFiMode_802_11a);
            else if (strcmp(val, "b") == 0)
                HDK_Set_PN_WiFiMode(pStruct, HDK_Element_PN_Mode, HDK_Enum_PN_WiFiMode_802_11b);
            else if (strcmp(val, "g") == 0)
                HDK_Set_PN_WiFiMode(pStruct, HDK_Element_PN_Mode, HDK_Enum_PN_WiFiMode_802_11g);
            else if (strcmp(val, "n") == 0)
                HDK_Set_PN_WiFiMode(pStruct, HDK_Element_PN_Mode, HDK_Enum_PN_WiFiMode_802_11n);
            else if (strcmp(val, "a,n") == 0 || strcmp(val, "n,a") == 0)
                HDK_Set_PN_WiFiMode(pStruct, HDK_Element_PN_Mode, HDK_Enum_PN_WiFiMode_802_11an);
            else if (strcmp(val, "g,n") == 0 || strcmp(val, "n,g") == 0)
                HDK_Set_PN_WiFiMode(pStruct, HDK_Element_PN_Mode, HDK_Enum_PN_WiFiMode_802_11gn);
            else if (strchr(val, 'b') && strchr(val, 'g') && strchr(val, 'n'))
                HDK_Set_PN_WiFiMode(pStruct, HDK_Element_PN_Mode, HDK_Enum_PN_WiFiMode_802_11bgn);
            else if (strcmp(val, "b,n") == 0 || strcmp(val, "n,b") == 0)
                HDK_Set_PN_WiFiMode(pStruct, HDK_Element_PN_Mode, HDK_Enum_PN_WiFiMode_802_11bn);
            else if (strcmp(val, "b,g") == 0 || strcmp(val, "g,b") == 0)
                HDK_Set_PN_WiFiMode(pStruct, HDK_Element_PN_Mode, HDK_Enum_PN_WiFiMode_802_11bg);
            else
                HDK_Set_PN_WiFiMode(pStruct, HDK_Element_PN_Mode, HDK_Enum_PN_WiFiMode_);

            break;

        case HDK_DeviceValue_WANMacAddress:
			 snprintf(tmpPath, sizeof(tmpPath), "Device.DeviceInfo.X_CISCO_COM_BaseMacAddress");

             if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
				return 0;

			 if (sscanf(val, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", 
                       &macAddr.a, &macAddr.b, &macAddr.c,
                       &macAddr.d, &macAddr.e, &macAddr.f) != 6)
                return 0;
			 HDK_Set_MACAddress(pStruct, HDK_Element_PN_MacAddress, &macAddr);
			 break;

        case HDK_DeviceValue_MacAddress:
            if ((strVal = HDK_Get_String(pInput, HDK_Element_PN_RadioID)) == NULL)
                return 0;

            insNum = NELEMS(insPath);

            /* see if there is a Radio list */            
            if (MBus_FindObjectIns(mbus, "Device.WiFi.Radio.", NULL, NULL, insPath, &insNum) != 0)
                return 0;
			
            /* see if there is a Home Security WiFi Radio */
            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiRadio", pathVal, sizeof(pathVal)) != 0)
            {
                log_printf(LOG_ERR, "Can't get multilan Home wifi radio\n");
                //return -1;
				//zqiu: return 0 when error
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.Alias", pathVal);

            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;

            /* see if alias is the same */
            if (strcmp(val, strVal))
            {
                log_printf(LOG_ERR, "Error input Radio ID\n");
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.MACAddress", pathVal);

            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;
            
            if (sscanf(val, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", 
                       &macAddr.a, &macAddr.b, &macAddr.c,
                       &macAddr.d, &macAddr.e, &macAddr.f) != 6)
                return 0;

            HDK_Set_MACAddress(pStruct, HDK_Element_PN_MacAddress, &macAddr);
            break;

        case HDK_DeviceValue_WLanSSID:
            if ((strVal = HDK_Get_String(pInput, HDK_Element_PN_RadioID)) == NULL)
                return 0;

            insNum = NELEMS(insPath);
           
            /* see if there is a Radio list */ 
            if (MBus_FindObjectIns(mbus, "Device.WiFi.Radio.", NULL, NULL, insPath, &insNum) != 0)
                return 0;
			
            /* see if there is a Home Security WiFi Radio */
            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiRadio", pathVal, sizeof(pathVal)) != 0)
            {
                log_printf(LOG_ERR, "Can't get multilan Home wifi radio\n");
                //return -1;
				//zqiu: return 0 when error
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.Alias", pathVal);

            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;

            /* see if alias is the same */
            if (strcmp(val, strVal))
            {
                log_printf(LOG_ERR, "Error input Radio ID\n");
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.SSID", pathVal);

            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;

            HDK_Set_String(pStruct, HDK_Element_PN_SSID, val);
            break;

        case HDK_DeviceValue_WLanSSIDBroadcast:
            if ((strVal = HDK_Get_String(pInput, HDK_Element_PN_RadioID)) == NULL)
                return 0;

            insNum = NELEMS(insPath);
            
            /* see if there is a Radio list */ 
            if (MBus_FindObjectIns(mbus, "Device.WiFi.Radio.", NULL, NULL, insPath, &insNum) != 0)
                return 0;
			
            /* see if there is a Home Security WiFi Radio */
            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiRadio", pathVal, sizeof(pathVal)) != 0)
            {
                log_printf(LOG_ERR, "Can't get multilan Home wifi radio\n");
                //return -1;
				//zqiu: return 0 when error
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.Alias", pathVal);

            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;

            /* see if alias is the same */
            if (strcmp(val, strVal))
            {
	        log_printf(LOG_ERR, "Error input Radio ID\n");
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.SSIDAdvertisementEnabled", pathVal);

            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;
            
            if (strcmp(val, "true") == 0)
                HDK_Set_Bool(pStruct, HDK_Element_PN_SSIDBroadcast, true);
            else if (strcmp(val, "false") == 0)
                HDK_Set_Bool(pStruct, HDK_Element_PN_SSIDBroadcast, false);
            else 
                return 0;
            
            break;

        case HDK_DeviceValue_WLanChannelWidth:
            if ((strVal = HDK_Get_String(pInput, HDK_Element_PN_RadioID)) == NULL)
                return 0;

            insNum = NELEMS(insPath);
           
            /* see if there is a Radio list */ 
            if (MBus_FindObjectIns(mbus, "Device.WiFi.Radio.", NULL, NULL, insPath, &insNum) != 0)
                return 0;
			
            /* see if there is a Home Security WiFi Radio */
            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiRadio", pathVal, sizeof(pathVal)) != 0)
            {
                log_printf(LOG_ERR, "Can't get multilan Home wifi radio\n");
                //return -1;
				//zqiu: return 0 when error
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.Alias", pathVal);

            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;

            /* see if alias is the same */
            if (strcmp(val, strVal))
            {
                log_printf(LOG_ERR, "Error input Radio ID\n");
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.OperatingChannelBandwidth", pathVal);

            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;
            
            if (!strcmp(val, "20MHz"))
            {
                HDK_Set_Int(pStruct, HDK_Element_PN_ChannelWidth, 20);
            }
            else if (!strcmp(val, "40MHz"))
            {
                HDK_Set_Int(pStruct, HDK_Element_PN_ChannelWidth, 40);
            }
            else if (!strcmp(val, "Auto")) 
				HDK_Set_Int(pStruct, HDK_Element_PN_ChannelWidth, 0);
			else
                return 0;
            break;

        case HDK_DeviceValue_WLanChannel:
            if ((strVal = HDK_Get_String(pInput, HDK_Element_PN_RadioID)) == NULL)
                return 0;

            insNum = NELEMS(insPath);
           
            /* see if there is a Radio list */ 
            if (MBus_FindObjectIns(mbus, "Device.WiFi.Radio.", NULL, NULL, insPath, &insNum) != 0)
                return 0;
			
            /* see if there is a Home Security WiFi Radio */
            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiRadio", pathVal, sizeof(pathVal)) != 0)
            {
                log_printf(LOG_ERR, "Can't get multilan Home wifi radio\n");
                //return -1;
				//zqiu: return 0 when error
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.Alias", pathVal);

            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;

            /* see if alias is the same */
            if (strcmp(val, strVal))
            {
                log_printf(LOG_ERR, "Error input Radio ID\n");
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.Channel", pathVal);

            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;
            
            HDK_Set_Int(pStruct, HDK_Element_PN_Channel, atoi(val));
            break;

        case HDK_DeviceValue_WLanSecondaryChannel:
            if ((strVal = HDK_Get_String(pInput, HDK_Element_PN_RadioID)) == NULL)
                return 0;

            insNum = NELEMS(insPath);
           
            /* see if there is a Radio list */ 
            if (MBus_FindObjectIns(mbus, "Device.WiFi.Radio.", NULL, NULL, insPath, &insNum) != 0)
                return 0;
			
            /* see if there is a Home Security WiFi Radio */
            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiRadio", pathVal, sizeof(pathVal)) != 0)
            {
                log_printf(LOG_ERR, "Can't get multilan Home wifi radio\n");
                //return -1;
				//zqiu: return 0 when error
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.Alias", pathVal);

            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;

            /* see if alias is the same */
            if (strcmp(val, strVal))
            {
                log_printf(LOG_ERR, "Error input Radio ID\n");
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.ExtensionChannel", pathVal);

            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;

            if (strcmp(val, "AboveControlChannel") == 0)
                HDK_Set_Int(pStruct, HDK_Element_PN_SecondaryChannel, atoi("1"));
            else if (strcmp(val, "BelowControlChannel") == 0)
                HDK_Set_Int(pStruct, HDK_Element_PN_SecondaryChannel, atoi("-1"));
            else
                HDK_Set_Int(pStruct, HDK_Element_PN_SecondaryChannel, atoi("0"));

            break;

        case HDK_DeviceValue_WLanQoS:
            if ((strVal = HDK_Get_String(pInput, HDK_Element_PN_RadioID)) == NULL)
                return 0;

            insNum = NELEMS(insPath);
           
            /* see if there is a Radio list */ 
            if (MBus_FindObjectIns(mbus, "Device.WiFi.Radio.", NULL, NULL, insPath, &insNum) != 0)
                return 0;
			
            /* see if there is a Home Security WiFi Radio */
            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiRadio", pathVal, sizeof(pathVal)) != 0)
            {
                log_printf(LOG_ERR, "Can't get multilan Home wifi radio\n");
                //return -1;
				//zqiu: return 0 when error
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.Alias", pathVal);

            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;

            /* see if alias is the same */
            if (strcmp(val, strVal))
            {
                log_printf(LOG_ERR, "Error input Radio ID\n");
                return 0;
            }

            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiAp", pathVal, sizeof(pathVal)) != 0)
            {
                log_printf(LOG_ERR, "Can't get multilan Home wifi access point\n");
                //return -1;
				//zqiu: return 0 when error
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.WMMEnable", pathVal);
            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;

            if (strcmp(val, "true") == 0)
                HDK_Set_Bool(pStruct, HDK_Element_PN_QoS, true);
            else if (strcmp(val, "false") == 0)
                HDK_Set_Bool(pStruct, HDK_Element_PN_QoS, false);
            else 
                return 0;

            break;

        case HDK_DeviceValue_WanAutoDetectType:
            break;

        case HDK_DeviceValue_WanStatus:
            break;

        case HDK_DeviceValue_WanSupportedTypes:
            break;

        case HDK_DeviceValue_WanType:
            //2013.3.7, USG only supports DHCP now
            HDK_Set_PN_WANType(pStruct, HDK_Element_PN_Type, HDK_Enum_PN_WANType_DHCP);
            break;

        case HDK_DeviceValue_WanUsername:
            //2013.3.7, USG only supports DHCP now
            HDK_Set_String(pStruct, HDK_Element_PN_Username, "");
            break;

        case HDK_DeviceValue_WanPassword:
            //2013.3.7, USG only supports DHCP now
            HDK_Set_String(pStruct, HDK_Element_PN_Password, "");
            break;

        case HDK_DeviceValue_WanMaxIdleTime:
            //2013.3.7, USG only supports DHCP now, never timeout
            HDK_Set_Int(pStruct, HDK_Element_PN_MaxIdleTime, 0);
            break;

        case HDK_DeviceValue_WanAutoReconnect:
            //2013.3.7, USG only supports DHCP now, no need to reconnect
            HDK_Set_Bool(pStruct, HDK_Element_PN_AutoReconnect, false);
            break;

        case HDK_DeviceValue_WanAuthService:
            /* not supported */
            HDK_Set_String(pStruct, HDK_Element_PN_ServiceName, "");
            break;

        case HDK_DeviceValue_WanPPPoEService:
            break;

        case HDK_DeviceValue_WanLoginService:
            HDK_Set_String(pStruct, HDK_Element_PN_ServiceName, "");
            /* not supported */
            break;

        case HDK_DeviceValue_WanIP:

            /* find number of Client object instance */
            if (MBus_FindObjectIns(mbus, "Device.DHCPv4.Client.", NULL, NULL, insPath, &insNum) != 0)
                return 0;
   
            /* find the first available Client instance */
            for (i = 0; i < insNum; i++)
            {
                if (insPath[i] != NULL)
                {
                    printf("insPath[%d] = %s\n\n", i, insPath[i]);
                    break;
                 }
            }

            /* use first available Client instance */
            snprintf(tmpPath, sizeof(tmpPath), "%sIPAddress", insPath[i]);
            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;
			if (sscanf(val, "%hhu.%hhu.%hhu.%hhu", 
                            &ipAddr.a, &ipAddr.b, &ipAddr.c, &ipAddr.d) != 4)
				return 0;
			HDK_Set_IPAddress(pStruct, HDK_Element_PN_IPAddress, &ipAddr);
            break;

        case HDK_DeviceValue_WanSubnetMask:

            /* find number of Client object instance */
            if (MBus_FindObjectIns(mbus, "Device.DHCPv4.Client.", NULL, NULL, insPath, &insNum) != 0)
                return 0;
   
            /* find the first available Client instance */
            for (i = 0; i < insNum; i++)
            {
                if (insPath[i] != NULL)
                {
                    printf("insPath[%d] = %s\n\n", i, insPath[i]);
                    break;
                 }
            }

            /* use first available Client instance */
            snprintf(tmpPath, sizeof(tmpPath), "%sSubnetMask", insPath[i]);
            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;
			if (sscanf(val, "%hhu.%hhu.%hhu.%hhu", 
                            &ipAddr.a, &ipAddr.b, &ipAddr.c, &ipAddr.d) != 4)
				return 0;
			HDK_Set_IPAddress(pStruct, HDK_Element_PN_SubnetMask, &ipAddr);
            break;

        case HDK_DeviceValue_WanGateway:

            /* find number of Client object instance */
            if (MBus_FindObjectIns(mbus, "Device.DHCPv4.Client.", NULL, NULL, insPath, &insNum) != 0)
                return 0;
   
            /* find the first available Client instance */
            for (i = 0; i < insNum; i++)
            {
                if (insPath[i] != NULL)
                {
                    printf("insPath[%d] = %s\n\n", i, insPath[i]);
                    break;
                 }
            }

            /* use first available Client instance */
            snprintf(tmpPath, sizeof(tmpPath), "%sIPRouters", insPath[i]);
            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;
			if (!strcmp(val, ""))
				strcpy(val, "0.0.0.0");
			if (sscanf(val, "%hhu.%hhu.%hhu.%hhu", 
                            &ipAddr.a, &ipAddr.b, &ipAddr.c, &ipAddr.d) != 4)
				return 0;
			HDK_Set_IPAddress(pStruct, HDK_Element_PN_Gateway, &ipAddr);
            break;

        case HDK_DeviceValue_WanDNSSettings:
            //USG not support DNS relay or proxy, so LAN DNS will the same as WAN side
            if ((pStr1 = HDK_Set_Struct(pStruct, HDK_Element_PN_DNS)) == NULL)
                return 0;

            insNum = NELEMS(insPath);			
			snprintf(tmpPath, sizeof(tmpPath), "Device.DNS.Client.ServerNumberOfEntries");
				
			if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
			   return 0;
			insNum = atoi(val);
			
			if (!insNum)
			{
				memset(&ipAddr, 0, sizeof(ipAddr));
				HDK_Set_IPAddress(pStr1, HDK_Element_PN_Primary, &ipAddr);
                HDK_Set_IPAddress(pStr1, HDK_Element_PN_Secondary, &ipAddr);
				break;
			}
            for (i = 0, j = 0; i < insNum && j < 3; i++)
            {
				snprintf(tmpPath, sizeof(tmpPath), "Device.DNS.Client.Server.%d.Type", i+1);
				if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                    return 0;

				if(strcmp(val, "DHCPv4"))
				{
					log_printf(LOG_WARNING, "Don't support IPv6, bypass it");
					continue;
				}

                snprintf(tmpPath, sizeof(tmpPath), "Device.DNS.Client.Server.%d.DNSServer", i+1);
				
                if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                    return 0;

				if (!strcmp(val, ""))
					strcpy(val, "0.0.0.0");

                if (sscanf(val, "%hhu.%hhu.%hhu.%hhu", 
                            &ipAddr.a, &ipAddr.b, &ipAddr.c, &ipAddr.d) != 4)
                    return 0;
                if (j == 0)
                {
                    HDK_Set_IPAddress(pStr1, HDK_Element_PN_Primary, &ipAddr);
					memset(&ipAddr, 0, sizeof(ipAddr));
                    HDK_Set_IPAddress(pStr1, HDK_Element_PN_Secondary, &ipAddr);
					j++;
                }
                else if (j == 1)
                {
                    HDK_Set_IPAddress(pStr1, HDK_Element_PN_Secondary, &ipAddr);
					j++;
                }
                else if (j == 2)
                {
                    HDK_Set_IPAddress(pStr1, HDK_Element_PN_Tertiary, &ipAddr);
					j++;
                }               
                
            }
            break;

        case HDK_DeviceValue_WanMTU:
            snprintf(tmpPath, sizeof(tmpPath), "com.cisco.spvtg.ccsp.pam.Helper.FirstUpstreamIpInteface.MaxMTUSize");
            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;

            HDK_Set_Int(pStruct, HDK_Element_PN_MTU, atoi(val));
            break;

        default:
            log_printf(LOG_ERR, "unknow type");
            return 0;
    }

    return 1;
}

int HDK_Device_SetValue(void* pDeviceCtx, HDK_DeviceValue eValue, HDK_Struct* pStruct)
{
    HDK_Context *hdkCtx;
    MBusObj_t *mbus;
    char insPath[MAX_INSTANCE][MAX_PATH_NAME];
    int insNum, newIns, i, j;
    char *strVal;
    // extra length added to resolve format truncation
    char tmpPath[MAX_PATH_NAME+44];
    // extra length added to resolve format overflow
    char path[MAX_PATH_NAME+40];
    int *boolVal;
    int *intVal;
    HDK_IPAddress *ipAddr;
    HDK_MACAddress *macAddr;
    char tmpBuf[MAX_BUF];
    char ipBuf[64], addr[32], tmp[20], macBuf[64];
    char val[MAX_BUF];
    char pathVal[MAX_BUF];
    HDK_Struct *pStr1, *pStr2;
    HDK_Member *pMember;
    HDK_Enum_PN_WiFiMode *wifimode;
    HDK_Enum_PN_IPProtocol *ip_prot;
	MBusParam_t paramsSet[MAX_PARAMS];
    int paramOff = 0;
	struct itimerval time_val;

    if ((hdkCtx = (HDK_Context *)pDeviceCtx) == NULL
            || (mbus = hdkCtx->mbus) == NULL)
    {
        return 0;
    }

    switch (eValue)
    {
        case HDK_DeviceValue_PortMappings:
			j = 0;
            intVal = HDK_Get_Int(pStruct, HDK_Element_PN_ExternalPort);
            ip_prot = HDK_Get_PN_IPProtocol(pStruct, HDK_Element_PN_PortMappingProtocol);

            insNum = NELEMS(insPath);
            
            if (MBus_FindObjectIns(mbus, "Device.NAT.PortMapping.", NULL, NULL, insPath, &insNum) != 0)
			{
				log_printf(LOG_ERR, "Get port mapping rules failed\n");
                return 0;
			}

            for (i = 0; i < insNum; i++)
            {
                //HNAP rules have special IP prefix
                snprintf(tmpPath, sizeof(tmpPath), "%sInternalClient", insPath[i]);
                
                if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                    return 0;

                if (strncmp(val, HOMESECURITY_IP_PREFIX, strlen(HOMESECURITY_IP_PREFIX)))
				{
					log_printf(LOG_WARNING, "%s is not HNAP rules\n", insPath[i]);
                    continue;
				}
                
                snprintf(tmpPath, sizeof(tmpPath), "%sExternalPort", insPath[i]);

                if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                    return 0;
                
				if(atoi(val) == *intVal)
                {
                    //hnap only support tcp or udp not both
                    snprintf(tmpPath, sizeof(tmpPath), "%sProtocol", insPath[i]);

                    if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                        return 0;

                    if (!strcmp(val, "UDP"))
                    {
                        if (*ip_prot == HDK_Enum_PN_IPProtocol_UDP)
                            break;
                    }
                    else if (!strcmp(val, "TCP"))
                    {
                        if (*ip_prot == HDK_Enum_PN_IPProtocol_TCP)
                            break;
                    }
					/*
                    //suppose HNAP rules shouldn't have this 
                    else if (!strcmp(val, "BOTH"))
                    {
                        if (*ip_prot == HDK_Enum_PN_IPProtocol_UDP || *ip_prot == HDK_Enum_PN_IPProtocol_TCP)
                            break;
                    }*/
                    else 
                        continue;
                }
            }
			
			//No matter AddPortMapping or DelPortMapping, we can only change Home Security rule.  
            if (i < insNum)
			{
				snprintf(tmpPath, sizeof(tmpPath), "%sInternalClient", insPath[i]);
                
                if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                    return 0;

                if (strncmp(val, HOMESECURITY_IP_PREFIX, strlen(HOMESECURITY_IP_PREFIX)))
				{
					log_printf(LOG_ERR, "%s is not Home Security rules, can't be over-writen\n", insPath[i]);
                    return 0;
				}
					
				snprintf(tmpPath, sizeof(tmpPath), "%sInternalPort", insPath[i]);
                
                if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                    return 0;

                if (!strcmp(val, "0"))
				{
					log_printf(LOG_ERR, "%s is not Home Security rules, can't be over-writen\n", insPath[i]);
                    return 0;
				}
			}

            //add new mapping instance only if we are doing add
            if (HDK_Get_Int(pStruct, HDK_Element_PN_InternalPort) != NULL && i >= insNum)
            {
                if (MBus_AddObjectIns(mbus, "Device.NAT.PortMapping.", &newIns) != 0)
				{
					log_printf(LOG_ERR, "Adding port mapping rule failed\n");
                    return 0;
				}
				j = 1;
                //instancfe too many
                if ((insNum + 1) >= (int)NELEMS(insPath))
				{
					log_printf(LOG_ERR, "Too many port mapping rules\n");
					MBus_DelObjectIns(mbus, "Device.NAT.PortMapping.", newIns);
                    return 0;
				}
                sprintf(insPath[i], "Device.NAT.PortMapping.%d.", newIns);
            }
            
            //this means we are adding
            if (HDK_Get_Int(pStruct, HDK_Element_PN_InternalPort) != NULL)
            {   
				paramOff = 0;
				LoadMBusParam(paramsSet, &paramOff, "Description", HDK_Get_StringEx(pStruct, HDK_Element_PN_PortMappingDescription, HOMESECURITY_NAME_PREFIX), MBUS_PT_STRING);
				LoadMBusParam(paramsSet, &paramOff, "Enable", "true", MBUS_PT_BOOL);

				ipAddr = HDK_Get_IPAddress(pStruct, HDK_Element_PN_InternalClient);
				snprintf(addr, sizeof(addr), "%u.%u.%u.%u", ipAddr->a, ipAddr->b, ipAddr->c, ipAddr->d);
				LoadMBusParam(paramsSet, &paramOff, "InternalClient", addr, MBUS_PT_STRING);
				sprintf(val, "%u", *intVal);	
				LoadMBusParam(paramsSet, &paramOff, "ExternalPort", val, MBUS_PT_UINT);
				LoadMBusParam(paramsSet, &paramOff, "ExternalPortEndRange", val, MBUS_PT_UINT);

                intVal = HDK_Get_Int(pStruct, HDK_Element_PN_InternalPort);
				if (*intVal == 0)
				{
					log_printf(LOG_ERR, "Invalid internal mapping port\n");
					return 0;
				}
				sprintf(tmp, "%u", *intVal);	
				LoadMBusParam(paramsSet, &paramOff, "InternalPort", tmp, MBUS_PT_UINT);

				ip_prot = HDK_Get_PN_IPProtocol(pStruct, HDK_Element_PN_PortMappingProtocol);
				if (*ip_prot == HDK_Enum_PN_IPProtocol_UDP)
				{
					LoadMBusParam(paramsSet, &paramOff, "Protocol", "UDP", MBUS_PT_STRING);
				}
				else if (*ip_prot == HDK_Enum_PN_IPProtocol_TCP)
				{
					LoadMBusParam(paramsSet, &paramOff, "Protocol", "TCP", MBUS_PT_STRING);
				}

				if (MBus_SetParamVect(mbus, insPath[i], paramsSet, paramOff, 1))
				{
					log_printf(LOG_ERR, "set %s failed\n", insPath[i]);
					if(j)
						MBus_DelObjectIns(mbus, "Device.NAT.PortMapping.", newIns);
					return 0;
				}
            }
            //this means rule deletion
            else
            {
                if (i >= insNum)
                    return 0;
                /*
                // port forwarding not HS port forwarding
                sprintf(tmpPath, "%sInternalPort", insPath[i]);
                if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                    return 0;

                if (!atoi(val))
				{
					log_printf(LOG_ERR, "This external port was controled by Port Mapping\n");
                    return 0;
				}
				
                // not hnap own rule
                sprintf(tmpPath, "%sDescription", insPath[i]);
                if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                    return 0;

                if (strncmp(val, HOMESECURITY_NAME_PREFIX, strlen(HOMESECURITY_NAME_PREFIX)))
				{
					log_printf(LOG_ERR, "This mapping not belongs to HNAP, can't delete it\n");
                    return 0;
				}*/

                if (sscanf(insPath[i], "Device.NAT.PortMapping.%d.", &i) != 1)
                {
                    log_printf(LOG_ERR, "bad reference to port mapping rule");
                    return 0;
                }

                if (MBus_DelObjectIns(mbus, "Device.NAT.PortMapping.", i) != 0)
                    return 0;
            }
            break;
            
        case HDK_DeviceValue_WLanMode:
            if ((strVal = HDK_Get_String(pStruct, HDK_Element_PN_RadioID)) == NULL)
                return 0;

            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiRadio", pathVal, sizeof(pathVal)) != 0)
            {
                log_printf(LOG_ERR, "Can't get multilan Home wifi radio\n");
                //return -1;
				//zqiu: return 0 when error
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.Alias", pathVal);

            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
				return 0;

			if (strcmp(val, strVal))
            {
				log_printf(LOG_ERR, "error input Radio ID\n");
				return 0;
			}
	
            snprintf(tmpPath, sizeof(tmpPath), "%s.OperatingStandards", pathVal);
            wifimode = HDK_Get_PN_WiFiMode(pStruct, HDK_Element_PN_Mode);

            if (wifimode == NULL)
                return 0;
			
            switch (*wifimode)
            {
                case HDK_Enum_PN_WiFiMode_802_11a:
                    if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_STRING, "a", 0) != 0)
                        return 0;
                    break;

                case HDK_Enum_PN_WiFiMode_802_11b:
                    if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_STRING, "b", 0) != 0)
                        return 0;
                    break;

                case HDK_Enum_PN_WiFiMode_802_11g:
                    if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_STRING, "g", 0) != 0)
                        return 0;
                    break;

                case HDK_Enum_PN_WiFiMode_802_11n:
                    if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_STRING, "n", 0) != 0)
                        return 0;
                    break;

                case HDK_Enum_PN_WiFiMode_802_11an:
                    if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_STRING, "a,n", 0) != 0)
                        return 0;
                    break;

                case HDK_Enum_PN_WiFiMode_802_11bg:
                    if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_STRING, "b,g", 0) != 0)
                        return 0;
                    break;

                case HDK_Enum_PN_WiFiMode_802_11bn:
                    if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_STRING, "b,n", 0) != 0)
                        return 0;
                    break;

                case HDK_Enum_PN_WiFiMode_802_11gn:
                    if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_STRING, "g,n", 0) != 0)
                        return 0;
                    break;

                case HDK_Enum_PN_WiFiMode_802_11bgn:
                    if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_STRING, "b,g,n", 0) != 0)
                        return 0;
                    break;
                default:
                    return 1;
            }
            break;

		case HDK_DeviceValue_WLanEnabled:
            strVal = HDK_Get_String(pStruct, HDK_Element_PN_RadioID);

            if (strVal == NULL)
                return 0;

            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiRadio", pathVal, sizeof(pathVal)) != 0)
            {
                log_printf(LOG_ERR, "Can't get multilan Home wifi radio\n");
                //return -1;
				//zqiu: return 0 when error
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.Alias", pathVal);

            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
				return 0;

			if (strcmp(val, strVal))
            {
				log_printf(LOG_ERR, "error input Radio ID\n");
				return 0;
			}

			boolVal = HDK_Get_Bool(pStruct, HDK_Element_PN_Enabled);

			if (boolVal == NULL)
				return 0;
			/*
			if (MBus_SetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiRadio.Enable", MBUS_PT_BOOL, *boolVal == false ? "false" : "true", 0) != 0)
                return 0;*/
			break;

        case HDK_DeviceValue_WLanSSID:
            if ((strVal = HDK_Get_String(pStruct, HDK_Element_PN_RadioID)) == NULL)
                return 0;

            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiRadio", pathVal, sizeof(pathVal)) != 0)
            {
                log_printf(LOG_ERR, "Can't get multilan Home wifi radio\n");
                //return -1;
				//zqiu: return 0 when error
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.Alias", pathVal);

            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;

            if (strcmp(val, strVal))
            {
                log_printf(LOG_ERR, "error input Radio ID\n");
                return 0;
            }

            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiSsid", pathVal, sizeof(pathVal)) != 0)
            {
                log_printf(LOG_ERR, "Can't get multilan Home wifi ssid\n");
                //return -1;
				//zqiu: return 0 when error
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.SSID", pathVal);

            strVal = HDK_Get_String(pStruct, HDK_Element_PN_SSID);

            if (strVal == NULL)
                return 0;

            if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_STRING, strVal, 1) != 0)
                return 0;
            break;

        case HDK_DeviceValue_WLanSSIDBroadcast:
            if ((strVal = HDK_Get_String(pStruct, HDK_Element_PN_RadioID)) == NULL)
                return 0;

            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiRadio", pathVal, sizeof(pathVal)) != 0)
            {
                log_printf(LOG_ERR, "Can't get multilan Home wifi radio\n");
                //return -1;
				//zqiu: return 0 when error
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.Alias", pathVal);

            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;

            if (strcmp(val, strVal))
            {
                log_printf(LOG_ERR, "error input Radio ID\n");
                return 0;
            }

            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiAp", pathVal, sizeof(pathVal)) != 0)
            {
                log_printf(LOG_ERR, "Can't get multilan Home wifi access point\n");
                //return -1;
				//zqiu: return 0 when error
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.SSIDAdvertisementEnabled", pathVal);
            boolVal = HDK_Get_Bool(pStruct, HDK_Element_PN_SSIDBroadcast);

            if (boolVal == NULL)
                return 0;
			/*
            if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_BOOL, *boolVal == false ? "false" : "true", 1) != 0)
                return 0;
				*/
            break;

        case HDK_DeviceValue_WLanChannelWidth:
            if ((strVal = HDK_Get_String(pStruct, HDK_Element_PN_RadioID)) == NULL)
                return 0;

            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiRadio", pathVal, sizeof(pathVal)) != 0)
            {
                log_printf(LOG_ERR, "Can't get multilan Home wifi radio\n");
                //return -1;
				//zqiu: return 0 when error
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.Alias", pathVal);

            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
				return 0;

			if (strcmp(val, strVal))
            {
				log_printf(LOG_ERR, "error input Radio ID\n");
				return 0;
			}

            snprintf(tmpPath, sizeof(tmpPath), "%s.OperatingChannelBandwidth", pathVal);
            intVal = HDK_Get_Int(pStruct, HDK_Element_PN_ChannelWidth);

            if (intVal == NULL)
                return 0;
			if (*intVal == 0)
			{
				strcpy(tmpBuf, "Auto");
			}
			else if (*intVal == 20 || *intVal == 40)
			{
				sprintf(tmpBuf, "%dMHz", *intVal);
			}
			else
				return 0;

            if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_STRING, tmpBuf, 1) != 0)
                return 0;
            
            if (*intVal == 0 || *intVal == 40)
            {
                snprintf(tmpPath, sizeof(tmpPath), "%s.ExtensionChannel", pathVal);
                int *tmp;
                tmp = HDK_Get_Int(pStruct, HDK_Element_PN_SecondaryChannel);
				intVal = HDK_Get_Int(pStruct, HDK_Element_PN_Channel);

                if (tmp == NULL)
                    return 0;

				if (*tmp == 0)
				{
					if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_STRING, "Auto", 0) != 0)
						return 0;
				}
				else if (*tmp < *intVal)
				{
					if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_STRING, "BelowControlChannel", 0) != 0)
						return 0;
				}
				else
					if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_STRING, "AboveControlChannel", 0) != 0)
						return 0;
            }
            
            break;

        case HDK_DeviceValue_WLanChannel:
            if ((strVal = HDK_Get_String(pStruct, HDK_Element_PN_RadioID)) == NULL)
                return 0;

            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiRadio", pathVal, sizeof(pathVal)) != 0)
            {
                log_printf(LOG_ERR, "Can't get multilan Home wifi radio\n");
                //return -1;
				//zqiu: return 0 when error
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.Alias", pathVal);

            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
				return 0;

			if (strcmp(val, strVal))
            {
				log_printf(LOG_ERR, "error input Radio ID\n");
				return 0;
			}

            snprintf(tmpPath, sizeof(tmpPath), "%s.Channel", pathVal);
            intVal = HDK_Get_Int(pStruct, HDK_Element_PN_Channel);

            if (intVal == NULL)
                return 0;

            if (*intVal == 0)
            {
                snprintf(tmpPath, sizeof(tmpPath), "%s.AutoChannelEnable", pathVal);
                if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_BOOL, "true", 0) != 0)
                    return 0;
            }
            else
            {
                sprintf(tmpBuf, "%d", *intVal);
                snprintf(tmpPath, sizeof(tmpPath), "%s.Channel", pathVal);
                if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_UINT, tmpBuf, 0) != 0)
                    return 0;
            }
            break;

        case HDK_DeviceValue_WLanQoS:
            if ((strVal = HDK_Get_String(pStruct, HDK_Element_PN_RadioID)) == NULL)
                return 0;

            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiRadio", pathVal, sizeof(pathVal)) != 0)
            {
                log_printf(LOG_ERR, "Can't get multilan Home wifi radio\n");
                //return -1;
				//zqiu: return 0 when error
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.Alias", pathVal);
           
            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;

            if (strcmp(val, strVal))
            {
                log_printf(LOG_ERR, "error input Radio ID\n");
                return 0;
            }
			/*
            snprintf(tmpPath, sizeof(tmpPath), "Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiAp.WMMEnable");
            boolVal = HDK_Get_Bool(pStruct, HDK_Element_PN_QoS);

            if (boolVal == NULL)
                return 0;

            if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_BOOL, *boolVal == false ? "false" : "true", 1) != 0)
                return 0;
			*/
            snprintf(tmpPath, sizeof(tmpPath), "%s.", pathVal);
            if (MBus_Commit(mbus, tmpPath))
            {
                log_printf(LOG_ERR, "commit error\n");
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.X_CISCO_COM_ApplySetting", pathVal);
            if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_BOOL, "true", 1) != 0)
                return 0;

            break;
        
        case HDK_DeviceValue_WLanKeyRenewal:
            strVal = HDK_Get_String(pStruct, HDK_Element_PN_RadioID);

            if (strVal == NULL)
                return 0;

            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiRadio", pathVal, sizeof(pathVal)) != 0)
            {
                log_printf(LOG_ERR, "Can't get multilan Home wifi radio\n");
                //return -1;
				//zqiu: return 0 when error
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.Alias", pathVal);

            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;

            if (strcmp(val, strVal))
            {
                log_printf(LOG_ERR, "error input Radio ID\n");
                return 0;
            }

            intVal = HDK_Get_Int(pStruct, HDK_Element_PN_KeyRenewal);

            if (intVal == NULL)
                return 0;

            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiAp", pathVal, sizeof(pathVal)) != 0)
            {
                log_printf(LOG_ERR, "Can't get multilan Home wifi access point\n");
                //return -1;
				//zqiu: return 0 when error
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.Security.RekeyingInterval", pathVal);

            sprintf(tmpBuf, "%u", *intVal);
            if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_UINT, tmpBuf, 1) != 0)
                return 0;

            break;

        case HDK_DeviceValue_WLanRadiusIP1:
        case HDK_DeviceValue_WLanRadiusIP2:
        case HDK_DeviceValue_WLanRadiusPort1:
        case HDK_DeviceValue_WLanRadiusPort2:
        case HDK_DeviceValue_WLanRadiusSecret1:
        case HDK_DeviceValue_WLanRadiusSecret2:
            strVal = HDK_Get_String(pStruct, HDK_Element_PN_RadioID);

            if (strVal == NULL)
                return 0;

            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiRadio", pathVal, sizeof(pathVal)) != 0)
            {
                log_printf(LOG_ERR, "Can't get multilan Home wifi radio\n");
                //return -1;
				//zqiu: return 0 when error
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.Alias", pathVal);

            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;

            if (strcmp(val, strVal))
            {
                log_printf(LOG_ERR, "error input Radio ID\n");
                return 0;
            }

            //radius 1 ip address
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiAp", pathVal, sizeof(pathVal)) != 0)
            {
                log_printf(LOG_ERR, "Can't get multilan Home wifi access point\n");
                //return -1;
				//zqiu: return 0 when error
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.Security.RadiusServerIPAddr", pathVal);

            ipAddr = HDK_Get_IPAddress(pStruct, HDK_Element_PN_RadiusIP1);

            if (ipAddr == NULL)
                return 0;

            snprintf(tmpBuf, sizeof(tmpBuf), "%u.%u.%u.%u", ipAddr->a, ipAddr->b, ipAddr->c, ipAddr->d);

            if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_STRING, tmpBuf, 0) != 0)
                return 0;

            //radius 1 port
            snprintf(tmpPath, sizeof(tmpPath), "%s.Security.RadiusServerPort", pathVal);

            intVal = HDK_Get_Int(pStruct, HDK_Element_PN_RadiusPort1);

            if (intVal == NULL)
                return 0;

			sprintf(tmpBuf, "%u", *intVal);

            if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_UINT, tmpBuf, 0) != 0)
                return 0;

            //radius 1 secret
            snprintf(tmpPath, sizeof(tmpPath), "%s.Security.RadiusSecret", pathVal);

            strVal = HDK_Get_String(pStruct, HDK_Element_PN_RadiusSecret1);

            if (strVal == NULL)
                return 0;

            if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_STRING, strVal, 0) != 0)
                return 0;

            //radius 2 ip address, port secret is not supported in ccsp TR181, just ignore
            break;

        case HDK_DeviceValue_WLanSecurityEnabled:
            strVal = HDK_Get_String(pStruct, HDK_Element_PN_RadioID);

            if (strVal == NULL)
                return 0;

            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiRadio", pathVal, sizeof(pathVal)) != 0)
            {
                log_printf(LOG_ERR, "Can't get multilan Home wifi radio\n");
                //return -1;
				//zqiu: return 0 when error
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.Alias", pathVal);

            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
				return 0;

			if (strcmp(val, strVal))
            {
				log_printf(LOG_ERR, "error input Radio ID\n");
				return 0;
			}

            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiAp", pathVal, sizeof(pathVal)) != 0)
            {
                log_printf(LOG_ERR, "Can't get multilan Home wifi access point\n");
                //return -1;
				//zqiu: return 0 when error
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.Security.ModeEnabled", pathVal);

            boolVal = HDK_Get_Bool(pStruct, HDK_Element_PN_Enabled);

            if (boolVal == NULL)
                return 0;

            if (*boolVal == false)
                if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_STRING, "None", 0) != 0)
                    return 0;
            break;

        case HDK_DeviceValue_WLanType:
            strVal = HDK_Get_String(pStruct, HDK_Element_PN_RadioID);

            if (strVal == NULL)
                return 0;

            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiRadio", pathVal, sizeof(pathVal)) != 0)
            {
                log_printf(LOG_ERR, "Can't get multilan Home wifi radio\n");
                //return -1;
				//zqiu: return 0 when error
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.Alias", pathVal);

            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
				return 0;

			if (strcmp(val, strVal))
            {
				log_printf(LOG_ERR, "error input Radio ID\n");
				return 0;
			}

            boolVal = HDK_Get_Bool(pStruct, HDK_Element_PN_Enabled);

            if (boolVal == NULL)
                return 0;
            
            if ( *boolVal == false)
                break;

            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiAp", pathVal, sizeof(pathVal)) != 0)
            {
                log_printf(LOG_ERR, "Can't get multilan Home wifi access point\n");
                //return -1;
				//zqiu: return 0 when error
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.Security.ModeEnabled", pathVal);

            HDK_Enum_PN_WiFiSecurity *eWiFiSecurity;
            eWiFiSecurity = HDK_Get_PN_WiFiSecurity(pStruct, HDK_Element_PN_Type);

            if (eWiFiSecurity == NULL)
                return 0;
            switch (*eWiFiSecurity)
            {
                case HDK_Enum_PN_WiFiSecurity_NONE:
                    if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_STRING, "None", 0) != 0)
                        return 0;
                    break;

                case HDK_Enum_PN_WiFiSecurity_WEP_64:
                    if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_STRING, "WEP-64", 0) != 0)
                        return 0;
                    break;

                case HDK_Enum_PN_WiFiSecurity_WEP_128:
                    if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_STRING, "WEP-128", 0) != 0)
                        return 0;
                    break;

                case HDK_Enum_PN_WiFiSecurity_WPA_Personal:
                    if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_STRING, "WPA-Personal", 0) != 0)
                        return 0;
                    break;

                case HDK_Enum_PN_WiFiSecurity_WPA2_Personal:
                    if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_STRING, "WPA2-Personal", 0) != 0)
                        return 0;
                    break;

                case HDK_Enum_PN_WiFiSecurity_WPA_WPA2_Personal:
                    if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_STRING, "WPA-WPA2-Personal", 0) != 0)
                        return 0;
                    break;

                case HDK_Enum_PN_WiFiSecurity_WPA_Enterprise:
                    if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_STRING, "WPA-Enterprise", 0) != 0)
                        return 0;
                    break;

                case HDK_Enum_PN_WiFiSecurity_WPA2_Enterprise:
                    if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_STRING, "WPA2-Enterprise", 0) != 0)
                        return 0;
                    break;

                case HDK_Enum_PN_WiFiSecurity_WPA_WPA2_Enterprise:
                    if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_STRING, "WPA-WPA2-Enterprise", 0) != 0)
                        return 0;
                    break;

                default:
                    return 0;
            }
            //Set wifi encryption
            snprintf(tmpPath, sizeof(tmpPath), "%s.Security.X_CISCO_COM_EncryptionMethod", pathVal);
            HDK_Enum_PN_WiFiEncryption *eWiFiEncryption;
            eWiFiEncryption = HDK_Get_PN_WiFiEncryption(pStruct, HDK_Element_PN_Encryption);

            if (eWiFiEncryption == NULL)
                return 0;

            switch (*eWiFiEncryption)
            {
                case HDK_Enum_PN_WiFiEncryption_AES:
                    if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_STRING, "AES", 0) != 0)
                        return 0;
                    break;

                case HDK_Enum_PN_WiFiEncryption_TKIP:
                    if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_STRING, "TKIP", 0) != 0)
                        return 0;
                    break;

                case HDK_Enum_PN_WiFiEncryption_TKIPORAES:
                    if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_STRING, "AES+TKIP", 0) != 0)
                        return 0;
                    break;

                default:
					if(*eWiFiSecurity == HDK_Enum_PN_WiFiSecurity_WPA2_Personal || 
					   *eWiFiSecurity == HDK_Enum_PN_WiFiSecurity_WPA_Personal || 
					   *eWiFiSecurity == HDK_Enum_PN_WiFiSecurity_WPA_WPA2_Personal)
					{
						log_printf(LOG_ERR, "Unknown encryption type\n");
						return 0;
					}
            }
            break;

        case HDK_DeviceValue_WLanKey:
            strVal = HDK_Get_String(pStruct, HDK_Element_PN_RadioID);

            if (strVal == NULL)
                return 0;

            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiRadio", pathVal, sizeof(pathVal)) != 0)
            {
                log_printf(LOG_ERR, "Can't get multilan Home wifi radio\n");
                //return -1;
				//zqiu: return 0 when error
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.Alias", pathVal);

            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;

            if (strcmp(val, strVal))
            {
                log_printf(LOG_ERR, "error input Radio ID\n");
                return 0;
            }

            boolVal = HDK_Get_Bool(pStruct, HDK_Element_PN_Enabled);

            if(*boolVal == false)
            {
                snprintf(tmpPath, sizeof(tmpPath), "%s.X_CISCO_COM_ApplySetting", pathVal);
                if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_BOOL, "true", 1) != 0)
                {
                    log_printf(LOG_ERR, "Can't apply Home Security WiFi radio settings\n");
                    return 0;
                }

                break;
            }

            eWiFiSecurity = HDK_Get_PN_WiFiSecurity(pStruct, HDK_Element_PN_Type);

            if (eWiFiSecurity == NULL)
                return 0;
			
            if (*eWiFiSecurity == HDK_Enum_PN_WiFiSecurity_NONE || *eWiFiSecurity == HDK_Enum_PN_WiFiSecurity_WPA_WPA2_Enterprise
                || *eWiFiSecurity == HDK_Enum_PN_WiFiSecurity_WPA_Enterprise || *eWiFiSecurity == HDK_Enum_PN_WiFiSecurity_WPA2_Enterprise)
            {
                snprintf(tmpPath, sizeof(tmpPath), "%s.X_CISCO_COM_ApplySetting", pathVal);
                if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_BOOL, "true", 1) != 0)
                {
                    log_printf(LOG_ERR, "Can't apply Home Security WiFi radio settings\n");
                    return 0;
                }
                break;
            }

            strVal = HDK_Get_String(pStruct, HDK_Element_PN_Key);

            if (strVal == NULL)
                return 0;

            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiAp", pathVal, sizeof(pathVal)) != 0)
            {
                log_printf(LOG_ERR, "Can't get multilan Home wifi access point\n");
                //return -1;
				//zqiu: return 0 when error
                return 0;
            }

            if (!strncmp(strVal, "WEP-", strlen("WEP-")))
            {
				if (*eWiFiSecurity == HDK_Enum_PN_WiFiSecurity_WEP_64)
				{
                                        snprintf(tmpPath, sizeof(tmpPath), "%s.Security.X_CISCO_COM_WEPKey64Bit.1.WEPKey", pathVal);
					if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_STRING, strVal + 4, 0) != 0)
						return 0;
				}
				else if (*eWiFiSecurity == HDK_Enum_PN_WiFiSecurity_WEP_128)
				{
                                        snprintf(tmpPath, sizeof(tmpPath), "%s.Security.X_CISCO_COM_WEPKey128Bit.1.WEPKey", pathVal);
					if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_STRING, strVal + 4, 0) != 0)
						return 0;
				}
				else
				{
					log_printf(LOG_ERR, "Invalid encryption key\n");
					return 0;
				}
            }
            else if (!strncmp(strVal, "WPA-", strlen("WPA-")))
            {
				if (*eWiFiSecurity == HDK_Enum_PN_WiFiSecurity_WEP_64 || *eWiFiSecurity == HDK_Enum_PN_WiFiSecurity_WEP_128)
				{
					log_printf(LOG_ERR, "Invalid encryption key\n");
					return 0;
				}

                snprintf(tmpPath, sizeof(tmpPath), "%s.Security.X_COMCAST-COM_KeyPassphrase", pathVal);
                if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_STRING, strVal + 4, 0) != 0)
                    return 0;
		snprintf(tmpPath, sizeof(tmpPath), "%s.Security.X_COMCAST-COM_KeyPassphrase", "Device.WiFi.AccessPoint.4");
		if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_STRING, strVal + 4, 0) != 0)
                    return 0;
            }
            else
                return 0;
			
            snprintf(tmpPath, sizeof(tmpPath), "%s.", pathVal);
                
            if (MBus_Commit(mbus, tmpPath) != 0)
            {
                log_printf(LOG_ERR, "commit %s failed\n", tmpPath);
                return 0;
            }

            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityWiFiRadio", pathVal, sizeof(pathVal)) != 0)
            {
                log_printf(LOG_ERR, "Can't get multilan Home wifi radio\n");
                //return -1;
				//zqiu: return 0 when error
                return 0;
            }

            snprintf(tmpPath, sizeof(tmpPath), "%s.X_CISCO_COM_ApplySetting", pathVal);
            if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_BOOL, "true", 1) != 0)
            {
                log_printf(LOG_ERR, "Can't apply Home Security WiFi radio settings\n");
                return 0;
            }
            break;
                
        case HDK_DeviceValue_RemotePort:
            intVal = HDK_Get_Int(pStruct, HDK_Element_PN_RemotePort);

            if (intVal == NULL)
                return 0;
			sprintf(tmpBuf, "%u", *intVal);
            snprintf(tmpPath, MAX_PATH_NAME, "Device.UserInterface.X_CISCO_COM_RemoteAccess.HttpEnable");
            if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                return 0;
            
			if (!strcmp(val, "true"))
            {
                // if http is enable, we think this will take effect on it.
                if (MBus_SetParamVal(mbus, "Device.UserInterface.X_CISCO_COM_RemoteAccess.HttpPort", MBUS_PT_UINT, tmpBuf, 1) != 0)
                    return 0;
            }
            else
            {
                snprintf(tmpPath, MAX_PATH_NAME, "Device.UserInterface.X_CISCO_COM_RemoteAccess.HttpsEnable");
                if (MBus_GetParamVal(mbus, tmpPath, val, sizeof(val)) != 0)
                    return 0;

                if (!strcmp(val, "true"))
                {
                    // if https is enable, we think this will take effect on it.
                    if (MBus_SetParamVal(mbus, "Device.UserInterface.X_CISCO_COM_RemoteAccess.HttpsPort", MBUS_PT_UINT, tmpBuf, 1) != 0)
                        return 0;
                }
                else
                {
                    // if both are disabled, set http 
                    if (MBus_SetParamVal(mbus, "Device.UserInterface.X_CISCO_COM_RemoteAccess.HttpPort", MBUS_PT_UINT, tmpBuf, 1) != 0)
                        return 0;
                }    
            }
            break;
        case HDK_DeviceValue_RemoteSSL:
            //only use this to control http remote access
            boolVal = HDK_Get_Bool(pStruct, HDK_Element_PN_RemoteSSL);

            if (boolVal == NULL)
                return 0;

            if (*boolVal == false)
                strVal = "false";
            else if (*boolVal == true)
                strVal = "true";
            else
                return 0;

            if (MBus_SetParamVal(mbus, "Device.UserInterface.X_CISCO_COM_RemoteAccess.HttpsEnable", MBUS_PT_BOOL, strVal, 1) != 0)
                return 0;
            break;
        case HDK_DeviceValue_ManageRemote:
            //only use this to control http remote access
            boolVal = HDK_Get_Bool(pStruct, HDK_Element_PN_ManageRemote);

            if (boolVal == NULL)
                return 0;

            if (*boolVal == false)
                strVal = "false";
            else if (*boolVal == true)
                strVal = "true";
            else
                return 0;
            
            if (MBus_SetParamVal(mbus, "Device.UserInterface.X_CISCO_COM_RemoteAccess.HttpEnable", MBUS_PT_BOOL, strVal, 1) != 0)
                return 0;
            break;
            
        case HDK_DeviceValue_DeviceName:
            /* no support now */
            break;

        case HDK_DeviceValue_AdminPassword:
            /* if the user "admin" is no exist, we add it */
            insNum = NELEMS(insPath);
            if (MBus_FindObjectIns(mbus, "Device.Users.User.", 
                        "Username", "admin", insPath, &insNum) != 0
                    || insNum == 0)
            {
                if (MBus_AddObjectIns(mbus, "Device.Users.User.", &newIns) != 0)
                    return 0;

                snprintf(tmpPath, MAX_PATH_NAME, "Device.Users.User.%d.Username", newIns);
                if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_STRING, "admin", 1) != 0)
                    return 0;

                snprintf(insPath[0], MAX_PATH_NAME, "Device.Users.User.%d.", newIns);
            }

            if ((strVal = HDK_Get_String(pStruct, HDK_Element_PN_AdminPassword)) == NULL)
                return 0;
            snprintf(tmpPath, sizeof(tmpPath), "%sX_CISCO_COM_Password", insPath[0]);
            if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_STRING, strVal, 1) != 0)
                return 0;

            break;

        case HDK_DeviceValue_Username:
            /* username can't be set */
            break;

        case HDK_DeviceValue_TimeZone:
			/*
            if ((strVal = HDK_Get_String(pStruct, HDK_Element_PN_TimeZone)) == NULL)
                return 0;

            if (MBus_SetParamVal(mbus, "Device.Time.LocalTimeZone", MBUS_PT_STRING, strVal, 1)!= 0)
                return 0;*/
            break;

        case HDK_DeviceValue_AutoAdjustDST:
            if ((boolVal = HDK_Get_Bool(pStruct, HDK_Element_PN_AutoAdjustDST)) == NULL)
                return 0;

            /* no TR-181 parameter found */
            break;

        case HDK_DeviceValue_SSL:
            if ((boolVal = HDK_Get_Bool(pStruct, HDK_Element_PN_SSL)) == NULL)
                return 0;

            if (*boolVal == false)
                strVal = "false";
            else if (*boolVal == true)
                strVal = "true";
            else
                return 0;

            if (MBus_SetParamVal(mbus, "Device.UserInterface.X_CISCO_COM_RemoteAccess.HttpsEnable", MBUS_PT_BOOL, strVal, 1) != 0)
                return 0;
            break;

        case HDK_DeviceValue_Locale:
            if ((strVal = HDK_Get_String(pStruct, HDK_Element_PN_Locale)) == NULL)
                return 0;
            break;

        case HDK_DeviceValue_RenewWanTimeout:
            /*
            switch (GetWanType(mbus))
            {
                case HDK_Enum_PN_WANType_StaticPPPoE:
                    MBus_SetParamVal(mbus, "Device.PPP.Interface.1.Reset", 
                                MBUS_PT_BOOL, "1", 1);
                    break;

                case HDK_Enum_PN_WANType_DHCPPPPoE:
                    MBus_SetParamVal(mbus, "Device.PPP.Interface.1.Reset", 
                                MBUS_PT_BOOL, "1", 1);
                    MBus_SetParamVal(mbus, "Device.DHCPv4.Client.1.Renew", 
                                MBUS_PT_BOOL, "1", 1);
                    break;

                case HDK_Enum_PN_WANType_DHCP:
                    MBus_SetParamVal(mbus, "Device.DHCPv4.Client.1.Renew", 
                                MBUS_PT_BOOL, "1", 1);
                    break;

                case HDK_Enum_PN_WANType_Static:
                case HDK_Enum_PN_WANType_UNKNOWN:
                default:
                    syslog(LOG_ERR, "%s: can't renew wan time out", __FUNCTION__);
                    break;
            }
            */
            /* TR-181 parameter not found */
            break;

        case HDK_DeviceValue_LanIPAddress:
            if ((ipAddr = HDK_Get_IPAddress(pStruct, HDK_Element_PN_RouterIPAddress)) == NULL)
                return 0;

            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityDHCPv4ServerPool", pathVal, sizeof(pathVal)) != 0)
            {   
                log_printf(LOG_ERR, "Can't get multilan Home server pool\n");
                //return -1;
				//zqiu: return 0 when error
                return 0; 
            }   
		
            snprintf(tmpPath, sizeof(tmpPath), "%s.IPRouters", pathVal);
            snprintf(tmpBuf, sizeof(tmpBuf), "%u.%u.%u.%u", 
                    ipAddr->a, ipAddr->b, ipAddr->c, ipAddr->d);

            if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_STRING, tmpBuf, 1) != 0)
                return 0;

            break;

        case HDK_DeviceValue_LanSubnetMask:
            if ((ipAddr = HDK_Get_IPAddress(pStruct, HDK_Element_PN_RouterSubnetMask)) == NULL)
                return 0;

            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityDHCPv4ServerPool", pathVal, sizeof(pathVal)) != 0)
            {   
                log_printf(LOG_ERR, "Can't get multilan Home server pool\n");
                //return -1;
				//zqiu: return 0 when error
                return 0; 
            }   

            snprintf(tmpPath, sizeof(tmpPath), "%s.SubnetMask", pathVal);
            snprintf(tmpBuf, sizeof(tmpBuf), "%u.%u.%u.%u", ipAddr->a, ipAddr->b, ipAddr->c, ipAddr->d);
			
            if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_STRING, tmpBuf, 1) != 0)
                return 0;
            break;

        case HDK_DeviceValue_DHCPEnabled:
			/*
            if ((intVal = HDK_Get_Int(pStruct, HDK_Element_PN_DHCPServerEnabled)) == NULL)
                return 0;

            if (*intVal)
                strVal = "true";
            else
                strVal = "false";

            if (MBus_SetParamVal(mbus, "Device.DHCPv4.Server.Enable", 
                        MBUS_PT_BOOL, strVal, 1) != 0)
                return 0;

            if (!FindFirstInstance(mbus, "Device.DHCPv4.Server.Pool.", tmpBuf, sizeof(tmpBuf)))
                return 0;

            snprintf(tmpPath, sizeof(tmpPath), "%sEnable", tmpBuf);
            if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_BOOL, strVal, 1) != 0)
                ;//return 0; XXX: Data Model/Back-End will fail

            break;*/

        case HDK_DeviceValue_DHCPFirstIP:
            if ((ipAddr = HDK_Get_IPAddress(pStruct, HDK_Element_PN_IPAddressFirst)) == NULL)
                return 0;

            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityDHCPv4ServerPool", pathVal, sizeof(pathVal)) != 0)
            {   
                log_printf(LOG_ERR, "Can't get multilan Home server pool\n");
                //return -1;
				//zqiu: return 0 when error
                return 0; 
            }   

            snprintf(tmpPath, sizeof(tmpPath), "%s.MinAddress", pathVal);
            snprintf(tmpBuf, sizeof(tmpBuf), "%u.%u.%u.%u", ipAddr->a, ipAddr->b, ipAddr->c, ipAddr->d);

            if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_STRING, tmpBuf, 1) != 0)
                return 0;
            break;

        case HDK_DeviceValue_DHCPLastIP:
            if ((ipAddr = HDK_Get_IPAddress(pStruct, HDK_Element_PN_IPAddressLast)) == NULL)
                return 0;

            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityDHCPv4ServerPool", pathVal, sizeof(pathVal)) != 0)
            {   
                log_printf(LOG_ERR, "Can't get multilan Home server pool\n");
                //return -1;
				//zqiu: return 0 when error
                return 0; 
            }   

            snprintf(tmpPath, sizeof(tmpPath), "%s.MaxAddress", pathVal);
            snprintf(tmpBuf, sizeof(tmpBuf), "%u.%u.%u.%u", ipAddr->a, ipAddr->b, ipAddr->c, ipAddr->d);

            if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_STRING, tmpBuf, 1) != 0)
                return 0;
            break;

        case HDK_DeviceValue_DHCPLeaseTime:
            if ((intVal = HDK_Get_Int(pStruct, HDK_Element_PN_LeaseTime)) == NULL)
                return 0;

            snprintf(val, sizeof(val), "%d", *intVal);

            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityDHCPv4ServerPool", pathVal, sizeof(pathVal)) != 0)
            {   
                log_printf(LOG_ERR, "Can't get multilan Home server pool\n");
                //return -1;
				//zqiu: return 0 when error
                return 0; 
            }   

            snprintf(tmpPath, sizeof(tmpPath), "%s.LeaseTime", pathVal);
            if (MBus_SetParamVal(mbus, tmpPath, MBUS_PT_INT, val, 1) != 0)
                return 0;
            break;

        case HDK_DeviceValue_DHCPReservations:
            //Get instance 
            if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityDHCPv4ServerPool", pathVal, sizeof(pathVal)) != 0)
            {   
                log_printf(LOG_ERR, "Can't get multilan Home server pool\n");
                //return -1;
				//zqiu: return 0 when error
                return 0; 
            }   

            snprintf(tmpPath, sizeof(tmpPath), "%s.StaticAddress.", pathVal);

            if ((pStr1 = HDK_Get_Struct(pStruct, HDK_Element_PN_DHCPReservations)) == NULL)
                return 0;

            val[0] = '\0';
            for (pMember = pStr1->pHead; pMember != NULL; pMember = pMember->pNext)
            {
                if ((pStr2 = HDK_Get_StructMember(pMember)) == NULL)
                    continue;

                if (MBus_AddObjectIns(mbus, tmpPath, &insNum))
                {
                    log_printf(LOG_ERR, "adding dhcp reservation failed\n");
                    return 0;
                }
                sprintf(path, "%s.StaticAddress.%d.Yiaddr", pathVal, insNum);

                if ((ipAddr = HDK_Get_IPAddress(pStr2, HDK_Element_PN_IPAddress)) == NULL)
                    return 0;;

                snprintf(ipBuf, sizeof(ipBuf), "%u.%u.%u.%u", ipAddr->a, ipAddr->b, ipAddr->c, ipAddr->d);

                if (MBus_SetParamVal(mbus, path, MBUS_PT_STRING, ipBuf, 0) != 0)
                    return 0;
                sprintf(path, "%s.StaticAddress.%d.Chaddr", pathVal, insNum);
                if ((macAddr = HDK_Get_MACAddress(pStr2, HDK_Element_PN_MacAddress)) == NULL)
                    return 0;;

                snprintf(macBuf, sizeof(ipBuf), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", macAddr->a, macAddr->b, macAddr->c, macAddr->d, macAddr->e, macAddr->f);

                if (MBus_SetParamVal(mbus, path, MBUS_PT_STRING, macBuf, 0) != 0)
                    return 0;
                sprintf(path, "%s.StaticAddress.%d.X_CISCO_COM_DeviceName", pathVal, insNum);
                if ((strVal = HDK_Get_String(pStr2, HDK_Element_PN_DeviceName)) == NULL)
                    return 0;;

                if (MBus_SetParamVal(mbus, path, MBUS_PT_STRING, strVal, 0) != 0)
                    return 0;
            }
            if (MBus_Commit(mbus, tmpPath) != 0)
            {
                log_printf(LOG_ERR, "commit dhcp reservations failed\n");
                return 0;
            }
            break;
		
		case HDK_DeviceValue_BridgeConnect:
			memset(&time_val, 0, sizeof(time_val));
			char buf[4];
			//only support ethernet port 4
			intVal = HDK_Get_Int(pStruct, HDK_Element_PN_EthernetPort);

			if(intVal == NULL || (*intVal != 0 && *intVal != 4))
			{
				log_printf(LOG_ERR, "Error ethernet port\n");
				return 0;
			}

			memset(buf, 0, sizeof(buf));
			syscfg_get(NULL, "HomeSecurityEthernet4Flag", buf, sizeof(buf));

			if (!strcmp(buf, "1"))
			{
				setitimer(ITIMER_REAL, &time_val, NULL);
				break;
			}
			
			intVal = HDK_Get_Int(pStruct, HDK_Element_PN_Minutes);

			if(intVal == NULL)
			{
				log_printf(LOG_ERR, "Error bridge valid life\n");
				return 0;
			}

			if(*intVal == 0)
			{
				//disable Home Security bridge
				setitimer(ITIMER_REAL, &time_val, NULL);

				if(HDK_Stop_HomeSecurity_Bridge(mbus))
					log_printf(LOG_ERR, "Stop eth4 home security vlan failed\n");
			}
			else
			{
				setitimer(ITIMER_REAL, &time_val, NULL);
				time_val.it_value.tv_sec = *intVal * 60;
				setitimer(ITIMER_REAL, &time_val, NULL);
				
				if(HDK_Start_HomeSecurity_Bridge(mbus))
					log_printf(LOG_ERR, "Start eth4 home security vlan failed\n");
			}

			break;

        case HDK_DeviceValue_WanType:
            break;

        case HDK_DeviceValue_WanMTU:
            break;

        case HDK_DeviceValue_WanDNSSettings:
            // TODO:
            break;

        case HDK_DeviceValue_MacAddress:
            /* CANNOT be set in TR-181 */
            break;

        case HDK_DeviceValue_WanUsername:
            break;

        case HDK_DeviceValue_WanPassword:
            break;

        case HDK_DeviceValue_WanMaxIdleTime:
            break;

        case HDK_DeviceValue_WanAutoReconnect:
            break;

        case HDK_DeviceValue_WanAuthService:
            /* not supported */
            break;

        case HDK_DeviceValue_WanPPPoEService:
            break;

        case HDK_DeviceValue_WanLoginService:
            /* not supported */
            break;

        case HDK_DeviceValue_WanIP:
            /* not supported, as TES301 */
            break;

        case HDK_DeviceValue_WanSubnetMask:
            /* not supported, as TES301 */
            break;

        case HDK_DeviceValue_WanGateway:
            /* not supported, as TES301 */
            break;

        default:
            log_printf(LOG_ERR, " unknown type\n");
            return 0;
    }

    return 1;
}

/* return 1 if valid and 0 if invalid */
int HDK_Device_ValidateValue(void* pDeviceCtx, HDK_DeviceValue eValue, HDK_Struct* pStruct)
{
    HDK_Context *hdkCtx;
    MBusObj_t *mbus;
    char *val;
    int *intval;
    /* float timeOfs; */
	HDK_Enum_PN_IPProtocol *ip_prot;

    if ((hdkCtx = (HDK_Context *)pDeviceCtx) == NULL || (mbus = hdkCtx->mbus) == NULL || !pStruct)
    {
        return 0;
    }

    switch (eValue)
    {
        case HDK_DeviceValue_PortMappings:
            ip_prot = HDK_Get_PN_IPProtocol(pStruct, HDK_Element_PN_PortMappingProtocol);

            if (ip_prot == NULL || (*ip_prot != HDK_Enum_PN_IPProtocol_UDP && *ip_prot != HDK_Enum_PN_IPProtocol_TCP))
			{
                return 0;
			}
            break;
		case HDK_DeviceValue_WLanSSID:
			break;
        case HDK_DeviceValue_WLanKeyRenewal:
            intval = HDK_Get_Int(pStruct, HDK_Element_PN_KeyRenewal);
            if (intval == NULL)
                return 0;
            if (*intval < 0)
                return 0;
            break;
        case HDK_DeviceValue_DeviceName:
            val = HDK_Get_String(pStruct, HDK_Element_PN_DeviceName);
            if (!val || strlen(val) == 0 || strlen(val) > MAX_DEVNAME_LEN)
            {
                log_printf(LOG_ERR, "Deivce name is invalid");
                return 0;
            }
            break;

        case HDK_DeviceValue_AdminPassword:
            val = HDK_Get_String(pStruct, HDK_Element_PN_AdminPassword);
            if (!val  || strlen(val) == 0 || strlen(val) > MAX_PASSWD_LEN)
            {
                log_printf(LOG_ERR, "Admin password is invalid");
                return 0;
            }
            break;

        case HDK_DeviceValue_TimeZone:
			/*
            val = HDK_Get_String(pStruct, HDK_Element_PN_TimeZone);
            if (!val || sscanf(val, "UTC%f", &timeOfs) != 1)
            {
                log_printf(LOG_ERR, "TimeZone is invalid");
                return 0;
            }

            timeOfs *= 60;
            if (timeOfs < -7200 || timeOfs > 7800 || timeOfs % 30)
            {
                log_printf(LOG_ERR, "TimeZone is invalid");
                return 0;
            }
			*/
            break;

        case HDK_DeviceValue_Locale:
            val = HDK_Get_String(pStruct, HDK_Element_PN_Locale);
            if (!val)
            {
                return 0;
            }

            if (strcmp(val, "en-US") && strcmp(val, ""))
            {
                //usg not support locale, but we know it will only be deployed in US
                return 0;
            }
            break;

        case HDK_DeviceValue_LanIPAddress:
            /* TES must be 192.168.0.xxx, but is's not needed */
            break;

        case HDK_DeviceValue_LanSubnetMask:
            /* TES must be 255.255.255.0, ... , but is's not needed */
            break;

        case HDK_DeviceValue_DHCPEnabled:
            break;

        case HDK_DeviceValue_DHCPFirstIP:
            /* TES must be 192.168.0.xxx, but is's not needed */
            break;

        case HDK_DeviceValue_DHCPLastIP:
            /* TES must be 192.168.0.xxx, but is's not needed */
            break;

        case HDK_DeviceValue_DHCPLeaseTime:
            break;

        case HDK_DeviceValue_DHCPReservations:
            /* TES must be 192.168.0.xxx, but is's not needed */
            break;

        case HDK_DeviceValue_Username:
        case HDK_DeviceValue_SSL:
        case HDK_DeviceValue_RenewWanTimeout:
        case HDK_DeviceValue_AutoAdjustDST:
        case HDK_DeviceValue_WanType:
        case HDK_DeviceValue_WanMTU:
        case HDK_DeviceValue_WanUsername:
        case HDK_DeviceValue_WanPassword:
        case HDK_DeviceValue_WanMaxIdleTime:
        case HDK_DeviceValue_WanAutoReconnect:
        case HDK_DeviceValue_WanAuthService:
        case HDK_DeviceValue_WanPPPoEService:
        case HDK_DeviceValue_WanLoginService:
        case HDK_DeviceValue_WanIP:
        case HDK_DeviceValue_WanSubnetMask:
        case HDK_DeviceValue_WanGateway:
        case HDK_DeviceValue_WanDNSSettings:
        case HDK_DeviceValue_MacAddress:
        case HDK_DeviceValue_DomainName:
        case HDK_DeviceValue_WiredQoS:
		case HDK_DeviceValue_WLanSecurityEnabled:
		case HDK_DeviceValue_WLanType:
		case HDK_DeviceValue_WLanEncryption:
		case HDK_DeviceValue_WLanKey:
		case HDK_DeviceValue_WLanRadiusIP1:
		case HDK_DeviceValue_WLanRadiusIP2:
		case HDK_DeviceValue_WLanRadiusPort1:
		case HDK_DeviceValue_WLanRadiusPort2:
		case HDK_DeviceValue_WLanRadiusSecret1:
		case HDK_DeviceValue_WLanRadiusSecret2:
		case HDK_DeviceValue_WLanEnabled:
		case HDK_DeviceValue_WLanMode:
		case HDK_DeviceValue_WLanSSIDBroadcast:
		case HDK_DeviceValue_WLanChannelWidth:
		case HDK_DeviceValue_WLanChannel:
		case HDK_DeviceValue_WLanQoS:
		case HDK_DeviceValue_WLanSecondaryChannel:
            break;

        default:
            return 0;
    }

    return 1;
}

int HDK_Stop_HomeSecurity_Bridge(MBusObj_t *ccsp_bus)
{
	char val[MAX_BUF];
	char path[MAX_BUF];

	if(ccsp_bus == NULL)
	{
		log_printf(LOG_ERR, "ccsp bus handle is empty\n");
		return -1;
	}

	//Get Home Security bridge instance 
	if (MBus_GetParamVal(ccsp_bus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityBridge", path, sizeof(path)) != 0)
	{
		log_printf(LOG_ERR, "Can't get multilan Home Security bridge\n");
		return -1;
	}
	strcat(path, ".Port.");
	//Get port 4 bridge port number
	if (MBus_GetParamVal(ccsp_bus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityBridgePorts", path + strlen(path), sizeof(path) - strlen(path)) != 0)
	{
		log_printf(LOG_ERR, "Can't get port number in Home Security bridge\n");
		return -1;
	}
	
	strcat(path, ".Enable");
	if (MBus_GetParamVal(ccsp_bus, path, val, sizeof(val)) != 0)
	{
		log_printf(LOG_ERR, "Can't get bridge 2 port enable or not\n");
		return -1;
	}

	if (!strcmp(val, "true"))
	{
		if (MBus_SetParamVal(ccsp_bus, path, MBUS_PT_BOOL, "false", 1) != 0)
		{
			log_printf(LOG_ERR, "Can't detach port from Home Security VLAN\n")
			return -1;
		}
	}


	//Get home bridge instance 
	if (MBus_GetParamVal(ccsp_bus, "Device.X_CISCO_COM_MultiLAN.PrimaryLANBridge", path, sizeof(path)) != 0)
	{
		log_printf(LOG_ERR, "Can't get multilan Home bridge\n");
		return -1;
	}
	strcat(path, ".Port.");
	//Get port 4 bridge port number
	if (MBus_GetParamVal(ccsp_bus, "Device.X_CISCO_COM_MultiLAN.PrimaryLANBridgeHSPorts", path + strlen(path), sizeof(path) - strlen(path)) != 0)
	{
		log_printf(LOG_ERR, "Can't get port number in Home bridge\n");
		return -1;
	}
	
	strcat(path, ".Enable");

	if (MBus_GetParamVal(ccsp_bus, path, val, sizeof(val)) != 0)
	{
		log_printf(LOG_ERR, "Can't get bridge 1 port enable or not\n");
		return -1;
	}

	if (!strcmp(val, "false"))
	{
		if (MBus_SetParamVal(ccsp_bus, path, MBUS_PT_BOOL, "true", 1) != 0)
		{
			log_printf(LOG_ERR, "Can't attach port to home VLAN\n")
			return -1;
		}
	}
	return 0;
}

int HDK_Start_HomeSecurity_Bridge(MBusObj_t *ccsp_bus)
{
	char val[MAX_BUF];
	char path[MAX_BUF];

	if(ccsp_bus == NULL)
	{
		log_printf(LOG_ERR, "ccsp bus handle is empty\n");
		return -1;
	}

	//Get home bridge instance 
	if (MBus_GetParamVal(ccsp_bus, "Device.X_CISCO_COM_MultiLAN.PrimaryLANBridge", path, sizeof(path)) != 0)
	{
		log_printf(LOG_ERR, "Can't get multilan Home bridge\n");
		return -1;
	}
	strcat(path, ".Port.");
	//Get port 4 bridge port number
	if (MBus_GetParamVal(ccsp_bus, "Device.X_CISCO_COM_MultiLAN.PrimaryLANBridgeHSPorts", path + strlen(path), sizeof(path) - strlen(path)) != 0)
	{
		log_printf(LOG_ERR, "Can't get port number in Home bridge\n");
		return -1;
	}
	
	strcat(path, ".Enable");

	if (MBus_GetParamVal(ccsp_bus, path, val, sizeof(val)) != 0)
	{
		log_printf(LOG_ERR, "Can't get bridge 1 port enable or not\n");
		return -1;
	}

	if (!strcmp(val, "true"))
	{
		if (MBus_SetParamVal(ccsp_bus, path, MBUS_PT_BOOL, "false", 1) != 0)
		{
			log_printf(LOG_ERR, "Can't detach port from home VLAN\n")
			return -1;
		}
	}

	//Get Home Security bridge instance 
	if (MBus_GetParamVal(ccsp_bus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityBridge", path, sizeof(path)) != 0)
	{
		log_printf(LOG_ERR, "Can't get multilan Home Security bridge\n");
		return -1;
	}

	strcat(path, ".Port.");

	//Get port 4 bridge port number
	if (MBus_GetParamVal(ccsp_bus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityBridgePorts", path + strlen(path), sizeof(path) - strlen(path)) != 0)
	{
		log_printf(LOG_ERR, "Can't get port number in Home Security bridge\n");
		return -1;
	}
	
	strcat(path, ".Enable");
	if (MBus_GetParamVal(ccsp_bus, path, val, sizeof(val)) != 0)
	{
		log_printf(LOG_ERR, "Can't get bridge 2 port enable or not\n");
		return -1;
	}

	if (!strcmp(val, "false"))
	{
		if (MBus_SetParamVal(ccsp_bus, path, MBUS_PT_BOOL, "true", 1) != 0)
		{
			log_printf(LOG_ERR, "Can't attach port to Home Security VLAN\n")
			return -1;
		}
	}
	return 0;
}
#endif
