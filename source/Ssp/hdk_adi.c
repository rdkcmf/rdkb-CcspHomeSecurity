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
 * hdk_device.c - Abstract device HNAP implementation
 */

#include "hdk_data.h"
#include "hdk.h"
#include "hdk_adi.h"
#include "hdk_methods.h"
#include "chs_log.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define  MAX_STRING_LEN    256 /*RDKB-7468, CID-33129, max string len*/
#if defined(HDK_METHOD_PN_SETDEVICESETTINGS2) || defined(HDK_METHOD_PN_SETROUTERLANSETTINGS2) || \
    defined(HDK_METHOD_PN_SETROUTERSETTINGS) || defined(HDK_METHOD_PN_SETWANSETTINGS)

/* Helper function to check whether or not the setting of a particular device is allowed */
static int HDK_Device_CheckValue(void* pDeviceCtx, HDK_DeviceValue eValue)
{
	switch (eValue)
	{
		case HDK_DeviceValue_UsernameSetAllowed:
			return 0;

		case HDK_DeviceValue_TimeZoneSetAllowed:
		case HDK_DeviceValue_DomainNameChangeAllowed:
		case HDK_DeviceValue_WiredQoSAllowed:
			return 0;
		
		default :
			return 1;
	}
    HDK_Struct sTemp;
    int iReturn;

    /* Initialize temprary struct */
    HDK_Struct_Init(&sTemp);

    HDK_Device_GetValue(pDeviceCtx, &sTemp, eValue, 0);
    iReturn = HDK_Get_BoolEx(&sTemp, HDK_Element__UNKNOWN__, 0);

    /* Free the temp struct */
    HDK_Struct_Free(&sTemp);

    return iReturn;
}

#endif /* defined(HDK_METHOD_PN_SETDEVICESETTINGS2) || ... */


#ifdef HDK_METHOD_PN_SETROUTERLANSETTINGS2

/* Helper function which returns 1 only in the case where pIpAddr1 and pIpAddr2 are equivalent */
static int HDK_Util_AreIPsDuplicate(HDK_IPAddress* pIpAddr1, HDK_IPAddress* pIpAddr2)
{
    if (!pIpAddr1 || !pIpAddr2)
    {
        return 0;
    }

    if (pIpAddr1->a == pIpAddr2->a &&
        pIpAddr1->b == pIpAddr2->b &&
        pIpAddr1->c == pIpAddr2->c &&
        pIpAddr1->d == pIpAddr2->d)
    {
        return 1;
    }

    return 0;
}

#endif /* HDK_METHOD_PN_SETROUTERLANSETTINGS2 */


#if defined(HDK_METHOD_PN_SETROUTERLANSETTINGS2) || defined(HDK_METHOD_PN_SETMACFILTERS2)

/* Helper function which returns 1 only in the case where pMacAddr1 and pMacAddr2 are equivalent */
static int HDK_Util_AreMACsDuplicate(HDK_MACAddress* pMacAddr1, HDK_MACAddress* pMacAddr2)
{
    if (!pMacAddr1 || !pMacAddr2)
    {
        return 0;
    }

    if (pMacAddr1->a == pMacAddr2->a &&
        pMacAddr1->b == pMacAddr2->b &&
        pMacAddr1->c == pMacAddr2->c &&
        pMacAddr1->d == pMacAddr2->d &&
        pMacAddr1->e == pMacAddr2->e &&
        pMacAddr1->f == pMacAddr2->f)
    {
        return 1;
    }

    return 0;
}

#endif /* defined(HDK_METHOD_PN_SETDEVICESETTINGS2) || ... */


#if defined(HDK_METHOD_PN_ADDPORTMAPPING) || defined(HDK_METHOD_PN_SETROUTERLANSETTINGS2) || \
    defined(HDK_METHOD_PN_SETWANSETTINGS)

/*
 * Helper function, which validates that the entire range of IPs between pIPFirst and
 * pIPLast are within the subnet computed from the pDevIP and pDevSubnet.
 */
static int HDK_Util_ValidateIPRange(HDK_IPAddress* pDevIp, HDK_IPAddress* pDevSubnet,
                                     HDK_IPAddress* pIpFirst, HDK_IPAddress* pIpLast)
{
    unsigned int uiFirstIpDec;
    unsigned int uiLastIpDec;

    if (!pDevIp || !pDevSubnet || !pIpFirst || !pIpLast)
    {
        return 0;
    }

    /* If the first IP is not in the subnet, return error */
    if ((pDevIp->a & pDevSubnet->a) != (pIpFirst->a & pDevSubnet->a) ||
        (pDevIp->b & pDevSubnet->b) != (pIpFirst->b & pDevSubnet->b) ||
        (pDevIp->c & pDevSubnet->c) != (pIpFirst->c & pDevSubnet->c) ||
        (pDevIp->d & pDevSubnet->d) != (pIpFirst->d & pDevSubnet->d))
    {
        return 0;
    }

    /* If the second IP is not in the subnet, return error */
    if ((pDevIp->a & pDevSubnet->a) != (pIpLast->a & pDevSubnet->a) ||
        (pDevIp->b & pDevSubnet->b) != (pIpLast->b & pDevSubnet->b) ||
        (pDevIp->c & pDevSubnet->c) != (pIpLast->c & pDevSubnet->c) ||
        (pDevIp->d & pDevSubnet->d) != (pIpLast->d & pDevSubnet->d))
    {
        return 0;
    }

    /*
     * Convert the IP address to a decimal value by multiplying each
     * octet by the appropriate power of 256 and summing them together.
     * This makes it easy to determine if the last IP is greater than
     * the first.
     */
    uiFirstIpDec = (pIpFirst->a << 24) + (pIpFirst->b << 16) + (pIpFirst->c << 8) + pIpFirst->d;
    uiLastIpDec = (pIpLast->a << 24) + (pIpLast->b << 16) + (pIpLast->c << 8) + pIpLast->d;

    if (uiFirstIpDec > uiLastIpDec)
    {
        return 0;
    }

    return 1;
}

/*
 * Helper function, which validates that pIPAddress falls within the subnet computed
 * from pDevIP and pDevSubnet.
 */
static int HDK_Util_ValidateIPSubnet(HDK_IPAddress* pDevIp, HDK_IPAddress* pDevSubnet,
                                      HDK_IPAddress* pIpAddress)
{
    return HDK_Util_ValidateIPRange(pDevIp, pDevSubnet, pIpAddress, pIpAddress);
}

#endif /* defined(HDK_METHOD_PN_ADDPORTMAPPING) || ... */


#if defined(HDK_METHOD_PN_SETROUTERLANSETTINGS2) || defined(HDK_METHOD_PN_SETWANSETTINGS)

/*
 * Helper function, which simply validates that a subnet mask is in the
 * correct form, ie. all 1's followed by all 0's (in binary form)
 *
 */
static int HDK_Util_ValidateSubnetMask(HDK_IPAddress* pSubnetMask)
{

    unsigned int uiIP;

    /* If the IP is NULL or the Class A octet is not 255, then this is not a valid subnet */
    if (!pSubnetMask || (pSubnetMask->a != 255))
    {
        return 0;
    }

    /*
     * Convert the subnet mask to a decimal value by multiplying each
     * octet by the appropriate power of 256 and summing them together.
     */
    uiIP = (pSubnetMask->a << 24) + (pSubnetMask->b << 16) + (pSubnetMask->c << 8) + pSubnetMask->d;

    /* Shift value until all of the 0's on right are gone */
    for (; uiIP > 0 && !(uiIP & 0x01); uiIP >>= 1) {}

    /* Keep shifting to the right, making sure that the value is always odd */
    for (; uiIP > 0;)
    {
        if (!(uiIP & 0x01))
        {
            return 0;
        }

        uiIP >>= 1;
    }

    return 1;
}

#endif /* defined(HDK_METHOD_PN_SETROUTERLANSETTINGS2) || ... */


#if defined(HDK_METHOD_PN_SETWLANRADIOSECURITY) || defined(HDK_METHOD_PN_SETWIRELESSCLIENTSETTINGS)

static int HDK_Util_ValidateEncryptionKey(char* pszKey, HDK_Enum_PN_WiFiEncryption eWiFiEncryption)
{
    int fIsAscii = 0;
    char* psz;

    /* Validate length */
    switch(eWiFiEncryption)
    {
        case HDK_Enum_PN_WiFiEncryption_WEP_64:
            {
                if (strlen(pszKey) != 14 && strlen(pszKey) != 9)
                {
                    return 0;
                }
				
				return 1;
            }
            break;

        case HDK_Enum_PN_WiFiEncryption_WEP_128:
            {
                if (strlen(pszKey) != 30 && strlen(pszKey) != 17)
                {
                    return 0;
                }
				return 1;
            }
            break;

        case HDK_Enum_PN_WiFiEncryption_AES:
        case HDK_Enum_PN_WiFiEncryption_TKIP:
        case HDK_Enum_PN_WiFiEncryption_TKIPORAES:
            {
                if (strlen(pszKey) >= 8 && strlen(pszKey) <= 63)
                {
                    fIsAscii = 1;
                    break;
                }
                else if (strlen(pszKey) != 64)
                {
                    return 0;
                }
            }
            break;
        default:
            return 1;
    }

    if (fIsAscii)
    {
        /* Validate ascii */
        for(psz = pszKey; *psz; ++psz)
        {
            /* Valid ascii values are 0x20-0x7E (inclusive) */
            if ((int)*psz < 0x20 || (int)*psz > 0x7E)
            {
                return 0;
            }
        }
    }
    else
    {
        /* Validate hex */
        for(psz = pszKey; *psz; ++psz)
        {
            /* Valid hex values are 0x30-0x39, or 0x41-0x46, or 0x61-0x66 (inclusive) */
            if (!((int)*psz >= 0x30 && (int)*psz <= 0x39) &&
                !((int)*psz >= 0x41 && (int)*psz <= 0x46) &&
                !((int)*psz >= 0x61 && (int)*psz <= 0x66))
            {
                return 0;
            }
        }
    }

    return 1;
}

#endif /* defined(HDK_METHOD_PN_SETWLANRADIOSECURITY) || defined(HDK_METHOD_PN_SETWIRELESSCLIENTSETTINGS) */


/*
 * http://purenetworks.com/HNAP1/AddPortMapping
 */
#ifdef HDK_METHOD_PN_ADDPORTMAPPING

void HDK_Method_PN_AddPortMapping(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    HDK_IPAddress* pIpAddr;
    HDK_Enum_PN_IPProtocol* peProtocol = 0;
    HDK_Struct* psPortMappings = 0;
    HDK_Struct* psPortMapping;

    HDK_Struct sTemp;
    HDK_Struct_Init(&sTemp);
    char tmp_str[MAX_STRING_LEN] = {0};
    int *val;
    char* portMapDesc = NULL;
    val = HDK_Get_Int(pInput, HDK_Element_PN_InternalPort);

    if (!val || !*val)
    {
		log_printf(LOG_ERR, "Error internal port\n");
        HDK_Set_Result(pOutput, HDK_Element_PN_AddPortMappingResult, HDK_Enum_Result_ERROR);
        goto finish;
    }

    /* Unknown protocol? */
    peProtocol = HDK_Get_PN_IPProtocol(pInput, HDK_Element_PN_PortMappingProtocol);
    if (*peProtocol == HDK_Enum_PN_IPProtocol__UNKNOWN__)
    {
		log_printf(LOG_ERR, "Unknown port mapping protocol\n");
        HDK_Set_Result(pOutput, HDK_Element_PN_AddPortMappingResult, HDK_Enum_Result_ERROR);
        goto finish;
    }

    /* Make sure this mapping doesn't already exist */
    HDK_Device_GetValue(pDeviceCtx, &sTemp, HDK_DeviceValue_PortMappings, 0);
    if ((psPortMappings = HDK_Get_Struct(&sTemp, HDK_Element_PN_PortMappings)) != 0)
    {
        int* pi;
        int* piExternalPort;
        HDK_Enum_PN_IPProtocol* pe;
        HDK_Member* pmPortMapping;

        piExternalPort = HDK_Get_Int(pInput, HDK_Element_PN_ExternalPort);

        if (*piExternalPort == 0)
        {
			log_printf(LOG_ERR, "External port is zero\n");
            HDK_Set_Result(pOutput, HDK_Element_PN_AddPortMappingResult, HDK_Enum_Result_ERROR);
            goto finish;
        }

        /* Loop over the array searching for a duplicate port mapping */
        for (pmPortMapping = psPortMappings->pHead; pmPortMapping; pmPortMapping = pmPortMapping->pNext)
        {
            pe = HDK_Get_PN_IPProtocol(HDK_Get_StructMember(pmPortMapping), HDK_Element_PN_PortMappingProtocol);
            pi = HDK_Get_Int(HDK_Get_StructMember(pmPortMapping), HDK_Element_PN_ExternalPort);

            /* If we have a protocol, and it matches, or no protocol, and the ports match, then return an error */
            if (((pe && *peProtocol == *pe) || !pe) &&
                (pi && *piExternalPort == *pi))
            {
				log_printf(LOG_ERR, "External was already mapped\n");
                HDK_Set_Result(pOutput, HDK_Element_PN_AddPortMappingResult, HDK_Enum_Result_ERROR);
                goto finish;
            }
        }
    }
    else
    {
        /* If we don't have a port mapping struct, then create a new one */
        psPortMappings = HDK_Set_Struct(&sTemp, HDK_Element_PN_PortMappings);
    }

    /* Return an error if we don't have and couldn't create a port mapping struct */
    if (!psPortMappings)
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_AddPortMappingResult, HDK_Enum_Result_ERROR);
        goto finish;
    }

    HDK_Device_GetValue(pDeviceCtx, &sTemp, HDK_DeviceValue_LanIPAddress, 0);
    HDK_Device_GetValue(pDeviceCtx, &sTemp, HDK_DeviceValue_LanSubnetMask, 0);

    /* Validate the IPAddress */
    pIpAddr = HDK_Get_IPAddress(pInput, HDK_Element_PN_InternalClient);
    if (!HDK_Util_ValidateIPSubnet(HDK_Get_IPAddress(&sTemp, HDK_Element_PN_RouterIPAddress),
                                    HDK_Get_IPAddress(&sTemp, HDK_Element_PN_RouterSubnetMask),
                                    pIpAddr))
    {
		log_printf(LOG_ERR, "Internal client is error, not in Home Security range\n");
        HDK_Set_Result(pOutput, HDK_Element_PN_AddPortMappingResult, HDK_Enum_Result_ERROR);
        goto finish;
    }

    /* Add a new port mapping to the struct */
    psPortMapping = HDK_Append_Struct(psPortMappings, HDK_Element_PN_PortMapping);
    if (!psPortMapping)
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_AddPortMappingResult, HDK_Enum_Result_ERROR);
        goto finish;
    }

    /* Set the required elements */
    //add specific description prefix to distinguish them with other port forwarding rule, such as web gui and upnp 
    strcpy(tmp_str, "");
    portMapDesc = HDK_Get_String(pInput, HDK_Element_PN_PortMappingDescription);/*RDKB-7468,CID-33129, may return null */
    if(portMapDesc)/*RDKB-7468, CID-33129, null check before use */
    {
       strncat(tmp_str, portMapDesc, sizeof(tmp_str) - strlen(tmp_str) - 1);
       tmp_str[sizeof(tmp_str) - 1] = '\0';
    }
    else
    {
        log_printf(LOG_ERR, "Error in getting PN Port Mapping Description \n");
    }
    HDK_Set_String(psPortMapping, HDK_Element_PN_PortMappingDescription, tmp_str);
    HDK_Set_IPAddress(psPortMapping, HDK_Element_PN_InternalClient, pIpAddr);
    HDK_Set_PN_IPProtocol(psPortMapping, HDK_Element_PN_PortMappingProtocol, *peProtocol);
    HDK_Set_Int(psPortMapping, HDK_Element_PN_ExternalPort, HDK_Get_IntEx(pInput, HDK_Element_PN_ExternalPort, 0));
    HDK_Set_Int(psPortMapping, HDK_Element_PN_InternalPort, HDK_Get_IntEx(pInput, HDK_Element_PN_InternalPort, 0));

    /* Validate and set the port mappings. If any fail, return an error. */
    if (!(HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_PortMappings, psPortMapping) &&
          HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_PortMappings, psPortMapping)))
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_AddPortMappingResult, HDK_Enum_Result_ERROR);
    }

finish:

    /* Free the temp struct */
    HDK_Struct_Free(&sTemp);
}

#endif /* #ifdef HDK_METHOD_PN_ADDPORTMAPPING */


/*
 * http://purenetworks.com/HNAP1/DeletePortMapping
 */
#ifdef HDK_METHOD_PN_DELETEPORTMAPPING

void HDK_Method_PN_DeletePortMapping(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* If we didn't find the port mapping member to delete, or validating/setting the new port mapping array fails, return error */
    if (!(HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_PortMappings, pInput) &&
          HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_PortMappings, pInput)))
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_DeletePortMappingResult, HDK_Enum_Result_ERROR);
    }
}

#endif /* #ifdef HDK_METHOD_PN_DELETEPORTMAPPING */


/*
 * http://purenetworks.com/HNAP1/DownloadSpeedTest
 */
#ifdef HDK_METHOD_PN_DOWNLOADSPEEDTEST_TEST

static void HDK_Method_PN_StreamOut_ByteStream(void* pDeviceCtx, HDK_Struct* pInput,
                                             void* pMemberCtx, void* pStreamCtx, HDK_StreamOutFn pfnStreamOut)
{
    static char s_szStr[] = "OIYVBLLYUFKHVlhvggljhvyrtcjl";
    int cBytes;
    int iBytes;

    /* Unused parameters */
    (void)pStreamCtx;

    /* Stream the requested number of bytes */
    cBytes = HDK_Get_IntEx(pInput, HDK_Element_PN_Bytes, 0);
    for (iBytes = 0; iBytes < cBytes; iBytes += sizeof(s_szStr) - 1)
    {
        size_t cbStream = cBytes - iBytes;
        if (cbStream > sizeof(s_szStr) - 1)
        {
            cbStream = sizeof(s_szStr) - 1;
        }
        pfnStreamOut(pDeviceCtx, pMemberCtx, s_szStr, cbStream);
    }
}

void HDK_Method_PN_DownloadSpeedTest(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    int* piBytes;
    int* piBufSize;

    /* Unused parameters */
    (void)pDeviceCtx;

    /* Validate parameters */
    piBytes = HDK_Get_Int(pInput, HDK_Element_PN_Bytes);
    piBufSize = HDK_Get_Int(pInput, HDK_Element_PN_BufferSize);
    if (*piBytes < 0 || *piBufSize <= 0)
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_DownloadSpeedTestResult, HDK_Enum_Result_ERROR);
        return;
    }

    /* String-encoded Output stream */
    HDK_Set_Stream(pOutput, HDK_Element_PN_ByteStream, HDK_Method_PN_StreamOut_ByteStream, 0);
}

#endif /* #ifdef HDK_METHOD_PN_DOWNLOADSPEEDTEST */


/*
 * http://purenetworks.com/HNAP1/GetClientStats
 */
#ifdef HDK_METHOD_PN_GETCLIENTSTATS

void HDK_Method_PN_GetClientStats(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pInput;

    /* Set the client stats into the output struct */
    HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_ClientStats, 0);
}

#endif /* #ifdef HDK_METHOD_PN_GETCLIENTSTATS */


/*
 * http://purenetworks.com/HNAP1/GetConnectedDevices
 */
#ifdef HDK_METHOD_PN_GETCONNECTEDDEVICES

void HDK_Method_PN_GetConnectedDevices(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pInput;

    /* Set connected clients struct into output struct */
    if ( !HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_ConnectedClients, 0) )
	{
		HDK_Set_Result(pOutput, HDK_Element_PN_GetConnectedDevicesResult, HDK_Enum_Result_ERROR);
	}
}

#endif /* #ifdef HDK_METHOD_PN_GETCONNECTEDDEVICES */


/*
 * http://purenetworks.com/HNAP1/GetDeviceSettings
 */
#ifdef HDK_METHOD_PN_GETDEVICESETTINGS

void HDK_Method_PN_GetDeviceSettings(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pInput;


    /* Set device settings into output struct */
    if ( HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_DeviceType, 0) &&
		 HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_DeviceName, 0) &&
		 HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_VendorName, 0) &&
		 HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_ModelDescription, 0) &&
		 HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_ModelName, 0) &&
		 HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_FirmwareVersion, 0) &&
		 HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_PresentationURL, 0) &&
		 HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_SubDeviceURLs, 0) &&
		 HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_Tasks, 0) )
	{
		HDK_Set_PN_GetDeviceSettings_SOAPActions(pOutput);
		return;
	}
	else
	{
        HDK_Set_Result(pOutput, HDK_Element_PN_GetDeviceSettingsResult, HDK_Enum_Result_ERROR);
		return;
	}
}

#endif /* #ifdef HDK_METHOD_PN_GETDEVICESETTINGS */


/*
 * http://purenetworks.com/HNAP1/GetDeviceSettings2
 */
#ifdef HDK_METHOD_PN_GETDEVICESETTINGS2

void HDK_Method_PN_GetDeviceSettings2(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pInput;

    /* Set device settings 2 into output struct */
	
    if ( !HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_SerialNumber, 0) ||
		!HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_TimeZone, 0) ||
		!HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_AutoAdjustDST, 0) ||
		!HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_Locale, 0) ||
		!HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_SSL, 0) ||
		!HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_SupportedLocales, 0) )
	{
		HDK_Set_Result(pOutput, HDK_Element_PN_GetDeviceSettings2Result, HDK_Enum_Result_ERROR);
	}
}

#endif /* #ifdef HDK_METHOD_PN_GETDEVICESETTINGS2 */


/*
 * http://purenetworks.com/HNAP1/GetFirmwareSettings
 */
#ifdef HDK_METHOD_PN_GETFIRMWARESETTINGS

void HDK_Method_PN_GetFirmwareSettings(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pInput;

    /* Set firmware settings into output struct */
	
    if ( !HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_FirmwareVersion, 0) ||
		!HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_VendorName, 0) ||
		!HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_ModelName, 0) ||
		!HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_ModelRevision, 0) ||
		!HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_FirmwareDate, 0) ||
		!HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_UpdateMethods, 0) )
	{
		HDK_Set_Result(pOutput, HDK_Element_PN_GetFirmwareSettingsResult, HDK_Enum_Result_ERROR);
	}
}

#endif /* #ifdef HDK_METHOD_PN_GETFIRMWARESETTINGS */


/*
 * http://purenetworks.com/HNAP1/GetMACFilters2
 */
#ifdef HDK_METHOD_PN_GETMACFILTERS2

void HDK_Method_PN_GetMACFilters2(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pInput;

    /* Set mac filters 2 values into output struct */
    HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_MACFilterEnabled, 0);
    HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_MACFilterAllowList, 0);
    HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_MACList, 0);
}

#endif /* #ifdef HDK_METHOD_PN_GETMACFILTERS2 */


/*
 * http://purenetworks.com/HNAP1/GetNetworkStats
 */
#ifdef HDK_METHOD_PN_GETNETWORKSTATS

void HDK_Method_PN_GetNetworkStats(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pInput;

    /* Set network stats values into output struct */
    HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_DeviceNetworkStats, 0);
}

#endif /* #ifdef HDK_METHOD_PN_GETNETWORKSTATS */


/*
 * http://purenetworks.com/HNAP1/GetPortMappings
 */
#ifdef HDK_METHOD_PN_GETPORTMAPPINGS

void HDK_Method_PN_GetPortMappings(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pInput;

    /* Set port mappings values into output struct */
	if (!HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_PortMappings, 0))
	{
		HDK_Set_Result(pOutput, HDK_Element_PN_GetPortMappingsResult, HDK_Enum_Result_ERROR);
	}
}

#endif /* #ifdef HDK_METHOD_PN_GETPORTMAPPINGS */


/*
 * http://purenetworks.com/HNAP1/GetRouterLanSettings2
 */
#ifdef HDK_METHOD_PN_GETROUTERLANSETTINGS2

void HDK_Method_PN_GetRouterLanSettings2(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    (void)pInput;

    /* Set router lan settings 2 values into output struct */
    if (!HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_LanIPAddress, 0)
            || !HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_LanSubnetMask, 0)
            || !HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_DHCPEnabled, 0)
            || !HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_DHCPFirstIP, 0)
            || !HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_DHCPLastIP, 0)
            || !HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_DHCPLeaseTime, 0)
            || !HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_DHCPReservations, 0))
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_GetRouterLanSettings2Result, HDK_Enum_Result_ERROR);
    }
}

#endif /* #ifdef HDK_METHOD_PN_GETROUTERLANSETTINGS2 */


/*
 * http://purenetworks.com/HNAP1/GetRouterSettings
 */
#ifdef HDK_METHOD_PN_GETROUTERSETTINGS

void HDK_Method_PN_GetRouterSettings(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pInput;

    /* Set router settings values into output struct */
	if(!HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_ManageRemote, 0)
	   || !HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_ManageWireless, 0)
       || !HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_DomainName, 0)
       || !HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WiredQoS, 0)
       || !HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WPSPin, 0))
	{
		HDK_Set_Result(pOutput, HDK_Element_PN_GetRouterSettingsResult, HDK_Enum_Result_ERROR);
	}
}

#endif /* #ifdef HDK_METHOD_PN_GETROUTERSETTINGS */


/*
 * http://purenetworks.com/HNAP1/GetWirelessClientCapabilities
 */
#ifdef HDK_METHOD_PN_GETWIRELESSCLIENTCAPABILITIES

void HDK_Method_PN_GetWirelessClientCapabilities(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pInput;

    /* Set wireless client capabilities into output struct */
    HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WiFiClientSupportedNetworkTypes, 0);
    HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WiFiClientSupportedSecurity, 0);
}

#endif /* #ifdef HDK_METHOD_PN_GETWIRELESSCLIENTCAPABILITIES */


/*
 * http://purenetworks.com/HNAP1/GetWirelessClientConnectionInfo
 */
#ifdef HDK_METHOD_PN_GETWIRELESSCLIENTCONNECTIONINFO

void HDK_Method_PN_GetWirelessClientConnectionInfo(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pInput;

    /* Set wireless client connection info into output struct */
    HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WiFiClientSSID, 0);
    HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WiFiClientFrequency, 0);
    HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WiFiClientMode, 0);
    HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WiFiClientChannelWidth, 0);
    HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WiFiClientChannel, 0);
    HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WiFiClientSignalStrength, 0);
    HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WiFiClientNoise, 0);
    HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WiFiClientLinkSpeedIn, 0);
    HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WiFiClientLinkSpeedOut, 0);
    HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WiFiClientWmmEnabled, 0);
}

#endif /* #ifdef HDK_METHOD_PN_GETWIRELESSCONNECTIONINFO */


/*
 * http://purenetworks.com/HNAP1/GetWirelessClientSettings
 */
#ifdef HDK_METHOD_PN_GETWIRELESSCLIENTSETTINGS

void HDK_Method_PN_GetWirelessClientSettings(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pInput;

    /* Set wireless client settings into output struct */
    HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WiFiClientSSID, 0);
    HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WiFiClientNetworkType, 0);
    HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WiFiClientSecurityEnabled, 0);
    HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WiFiClientSecurityType, 0);
    HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WiFiClientEncryption, 0);
    HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WiFiClientKey, 0);
    HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WiFiClientConnected, 0);
}

#endif /* #ifdef HDK_METHOD_PN_GETWIRELESSCLIENTSETTINGS */


/*
 * http://purenetworks.com/HNAP1/GetWLanRadioFrequencies
 */
#ifdef HDK_METHOD_PN_GETWLANRADIOFREQUENCIES

void HDK_Method_PN_GetWLanRadioFrequencies(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pInput;

    /* Set radio frequency infos into output struct */
    HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_RadioFrequencyInfos, 0);
}

#endif /* #ifdef HDK_METHOD_PN_GETWLANRADIOFREQUENCIES */


/*
 * http://purenetworks.com/HNAP1/GetWLanRadioSecurity
 */
#ifdef HDK_METHOD_PN_GETWLANRADIOSECURITY

void HDK_Method_PN_GetWLanRadioSecurity(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    HDK_Member* pmRadio = 0;
    HDK_Struct* psRadios;

    HDK_Struct sTemp;
    HDK_Struct_Init(&sTemp);

    /* First off, make sure this is a valid RadioID */
    HDK_Device_GetValue(pDeviceCtx, &sTemp, HDK_DeviceValue_RadioInfos, 0);
    if ((psRadios = HDK_Get_Struct(&sTemp, HDK_Element_PN_RadioInfos)) != 0)
    {
        char* pszRadioId = HDK_Get_String(pInput, HDK_Element_PN_RadioID);
        char* psz;

        /* Iterate over the RadioInfos */
        for (pmRadio = psRadios->pHead; pmRadio; pmRadio = pmRadio->pNext)
        {
            psz = HDK_Get_String(HDK_Get_StructMember(pmRadio), HDK_Element_PN_RadioID);
            if (psz && !strcmp(psz, pszRadioId))
            {
                /* Valid radio id */
                break;
            }
        }
    }

    /* If we don't have a radio info member, then return corresponding error */
    if (!pmRadio)
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_GetWLanRadioSecurityResult, HDK_Enum_Result_ERROR_BAD_RADIOID);
        goto finish;
    }

    /* Set wlan radio settings into the output struct */
    if (!HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WLanSecurityEnabled, pInput)
            //|| !HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WLanType, pInput)
            || !HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WLanEncryption, pInput)
            //|| !HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WLanKey, pInput)
            //|| !HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WLanKeyRenewal, pInput)
            || !HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WLanRadiusIP1, pInput))
            //|| !HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WLanRadiusPort1, pInput)
            //|| !HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WLanRadiusSecret1, pInput)
            //|| !HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WLanRadiusIP2, pInput)
            //|| !HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WLanRadiusPort2, pInput)
            //|| !HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WLanRadiusSecret2, pInput))
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_GetWLanRadioSecurityResult, HDK_Enum_Result_ERROR);
    }

finish:

    /* Free the temp struct */
    HDK_Struct_Free(&sTemp);
}

#endif /* #ifdef HDK_METHOD_PN_GETWLANRADIOSECURITY */


/*
 * http://purenetworks.com/HNAP1/GetWLanRadioSettings
 */
#ifdef HDK_METHOD_PN_GETWLANRADIOSETTINGS

void HDK_Method_PN_GetWLanRadioSettings(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    HDK_Member* pmRadio = 0;
    HDK_Struct* psRadios;

    HDK_Struct sTemp;
    HDK_Struct_Init(&sTemp);

    /* First off, make sure this is a valid RadioID */
    HDK_Device_GetValue(pDeviceCtx, &sTemp, HDK_DeviceValue_RadioInfos, 0);
    if ((psRadios = HDK_Get_Struct(&sTemp, HDK_Element_PN_RadioInfos)) != 0)
    {
        char* pszRadioId = HDK_Get_String(pInput, HDK_Element_PN_RadioID);
        char* psz;

        /* Iterate over the RadioInfos */
        for (pmRadio = psRadios->pHead; pmRadio; pmRadio = pmRadio->pNext)
        {
            psz = HDK_Get_String(HDK_Get_StructMember(pmRadio), HDK_Element_PN_RadioID);
            if (psz && !strcmp(psz, pszRadioId))
            {
                /* Valid radio id */
                break;
            }
        }
    }

    /* If we don't have a radio info member, then return corresponding error */
    if (!pmRadio)
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_GetWLanRadioSettingsResult, HDK_Enum_Result_ERROR_BAD_RADIOID);
        goto finish;
    }

    /* Set wlan radio settings into the output struct */
    if (!HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WLanEnabled, pInput)
            || !HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WLanMode, pInput)
            || !HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_MacAddress, pInput)
            || !HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WLanSSID, pInput)
            || !HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WLanSSIDBroadcast, pInput)
            || !HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WLanChannelWidth, pInput)
            || !HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WLanChannel, pInput)
            || !HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WLanSecondaryChannel, pInput)
            || !HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WLanQoS, pInput))
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_GetWLanRadioSettingsResult, HDK_Enum_Result_ERROR);
    }

finish:

    /* Free the temp struct */
    HDK_Struct_Free(&sTemp);
}

#endif /* #ifdef HDK_METHOD_PN_GETWLANRADIOSETTINGS */


/*
 * http://purenetworks.com/HNAP1/GetWLanRadios
 */
#ifdef HDK_METHOD_PN_GETWLANRADIOS

void HDK_Method_PN_GetWLanRadios(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pInput;

    /* Set radio infos into output struct */

    if ( !HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_RadioInfos, 0) )
	{
		HDK_Set_Result(pOutput, HDK_Element_PN_GetWLanRadiosResult, HDK_Enum_Result_ERROR);
	}
}

#endif /* #ifdef HDK_METHOD_PN_GETWLANRADIOS */


/*
 * http://purenetworks.com/HNAP1/GetWanInfo
 */
#ifdef HDK_METHOD_PN_GETWANINFO

void HDK_Method_PN_GetWanInfo(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pInput;

    /* Retrieve wan info into output struct */
	
    if ( !HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WanAutoDetectType, 0) ||
		!HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WanStatus, 0) ||
		!HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WanSupportedTypes, 0) )
	{
		HDK_Set_Result(pOutput, HDK_Element_PN_GetWanInfoResult, HDK_Enum_Result_ERROR);	
	}
}

#endif /* #ifdef HDK_METHOD_PN_GETWANINFO */


/*
 * http://purenetworks.com/HNAP1/GetWanSettings
 */
#ifdef HDK_METHOD_PN_GETWANSETTINGS

void HDK_Method_PN_GetWanSettings(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
/*
	if ( !HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WanType, 0) ||
		!HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WanUsername, 0) ||
		!HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WanIP, 0) ||
		!HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WanSubnetMask, 0) ||
        !HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WanGateway, 0) ||
		!HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WanDNSSettings, 0) ||
		!HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_MacAddress, 0) )
	{
		HDK_Set_Result(pOutput, HDK_Element_PN_GetWanSettingsResult, HDK_Enum_Result_ERROR);
		return;
	}
	return;
*/
    HDK_Struct sTemp;
    HDK_Enum_PN_WANType eWanType;
    HDK_IPAddress defaultIP = {0,0,0,0};

    /* Unused parameters */
    (void)pInput;

    /* Initialize temprary struct */
    HDK_Struct_Init(&sTemp);

    /* Retrieve the values needed for comparison first */
    HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WanType, 0);
    HDK_Device_GetValue(pDeviceCtx, &sTemp, HDK_DeviceValue_WanStatus, 0);
    eWanType = HDK_Get_PN_WANTypeEx(pOutput, HDK_Element_PN_Type, HDK_Enum_PN_WANType__UNKNOWN__);

    /*
     * If the WANType is not one of the following, then we need to set the username,
     * password, servicename, maxidletime, & autoreconnect into the output struct
     */
    if (eWanType != HDK_Enum_PN_WANType_DHCP && eWanType != HDK_Enum_PN_WANType_Static &&
        eWanType != HDK_Enum_PN_WANType_BridgedOnly && eWanType != HDK_Enum_PN_WANType_Dynamic1483Bridged &&
        eWanType != HDK_Enum_PN_WANType_Static1483Bridged && eWanType != HDK_Enum_PN_WANType_Static1483Routed &&
        eWanType != HDK_Enum_PN_WANType_StaticIPOA)
    {
        HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WanUsername, 0);
        HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WanPassword, 0);
        HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WanMaxIdleTime, 0);
        HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WanAutoReconnect, 0);

        /* The service name depends upon the WANType */
        if (eWanType == HDK_Enum_PN_WANType_BigPond)
        {
            HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WanAuthService, 0);
        }
        else if (eWanType == HDK_Enum_PN_WANType_DHCPPPPoE || eWanType == HDK_Enum_PN_WANType_StaticPPPoE ||
                 eWanType == HDK_Enum_PN_WANType_StaticPPPOA || eWanType == HDK_Enum_PN_WANType_DynamicPPPOA)
        {
            HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WanPPPoEService, 0);
        }
        else if (eWanType == HDK_Enum_PN_WANType_DynamicL2TP || eWanType == HDK_Enum_PN_WANType_DynamicPPTP ||
                 eWanType == HDK_Enum_PN_WANType_StaticL2TP || eWanType == HDK_Enum_PN_WANType_StaticPPTP)
        {
            HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WanLoginService, 0);
        }
        else
        {
            HDK_Set_String(pOutput, HDK_Element_PN_ServiceName, "");
        }
    }
    else
	{
        HDK_Set_String(pOutput, HDK_Element_PN_Username, "");
        HDK_Set_String(pOutput, HDK_Element_PN_Password, "");
        HDK_Set_String(pOutput, HDK_Element_PN_ServiceName, "");
        HDK_Set_Int(pOutput, HDK_Element_PN_MaxIdleTime, 0);
        HDK_Set_Bool(pOutput, HDK_Element_PN_AutoReconnect, 0);
    }

    if (eWanType == HDK_Enum_PN_WANType_DHCP || eWanType == HDK_Enum_PN_WANType_DHCPPPPoE ||
        eWanType == HDK_Enum_PN_WANType_DynamicL2TP || eWanType == HDK_Enum_PN_WANType_DynamicPPTP ||
        eWanType == HDK_Enum_PN_WANType_DynamicPPPOA || eWanType == HDK_Enum_PN_WANType_Dynamic1483Bridged ||
        eWanType == HDK_Enum_PN_WANType_BigPond)
    {
         HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WanIP, 0);
         HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WanSubnetMask, 0);
         HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WanGateway, 0);
    }
    /* If the WANType is bridged-only then leave the IPAddress, SubnetMask, and Gateway blank */
    else if (eWanType != HDK_Enum_PN_WANType_BridgedOnly)
    {
        /* The IPAddress should always be set for Static* WANTypes */
        HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WanIP, 0);

        /* The SubnetMask and Gateway should be set if not one of the following */
        if (eWanType != HDK_Enum_PN_WANType_StaticPPPoE && eWanType != HDK_Enum_PN_WANType_StaticPPPOA)
        {
            HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WanSubnetMask, 0);
            HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WanGateway, 0);
        }
        else
        {
            HDK_Set_IPAddress(pOutput, HDK_Element_PN_SubnetMask, &defaultIP);
            HDK_Set_IPAddress(pOutput, HDK_Element_PN_Gateway, &defaultIP);
        }
    }
    else
    {
            HDK_Set_IPAddress(pOutput, HDK_Element_PN_IPAddress, &defaultIP);
            HDK_Set_IPAddress(pOutput, HDK_Element_PN_SubnetMask, &defaultIP);
            HDK_Set_IPAddress(pOutput, HDK_Element_PN_Gateway, &defaultIP);
    }

    /* If the WANType is not bridged-only, then set DNSSettings in output struct */
    if (eWanType != HDK_Enum_PN_WANType_BridgedOnly)
    {
        HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WanDNSSettings, 0);
    }
    else
    {
        HDK_Struct* pDNS = HDK_Set_Struct(pOutput, HDK_Element_PN_DNS);

        if (pDNS)
        {
            HDK_Set_IPAddress(pDNS, HDK_Element_PN_Primary, &defaultIP);
            HDK_Set_IPAddress(pDNS, HDK_Element_PN_Secondary, &defaultIP);
        }
    }

    /* Set the remainging Wan settings into the output struct */
    HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WANMacAddress, 0);
    HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_WanMTU, 0);

    /* Free the temp struct */
    HDK_Struct_Free(&sTemp);
}

#endif /* #ifdef HDK_METHOD_PN_GETWANSETTINGS */


/*
 * http://purenetworks.com/HNAP1/RenewWanConnection
 */
#ifdef HDK_METHOD_PN_RENEWWANCONNECTION

void HDK_Method_PN_RenewWanConnection(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Validate and set renew wan timeout (setting triggers renew) */
    if (!HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_RenewWanTimeout, pInput) ||
        !HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_RenewWanTimeout, pInput))
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_RenewWanConnectionResult, HDK_Enum_Result_ERROR);
    }
}

#endif /* #ifdef HDK_METHOD_PN_RENEWWANCONNECTION */


/*
 * http://purenetworks.com/HNAP1/SetAccessPointMode
 */
#ifdef HDK_METHOD_PN_SETACCESSPOINTMODE

void HDK_Method_PN_SetAccessPointMode(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Validate and set access point mode. If either fail, return an error. */
    if (!HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_AccessPointMode, pInput) ||
        !HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_AccessPointMode, pInput))
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_SetAccessPointModeResult, HDK_Enum_Result_ERROR);
        return;
    }

    /* Set the new IPAddress in the output struct */
    HDK_Device_GetValue(pDeviceCtx, pOutput, HDK_DeviceValue_NewIPAddress, 0);
}

#endif /* #ifdef HDK_METHOD_PN_SETACCESSPOINTMODE */


/*
 * http://purenetworks.com/HNAP1/SetDeviceSettings
 */
#ifdef HDK_METHOD_PN_SETDEVICESETTINGS

void HDK_Method_PN_SetDeviceSettings(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Validate and set the device values. If any fail, return an error. */
    if (!(HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_DeviceName, pInput) &&
          HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_AdminPassword, pInput)) ||
        !(HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_DeviceName, pInput) &&
          HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_AdminPassword, pInput)))
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_SetDeviceSettingsResult, HDK_Enum_Result_ERROR);
    }
}

#endif /* #ifdef HDK_METHOD_PN_SETDEVICESETTINGS */


/*
 * http://purenetworks.com/HNAP1/SetDeviceSettings2
 */
#ifdef HDK_METHOD_PN_SETDEVICESETTINGS2

void HDK_Method_PN_SetDeviceSettings2(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    HDK_Struct sTemp;
    HDK_Struct_Init(&sTemp);
    char* pPNTimeZone = HDK_Get_String(pInput, HDK_Element_PN_TimeZone); /*RDKB-7468, CID-32919, if null time zone may not be supported*/
    char* psTempTimeZone = HDK_Get_String(&sTemp, HDK_Element_PN_TimeZone);
    char* pPNUserName = HDK_Get_String(pInput, HDK_Element_PN_Username);

    /*
     * Check if setting the username is supported by the device. If not, then we need to
     * return an error if the client is attempting to set the username to something
     * other than what was used to authenticate the HTTP POST (current device username)
     */
    if (!HDK_Device_CheckValue(pDeviceCtx, HDK_DeviceValue_UsernameSetAllowed))
    {
        HDK_Device_GetValue(pDeviceCtx, &sTemp, HDK_DeviceValue_Username, 0);

        if (!pPNUserName || strcmp(pPNUserName, HDK_Get_StringEx(&sTemp, HDK_Element_PN_Username, "")))
        {
            HDK_Set_Result(pOutput, HDK_Element_PN_SetDeviceSettings2Result, HDK_Enum_Result_ERROR_USERNAME_NOT_SUPPORTED);
            goto finish;
        }
    }

    /*
     * Check if setting the timezone is supported by the device. If not, then we need to
     * return an error if the client is attempting to set the timezone to something
     * other than what the current timezone setting is
     */
    if (!HDK_Device_CheckValue(pDeviceCtx, HDK_DeviceValue_TimeZoneSetAllowed))
    {
        HDK_Device_GetValue(pDeviceCtx, &sTemp, HDK_DeviceValue_TimeZone, 0);

        if (!pPNTimeZone || !psTempTimeZone || (strcmp(pPNTimeZone, psTempTimeZone) 
            && strcmp(pPNTimeZone, "")))
        {
            HDK_Set_Result(pOutput, HDK_Element_PN_SetDeviceSettings2Result, HDK_Enum_Result_ERROR_TIMEZONE_NOT_SUPPORTED);
            goto finish;
        }
    }

    /* If we're disabling SSL and RemoteSSL is enabled, return error if RemoteSSLNeedsSSL */
	/*
    if (!HDK_Get_BoolEx(pInput, HDK_Element_PN_SSL, 0))
    {
        HDK_Device_GetValue(pDeviceCtx, &sTemp, HDK_DeviceValue_RemoteSSL, 0);

        if (HDK_Get_BoolEx(&sTemp, HDK_Element_PN_RemoteSSL, 1) &&
            HDK_Device_CheckValue(pDeviceCtx, HDK_DeviceValue_RemoteSSLNeedsSSL))
        {
            HDK_Set_Result(pOutput, HDK_Element_PN_SetDeviceSettings2Result, HDK_Enum_Result_ERROR_REMOTE_SSL_NEEDS_SSL);
            goto finish;
        }
    }
	*/

    /* Validate the username and return corresponding error if device rejects */
    if (!HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_Username, pInput))
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_SetDeviceSettings2Result, HDK_Enum_Result_ERROR_USERNAME_NOT_SUPPORTED);
        goto finish;
    }

    /* Validate the timezone and return corresponding error if device rejects */
    if (!HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_TimeZone, pInput))
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_SetDeviceSettings2Result, HDK_Enum_Result_ERROR_TIMEZONE_NOT_SUPPORTED);
        goto finish;
    }

    /* Validate and set the remainging device values. If any fail, return an error. */
    if (!(HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_AutoAdjustDST, pInput) &&
          HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_SSL, pInput) &&
          HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_Locale, pInput)) ||
        !(HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_Username, pInput) &&
          HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_TimeZone, pInput) &&
          HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_AutoAdjustDST, pInput) &&
          HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_SSL, pInput) &&
          HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_Locale, pInput)))
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_SetDeviceSettings2Result, HDK_Enum_Result_ERROR);
    }

finish:

    /* Free the temp struct */
    HDK_Struct_Free(&sTemp);
}

#endif /* #ifdef HDK_METHOD_PN_SETDEVICESETTINGS2 */


/*
 * http://purenetworks.com/HNAP1/SetMACFilters2
 */
#ifdef HDK_METHOD_PN_SETMACFILTERS2

void HDK_Method_PN_SetMACFilters2(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    HDK_MACAddress* pMac1;
    HDK_MACAddress* pMac2;
    HDK_Member* pmMACInfo1;
    HDK_Member* pmMACInfo2;

    HDK_Struct* psMACList = HDK_Get_Struct(pInput, HDK_Element_PN_MACList);

    /* Verify that we don't have any duplicate MACs */
    for (pmMACInfo1 = psMACList->pHead; pmMACInfo1; pmMACInfo1 = pmMACInfo1->pNext)
    {
        pMac1 = HDK_Get_MACAddress(HDK_Get_StructMember(pmMACInfo1), HDK_Element_PN_MacAddress);

        for (pmMACInfo2 = psMACList->pHead; pmMACInfo2; pmMACInfo2 = pmMACInfo2->pNext)
        {
            pMac2 = HDK_Get_MACAddress(HDK_Get_StructMember(pmMACInfo2), HDK_Element_PN_MacAddress);

            /* If it's the same MACInfo member continue */
            if (pmMACInfo2 == pmMACInfo1)
            {
                continue;
            }
            if (HDK_Util_AreMACsDuplicate(pMac1, pMac2))
            {
                HDK_Set_Result(pOutput, HDK_Element_PN_SetMACFilters2Result, HDK_Enum_Result_ERROR);
                return;
            }
        }
    }

    /* Validate and set the device values. If any fail, return an error. */
    if (!(HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_MACFilterEnabled, pInput) &&
          HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_MACFilterAllowList, pInput) &&
          HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_MACList, pInput)) ||
        !(HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_MACFilterEnabled, pInput) &&
          HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_MACFilterAllowList, pInput) &&
          HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_MACList, pInput)))
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_SetMACFilters2Result, HDK_Enum_Result_ERROR);
    }
}

#endif /* #ifdef HDK_METHOD_PN_SETMACFILTERS2 */


/*
 * http://purenetworks.com/HNAP1/SetRouterLanSettings2
 */
#ifdef HDK_METHOD_PN_SETROUTERLANSETTINGS2

void HDK_Method_PN_SetRouterLanSettings2(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    int* pnLeaseTime;
    HDK_IPAddress* pRouterIP;
    HDK_IPAddress* pRouterSubnet;
    HDK_IPAddress* pIpFirst;
    HDK_IPAddress* pIpLast;

    /* Retrive values needed for validation */
    pRouterIP = HDK_Get_IPAddress(pInput, HDK_Element_PN_RouterIPAddress);
    pRouterSubnet = HDK_Get_IPAddress(pInput, HDK_Element_PN_RouterSubnetMask);
    pIpFirst = HDK_Get_IPAddress(pInput, HDK_Element_PN_IPAddressFirst);
    pIpLast = HDK_Get_IPAddress(pInput, HDK_Element_PN_IPAddressLast);

    /* Validate the router IP address */
    if ((!(pRouterIP->a || pRouterIP->b || pRouterIP->c || pRouterIP->d))
		|| (pRouterIP->a != 172|| pRouterIP->b != 16 || pRouterIP->c != 12))
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_SetRouterLanSettings2Result, HDK_Enum_Result_ERROR_BAD_IP_ADDRESS);
        return;
    }
	
    /* Validate the router subnet mask */
    if (!HDK_Util_ValidateSubnetMask(pRouterSubnet))
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_SetRouterLanSettings2Result, HDK_Enum_Result_ERROR_BAD_SUBNET);
        return;
    }

    /* Validate the DHCP range, returning corresponding error. */
    if (!HDK_Util_ValidateIPRange(pRouterIP, pRouterSubnet, pIpFirst, pIpLast))
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_SetRouterLanSettings2Result, HDK_Enum_Result_ERROR_BAD_IP_RANGE);
        return;
    }

    /* Validate the lease time */
    pnLeaseTime = HDK_Get_Int(pInput, HDK_Element_PN_LeaseTime);
    if (*pnLeaseTime <= 0)
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_SetRouterLanSettings2Result, HDK_Enum_Result_ERROR);
        return;
    }

    /* Retrieve the DHCP reservations for validation */
    {
        HDK_IPAddress* pIp1;
        HDK_IPAddress* pIp2;
        HDK_MACAddress* pMac1;
        HDK_MACAddress* pMac2;
        HDK_Member* pmDHCP1;
        HDK_Member* pmDHCP2;
        HDK_Struct* psDHCPs = HDK_Get_Struct(pInput, HDK_Element_PN_DHCPReservations);

        /* Check that the device supports DHCP reservations */
        if (!HDK_Device_CheckValue(pDeviceCtx, HDK_DeviceValue_DHCPReservationsAllowed))
        {
            HDK_Set_Result(pOutput, HDK_Element_PN_SetRouterLanSettings2Result, HDK_Enum_Result_ERROR_RESERVATIONS_NOT_SUPPORTED);
            return;
        }

		/* Check if DHCP is enabled */
		if (HDK_Get_BoolEx(pInput, HDK_Element_PN_DHCPServerEnabled, false) == false)
        {
            HDK_Set_Result(pOutput, HDK_Element_PN_SetRouterLanSettings2Result, HDK_Enum_Result_ERROR);
            return;
        }

        /*RDKB-7468, CID-32917, null check before use */
        if(psDHCPs)
        {
            /* Iterate over the DHCP reservations */
            for (pmDHCP1 = psDHCPs->pHead; pmDHCP1; pmDHCP1 = pmDHCP1->pNext)
            {
                /*
                 * Validate the IP address of the DHCP reservation is in the device subnet.
                 */
                pIp1 = HDK_Get_IPAddress(HDK_Get_StructMember(pmDHCP1), HDK_Element_PN_IPAddress);
                if (!HDK_Util_ValidateIPSubnet(pRouterIP, pRouterSubnet, pIp1) ||
                    HDK_Util_AreIPsDuplicate(pRouterIP, pIp1))
                {
                    HDK_Set_Result(pOutput, HDK_Element_PN_SetRouterLanSettings2Result, HDK_Enum_Result_ERROR_BAD_RESERVATION);
                    return;
                }
                /* Go through the DHCP reservations and validate that there are no duplicate IPs or MACs */
                pMac1 = HDK_Get_MACAddress(HDK_Get_StructMember(pmDHCP1), HDK_Element_PN_MacAddress);
                for (pmDHCP2 = psDHCPs->pHead; pmDHCP2; pmDHCP2 = pmDHCP2->pNext)
                {
                    /* If it's the same DHCP member continue */
                    if (pmDHCP2 == pmDHCP1)
                    {
                        continue;
                    }

                    pIp2 = HDK_Get_IPAddress(HDK_Get_StructMember(pmDHCP2), HDK_Element_PN_IPAddress);
                    pMac2 = HDK_Get_MACAddress(HDK_Get_StructMember(pmDHCP2), HDK_Element_PN_MacAddress);
                    if (HDK_Util_AreIPsDuplicate(pIp1, pIp2) || HDK_Util_AreMACsDuplicate(pMac1, pMac2))
                    {
                        HDK_Set_Result(pOutput, HDK_Element_PN_SetRouterLanSettings2Result, HDK_Enum_Result_ERROR_BAD_RESERVATION);
                        return;
                    }
                }
            }
        }

    }

    /* Validate the router ip and return correspond error if device rejects */
    if (!HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_LanIPAddress, pInput))
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_SetRouterLanSettings2Result, HDK_Enum_Result_ERROR_BAD_IP_ADDRESS);
        return;
    }
    /* Validate the router subnet mask and return corresponding error if device rejects */
    if (!HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_LanSubnetMask, pInput))
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_SetRouterLanSettings2Result, HDK_Enum_Result_ERROR_BAD_SUBNET);
        return;
    }

    /* Validate and set the remaining device values. If any fail, return an error. */
    if (!(HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_DHCPFirstIP, pInput) &&
          HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_DHCPLastIP, pInput) &&
          HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_DHCPLeaseTime, pInput) &&
          HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_DHCPReservations, pInput)) ||
        !(HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_LanIPAddress, pInput) &&
          HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_LanSubnetMask, pInput) &&
          HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_DHCPEnabled, pInput) &&
          HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_DHCPFirstIP, pInput) &&
          HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_DHCPLastIP, pInput) &&
          HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_DHCPLeaseTime, pInput) /*&&
          HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_DHCPReservations, pInput)*/))
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_SetRouterLanSettings2Result, HDK_Enum_Result_ERROR);
    }
}

#endif /* #ifdef HDK_METHOD_PN_SETROUTERLANSETTINGS2 */


/*
 * http://purenetworks.com/HNAP1/SetRouterSettings
 */
#ifdef HDK_METHOD_PN_SETROUTERSETTINGS

void HDK_Method_PN_SetRouterSettings(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Initialize temprary struct */
    HDK_Struct sTemp;
    HDK_Struct_Init(&sTemp);
    char* pPNDomainName = HDK_Get_String(pInput, HDK_Element_PN_DomainName); /*RDKB-7468, CID-32922, may return null, domain name not supported*/
    /* Check that the client is allowed to change the domain name if they are attempting to and validate */
    if ( !pPNDomainName || (strcmp(pPNDomainName, "") &&
         !HDK_Device_CheckValue(pDeviceCtx, HDK_DeviceValue_DomainNameChangeAllowed)) ||
        !HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_DomainName, pInput))
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_SetRouterSettingsResult, HDK_Enum_Result_ERROR_DOMAIN_NOT_SUPPORTED);
        goto finish;
    }

    /* Check that wired qos is allowed if the client is attempting to enable it and validate */
    if ((HDK_Get_BoolEx(pInput, HDK_Element_PN_WiredQoS, 0) &&
         !HDK_Device_CheckValue(pDeviceCtx, HDK_DeviceValue_WiredQoSAllowed)) ||
        !HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_WiredQoS, pInput))
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_SetRouterSettingsResult, HDK_Enum_Result_ERROR_QOS_NOT_SUPPORTED);
        goto finish;
    }

    /* Validate and set the remaining device values. If any fail, return an error. 
    if (!(HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_ManageRemote, pInput) &&
          HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_ManageWireless, pInput) &&
          HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_RemotePort, pInput) &&
          HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_RemoteSSL, pInput)) ||
        !(HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_ManageRemote, pInput) &&
          HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_ManageWireless, pInput) &&
          HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_RemotePort, pInput) &&
          HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_RemoteSSL, pInput) &&
          HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_DomainName, pInput) &&
          HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_WiredQoS, pInput)))*/
    if (!HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_ManageRemote, pInput)
        || !HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_RemoteSSL,pInput)
        || !HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_RemotePort, pInput))
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_SetRouterSettingsResult, HDK_Enum_Result_ERROR);
    }

finish:

    /* Free the temprorary struct */
    HDK_Struct_Free(&sTemp);
}

#endif /* #ifdef HDK_METHOD_PN_SETROUTERSETTINGS */


/*
 * http://purenetworks.com/HNAP1/SetWirelessClientSettings
 */
#ifdef HDK_METHOD_PN_SETWIRELESSCLIENTSETTINGS

void HDK_Method_PN_SetWirelessClientSettings(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    int* pfEnabled;

    HDK_Enum_PN_WiFiClientNetworkType eWiFiNetworkType = HDK_Enum_PN_WiFiClientNetworkType__UNKNOWN__;
    HDK_Member* pmNetworkType = 0;
    HDK_Struct* psNetworkTypes;

    HDK_Struct sTemp;
    HDK_Struct_Init(&sTemp);

    /* Validate network type */
    HDK_Device_GetValue(pDeviceCtx, &sTemp, HDK_DeviceValue_WiFiClientSupportedNetworkTypes, 0);
    if ((psNetworkTypes = HDK_Get_Struct(&sTemp, HDK_Element_PN_SupportedNetworkType)) != 0)
    {
        HDK_Enum_PN_WiFiClientNetworkType* pe;
        eWiFiNetworkType = HDK_Get_PN_WiFiClientNetworkTypeEx(pInput, HDK_Element_PN_NetworkType, HDK_Enum_PN_WiFiClientNetworkType__UNKNOWN__);

        /* Iterate over the NetworkType array, searching for the network type the client is attempting to set */
        for (pmNetworkType = psNetworkTypes->pHead; pmNetworkType; pmNetworkType = pmNetworkType->pNext)
        {
            pe = HDK_Get_PN_WiFiClientNetworkTypeMember(pmNetworkType);
            /*
             * If the network type we're trying to set is supported by this device,
             * then we're good.
             */
            if (pe && *pe == eWiFiNetworkType)
            {
                /* Valid network type */
                break;
            }
        }
    }

    /* If we don't have a wifi network type member, then return corresponding member */
    if (!pmNetworkType)
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_SetWirelessClientSettingsResult, HDK_Enum_Result_ERROR_NETWORK_TYPE_NOT_SUPPORTED);
        goto finish;
    }

    /* Skip WiFiClientSecurity and WiFiEncryption validation if the client is not enabling */
    pfEnabled = HDK_Get_Bool(pInput, HDK_Element_PN_SecurityEnabled);
    if (*pfEnabled)
    {
        char* pszKey;

        HDK_Enum_PN_WiFiClientSecurity eWiFiSecurity = HDK_Enum_PN_WiFiClientSecurity__UNKNOWN__;
        HDK_Enum_PN_WiFiEncryption eWiFiEncryption = HDK_Enum_PN_WiFiEncryption__UNKNOWN__;

        HDK_Member* pmEncryption = 0;
        HDK_Member* pmSecurityInfo = 0;
        HDK_Struct* psEncryptions;
        HDK_Struct* psSecurityInfos;

        /* Validate security type */
        HDK_Device_GetValue(pDeviceCtx, &sTemp, HDK_DeviceValue_WiFiClientSupportedSecurity, 0);
        if ((psSecurityInfos = HDK_Get_Struct(&sTemp, HDK_Element_PN_SupportedSecurity)) != 0)
        {
            HDK_Enum_PN_WiFiClientSecurity* pe;
            eWiFiSecurity = HDK_Get_PN_WiFiClientSecurityEx(pInput, HDK_Element_PN_SecurityType, HDK_Enum_PN_WiFiClientSecurity__UNKNOWN__);

            /* Iterate over the SecurityInfo array, searching for the security type the client is attempting to set */
            for (pmSecurityInfo = psSecurityInfos->pHead; pmSecurityInfo; pmSecurityInfo = pmSecurityInfo->pNext)
            {
                pe = HDK_Get_PN_WiFiClientSecurity(HDK_Get_StructMember(pmSecurityInfo), HDK_Element_PN_SecurityType);
                /*
                 * If the security type we're trying to set is supported by this device,
                 * and the value is not blank, or we're disabling then we're good.
                 */
                if (pe && *pe == eWiFiSecurity &&
                    *pe != HDK_Enum_PN_WiFiClientSecurity_)
                {
                    /* Valid security type */
                    break;
                }
            }
        }

        /* If we don't have a wifi security member, then return corresponding member */
        if (!pmSecurityInfo)
        {
            HDK_Set_Result(pOutput, HDK_Element_PN_SetWirelessClientSettingsResult, HDK_Enum_Result_ERROR_TYPE_NOT_SUPPORTED);
            goto finish;
        }

        /* Validate encryption */
        if ((psEncryptions = HDK_Get_Struct(HDK_Get_StructMember(pmSecurityInfo), HDK_Element_PN_Encryptions)) != 0)
        {
            HDK_Enum_PN_WiFiEncryption* pe;
            eWiFiEncryption = HDK_Get_PN_WiFiEncryptionEx(pInput, HDK_Element_PN_Encryption, HDK_Enum_PN_WiFiEncryption__UNKNOWN__);

            /* Iterate over the Encryptions array, searching for the encryption type the client is attempting to set */
            for (pmEncryption = psEncryptions->pHead; pmEncryption; pmEncryption = pmEncryption->pNext)
            {
                pe = HDK_Get_PN_WiFiEncryptionMember(pmEncryption);
                /*
                 * If the encryption type we're trying to set is supported by this device,
                 * and the value is not blank, or we're disabling then we're good.
                 */
                if (pe && *pe == eWiFiEncryption &&
                    *pe != HDK_Enum_PN_WiFiEncryption_)
                {
                    /* Valid encryption type */
                    break;
                }
            }
        }

        /* If we don't have an encryption type memeber, then return corresponding error */
        if (!pmEncryption)
        {
            HDK_Set_Result(pOutput, HDK_Element_PN_SetWirelessClientSettingsResult, HDK_Enum_Result_ERROR_ENCRYPTION_NOT_SUPPORTED);
            goto finish;
        }

        /* Validate if the key */
        pszKey = HDK_Get_String(pInput, HDK_Element_PN_Key);
        if (!HDK_Util_ValidateEncryptionKey(pszKey, eWiFiEncryption))
        {
            HDK_Set_Result(pOutput, HDK_Element_PN_SetWirelessClientSettingsResult, HDK_Enum_Result_ERROR_ILLEGAL_KEY_VALUE);
            goto finish;
        }
    }

    /* Validate and set the wireless client values. If any fail, return an error. */
    if (!(HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_WiFiClientSSID, pInput) &&
          HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_WiFiClientNetworkType, pInput) &&
          HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_WiFiClientSecurityEnabled, pInput) &&
          HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_WiFiClientSecurityType, pInput) &&
          HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_WiFiClientEncryption, pInput) &&
          HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_WiFiClientKey, pInput)) ||
        !(HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_WiFiClientSSID, pInput) &&
          HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_WiFiClientNetworkType, pInput) &&
          HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_WiFiClientSecurityEnabled, pInput) &&
          HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_WiFiClientSecurityType, pInput) &&
          HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_WiFiClientEncryption, pInput) &&
          HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_WiFiClientKey, pInput)))
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_SetWirelessClientSettingsResult, HDK_Enum_Result_ERROR);
    }

finish:

    /* Free the temprorary struct */
    HDK_Struct_Free(&sTemp);
}

#endif /* #ifdef HDK_METHOD_PN_SETWIRELESSCLIENTSETTINGS */


/*
 * http://purenetworks.com/HNAP1/SetWLanRadioFrequency
 */
#ifdef HDK_METHOD_PN_SETWLANRADIOFREQUENCY

void HDK_Method_PN_SetWLanRadioFrequency(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    HDK_Member* pmFrequency = 0;
    HDK_Member* pmFrequencyInfo = 0;
    HDK_Struct* psFrequencies;
    HDK_Struct* psFrequencyInfos;

    HDK_Struct sTemp;
    HDK_Struct_Init(&sTemp);

    /* First off, make sure this is a valid RadioID */
    HDK_Device_GetValue(pDeviceCtx, &sTemp, HDK_DeviceValue_RadioFrequencyInfos, 0);
    if ((psFrequencyInfos = HDK_Get_Struct(&sTemp, HDK_Element_PN_RadioFrequencyInfos)) != 0)
    {
        char* pszRadioId = HDK_Get_String(pInput, HDK_Element_PN_RadioID);
        char* psz;

        /* Iterate over the RadioFreqquencyInfos searching for the RadioID the client is attempting to set */
        for (pmFrequencyInfo = psFrequencyInfos->pHead; pmFrequencyInfo; pmFrequencyInfo = pmFrequencyInfo->pNext)
        {
            psz = HDK_Get_String(HDK_Get_StructMember(pmFrequencyInfo), HDK_Element_PN_RadioID);
            if (psz && !strcmp(psz, pszRadioId))
            {
                /* Valid radio id */
                break;
            }
        }
    }

    /* If we don't have a radio frequency info member, then return corresponding error */
    if (!pmFrequencyInfo)
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_SetWLanRadioFrequencyResult, HDK_Enum_Result_ERROR_BAD_RADIOID);
        goto finish;
    }

    /* Iterate the frequencies and make sure it is valid */
    if ((psFrequencies = HDK_Get_Struct(HDK_Get_StructMember(pmFrequencyInfo), HDK_Element_PN_Frequencies)) != 0)
    {
        int* piFrequency = HDK_Get_Int(pInput, HDK_Element_PN_Frequency);
        int* pi;

        /* Iterate over the Frequencies searching for the Frequency the client is attempting to set */
        for (pmFrequency = psFrequencies->pHead; pmFrequency; pmFrequency = pmFrequency->pNext)
        {
            pi = HDK_Get_IntMember(pmFrequency);
            if (pi && *pi == *piFrequency)
            {
                /* Valid frequency */
                break;
            }
        }
    }

    /* Validate and set the frequency in the device */
    if (!pmFrequency ||
        !HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_RadioFrequency, pInput) ||
        !HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_RadioFrequency, pInput))
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_SetWLanRadioFrequencyResult, HDK_Enum_Result_ERROR);
    }

finish:

    /* Free the temprorary struct */
    HDK_Struct_Free(&sTemp);
}

#endif /* #ifdef HDK_METHOD_PN_SETWLANRADIOFREQUENCY */


/*
 * http://purenetworks.com/HNAP1/SetWLanRadioSecurity
 */
#ifdef HDK_METHOD_PN_SETWLANRADIOSECURITY

void HDK_Method_PN_SetWLanRadioSecurity(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    int* pfEnabled;
    HDK_Enum_PN_WiFiSecurity* pe = HDK_Get_PN_WiFiSecurity(pInput, HDK_Element_PN_Type);
    HDK_Enum_PN_WiFiSecurity eWiFiSecurity = {0};
    HDK_Enum_PN_WiFiEncryption eWiFiEncryption = *HDK_Get_PN_WiFiEncryption(pInput, HDK_Element_PN_Encryption);
    HDK_Member* pmRadio = 0;
    HDK_Struct* psRadios;
    HDK_Struct sTemp;

    /* Initialize the temporary structure */
    HDK_Struct_Init(&sTemp);

    /*RDKB-7468, CID-33009, null check before use */
    if(pe)
    {
        memcpy(&eWiFiSecurity, pe, sizeof(HDK_Enum_PN_WiFiSecurity));
    }

    /* First off, make sure this is a valid RadioID */
    HDK_Device_GetValue(pDeviceCtx, &sTemp, HDK_DeviceValue_RadioInfos, 0);
    if ((psRadios = HDK_Get_Struct(&sTemp, HDK_Element_PN_RadioInfos)) != 0)
    {
        char* pszRadioId = HDK_Get_String(pInput, HDK_Element_PN_RadioID);
        char* psz;

        /* Iterate over the RadioInfos */
        for (pmRadio = psRadios->pHead; pmRadio; pmRadio = pmRadio->pNext)
        {
            psz = HDK_Get_String(HDK_Get_StructMember(pmRadio), HDK_Element_PN_RadioID);
            if (psz && !strcmp(psz, pszRadioId) && *(HDK_Get_Int(HDK_Get_StructMember(pmRadio), HDK_Element_PN_Frequency)) == 2)
            {
                /* Valid radio id */
                break;
            }
        }
    }

    /* If we don't have a radio info member, then return corresponding error */
    if (!pmRadio)
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_SetWLanRadioSecurityResult, HDK_Enum_Result_ERROR_BAD_RADIOID);
        goto finish;
    }

    /* Skip WiFiSecurity and WiFiEncryption validation if the client is not enabling */
    pfEnabled = HDK_Get_Bool(pInput, HDK_Element_PN_Enabled);
    if (*pfEnabled)
    {
        char* pszKey;

        HDK_Member* pmEncryption = 0;
        HDK_Member* pmSecurityInfo = 0;
        HDK_Struct* psEncryptions;
        HDK_Struct* psSecurityInfos;

        /* Validate security type */
        if ((psSecurityInfos = HDK_Get_Struct(HDK_Get_StructMember(pmRadio), HDK_Element_PN_SupportedSecurity)) != 0)
        {
            /* Iterate over the SecurityInfo array, searching for the security type the client is attempting to set */
            HDK_Enum_PN_WiFiSecurity* pe;
            for (pmSecurityInfo = psSecurityInfos->pHead; pmSecurityInfo; pmSecurityInfo = pmSecurityInfo->pNext)
            {
                pe = HDK_Get_PN_WiFiSecurity(HDK_Get_StructMember(pmSecurityInfo), HDK_Element_PN_SecurityType);
                /*
                 * If the security type we're trying to set is supported by this device,
                 * and the value is not blank, or we're disabling then we're good.
                 */
                if (pe && *pe == eWiFiSecurity &&
                    *pe != HDK_Enum_PN_WiFiSecurity_)
                {
                    /* Valid security type */
                    break;
                }
            }
        }

        /* If we don't have a wifi security member, then return corresponding member */
        if (!pmSecurityInfo)
        {
            HDK_Set_Result(pOutput, HDK_Element_PN_SetWLanRadioSecurityResult, HDK_Enum_Result_ERROR_TYPE_NOT_SUPPORTED);
            goto finish;
        }

        if ((psEncryptions = HDK_Get_Struct(HDK_Get_StructMember(pmSecurityInfo), HDK_Element_PN_Encryptions)) != 0)
        {
            /* Iterate over the Encryptions array, searching for the encryption type the client is attempting to set */
            HDK_Enum_PN_WiFiEncryption* pe;
            for (pmEncryption = psEncryptions->pHead; pmEncryption; pmEncryption = pmEncryption->pNext)
            {
                pe = HDK_Get_PN_WiFiEncryptionMember(pmEncryption);
                /*
                 * If the encryption type we're trying to set is supported by this device,
                 * and the value is not blank, or we're disabling then we're good.
                 */
                if (pe && *pe == eWiFiEncryption &&
                    *pe != HDK_Enum_PN_WiFiEncryption_)
                {
                    /* Valid encryption type */
                    break;
                }
            }
        }

        /* If we don't have an encryption type memeber, then return corresponding error */
        if (!pmEncryption)
        {
            HDK_Set_Result(pOutput, HDK_Element_PN_SetWLanRadioSecurityResult, HDK_Enum_Result_ERROR_ENCRYPTION_NOT_SUPPORTED);
            goto finish;
        }


        /* Validate if the key if not an WPA/WPA2-RADUIS type */
        pszKey = HDK_Get_String(pInput, HDK_Element_PN_Key);
        if (eWiFiSecurity != HDK_Enum_PN_WiFiSecurity_WPA_Enterprise &&
            eWiFiSecurity != HDK_Enum_PN_WiFiSecurity_WPA2_Enterprise &&
            eWiFiSecurity != HDK_Enum_PN_WiFiSecurity_WPA_WPA2_Enterprise &&
            !HDK_Util_ValidateEncryptionKey(pszKey, eWiFiEncryption))
        {
            HDK_Set_Result(pOutput, HDK_Element_PN_SetWLanRadioSecurityResult, HDK_Enum_Result_ERROR_ILLEGAL_KEY_VALUE);
            goto finish;
        }
    }
    else
    {
        /* Validate type and encryption */
        if (eWiFiSecurity == HDK_Enum_PN_WiFiSecurity__UNKNOWN__)
        {
            HDK_Set_Result(pOutput, HDK_Element_PN_SetWLanRadioSecurityResult, HDK_Enum_Result_ERROR_TYPE_NOT_SUPPORTED);
            goto finish;
        }
        else if (eWiFiEncryption == HDK_Enum_PN_WiFiEncryption__UNKNOWN__)
        {
            HDK_Set_Result(pOutput, HDK_Element_PN_SetWLanRadioSecurityResult, HDK_Enum_Result_ERROR_ENCRYPTION_NOT_SUPPORTED);
            goto finish;
        }
    }

    /* Validate the renewal if security type is an WPA variant, returning corresponding error if device doesn't allow value */
    if ((eWiFiSecurity == HDK_Enum_PN_WiFiSecurity_WPA_Personal || eWiFiSecurity == HDK_Enum_PN_WiFiSecurity_WPA2_Personal ||
		 eWiFiSecurity == HDK_Enum_PN_WiFiSecurity_WPA_WPA2_Personal || eWiFiSecurity == HDK_Enum_PN_WiFiSecurity_WPA_WPA2_Enterprise ||	
         eWiFiSecurity == HDK_Enum_PN_WiFiSecurity_WPA_Enterprise || eWiFiSecurity == HDK_Enum_PN_WiFiSecurity_WPA2_Enterprise) &&
        !HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_WLanKeyRenewal, pInput))
    {	
        HDK_Set_Result(pOutput, HDK_Element_PN_SetWLanRadioSecurityResult, HDK_Enum_Result_ERROR_KEY_RENEWAL_BAD_VALUE);
        goto finish;
    }

    /* Validate the radius server values if security is a RADIUS variant, returning corresponding error if device doesn't allow one of the values */
    if ((eWiFiSecurity == HDK_Enum_PN_WiFiSecurity_WPA_Enterprise ||
         eWiFiSecurity == HDK_Enum_PN_WiFiSecurity_WPA2_Enterprise ||
         eWiFiSecurity == HDK_Enum_PN_WiFiSecurity_WPA_WPA2_Enterprise) &&
        (!(HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_WLanRadiusIP1, pInput) &&
           HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_WLanRadiusPort1, pInput) &&
           HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_WLanRadiusSecret1, pInput) &&
           HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_WLanRadiusIP2, pInput) &&
           HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_WLanRadiusPort2, pInput) &&
           HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_WLanRadiusSecret2, pInput))))
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_SetWLanRadioSecurityResult, HDK_Enum_Result_ERROR_BAD_RADIUS_VALUES);
        goto finish;
    }

    /* Validate the remaining device values. If any fail, return an error. */
    if (!(HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_WLanSecurityEnabled, pInput) &&
          HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_WLanType, pInput) &&
          HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_WLanEncryption, pInput) &&
          HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_WLanKey, pInput)))
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_SetWLanRadioSecurityResult, HDK_Enum_Result_ERROR);
        goto finish;
    }

    /* Set the renewal if disabling or the security type is an WPA variant */
    if ((!*pfEnabled ||
         eWiFiSecurity == HDK_Enum_PN_WiFiSecurity_WPA_Personal || eWiFiSecurity == HDK_Enum_PN_WiFiSecurity_WPA2_Personal ||
         eWiFiSecurity == HDK_Enum_PN_WiFiSecurity_WPA_WPA2_Personal || eWiFiSecurity == HDK_Enum_PN_WiFiSecurity_WPA_WPA2_Enterprise ||
         eWiFiSecurity == HDK_Enum_PN_WiFiSecurity_WPA_Enterprise || eWiFiSecurity == HDK_Enum_PN_WiFiSecurity_WPA2_Enterprise) &&
        !HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_WLanKeyRenewal, pInput))
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_SetWLanRadioSecurityResult, HDK_Enum_Result_ERROR_KEY_RENEWAL_BAD_VALUE);
        goto finish;
    }

    /* Set the radius server values if disabling or the security type is a RADIUS variant */
    if ((!*pfEnabled ||
         eWiFiSecurity == HDK_Enum_PN_WiFiSecurity_WPA_Enterprise ||
         eWiFiSecurity == HDK_Enum_PN_WiFiSecurity_WPA2_Enterprise ||
         eWiFiSecurity == HDK_Enum_PN_WiFiSecurity_WPA_WPA2_Enterprise) &&
	   (!(HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_WLanRadiusIP1, pInput))))/* &&
           HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_WLanRadiusPort1, pInput) &&
           HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_WLanRadiusSecret1, pInput) &&
           HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_WLanRadiusIP2, pInput) &&
           HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_WLanRadiusPort2, pInput) &&
           HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_WLanRadiusSecret2, pInput))))*/
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_SetWLanRadioSecurityResult, HDK_Enum_Result_ERROR_BAD_RADIUS_VALUES);
        goto finish;
    }

    /* Set the remaining device values. If any fail, return an error. */
    if (!(HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_WLanSecurityEnabled, pInput) &&
          HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_WLanType, pInput) &&
          //HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_WLanEncryption, pInput) &&
          HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_WLanKey, pInput)))
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_SetWLanRadioSecurityResult, HDK_Enum_Result_ERROR);
    }

finish:

    /* Free the temprorary struct */
    HDK_Struct_Free(&sTemp);
}
#endif /* #ifdef HDK_METHOD_PN_SETWLANRADIOSECURITY */


/*
 * http://purenetworks.com/HNAP1/SetWLanRadioSettings
 */
#ifdef HDK_METHOD_PN_SETWLANRADIOSETTINGS
void HDK_Method_PN_SetWLanRadioSettings(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    int* pfEnabled = 0;
    int* piChannel = 0;
    int* piChannelWidth = 0;
    HDK_Enum_PN_WiFiMode* peWiFiMode;
    HDK_Member* pmChannel = 0;
    HDK_Member* pmRadio = 0;
    HDK_Struct* psChannels;
    HDK_Struct* psRadios;
	int *val;

    HDK_Struct sTemp;
    HDK_Struct_Init(&sTemp);

    /* Get the RadioInfos array from the device */
    HDK_Device_GetValue(pDeviceCtx, &sTemp, HDK_DeviceValue_RadioInfos, 0);

    /* Validate the radio id */
    if ((psRadios = HDK_Get_Struct(&sTemp, HDK_Element_PN_RadioInfos)) != 0)
    {
        char* pszRadioId = HDK_Get_String(pInput, HDK_Element_PN_RadioID);
        char* psz;

        /* Iterate over the RadioInfos, searching for the radio id value the client is attempting to set */
        for (pmRadio = psRadios->pHead; pmRadio; pmRadio = pmRadio->pNext)
        {
            psz = HDK_Get_String(HDK_Get_StructMember(pmRadio), HDK_Element_PN_RadioID);
			val = HDK_Get_Int(HDK_Get_StructMember(pmRadio), HDK_Element_PN_Frequency);
			
            if (psz && !strcmp(psz, pszRadioId))
            {
                /* Valid radio id */
                break;
            }
        }
    }

    /* If we don't have a radio info member, then return corresponding error */
    if (!pmRadio)
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_SetWLanRadioSettingsResult, HDK_Enum_Result_ERROR_BAD_RADIOID);
        goto finish;
    }

    /* Skip WiFiMode validation if the client is not enabling */
    pfEnabled = HDK_Get_Bool(pInput, HDK_Element_PN_Enabled);
    peWiFiMode = HDK_Get_PN_WiFiMode(pInput, HDK_Element_PN_Mode);
    if (*pfEnabled)
    {
        HDK_Member* pmMode = 0;
        HDK_Struct* psModes;

        if ((psModes = HDK_Get_Struct(HDK_Get_StructMember(pmRadio), HDK_Element_PN_SupportedModes)) != 0)
        {
            HDK_Enum_PN_WiFiMode* pe;

            /* Iterate over the supported modes array searching for the wifimode value the client is attempting to set */
            for (pmMode = psModes->pHead; pmMode; pmMode = pmMode->pNext)
            {
                pe = HDK_Get_PN_WiFiModeMember(pmMode);
                if (pe && *pe == *peWiFiMode)
                {
                    /* Valid wifi mode */
                    break;
                }
            }
        }

        /* If we don't have a wifi mode member, then return corresponding error */
        if (!pmMode)
        {
            HDK_Set_Result(pOutput, HDK_Element_PN_SetWLanRadioSettingsResult, HDK_Enum_Result_ERROR_BAD_MODE);
            goto finish;
        }
    }
    else if (*peWiFiMode == HDK_Enum_PN_WiFiMode__UNKNOWN__)
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_SetWLanRadioSettingsResult, HDK_Enum_Result_ERROR_BAD_MODE);
        goto finish;
    }

    /* Validate the channel width, return corresponding error */
    piChannelWidth = HDK_Get_Int(pInput, HDK_Element_PN_ChannelWidth);
    if (!(*piChannelWidth == 0 || *piChannelWidth == 20 || *piChannelWidth == 40) ||
        (*piChannelWidth == 40 &&
         !(*peWiFiMode == HDK_Enum_PN_WiFiMode_802_11n || *peWiFiMode == HDK_Enum_PN_WiFiMode_802_11bn ||
           *peWiFiMode == HDK_Enum_PN_WiFiMode_802_11bgn || *peWiFiMode == HDK_Enum_PN_WiFiMode_802_11gn ||
           *peWiFiMode == HDK_Enum_PN_WiFiMode_802_11an)))
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_SetWLanRadioSettingsResult, HDK_Enum_Result_ERROR_BAD_CHANNEL_WIDTH);
        goto finish;
    }

    /* Validate the channel */
    if ((psChannels = HDK_Get_Struct(HDK_Get_StructMember(pmRadio), HDK_Element_PN_Channels)) != 0)
    {
        int* pi;
        piChannel = HDK_Get_Int(pInput, HDK_Element_PN_Channel);

        /* Iterate over the channels array searching for the channel value */
        for (pmChannel = psChannels->pHead; pmChannel; pmChannel = pmChannel->pNext)
        {
            pi = HDK_Get_IntMember(pmChannel);
            if (pi && *pi == *piChannel)
            {
                /* Valid channel */
                break;
            }
        }
    }

    /* If we don't have a channel member, then return corresponding error */
    if (*piChannel && !pmChannel)
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_SetWLanRadioSettingsResult, HDK_Enum_Result_ERROR_BAD_CHANNEL);
        goto finish;
    }

    /* Validate the SSID if we're not disabling */
    if (*pfEnabled &&
        !HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_WLanSSID, pInput))
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_SetWLanRadioSettingsResult, HDK_Enum_Result_ERROR_BAD_SSID);
        goto finish;
    }

    /* Validate and set the remaining device values. If any fail, return an error. */
    if (!(HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_WLanEnabled, pInput) &&
          HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_WLanMode, pInput) &&
          HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_WLanSSIDBroadcast, pInput) &&
          HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_WLanChannelWidth, pInput) &&
          HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_WLanChannel, pInput) &&
          HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_WLanQoS, pInput) &&
          HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_WLanSecondaryChannel, pInput)) ||
        !(HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_WLanEnabled, pInput) &&
          HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_WLanMode, pInput) &&
          HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_WLanSSID, pInput) &&
          HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_WLanSSIDBroadcast, pInput) &&
          HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_WLanChannelWidth, pInput) &&
          HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_WLanChannel, pInput) &&
          HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_WLanQoS, pInput) //&&
          /*HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_WLanSecondaryChannel, pInput)*/))

    {
        HDK_Set_Result(pOutput, HDK_Element_PN_SetWLanRadioSettingsResult, HDK_Enum_Result_ERROR);
    }

finish:

    /* Free the temporary struct */
    HDK_Struct_Free(&sTemp);
}

#endif /* #ifdef HDK_METHOD_PN_SETWLANRADIOSETTINGS */


/*
 * http://purenetworks.com/HNAP1/SetWanSettings
 */
#ifdef HDK_METHOD_PN_SETWANSETTINGS

void HDK_Method_PN_SetWanSettings(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    int* piMTU;
    HDK_DeviceValue eServiceName = HDK_DeviceValue__UNKNOWN__;
    HDK_Enum_PN_WANType eWanType = HDK_Enum_PN_WANType__UNKNOWN__;
    HDK_Struct* psSupportedTypes;


    HDK_Struct sTemp;
    HDK_Struct_Init(&sTemp);

    /* Get the SupportedTypes array from the device */
    HDK_Device_GetValue(pDeviceCtx, &sTemp, HDK_DeviceValue_WanSupportedTypes, 0);

    /* First off, validate that the device supports the WANType requested */
    if ((psSupportedTypes = HDK_Get_Struct(&sTemp, HDK_Element_PN_SupportedTypes)) != 0)
    {
        HDK_Member* pmSupportedType;
        HDK_Enum_PN_WANType* peWanType = HDK_Get_PN_WANType(pInput, HDK_Element_PN_Type);
        HDK_Enum_PN_WANType* pe;

        /* Iterate over the SupportedInfo array, searching for the wan type value the client is attempting to set */
        for (pmSupportedType = psSupportedTypes->pHead; pmSupportedType; pmSupportedType = pmSupportedType->pNext)
        {
            pe = HDK_Get_PN_WANTypeMember(pmSupportedType);
            if (pe && *pe == *peWanType)
            {
                /* Valid WANType, save for use later */
                eWanType = *pe;
                break;
            }
        }
    }

    /* If we don't have a supported type member, then return corresponding error */
    if (eWanType == HDK_Enum_PN_WANType__UNKNOWN__)
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_SetWanSettingsResult, HDK_Enum_Result_ERROR_BAD_WANTYPE);
        goto finish;
    }

    /* Validate the MTU value */
    piMTU = HDK_Get_Int(pInput, HDK_Element_PN_MTU);

    /* If the MTU is less than 0 or between 1 and 127 or greater than 1500, return an error */
    if ((*piMTU < 0) || (*piMTU > 0 && *piMTU < 128) || (*piMTU > 1500) ||
        /* The following WANTypes have a max MTU of 1492 */
        ((eWanType == HDK_Enum_PN_WANType_DHCPPPPoE || eWanType == HDK_Enum_PN_WANType_StaticPPPoE ||
          eWanType == HDK_Enum_PN_WANType_Static1483Bridged || eWanType == HDK_Enum_PN_WANType_Dynamic1483Bridged ||
          eWanType == HDK_Enum_PN_WANType_Static1483Routed || eWanType == HDK_Enum_PN_WANType_StaticPPPOA ||
          eWanType == HDK_Enum_PN_WANType_DynamicPPPOA || eWanType == HDK_Enum_PN_WANType_StaticIPOA) &&
         (*piMTU > 1492)) ||
        /* The following WANTypes have a max MTU of 1452 */
        ((eWanType == HDK_Enum_PN_WANType_StaticPPTP || eWanType == HDK_Enum_PN_WANType_DynamicPPTP) &&
         (*piMTU > 1452)) ||
        /* The following WANTypes have a max MTU of 1460 */
        ((eWanType == HDK_Enum_PN_WANType_StaticL2TP || eWanType == HDK_Enum_PN_WANType_DynamicL2TP) &&
         (*piMTU > 1460)))
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_SetWanSettingsResult, HDK_Enum_Result_ERROR);
        goto finish;
    }

    /* If MTU is 0, validate that the device supports auto mtu */
    if ((*piMTU == 0) &&
        !HDK_Device_CheckValue(pDeviceCtx, HDK_DeviceValue_AutoMTUAllowed))
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_SetWanSettingsResult, HDK_Enum_Result_ERROR_AUTO_MTU_NOT_SUPPORTED);
        goto finish;
    }

    /* Validate the WanType and MTU */
    if (!(HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_WanType, pInput) &&
          HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_WanMTU, pInput)))
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_SetWanSettingsResult, HDK_Enum_Result_ERROR);
        goto finish;
    }

    /*
     * If the WANType is not one of the follow, then we need to validate the username, password,
     * max idle time, & auto reconnect.
     */
    if (!(eWanType == HDK_Enum_PN_WANType_DHCP || eWanType == HDK_Enum_PN_WANType_Static ||
          eWanType == HDK_Enum_PN_WANType_BridgedOnly || eWanType == HDK_Enum_PN_WANType_Dynamic1483Bridged ||
          eWanType == HDK_Enum_PN_WANType_Static1483Bridged || eWanType == HDK_Enum_PN_WANType_Static1483Routed ||
          eWanType == HDK_Enum_PN_WANType_StaticIPOA))
    {
        char* pszUsername;
        char* pszPassword = 0;
        int* piMaxIdleTime = 0;
        int* pfAutoReconnect = 0;

        /* If AutoReconnect is true, then MaxIdleTime must be 0 and vice-versa */
        piMaxIdleTime = HDK_Get_Int(pInput, HDK_Element_PN_MaxIdleTime);
        pfAutoReconnect = HDK_Get_Bool(pInput, HDK_Element_PN_AutoReconnect);

        if ((*pfAutoReconnect && *piMaxIdleTime != 0) ||
            (*piMaxIdleTime == 0 && !*pfAutoReconnect))
        {
            HDK_Set_Result(pOutput, HDK_Element_PN_SetWanSettingsResult, HDK_Enum_Result_ERROR);
            goto finish;
        }

        pszUsername = HDK_Get_String(pInput, HDK_Element_PN_Username);
        pszPassword = HDK_Get_String(pInput, HDK_Element_PN_Password);

        /* Make sure the username/password are not empty */
        if (strlen(pszUsername) == 0 || strlen(pszPassword) == 0)
        {
            HDK_Set_Result(pOutput, HDK_Element_PN_SetWanSettingsResult, HDK_Enum_Result_ERROR);
            goto finish;
        }

        /* Validate the following values */
        if (!(HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_WanUsername, pInput) &&
              HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_WanPassword, pInput) &&
              HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_WanMaxIdleTime, pInput) &&
              HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_WanAutoReconnect, pInput)))
        {
            HDK_Set_Result(pOutput, HDK_Element_PN_SetWanSettingsResult, HDK_Enum_Result_ERROR);
            goto finish;
        }

        /* The service name depends further upon the WANType */
        if (eWanType == HDK_Enum_PN_WANType_BigPond)
        {
            eServiceName = HDK_DeviceValue_WanAuthService;
        }
        else if (eWanType == HDK_Enum_PN_WANType_DHCPPPPoE || eWanType == HDK_Enum_PN_WANType_StaticPPPoE ||
                 eWanType == HDK_Enum_PN_WANType_StaticPPPOA || eWanType == HDK_Enum_PN_WANType_DynamicPPPOA)
        {
            eServiceName = HDK_DeviceValue_WanPPPoEService;
        }
        else if (eWanType == HDK_Enum_PN_WANType_DynamicL2TP || eWanType == HDK_Enum_PN_WANType_DynamicPPTP ||
                 eWanType == HDK_Enum_PN_WANType_StaticL2TP || eWanType == HDK_Enum_PN_WANType_StaticPPTP)
        {
            eServiceName = HDK_DeviceValue_WanLoginService;
        }

        /* Validate the service name if we need to */
        if (eServiceName != HDK_DeviceValue__UNKNOWN__ &&
            !HDK_Device_ValidateValue(pDeviceCtx, eServiceName, pInput))
        {
            HDK_Set_Result(pOutput, HDK_Element_PN_SetWanSettingsResult, HDK_Enum_Result_ERROR);
            goto finish;
        }
    }

    /* If WANType is one of the following, validate the IPAddress, SubnetMask, and Gateway */
    if (eWanType == HDK_Enum_PN_WANType_Static || eWanType == HDK_Enum_PN_WANType_StaticL2TP ||
        eWanType == HDK_Enum_PN_WANType_StaticPPPoE || eWanType == HDK_Enum_PN_WANType_StaticPPTP ||
        eWanType == HDK_Enum_PN_WANType_Static1483Bridged || eWanType == HDK_Enum_PN_WANType_Static1483Routed ||
        eWanType == HDK_Enum_PN_WANType_StaticPPPOA || eWanType == HDK_Enum_PN_WANType_StaticIPOA)
    {
        HDK_IPAddress* pIPAddr = HDK_Get_IPAddress(pInput, HDK_Element_PN_IPAddress);
        HDK_IPAddress* pSubnet = HDK_Get_IPAddress(pInput, HDK_Element_PN_SubnetMask);
        HDK_IPAddress* pGateway = HDK_Get_IPAddress(pInput, HDK_Element_PN_Gateway);

        /* Check that the IPAddress is not 0.0.0.0 */
        if (!(pIPAddr->a || pIPAddr->b || pIPAddr->c || pIPAddr->d))
        {
            HDK_Set_Result(pOutput, HDK_Element_PN_SetWanSettingsResult, HDK_Enum_Result_ERROR);
            goto finish;
        }

        /* Validate the format of the SubnetMask */
        if (!HDK_Util_ValidateSubnetMask(pSubnet))
        {
            HDK_Set_Result(pOutput, HDK_Element_PN_SetWanSettingsResult, HDK_Enum_Result_ERROR);
            goto finish;
        }

        /* If it's a static type, then verify that the gateway is in the subnet */
        if (eWanType == HDK_Enum_PN_WANType_Static &&
            !HDK_Util_ValidateIPSubnet(pIPAddr, pSubnet, pGateway))
        {
            HDK_Set_Result(pOutput, HDK_Element_PN_SetWanSettingsResult, HDK_Enum_Result_ERROR);
            goto finish;
        }

        /* Validate the IPAddress */
        if (!HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_WanIP, pInput))
        {
            HDK_Set_Result(pOutput, HDK_Element_PN_SetWanSettingsResult, HDK_Enum_Result_ERROR);
            goto finish;
        }

        /* If the WANType is not the following, then validate SubnetMask and Gateway */
        if (eWanType != HDK_Enum_PN_WANType_StaticPPPoE &&
            eWanType != HDK_Enum_PN_WANType_StaticPPPOA &&
            !(HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_WanSubnetMask, pInput) &&
              HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_WanGateway, pInput)))
        {
            HDK_Set_Result(pOutput, HDK_Element_PN_SetWanSettingsResult, HDK_Enum_Result_ERROR);
            goto finish;
        }
    }

    /* Handle the DNS settings validation */
    {
        HDK_Struct* psDNS = HDK_Get_Struct(pInput, HDK_Element_PN_DNS);
        HDK_IPAddress* pIPPrimary = HDK_Get_IPAddress(psDNS, HDK_Element_PN_Primary);

        if (eWanType == HDK_Enum_PN_WANType_Static)
        {
            /* DNS settings must have at lease a primary IPAddress for this type */
            if (!(pIPPrimary->a || pIPPrimary->b || pIPPrimary->c || pIPPrimary->d))
            {
                HDK_Set_Result(pOutput, HDK_Element_PN_SetWanSettingsResult, HDK_Enum_Result_ERROR);
                goto finish;
            }
        }
        else if (eWanType == HDK_Enum_PN_WANType_BridgedOnly)
        {
            HDK_IPAddress ipDefault = {0,0,0,0};
            HDK_IPAddress* pIPSecondary = HDK_Get_IPAddress(psDNS, HDK_Element_PN_Secondary);
            /* Tertiary is optional, so use the 'Ex' form of get with a default IPAddress */
            HDK_IPAddress* pIPTertiary = HDK_Get_IPAddressEx(psDNS, HDK_Element_PN_Tertiary, &ipDefault);

            /* DNS settings must all be blank for this type */
            if ((pIPPrimary->a || pIPPrimary->b || pIPPrimary->c || pIPPrimary->d) ||
                (pIPSecondary->a || pIPSecondary->b || pIPSecondary->c || pIPSecondary->d) ||
                (pIPTertiary->a || pIPTertiary->b || pIPTertiary->c || pIPTertiary->d))
            {
                HDK_Set_Result(pOutput, HDK_Element_PN_SetWanSettingsResult, HDK_Enum_Result_ERROR);
                goto finish;
            }
        }
    }

    /* Validate the remaining values, returning error if any fail */
    if (!(HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_WanDNSSettings, pInput) &&
          HDK_Device_ValidateValue(pDeviceCtx, HDK_DeviceValue_MacAddress, pInput)))
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_SetWanSettingsResult, HDK_Enum_Result_ERROR);
        goto finish;
    }
    /* Attempt to set the following device values, returning an error if any fail */
    if (!(HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_WanType, pInput) &&
          HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_WanMTU, pInput) &&
          HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_WanDNSSettings, pInput) &&
          HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_MacAddress, pInput)))
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_SetWanSettingsResult, HDK_Enum_Result_ERROR);
        goto finish;
    }

	/*
    if (!(eWanType == HDK_Enum_PN_WANType_DHCP || eWanType == HDK_Enum_PN_WANType_Static ||
          eWanType == HDK_Enum_PN_WANType_BridgedOnly || eWanType == HDK_Enum_PN_WANType_Dynamic1483Bridged ||
          eWanType == HDK_Enum_PN_WANType_Static1483Bridged || eWanType == HDK_Enum_PN_WANType_Static1483Routed ||
          eWanType == HDK_Enum_PN_WANType_StaticIPOA))
    {
        if (!(HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_WanUsername, pInput) &&
              HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_WanPassword, pInput) &&
              HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_WanMaxIdleTime, pInput) &&
              HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_WanAutoReconnect, pInput)))
        {
            HDK_Set_Result(pOutput, HDK_Element_PN_SetWanSettingsResult, HDK_Enum_Result_ERROR);
            goto finish;
        }

        if (eServiceName != HDK_DeviceValue__UNKNOWN__ &&
            !HDK_Device_SetValue(pDeviceCtx, eServiceName, pInput))
        {
            HDK_Set_Result(pOutput, HDK_Element_PN_SetWanSettingsResult, HDK_Enum_Result_ERROR);
            goto finish;
        }
    }
	*/

	/*
    if (eWanType == HDK_Enum_PN_WANType_Static || eWanType == HDK_Enum_PN_WANType_StaticL2TP ||
        eWanType == HDK_Enum_PN_WANType_StaticPPPoE || eWanType == HDK_Enum_PN_WANType_StaticPPTP ||
        eWanType == HDK_Enum_PN_WANType_Static1483Bridged || eWanType == HDK_Enum_PN_WANType_Static1483Routed ||
        eWanType == HDK_Enum_PN_WANType_StaticPPPOA || eWanType == HDK_Enum_PN_WANType_StaticIPOA)
    {
        if (!HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_WanIP, pInput))
        {
            HDK_Set_Result(pOutput, HDK_Element_PN_SetWanSettingsResult, HDK_Enum_Result_ERROR);
            goto finish;
        }

        if (eWanType != HDK_Enum_PN_WANType_StaticPPPoE &&
            eWanType != HDK_Enum_PN_WANType_StaticPPPOA &&
            !(HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_WanSubnetMask, pInput) &&
              HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_WanGateway, pInput)))
        {
            HDK_Set_Result(pOutput, HDK_Element_PN_SetWanSettingsResult, HDK_Enum_Result_ERROR);
            goto finish;
        }
    }*/

	HDK_Set_Result(pOutput, HDK_Element_PN_SetWanSettingsResult, HDK_Enum_Result_REBOOT);	
finish:

    /* Free the temporary struct */
    HDK_Struct_Free(&sTemp);
}

#endif /* #ifdef HDK_METHOD_PN_SETWANSETTINGS */

#ifdef HDK_METHOD_PN_SETBRIDGECONNECT

void HDK_Method_PN_SetBridgeConnect(void* pDeviceCtx, HDK_Struct* pInput, HDK_Struct* pOutput)
{
    /* Unused parameters */
    (void)pDeviceCtx;
    (void)pInput;
    (void)pOutput;

	if (!HDK_Device_SetValue(pDeviceCtx, HDK_DeviceValue_BridgeConnect, pInput))
    {
        HDK_Set_Result(pOutput, HDK_Element_PN_SetBridgeConnectResult, HDK_Enum_Result_ERROR);
    }
}

#endif /* #ifdef HDK_METHOD_PN_SETBRIDGECONNECT */

