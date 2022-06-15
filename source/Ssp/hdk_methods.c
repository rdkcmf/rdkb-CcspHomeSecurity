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
 * hdk_methods.c - HNAP callback function definitions
 */

#include <stdio.h>
#include <stdlib.h>
#include "hdk_data.h"
#include "hdk_methods.h"
#include "hdk_adi.h"
#include "chs_log.h"
#include <string.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/reboot.h>
#include <linux/reboot.h>
#include <sys/types.h>



#if defined(HDK_METHOD_CISCO_HOTSPOT_CHECKPARENTALCONTROLSPASSWORD) || defined(HDK_METHOD_CISCO_HOTSPOT_RESETPARENTALCONTROLSPASSWORD) || defined(HDK_METHOD_CISCO_HOTSPOT_SETPARENTALCONTROLSPASSWORD)

static int checkPassword(const char* pass)
{
	int len = strlen(pass);
	int i = 0;
	char c;

	if(len <4 || len >32)
		return -1;

	for(i = 0; i < len; i++)
	{
	    c = pass[i];
		if(!((c >= 'A' && c <= 'Z' ) || 
			(c >= 'a' && c <= 'z') ||
			(c >= '0' && c <= '9')))
        {
			return -1;
		}
	}

	return 0;
}

#endif
/*
 * http://cisco.com/HNAPExt/HND/ActivateTMSSS
 */
#ifdef HDK_METHOD_CISCO_HND_ACTIVATETMSSS

void HDK_Method_Cisco_HND_ActivateTMSSS(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pDeviceCtx;
    (void)pInput;
    (void)pOutput;
  
    /*
 * The Trend Micro service is not supported thus the result is ERROR 
 */
	HDK_Set_String(pOutput, HDK_Element_Cisco_HND_TAVSN, "LSZF-0012-4528-2124-2240");
}

#endif /* #ifdef HDK_METHOD_CISCO_HND_ACTIVATETMSSS */


/*
 * http://cisco.com/HNAPExt/HND/GetDefaultPolicySetting
 */
#ifdef HDK_METHOD_CISCO_HND_GETDEFAULTPOLICYSETTING

void HDK_Method_Cisco_HND_GetDefaultPolicySetting(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pDeviceCtx;
    (void)pInput;
    (void)pOutput;

    /*
 * The Trend Micro service is not supported thus the result is ERROR 
 */
    HDK_Set_Result(pOutput, HDK_Element_Cisco_HND_GetDefaultPolicySettingResult, HDK_Enum_Result_ERROR);

}

#endif /* #ifdef HDK_METHOD_CISCO_HND_GETDEFAULTPOLICYSETTING */

/*
 * http://cisco.com/HNAPExt/HND/GetPolicySettings
 */
#ifdef HDK_METHOD_CISCO_HND_GETPOLICYSETTINGS

void HDK_Method_Cisco_HND_GetPolicySettings(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pDeviceCtx;
    (void)pInput;
    (void)pOutput;


error:
    HDK_Set_Result(pOutput, HDK_Element_Cisco_HND_GetPolicySettingsResult, HDK_Enum_Result_ERROR);  
    return;

success:
   return;
}

#endif /* #ifdef HDK_METHOD_CISCO_HND_GETPOLICYSETTINGS */


/*
 * http://cisco.com/HNAPExt/HND/GetPolicySettings2
 */
#ifdef HDK_METHOD_CISCO_HND_GETPOLICYSETTINGS2

void HDK_Method_Cisco_HND_GetPolicySettings2(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pDeviceCtx;
    (void)pInput;
    (void)pOutput;
}

#endif /* #ifdef HDK_METHOD_CISCO_HND_GETPOLICYSETTINGS2 */


/*
 * http://cisco.com/HNAPExt/HND/GetPolicySettingsCapabilities
 */
#ifdef HDK_METHOD_CISCO_HND_GETPOLICYSETTINGSCAPABILITIES

void HDK_Method_Cisco_HND_GetPolicySettingsCapabilities(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pDeviceCtx;
    (void)pInput;
    (void)pOutput;

    int firstAccessPolicyNumber = 0; // not supported
    int maxPolicyNumber;
    int maxPolicyName;
    int maxAppliedDeviceList;
    int maxBlockedURLArray;
    int maxBlockedURLString;
    int maxBlockedKeywordArray;
    int maxBlockedKeywordString;
    int maxBlockedCategoryArray = 0; // not supported

    /*
 *  fecth max numbers in TES301;
 */
    
    maxPolicyNumber = FW_TR_MAX_NUM;
    maxPolicyName = AVAGE_STR_LEN;
    maxAppliedDeviceList = FW_MAC_FILTER_LIST_NUM;
    maxBlockedURLArray = FW_URL_FILTER_MAX_NUM;
    maxBlockedURLString = GENERAL_STR_LEN;
    maxBlockedKeywordArray = FW_KEYWORD_FILTER_MAX_NUM;
    maxBlockedKeywordString = GENERAL_STR_LEN;

 
    HDK_Set_Int(pOutput, HDK_Element_Cisco_HND_FirstAccessPolicyNumber, firstAccessPolicyNumber); 
    HDK_Set_Int(pOutput, HDK_Element_Cisco_HND_MaxPolicyNumber, maxPolicyNumber); 

    HDK_Set_Int(pOutput, HDK_Element_Cisco_HND_MaxPolicyName, maxPolicyName); 
    HDK_Set_Int(pOutput, HDK_Element_Cisco_HND_MaxAppliedDeviceList, maxAppliedDeviceList); 
    HDK_Set_Int(pOutput, HDK_Element_Cisco_HND_MaxBlockedURLArray, maxBlockedURLArray); 
    HDK_Set_Int(pOutput, HDK_Element_Cisco_HND_MaxBlockedURLString, maxBlockedURLString); 
    HDK_Set_Int(pOutput, HDK_Element_Cisco_HND_MaxBlockedKeywordArray, maxBlockedKeywordArray); 
    HDK_Set_Int(pOutput, HDK_Element_Cisco_HND_MaxBlockedKeywordString, maxBlockedKeywordString); 
    HDK_Set_Int(pOutput, HDK_Element_Cisco_HND_MaxBlockedCategoryArray, maxBlockedCategoryArray); 
    
}

#endif /* #ifdef HDK_METHOD_CISCO_HND_GETPOLICYSETTINGSCAPABILITIES */


/*
 * http://cisco.com/HNAPExt/HND/GetPolicySettingsCapabilities2
 */
#ifdef HDK_METHOD_CISCO_HND_GETPOLICYSETTINGSCAPABILITIES2

void HDK_Method_Cisco_HND_GetPolicySettingsCapabilities2(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pDeviceCtx;
    (void)pInput;
    (void)pOutput;
}

#endif /* #ifdef HDK_METHOD_CISCO_HND_GETPOLICYSETTINGSCAPABILITIES2 */


/*
 * http://cisco.com/HNAPExt/HND/GetTMSSSLicense
 */
#ifdef HDK_METHOD_CISCO_HND_GETTMSSSLICENSE

void HDK_Method_Cisco_HND_GetTMSSSLicense(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pDeviceCtx;
    (void)pInput;
    (void)pOutput;
    
    
}

#endif /* #ifdef HDK_METHOD_CISCO_HND_GETTMSSSLICENSE */


/*
 * http://cisco.com/HNAPExt/HND/GetTMSSSSettings
 */
#ifdef HDK_METHOD_CISCO_HND_GETTMSSSSETTINGS

void HDK_Method_Cisco_HND_GetTMSSSSettings(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pDeviceCtx;
    (void)pInput;
    (void)pOutput;

    int wtpEnabled = 0;
    int wtpThreshold = 0;
    int pcEnabled = 1;    

    /*
 * check if parent control is enabled.
 */
//	pcEnabled = getEnabled();
	if(pcEnabled < 0)
	{
		HDK_Set_Result(pOutput, HDK_Element_Cisco_HND_GetTMSSSSettingsResult, HDK_Enum_Result_ERROR);
		return;
	}

    HDK_Set_Bool(pOutput, HDK_Element_Cisco_HND_WTPEnabled, wtpEnabled);
    HDK_Set_Int(pOutput, HDK_Element_Cisco_HND_WTPThreshold, wtpThreshold);
    HDK_Set_Bool(pOutput, HDK_Element_Cisco_HND_PCEnabled, pcEnabled);
}

#endif /* #ifdef HDK_METHOD_CISCO_HND_GETTMSSSSETTINGS */


/*
 * http://cisco.com/HNAPExt/HND/SetDefaultPolicySetting
 */
#ifdef HDK_METHOD_CISCO_HND_SETDEFAULTPOLICYSETTING

void HDK_Method_Cisco_HND_SetDefaultPolicySetting(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pDeviceCtx;
    (void)pInput;
    (void)pOutput;
 
    /**
 *  Trend Micro service is not supported, result is ERROR_SET_POLICY
 */
    HDK_Set_Result(pOutput, HDK_Element_Cisco_HND_SetDefaultPolicySettingResult, HDK_Enum_Result_ERROR_SET_ACCESSPOLICY);
}

#endif /* #ifdef HDK_METHOD_CISCO_HND_SETDEFAULTPOLICYSETTING */

/*
 * http://cisco.com/HNAPExt/HND/SetPolicySettings
 */
#ifdef HDK_METHOD_CISCO_HND_SETPOLICYSETTINGS

void HDK_Method_Cisco_HND_SetPolicySettings(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pDeviceCtx;
    (void)pInput;
    (void)pOutput;
  error:
    printf("========error in set policy======\n");
    HDK_Set_Result(pOutput, HDK_Element_Cisco_HND_SetPolicySettingsResult, result);  
    return;

success:
    return;
}

#endif /* #ifdef HDK_METHOD_CISCO_HND_SETPOLICYSETTINGS */


/*
 * http://cisco.com/HNAPExt/HND/SetPolicySettings2
 */
#ifdef HDK_METHOD_CISCO_HND_SETPOLICYSETTINGS2

void HDK_Method_Cisco_HND_SetPolicySettings2(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pDeviceCtx;
    (void)pInput;
    (void)pOutput;
}

#endif /* #ifdef HDK_METHOD_CISCO_HND_SETPOLICYSETTINGS2 */


/*
 * http://cisco.com/HNAPExt/HND/SetTMSSSSettings
 */
#ifdef HDK_METHOD_CISCO_HND_SETTMSSSSETTINGS

void HDK_Method_Cisco_HND_SetTMSSSSettings(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pDeviceCtx;
    (void)pInput;
    (void)pOutput;

    int pcEnabled = 0;
	int status;

    pcEnabled = HDK_Get_BoolEx(pInput, HDK_Element_Cisco_HND_PCEnabled, 0);

	printf("------enabled is: %d", pcEnabled);

    status = setEnabled(pcEnabled);
	if(status < 0)
	{
		HDK_Set_Result(pOutput, HDK_Element_Cisco_HND_SetTMSSSSettingsResult, HDK_Enum_Result_ERROR);
		return;
	}
 
    printf("----- pc enabled is: %d\n ", pcEnabled);    

    /*
 *  set pcEnabled into TES301
 */
}

#endif /* #ifdef HDK_METHOD_CISCO_HND_SETTMSSSSETTINGS */


/*
 * http://cisco.com/HNAPExt/HotSpot/AddWebGUIAuthExemption
 */
#ifdef HDK_METHOD_CISCO_HOTSPOT_ADDWEBGUIAUTHEXEMPTION

void HDK_Method_Cisco_HotSpot_AddWebGUIAuthExemption(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pDeviceCtx;
    (void)pInput;
    (void)pOutput;
}

#endif /* #ifdef HDK_METHOD_CISCO_HOTSPOT_ADDWEBGUIAUTHEXEMPTION */


/*
 * http://cisco.com/HNAPExt/HotSpot/CheckParentalControlsPassword
 */
#ifdef HDK_METHOD_CISCO_HOTSPOT_CHECKPARENTALCONTROLSPASSWORD

void HDK_Method_Cisco_HotSpot_CheckParentalControlsPassword(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pDeviceCtx;
    (void)pInput;
    (void)pOutput;

    char* password;
    char* oldpass = NULL;
	CAPI_RESULT_E result;

    password = HDK_Get_String(pInput, HDK_Element_Cisco_HotSpot_Password);

    if(checkPassword(password) < 0)
	{
		result = HDK_Enum_Result_ERROR_INCORRECT_PASSWORD;
		goto error;
	}
    /*
 *
 * get old parent control password
 *
 */ 

    oldpass = getPCPassword();

    if((strlen(oldpass) == 0) || strcmp(password, oldpass))
    {
        HDK_Set_Result(pOutput, HDK_Element_Cisco_HotSpot_CheckParentalControlsPasswordResult, HDK_Enum_Result_ERROR_INCORRECT_PASSWORD);
		goto error;
    }

	goto success;

error:
    HDK_Set_Result(pOutput, HDK_Element_Cisco_HotSpot_CheckParentalControlsPasswordResult, result);

success:
    free(oldpass);
	return;
}

#endif /* #ifdef HDK_METHOD_CISCO_HOTSPOT_CHECKPARENTALCONTROLSPASSWORD */


/*
 * http://cisco.com/HNAPExt/HotSpot/GetDeviceInfo
 */
#ifdef HDK_METHOD_CISCO_HOTSPOT_GETDEVICEINFO

void HDK_Method_Cisco_HotSpot_GetDeviceInfo(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pDeviceCtx;
    (void)pInput;
    (void)pOutput;

	CAPI_RESULT_E call_result;
	HDK_Struct *pStr1 = NULL, *pStr2 = NULL;
	DHCP_CLIENT_CFG_S dhcp_client;
	HDK_MACAddress mac_address;
	int flag, i;
	LAN_STATIC_IP_ASSIGN_CFG_S lan_static;
	char tmp[64];

	call_result = capi_get_runtime_dhcp_clients(&dhcp_client);

	if ( CAPI_FAILURE == call_result )
	{
		printf("GetDeviceInfo failed!\n");
		HDK_Set_Result(pOutput, HDK_Element_Cisco_HotSpot_GetDeviceInfoResult, HDK_Enum_Result_ERROR);
		return;
	}

	flag = 0;
	for(i = 0; i < MAX_DHCP_CLIENT; i++)
	{
		if ( strlen(dhcp_client.entry[i].ip) == 0 )
		{
			continue;
		}
		else
		{
			if ( flag == 0 )
			{
				flag = 1;	
				pStr1 = HDK_Set_Struct(pOutput, HDK_Element_Cisco_HotSpot_DeviceInfos);

				if ( pStr1 != NULL )
				{
					pStr2 = HDK_Set_Struct(pStr1, HDK_Element_Cisco_HotSpot_DeviceInfo);
				}
			}
			else
			{
				if ( pStr1 != NULL )
				{
					pStr2 = HDK_Append_Struct(pStr1, HDK_Element_Cisco_HotSpot_DeviceInfo);
				}
			}
																														
			if ( pStr2 != NULL )
			{
				sscanf(dhcp_client.entry[i].mac_address, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &mac_address.a, &mac_address.b, &mac_address.c, &mac_address.d, &mac_address.e, &mac_address.f);
				HDK_Set_MACAddress(pStr2, HDK_Element_Cisco_HotSpot_MACAddress, &mac_address);
				HDK_Set_String(pStr2, HDK_Element_Cisco_HotSpot_FriendlyName, dhcp_client.entry[i].hostname);
			}
		}	
	}
	call_result = capi_get_runtime_dhcp_static_clients(&dhcp_client);

	if ( CAPI_FAILURE == call_result )
	{
		printf("Get static DeviceInfo failed!\n");
		HDK_Set_Result(pOutput, HDK_Element_Cisco_HotSpot_GetDeviceInfoResult, HDK_Enum_Result_ERROR);
		return;
	}

	for(i = 0; i < MAX_DHCP_CLIENT; i++)
	{
		if ( strlen(dhcp_client.entry[i].ip) == 0 )
		{
			continue;
		}
		else
		{
			if ( flag == 0 )
			{
				flag = 1;	
				pStr1 = HDK_Set_Struct(pOutput, HDK_Element_Cisco_HotSpot_DeviceInfos);

				if ( pStr1 != NULL )
				{
					pStr2 = HDK_Set_Struct(pStr1, HDK_Element_Cisco_HotSpot_DeviceInfo);
				}
			}
			else
			{
				if ( pStr1 != NULL )
				{
					pStr2 = HDK_Append_Struct(pStr1, HDK_Element_Cisco_HotSpot_DeviceInfo);
				}
			}
		
			if ( pStr2 != NULL )
			{
				sscanf(dhcp_client.entry[i].mac_address, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &mac_address.a, &mac_address.b, &mac_address.c, &mac_address.d, &mac_address.e, &mac_address.f);
				HDK_Set_MACAddress(pStr2, HDK_Element_Cisco_HotSpot_MACAddress, &mac_address);
				HDK_Set_String(pStr2, HDK_Element_Cisco_HotSpot_FriendlyName, dhcp_client.entry[i].hostname);
			}		
		}	
	}
	call_result = capi_get_lan_static_ip_assign(&lan_static);

	if ( CAPI_FAILURE == call_result )
	{
		printf("Get static ip assign failed!\n");
		HDK_Set_Result(pOutput, HDK_Element_Cisco_HotSpot_GetDeviceInfoResult, HDK_Enum_Result_ERROR);
		return;
	}

	for(i = 0; i < MAX_STATIC_IP_ASSIGN_NUM; i++)
	{
		if ( lan_static.entry[i].ip == 0 )
		{
			continue;
		}
		else
		{
			if ( flag == 0 )
			{
				flag = 1;	
				pStr1 = HDK_Set_Struct(pOutput, HDK_Element_Cisco_HotSpot_DeviceInfos);

				if ( pStr1 != NULL )
				{
					pStr2 = HDK_Set_Struct(pStr1, HDK_Element_Cisco_HotSpot_DeviceInfo);
				}
			}
			else
			{
				if ( pStr1 != NULL )
				{
					pStr2 = HDK_Append_Struct(pStr1, HDK_Element_Cisco_HotSpot_DeviceInfo);
				}
			}
																														
			if ( pStr2 != NULL )
			{
				sscanf(lan_static.entry[i].mac, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &mac_address.a, &mac_address.b, &mac_address.c, &mac_address.d, &mac_address.e, &mac_address.f);
				HDK_Set_MACAddress(pStr2, HDK_Element_Cisco_HotSpot_MACAddress, &mac_address);
				strcpy(tmp, "Static_IP_Assigned_");
				sprintf(tmp + strlen(tmp), "%d", i);
				HDK_Set_String(pStr2, HDK_Element_Cisco_HotSpot_FriendlyName, tmp);
			}
		}	
	}
	return;
}

#endif /* #ifdef HDK_METHOD_CISCO_HOTSPOT_GETDEVICEINFO */


/*
 * http://cisco.com/HNAPExt/HotSpot/GetGuestNetwork
 */
#ifdef HDK_METHOD_CISCO_HOTSPOT_GETGUESTNETWORK

void HDK_Method_Cisco_HotSpot_GetGuestNetwork(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pDeviceCtx;
    (void)pInput;
    (void)pOutput;
	    HDK_Struct* ptmpStruct = NULL;
    
    int enable = 1;
    char ssid[WLAN_SSID_SIZE_MAX];
    char password[MIDDLE_STR_LEN];
    int maxGuestAllowed = 10;
	CAPI_RESULT_E result;
    int canBeActive = 1;
	WLAN_BASIC_CFG_S wlan;
	WLAN_SEC_CFG_S sec;

   /*
 *  call functions in TES301 to get enable,ssid,password ...
 * */
    result = capi_get_wlan_basic_index(&wlan,1);
	if( result != CAPI_SUCCESS)
		goto error;

	result = capi_get_wlan_security_index(&sec,1);
	if(result != CAPI_SUCCESS)
		goto error;

    enable = wlan.wlan_radio;
	strcpy(ssid, wlan.wlan_essid);
	strcpy(password, sec.wlan_passphrase);
	maxGuestAllowed = wlan.bss_maxassoc;
    
    ptmpStruct = pOutput;   
    if(!ptmpStruct)
        goto error;

    HDK_Set_Bool(ptmpStruct, HDK_Element_Cisco_HotSpot_Enabled, enable);
    HDK_Set_String(ptmpStruct, HDK_Element_Cisco_HotSpot_SSID, ssid);
    HDK_Set_String(ptmpStruct, HDK_Element_Cisco_HotSpot_Password, password);
    HDK_Set_Int(ptmpStruct, HDK_Element_Cisco_HotSpot_MaxGuestsAllowed, maxGuestAllowed);
    HDK_Set_Bool(ptmpStruct, HDK_Element_Cisco_HotSpot_CanBeActive, canBeActive);
  
    goto success; 

error:
    printf("-------- error in get guestnetwork -------------\n");
    HDK_Set_Result(pOutput, HDK_Element_Cisco_HotSpot_GetGuestNetworkResult, HDK_Enum_Result_ERROR);
    return ;

success:
    return;
}

#endif /* #ifdef HDK_METHOD_CISCO_HOTSPOT_GETGUESTNETWORK */


/*
 * http://cisco.com/HNAPExt/HotSpot/GetGuestNetworkLANSettings
 */
#ifdef HDK_METHOD_CISCO_HOTSPOT_GETGUESTNETWORKLANSETTINGS

void HDK_Method_Cisco_HotSpot_GetGuestNetworkLANSettings(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pDeviceCtx;
    (void)pInput;
    (void)pOutput;
}

#endif /* #ifdef HDK_METHOD_CISCO_HOTSPOT_GETGUESTNETWORKLANSETTINGS */


/*
 * http://cisco.com/HNAPExt/HotSpot/GetParentalControlsResetQuestion
 */
#ifdef HDK_METHOD_CISCO_HOTSPOT_GETPARENTALCONTROLSRESETQUESTION

void HDK_Method_Cisco_HotSpot_GetParentalControlsResetQuestion(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pDeviceCtx;
    (void)pInput;
    (void)pOutput;
 
    char* question;
    /*
 *
 * get reset question in TES301
 */
    question = getPCQuestion();

	if(strlen(question) < 1)
		goto error;

    HDK_Set_String(pOutput, HDK_Element_Cisco_HotSpot_Question, question);

    goto success;


error:
    printf("--------error in GetParentControlsResetQuestion --------\n");
    HDK_Set_Result(pOutput, HDK_Element_Cisco_HotSpot_GetParentalControlsResetQuestionResult, HDK_Enum_Result_ERROR); 
    return;

success:
    return;
   
}

#endif /* #ifdef HDK_METHOD_CISCO_HOTSPOT_GETPARENTALCONTROLSRESETQUESTION */


/*
 * http://cisco.com/HNAPExt/HotSpot/GetSwitchPortLEDSettings
 */
#ifdef HDK_METHOD_CISCO_HOTSPOT_GETSWITCHPORTLEDSETTINGS

void HDK_Method_Cisco_HotSpot_GetSwitchPortLEDSettings(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pDeviceCtx;
    (void)pInput;
    (void)pOutput;
	HDK_Set_Bool(pOutput, HDK_Element_Cisco_HotSpot_Enabled, 0);
}

#endif /* #ifdef HDK_METHOD_CISCO_HOTSPOT_GETSWITCHPORTLEDSETTINGS */


/*
 * http://cisco.com/HNAPExt/HotSpot/GetWANAccessStatuses
 */
#ifdef HDK_METHOD_CISCO_HOTSPOT_GETWANACCESSSTATUSES

void HDK_Method_Cisco_HotSpot_GetWANAccessStatuses(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pDeviceCtx;
    (void)pInput;
    (void)pOutput;
}

#endif /* #ifdef HDK_METHOD_CISCO_HOTSPOT_GETWANACCESSSTATUSES */


/*
 * http://cisco.com/HNAPExt/HotSpot/HasParentalControlsPassword
 */
#ifdef HDK_METHOD_CISCO_HOTSPOT_HASPARENTALCONTROLSPASSWORD

void HDK_Method_Cisco_HotSpot_HasParentalControlsPassword(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pDeviceCtx;
    (void)pInput;
    (void)pOutput;

    int hasPassword = 1;
	char * password;

    /*
 *
 * check if TES301 has parent control password
 *
 */
	password = getPCPassword();

	if(strlen(password) >1 )
	{
		hasPassword = 1;
	}else
	{
		hasPassword = 0;
	}


    HDK_Set_Bool(pOutput, HDK_Element_Cisco_HotSpot_HasPassword, hasPassword);
    
}

#endif /* #ifdef HDK_METHOD_CISCO_HOTSPOT_HASPARENTALCONTROLSPASSWORD */


/*
 * http://cisco.com/HNAPExt/HotSpot/ResetParentalControlsPassword
 */
#ifdef HDK_METHOD_CISCO_HOTSPOT_RESETPARENTALCONTROLSPASSWORD

void HDK_Method_Cisco_HotSpot_ResetParentalControlsPassword(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pDeviceCtx;
    (void)pInput;
    (void)pOutput;
    
    int setSuccess = 1;
    char* answer;
    char* newPass;
	CAPI_RESULT_E result;

    char* oldAnswer = NULL;

    answer = HDK_Get_String(pInput, HDK_Element_Cisco_HotSpot_Answer);
    newPass = HDK_Get_String(pInput, HDK_Element_Cisco_HotSpot_NewPassword); 

    /*
 *
 *  check if the new password is valid
 */
    if(checkPassword(newPass) < 0 )
    {
        result = HDK_Enum_Result_ERROR_INVALID_PASSWORD;
		goto error;
    }

    /*
 *  check if the answer is the right answer
 */
    oldAnswer = getAnswer();
	
    if(strcmp(answer, oldAnswer))
    {
        result = HDK_Enum_Result_ERROR_INVALID_ANSWER;
		goto error;
    }
  /*
 * set new password in to TES301
 */
  
   setSuccess = setPCPassword(newPass);
   if(setSuccess < 0)
   {
        result = HDK_Enum_Result_ERROR;
	    goto error;
   } 

   goto success;

error:
    HDK_Set_Result(pOutput, HDK_Element_Cisco_HotSpot_ResetParentalControlsPasswordResult, result);
    
success:
    free(oldAnswer);
    return;
}

#endif /* #ifdef HDK_METHOD_CISCO_HOTSPOT_RESETPARENTALCONTROLSPASSWORD */


/*
 * http://cisco.com/HNAPExt/HotSpot/SetDefaultWireless
 */
#ifdef HDK_METHOD_CISCO_HOTSPOT_SETDEFAULTWIRELESS

void HDK_Method_Cisco_HotSpot_SetDefaultWireless(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pDeviceCtx;
    (void)pInput;
    (void)pOutput;

	WLAN_BASIC_CFG_S wlan_basic, wlan_basic_bak;
	CAPI_RESULT_E call_result;
	WLAN_SEC_CFG_S wlan_sec, wlan_sec_bak;
	char *p;
	int *val;
	char cmd_buf[128];

	// validate value
	p = HDK_Get_String(pInput, HDK_Element_Cisco_HotSpot_Key);

	if ( (p == NULL) || strlen(p) < 8 || strlen(p) > 63 )
	{
		HDK_Set_Result(pOutput, HDK_Element_Cisco_HotSpot_SetDefaultWirelessResult, HDK_Enum_Result_ERROR_ILLEGAL_KEY_VALUE);
		return;
	}

	p = HDK_Get_String(pInput, HDK_Element_Cisco_HotSpot_GuestPassword);

	if ( (p == NULL) || strlen(p) < 8 || strlen(p) > 63 )
	{
		HDK_Set_Result(pOutput, HDK_Element_Cisco_HotSpot_SetDefaultWirelessResult, HDK_Enum_Result_ERROR_ILLEGAL_KEY_VALUE);
		return;
	}

	p = HDK_Get_String(pInput, HDK_Element_Cisco_HotSpot_SSID);

	if ( (p == NULL) || strlen(p) < 1 || strlen(p) >= WLAN_SSID_SIZE_MAX )
	{
		HDK_Set_Result(pOutput, HDK_Element_Cisco_HotSpot_SetDefaultWirelessResult, HDK_Enum_Result_ERROR_INVALID_SSID);
		return;
	}

	p = HDK_Get_String(pInput, HDK_Element_Cisco_HotSpot_GuestSSID);

	if ( (p == NULL) || strlen(p) < 1 || strlen(p) >= MIDDLE_STR_LEN )
	{
		HDK_Set_Result(pOutput, HDK_Element_Cisco_HotSpot_SetDefaultWirelessResult, HDK_Enum_Result_ERROR_INVALID_SSID);
		return;
	}

	val = HDK_Get_Int(pInput, HDK_Element_Cisco_HotSpot_MaxGuestsAllowed);

	if ( *val < 0 || *val > 32 )
	{
		HDK_Set_Result(pOutput, HDK_Element_Cisco_HotSpot_SetDefaultWirelessResult, HDK_Enum_Result_ERROR_INVALID_MAXGUESTSALLOWED);
		return;
	}

	call_result = capi_get_wlan_basic(&wlan_basic);

	if ( CAPI_FAILURE == call_result )
	{
		printf("Get WlanBasic for SetDefaultWireless failed!\n");
		HDK_Set_Result(pOutput, HDK_Element_Cisco_HotSpot_SetDefaultWirelessResult, HDK_Enum_Result_ERROR);
		return;
	}
	memcpy(&wlan_basic_bak, &wlan_basic, sizeof(wlan_basic));
	p = HDK_Get_String(pInput, HDK_Element_Cisco_HotSpot_SSID);
	strcpy(wlan_basic.wlan_essid, p);

	call_result = capi_get_wlan_security(&wlan_sec);

	if ( CAPI_FAILURE == call_result )
	{
		printf("Get WlanSecurity for SetDefaultWireless failed!\n");
		HDK_Set_Result(pOutput, HDK_Element_Cisco_HotSpot_SetDefaultWirelessResult, HDK_Enum_Result_ERROR);
		return;
	}
	memcpy(&wlan_sec_bak, &wlan_sec, sizeof(wlan_sec));
	p = HDK_Get_String(pInput, HDK_Element_Cisco_HotSpot_Key);
	strcpy(wlan_sec.wlan_passphrase, p);

	call_result = capi_set_wlan_basic(&wlan_basic);

	if ( CAPI_FAILURE == call_result )
	{
		printf("Set WlanBasic for SetDefaultWireless failed!\n");
		HDK_Set_Result(pOutput, HDK_Element_Cisco_HotSpot_SetDefaultWirelessResult, HDK_Enum_Result_ERROR);
		return;
	}
	call_result = capi_set_wlan_security(&wlan_sec);

	if ( CAPI_FAILURE == call_result )
	{
		printf("Set WlanSecurity for SetDefaultWireless failed!\n");
		HDK_Set_Result(pOutput, HDK_Element_Cisco_HotSpot_SetDefaultWirelessResult, HDK_Enum_Result_ERROR);
		
		if ( CAPI_FAILURE ==  capi_set_wlan_basic(&wlan_basic_bak) )
		{
			 capi_set_wlan_basic(&wlan_basic_bak);
		}
		return;
	}

	call_result = capi_get_wlan_basic_index(&wlan_basic, 1);

	if ( CAPI_FAILURE == call_result )
	{
		printf("Get GuestWlanBasic for SetDefaultWireless failed!\n");
		HDK_Set_Result(pOutput, HDK_Element_Cisco_HotSpot_SetDefaultWirelessResult, HDK_Enum_Result_ERROR);
		return;
	}
	memcpy(&wlan_basic_bak, &wlan_basic, sizeof(wlan_basic));
	p = HDK_Get_String(pInput, HDK_Element_Cisco_HotSpot_GuestSSID);
	strcpy(wlan_basic.wlan_essid, p);
	val = HDK_Get_Bool(pInput, HDK_Element_Cisco_HotSpot_GuestEnabled);
	wlan_basic.wlan_radio = *val;
	val = HDK_Get_Int(pInput, HDK_Element_Cisco_HotSpot_MaxGuestsAllowed);
	wlan_basic.bss_maxassoc = *val;

	call_result = capi_get_wlan_security_index(&wlan_sec, 1);

	if ( CAPI_FAILURE == call_result )
	{
		printf("Get GuestWlanSecurity for SetDefaultWireless failed!\n");
		HDK_Set_Result(pOutput, HDK_Element_Cisco_HotSpot_SetDefaultWirelessResult, HDK_Enum_Result_ERROR);
		return;
	}
	memcpy(&wlan_sec_bak, &wlan_sec, sizeof(wlan_sec));
	p = HDK_Get_String(pInput, HDK_Element_Cisco_HotSpot_GuestPassword);
	strcpy(wlan_sec.wlan_passphrase, p);

	call_result = capi_set_wlan_basic_index(&wlan_basic, 1);

	if ( CAPI_FAILURE == call_result )
	{
		printf("Set GuestWlanBasic for SetDefaultWireless failed!\n");
		HDK_Set_Result(pOutput, HDK_Element_Cisco_HotSpot_SetDefaultWirelessResult, HDK_Enum_Result_ERROR);
		return;		
	}

	call_result = capi_set_wlan_security_index(&wlan_sec, 1);

	if ( CAPI_FAILURE == call_result )
	{
		printf("Set GuestWlanSecurity for SetDefaultWireless failed!\n");
		HDK_Set_Result(pOutput, HDK_Element_Cisco_HotSpot_SetDefaultWirelessResult, HDK_Enum_Result_ERROR);
		
		if ( CAPI_FAILURE ==  capi_set_wlan_basic_index(&wlan_basic_bak, 1) )
		{
			 capi_set_wlan_basic_index(&wlan_basic_bak, 1);
		}
		return;
	}

	if ( !HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_DeviceName, pInput) )
	{
		HDK_Set_Result(pOutput, HDK_Element_Cisco_HotSpot_SetDefaultWirelessResult, HDK_Enum_Result_ERROR);
		return;
	}

	/*
	if ( sys_lib_init(SYS_MID_MAX) != 0 )
	{
		printf("Syslib initialization failed!\n");
		HDK_Set_Result(pOutput, HDK_Element_Cisco_HotSpot_SetDefaultWirelessResult, HDK_Enum_Result_ERROR);
		return;
	}
	if ( sys_notify_event(SYS_MSG_WLAN_CONFIGURE_CHANGED, 0, 0) != 0 )
	{
		printf("Notify Wlan module failed!\n");
		HDK_Set_Result(pOutput, HDK_Element_Cisco_HotSpot_SetDefaultWirelessResult, HDK_Enum_Result_ERROR);
		return;
	}
	*/
	
    memset(cmd_buf, 0, sizeof(cmd_buf));
    sprintf(cmd_buf, "%s%s %s", SYS_FMC_BIN_ROOT, "sys_notify_event", "SYS_MSG_WLAN_CONFIGURE_CHANGED");

    if ( system(cmd_buf) < 0 )
	{
		printf("Notify Wlan module failed!\n");
		HDK_Set_Result(pOutput, HDK_Element_Cisco_HotSpot_SetDefaultWirelessResult, HDK_Enum_Result_ERROR);
		return;
	}
	

	HDK_Set_Result(pOutput, HDK_Element_Cisco_HotSpot_SetDefaultWirelessResult, HDK_Enum_Result_REBOOT);
}

#endif /* #ifdef HDK_METHOD_CISCO_HOTSPOT_SETDEFAULTWIRELESS */


/*
 * http://cisco.com/HNAPExt/HotSpot/SetDeviceInfo
 */
#ifdef HDK_METHOD_CISCO_HOTSPOT_SETDEVICEINFO

void HDK_Method_Cisco_HotSpot_SetDeviceInfo(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pDeviceCtx;
    (void)pInput;
    (void)pOutput;
	CAPI_RESULT_E call_result;
	HDK_Struct *pStr1 = NULL, *pStr2 = NULL;
	DHCP_CLIENT_CFG_S dhcp_client;
	int i;
	char *p, tmp[MAC_ADDR_LEN];
	HDK_Member* pMember;

	call_result = capi_get_runtime_dhcp_clients(&dhcp_client);

	if ( CAPI_FAILURE == call_result )
	{
		printf("Get DHCP clients for SetDeviceInfos failed!\n");
		HDK_Set_Result(pOutput, HDK_Element_Cisco_HotSpot_SetDeviceInfoResult, HDK_Enum_Result_ERROR);
		return;
	}

	pStr1 = HDK_Get_Struct(pInput, HDK_Element_Cisco_HotSpot_DeviceInfos);

	if ( pStr1 == NULL )
	{
		return;
	}

	for(pMember = pStr1->pHead; pMember; pMember = pMember->pNext)
	{
		pStr2 = HDK_Get_StructMember(pMember);

		if ( pStr2 == NULL )
		{
			return;
		}

        p = (char *)HDK_Get_MACAddress(pStr2, HDK_Element_Cisco_HotSpot_MACAddress);

		if ( *p & 0x01 ) 
		{
			HDK_Set_Result(pOutput, HDK_Element_Cisco_HotSpot_SetDeviceInfoResult, HDK_Enum_Result_ERROR_UNKNOWN_MACADDRESS);
			return;
		}

		if ( !*p && !*(p+1) && !*(p+2) && !*(p+3) && !*(p+4) && !*(p+5) ) 
		{
			HDK_Set_Result(pOutput, HDK_Element_Cisco_HotSpot_SetDeviceInfoResult, HDK_Enum_Result_ERROR_UNKNOWN_MACADDRESS);
			return;
		}
		snprintf(tmp, MAC_ADDR_LEN, "%02x:%02x:%02x:%02x:%02x:%02x", *p, *(p+1), *(p+2), *(p+3), *(p+4), *(p+5));
		
		for(i = 0; i < MAX_DHCP_CLIENT; i++)
		{
			if ( strlen(dhcp_client.entry[i].ip) == 0 )
			{
				continue;
			}
			else
			{
				if ( !strcmp(dhcp_client.entry[i].mac_address, tmp) )
				{
					p = HDK_Get_String(pStr2, HDK_Element_Cisco_HotSpot_FriendlyName);
					strcpy(dhcp_client.entry[i].hostname, p);
					break;
				}
			}
		}

		if ( i == MAX_DHCP_CLIENT )
		{
			HDK_Set_Result(pOutput, HDK_Element_Cisco_HotSpot_SetDeviceInfoResult, HDK_Enum_Result_ERROR_UNKNOWN_MACADDRESS);
			return;
		}	
	}
}

#endif /* #ifdef HDK_METHOD_CISCO_HOTSPOT_SETDEVICEINFO */


/*
 * http://cisco.com/HNAPExt/HotSpot/SetGuestNetwork
 */
#ifdef HDK_METHOD_CISCO_HOTSPOT_SETGUESTNETWORK

void HDK_Method_Cisco_HotSpot_SetGuestNetwork(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pDeviceCtx;
    (void)pInput;
    (void)pOutput;

	int enabled;
    int canBeActive;
    int maxGuestAllowed;
    char* password;
    char* ssid;
	WLAN_BASIC_CFG_S wlan;
	WLAN_SEC_CFG_S sec;
	CAPI_RESULT_E result = CAPI_FAILURE;
	char cmd_buf[128];

    enabled = HDK_Get_BoolEx(pInput, HDK_Element_Cisco_HotSpot_Enabled, 0);
    canBeActive = HDK_Get_BoolEx(pInput, HDK_Element_Cisco_HotSpot_CanBeActive, 0);
    maxGuestAllowed = HDK_Get_IntEx(pInput,  HDK_Element_Cisco_HotSpot_MaxGuestsAllowed, 0);
    password = HDK_Get_String(pInput, HDK_Element_Cisco_HotSpot_Password);
    ssid = HDK_Get_String(pInput, HDK_Element_Cisco_HotSpot_SSID);

	if ( !password || strlen(password) < 8 || strlen(password) >= MIDDLE_STR_LEN )
	{
		goto error;
	}

	if ( !ssid || !strlen(ssid) )
	{
		goto error;
	}

    /*
 *
 * call functions in TES301 to set enabled, canBeActive, maxGuestAllowed, password, ssid
 */
	result = capi_get_wlan_basic_index(&wlan, 1);
	if(result == CAPI_FAILURE)
	{
		printf("Get guest network for SetGuestWork error!\n");
		goto error;
	}
	
	result = capi_get_wlan_security_index(&sec, 1);
	if(result == CAPI_FAILURE)
	{
		printf("Get guest network security for SetGuestWork error!\n");
		goto error;
	}

	wlan.wlan_radio = enabled;
	strcpy(wlan.wlan_essid, ssid);
	wlan.bss_maxassoc = maxGuestAllowed;

	strcpy(sec.wlan_passphrase, password);

	result = capi_set_wlan_basic_index(&wlan, 1);
	if(result == CAPI_FAILURE)
	{
		printf("SetGuestNetwork basic error!\n");
		goto error;
	}

	result = capi_set_wlan_security_index(&sec, 1);

	if(result == CAPI_FAILURE)
	{
		printf("SetGuestNetwork security error!\n");
		if ( capi_set_wlan_basic_index(&wlan, 1) == CAPI_FAILURE )
		{
			capi_set_wlan_basic_index(&wlan, 1);
		}
		goto error;
	}
	
	/*
	if ( sys_lib_init(SYS_MID_MAX) != 0 )
	{
		printf("Syslib initialization failed!\n");
		goto error;
	}
	if ( sys_notify_event(SYS_MSG_WLAN_CONFIGURE_CHANGED, 0, 0) != 0 )
	{
		printf("Notify Wlan module failed!\n");
		goto error;
	}
	*/
	
	memset(cmd_buf, 0, sizeof(cmd_buf));
    sprintf(cmd_buf, "%s%s %s", SYS_FMC_BIN_ROOT, "sys_notify_event", "SYS_MSG_WLAN_CONFIGURE_CHANGED");

    if ( system(cmd_buf) < 0 )
	{
		printf("Notify Wlan module failed!\n");
		goto error;
	}
	

	goto success;


error:
    printf("----------set guest network error!------------");
    HDK_Set_Result(pOutput, HDK_Element_Cisco_HotSpot_SetGuestNetworkResult, HDK_Enum_Result_ERROR);
    return;

success: 
    return;
}

#endif /* #ifdef HDK_METHOD_CISCO_HOTSPOT_SETGUESTNETWORK */


/*
 * http://cisco.com/HNAPExt/HotSpot/SetParentalControlsPassword
 */
#ifdef HDK_METHOD_CISCO_HOTSPOT_SETPARENTALCONTROLSPASSWORD

void HDK_Method_Cisco_HotSpot_SetParentalControlsPassword(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pDeviceCtx;
    (void)pInput;
    (void)pOutput;

    char* oldPassword;
    char* password;
    char* nativePass = NULL;
	CAPI_RESULT_E result;
	int status;

    oldPassword = HDK_Get_String(pInput, HDK_Element_Cisco_HotSpot_OldPassword);
    password = HDK_Get_String(pInput, HDK_Element_Cisco_HotSpot_NewPassword); 

    /*
 *   check if new password is valid
 *   password should be 4-32 characters and should be in [A-Za-z0-9]
 *
 */
    if(checkPassword(password) < 0)
	{
        result = HDK_Enum_Result_ERROR_INVALID_PASSWORD;
		goto error;
    }
    /*
 *  get old password in TES301
 */
   
    nativePass = getPCPassword();
	printf("native pass: %s, oldpass: %s, new pass: %s\n", nativePass, oldPassword, password);
    if((strlen(nativePass) != 0) &&
        strcmp(oldPassword, nativePass))
	{
		result = HDK_Enum_Result_ERROR_INCORRECT_PASSWORD;
		goto error;
	}

     /*
 *   set new password in TES301
 */
	status = setPCPassword(password);
    if(status < 0)
    {
        result = HDK_Enum_Result_ERROR;
        goto error;
    }

	goto success;

error:
    HDK_Set_Result(pOutput, HDK_Element_Cisco_HotSpot_SetParentalControlsPasswordResult, result);

success:
    free(nativePass);
    return;
}

#endif /* #ifdef HDK_METHOD_CISCO_HOTSPOT_SETPARENTALCONTROLSPASSWORD */


/*
 * http://cisco.com/HNAPExt/HotSpot/SetParentalControlsResetQuestion
 */
#ifdef HDK_METHOD_CISCO_HOTSPOT_SETPARENTALCONTROLSRESETQUESTION

void HDK_Method_Cisco_HotSpot_SetParentalControlsResetQuestion(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pDeviceCtx;
    (void)pInput;
    (void)pOutput;
  
    HDK_Enum_Result result;
    char* password;
    char* question;
    char* answer;
	char* nativePass;
	int status;

    password = HDK_Get_String(pInput, HDK_Element_Cisco_HotSpot_Password);
    question = HDK_Get_String(pInput, HDK_Element_Cisco_HotSpot_Question);
    answer = HDK_Get_String(pInput, HDK_Element_Cisco_HotSpot_Answer);

 /*
 * validate question and answer
 */
    if(strlen(question) < 1 || strlen(question) > 64)
    {
        result = HDK_Enum_Result_ERROR_INVALID_QUESTION;
        goto error;   
    }

    if(strlen(answer) < 1 || strlen(answer) > 32)
    {
        result = HDK_Enum_Result_ERROR_INVALID_ANSWER;
        goto error;   
    }


 /*
 * check if the password is correct
 */   
    nativePass = getPCPassword();

    if(strcmp(password, nativePass))
    {
        result = HDK_Enum_Result_ERROR_INCORRECT_PASSWORD;
        goto error;
    }
    
    status = setPCQuestion(question);
	if(status < 0)
	{
		result = HDK_Enum_Result_ERROR;
		goto error;
	}
	
	status = setAnswer(answer);
	if(status < 0)
	{
		result = HDK_Enum_Result_ERROR;
		goto error;
	}
   
    goto success;

error:
    HDK_Set_Result(pOutput, HDK_Element_Cisco_HotSpot_SetParentalControlsResetQuestionResult, result);
    return;

success: 
    return;
}

#endif /* #ifdef HDK_METHOD_CISCO_HOTSPOT_SETPARENTALCONTROLSRESETQUESTION */


/*
 * http://cisco.com/HNAPExt/HotSpot/SetSwitchPortLEDSettings
 */
#ifdef HDK_METHOD_CISCO_HOTSPOT_SETSWITCHPORTLEDSETTINGS

void HDK_Method_Cisco_HotSpot_SetSwitchPortLEDSettings(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pDeviceCtx;
    (void)pInput;
    (void)pOutput;
}

#endif /* #ifdef HDK_METHOD_CISCO_HOTSPOT_SETSWITCHPORTLEDSETTINGS */


/*
 * http://purenetworks.com/HNAP1/DownloadSpeedTest
 */
#ifdef HDK_METHOD_PN_DOWNLOADSPEEDTEST

void HDK_Method_PN_DownloadSpeedTest(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pDeviceCtx;
    (void)pInput;
    (void)pOutput;
}

#endif /* #ifdef HDK_METHOD_PN_DOWNLOADSPEEDTEST */


/*
 * http://purenetworks.com/HNAP1/FirmwareUpload
 */
#ifdef HDK_METHOD_PN_FIRMWAREUPLOAD

void HDK_Method_PN_FirmwareUpload(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pDeviceCtx;
    (void)pInput;
    (void)pOutput;
}

#endif /* #ifdef HDK_METHOD_PN_FIRMWAREUPLOAD */



/*
 * http://purenetworks.com/HNAP1/GetConfigBlob
 */
#ifdef HDK_METHOD_PN_GETCONFIGBLOB

void HDK_Method_PN_GetConfigBlob(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pDeviceCtx;
    (void)pInput;
    (void)pOutput;
}

#endif /* #ifdef HDK_METHOD_PN_GETCONFIGBLOB */



/*
 * http://purenetworks.com/HNAP1/IsDeviceReady
 */
#ifdef HDK_METHOD_PN_ISDEVICEREADY

void HDK_Method_PN_IsDeviceReady(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pDeviceCtx;
    (void)pInput;
    (void)pOutput;

	HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_IsDeviceReady, 0);
}

#endif /* #ifdef HDK_METHOD_PN_ISDEVICEREADY */


/*
 * http://purenetworks.com/HNAP1/Reboot
 */
#ifdef HDK_METHOD_PN_REBOOT

void HDK_Method_PN_Reboot(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
	pid_t pid;
    (void)pDeviceCtx;
    (void)pInput;
    (void)pOutput;

	HDK_Set_Result(pOutput, HDK_Element_PN_RebootResult, HDK_Enum_Result_REBOOT);

	if ( (pid = fork()) < 0 )
	{
		log_printf(LOG_ERR, "fork in reboot error!\n");
	}
	else
	{
		if ( pid == 0 )
		{
		    //guess 5s is enough to send the HNAP result back
			sleep(5);
			log_printf(LOG_WARNING, "Sync data and rebooting!\n");
			sync();
			reboot(LINUX_REBOOT_CMD_RESTART);
		}
	}
}

#endif /* #ifdef HDK_METHOD_PN_REBOOT */



/*
 * http://purenetworks.com/HNAP1/RestoreFactoryDefaults
 */
#ifdef HDK_METHOD_PN_RESTOREFACTORYDEFAULTS

void HDK_Method_PN_RestoreFactoryDefaults(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pDeviceCtx;
    (void)pInput;
    (void)pOutput;
}

#endif /* #ifdef HDK_METHOD_PN_RESTOREFACTORYDEFAULTS */


/*
 * http://purenetworks.com/HNAP1/SetConfigBlob
 */
#ifdef HDK_METHOD_PN_SETCONFIGBLOB

void HDK_Method_PN_SetConfigBlob(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pDeviceCtx;
    (void)pInput;
    (void)pOutput;
}

#endif
/* #ifdef HDK_METHOD_PN_SETCONFIGBLOB */


