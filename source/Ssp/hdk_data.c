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

#include "hdk_data.h"
#include "hdk_encode.h"
#include "hdk_methods.h"
#include "hdk_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*
 * Namespaces table
 */

static char* s_namespaces[] =
{
    "http://purenetworks.com/HNAP1/",
    "http://schemas.xmlsoap.org/soap/envelope/",
};

#define s_namespaces_GetString(ixNamespace) \
    (s_namespaces[(int)ixNamespace])

static char* s_namespaces_FindNamespace(char* pszNamespace, char* pszNamespaceEnd, unsigned int* pIXNamespace)
{
    unsigned int cchNamespace = (pszNamespaceEnd ? (unsigned int)(pszNamespaceEnd - pszNamespace) : strlen(pszNamespace));
    char** ppszNamespace;
    char** ppszNamespaceEnd = s_namespaces + sizeof(s_namespaces) / sizeof(*s_namespaces);
    for (ppszNamespace = s_namespaces; ppszNamespace != ppszNamespaceEnd; ppszNamespace++)
    {
        unsigned int cch = strlen(*ppszNamespace);
        if ((cch == cchNamespace && strncmp(*ppszNamespace, pszNamespace, cchNamespace) == 0))
        {
            *pIXNamespace = ppszNamespace - s_namespaces;
            return *ppszNamespace;
        }
    }
    return 0;
}


/*
 * HNAP element table
 */

typedef struct _HDK_ElementNode
{
    unsigned char ixNamespace;
    char* pszElement;
} HDK_ElementNode;

static int HDK_ElementNode_FindElement(char* pszNamespace, char* pszNamespaceEnd,
                                       char* pszElement, char* pszElementEnd,
                                       HDK_ElementNode* pelements, int cElements,
                                       unsigned int* pixElement)
{
    unsigned int ixNamespace;

    /* Find the namespace */
    if (!s_namespaces_FindNamespace(pszNamespace, pszNamespaceEnd, &ixNamespace))
    {
        return 0;
    }

    /* Binary search for the element */
    {
        unsigned int ix1 = 0;
        unsigned int ix2 = cElements - 1;
        while (ix1 <= ix2)
        {
            HDK_ElementNode* pElemNode = &pelements[(*pixElement = (ix1 + ix2) / 2)];
            int result = (ixNamespace < pElemNode->ixNamespace ? -1 :
                          (ixNamespace > pElemNode->ixNamespace ? 1 : 0));
            if (result == 0)
            {
                if (pszElementEnd)
                {
                    unsigned int cchElement = pszElementEnd - pszElement;
                    result = strncmp(pszElement, pElemNode->pszElement, cchElement);
                    if (result == 0 && *(pElemNode->pszElement + cchElement))
                    {
                        result = -1;
                    }
                }
                else
                {
                    result = strcmp(pszElement, pElemNode->pszElement);
                }
            }

            if (result == 0)
            {
                return 1;
            }
            else if (result < 0)
            {
                ix2 = *pixElement - 1;
            }
            else
            {
                ix1 = *pixElement + 1;
            }
        }
    }

    return 0;
}

static int HDK_ElementNode_FindElementFQ(char* pszElementFQ, char* pszElementFQEnd,
                                         HDK_ElementNode* pElements, int cElements,
                                         unsigned int* pixElement)
{
    char* pszNamespace;
    char* pszNamespaceEnd;
    char* pszElement = 0;

    /* Find the element */
    if (pszElementFQEnd)
    {
        char* p;
        for (p = pszElementFQEnd - 1; p != pszElementFQ; --p)
        {
            if (*p == '/')
            {
                pszElement = p + 1;
                break;
            }
        }
    }
    else
    {
        char* p;
        for (p = pszElementFQ; *p; ++p)
        {
            if (*p == '/')
            {
                pszElement = p + 1;
            }
        }
    }
    if (!pszElement)
    {
        return 0;
    }

    /* Search for the element */
    pszNamespace = pszElementFQ;
    pszNamespaceEnd = pszElement;
    if (HDK_ElementNode_FindElement(pszNamespace, pszNamespaceEnd, pszElement, pszElementFQEnd,
                                    pElements, cElements, pixElement))
    {
        return 1;
    }

    /* Namespace does not end with a '/'? */
    pszNamespaceEnd -= 1;
    if (pszNamespaceEnd > pszNamespace)
    {
        if (HDK_ElementNode_FindElement(pszNamespace, pszNamespaceEnd, pszElement, pszElementFQEnd,
                                        pElements, cElements, pixElement))
        {
            return 1;
        }
    }

    return 0;
}

static HDK_ElementNode s_elements[] =
{
    { /* HDK_Element__UNKNOWN__ */ 0, "" },
    { /* HDK_Element_PN_Active */ 0, "Active" },
    { /* HDK_Element_PN_AddPortMapping */ 0, "AddPortMapping" },
    { /* HDK_Element_PN_AddPortMappingResponse */ 0, "AddPortMappingResponse" },
    { /* HDK_Element_PN_AddPortMappingResult */ 0, "AddPortMappingResult" },
    { /* HDK_Element_PN_AdminPassword */ 0, "AdminPassword" },
    { /* HDK_Element_PN_AutoAdjustDST */ 0, "AutoAdjustDST" },
    { /* HDK_Element_PN_AutoReconnect */ 0, "AutoReconnect" },
    { /* HDK_Element_PN_Channel */ 0, "Channel" },
    { /* HDK_Element_PN_ChannelWidth */ 0, "ChannelWidth" },
    { /* HDK_Element_PN_Channels */ 0, "Channels" },
    { /* HDK_Element_PN_ConnectTime */ 0, "ConnectTime" },
    { /* HDK_Element_PN_ConnectedClient */ 0, "ConnectedClient" },
    { /* HDK_Element_PN_ConnectedClients */ 0, "ConnectedClients" },
    { /* HDK_Element_PN_DHCPReservation */ 0, "DHCPReservation" },
    { /* HDK_Element_PN_DHCPReservations */ 0, "DHCPReservations" },
    { /* HDK_Element_PN_DHCPServerEnabled */ 0, "DHCPServerEnabled" },
    { /* HDK_Element_PN_DNS */ 0, "DNS" },
    { /* HDK_Element_PN_DeletePortMapping */ 0, "DeletePortMapping" },
    { /* HDK_Element_PN_DeletePortMappingResponse */ 0, "DeletePortMappingResponse" },
    { /* HDK_Element_PN_DeletePortMappingResult */ 0, "DeletePortMappingResult" },
    { /* HDK_Element_PN_DeviceName */ 0, "DeviceName" },
    { /* HDK_Element_PN_DomainName */ 0, "DomainName" },
    { /* HDK_Element_PN_Enabled */ 0, "Enabled" },
    { /* HDK_Element_PN_Encryption */ 0, "Encryption" },
    { /* HDK_Element_PN_Encryptions */ 0, "Encryptions" },
    { /* HDK_Element_PN_EthernetPort */ 0, "EthernetPort" },
    { /* HDK_Element_PN_ExternalPort */ 0, "ExternalPort" },
    { /* HDK_Element_PN_FirmwareVersion */ 0, "FirmwareVersion" },
    { /* HDK_Element_PN_Frequency */ 0, "Frequency" },
    { /* HDK_Element_PN_Gateway */ 0, "Gateway" },
    { /* HDK_Element_PN_GetConnectedDevices */ 0, "GetConnectedDevices" },
    { /* HDK_Element_PN_GetConnectedDevicesResponse */ 0, "GetConnectedDevicesResponse" },
    { /* HDK_Element_PN_GetConnectedDevicesResult */ 0, "GetConnectedDevicesResult" },
    { /* HDK_Element_PN_GetDeviceSettings */ 0, "GetDeviceSettings" },
    { /* HDK_Element_PN_GetDeviceSettingsResponse */ 0, "GetDeviceSettingsResponse" },
    { /* HDK_Element_PN_GetDeviceSettingsResult */ 0, "GetDeviceSettingsResult" },
    { /* HDK_Element_PN_GetPortMappings */ 0, "GetPortMappings" },
    { /* HDK_Element_PN_GetPortMappingsResponse */ 0, "GetPortMappingsResponse" },
    { /* HDK_Element_PN_GetPortMappingsResult */ 0, "GetPortMappingsResult" },
    { /* HDK_Element_PN_GetRouterLanSettings2 */ 0, "GetRouterLanSettings2" },
    { /* HDK_Element_PN_GetRouterLanSettings2Response */ 0, "GetRouterLanSettings2Response" },
    { /* HDK_Element_PN_GetRouterLanSettings2Result */ 0, "GetRouterLanSettings2Result" },
    { /* HDK_Element_PN_GetRouterSettings */ 0, "GetRouterSettings" },
    { /* HDK_Element_PN_GetRouterSettingsResponse */ 0, "GetRouterSettingsResponse" },
    { /* HDK_Element_PN_GetRouterSettingsResult */ 0, "GetRouterSettingsResult" },
    { /* HDK_Element_PN_GetWLanRadioSecurity */ 0, "GetWLanRadioSecurity" },
    { /* HDK_Element_PN_GetWLanRadioSecurityResponse */ 0, "GetWLanRadioSecurityResponse" },
    { /* HDK_Element_PN_GetWLanRadioSecurityResult */ 0, "GetWLanRadioSecurityResult" },
    { /* HDK_Element_PN_GetWLanRadioSettings */ 0, "GetWLanRadioSettings" },
    { /* HDK_Element_PN_GetWLanRadioSettingsResponse */ 0, "GetWLanRadioSettingsResponse" },
    { /* HDK_Element_PN_GetWLanRadioSettingsResult */ 0, "GetWLanRadioSettingsResult" },
    { /* HDK_Element_PN_GetWLanRadios */ 0, "GetWLanRadios" },
    { /* HDK_Element_PN_GetWLanRadiosResponse */ 0, "GetWLanRadiosResponse" },
    { /* HDK_Element_PN_GetWLanRadiosResult */ 0, "GetWLanRadiosResult" },
    { /* HDK_Element_PN_GetWanSettings */ 0, "GetWanSettings" },
    { /* HDK_Element_PN_GetWanSettingsResponse */ 0, "GetWanSettingsResponse" },
    { /* HDK_Element_PN_GetWanSettingsResult */ 0, "GetWanSettingsResult" },
    { /* HDK_Element_PN_IPAddress */ 0, "IPAddress" },
    { /* HDK_Element_PN_IPAddressFirst */ 0, "IPAddressFirst" },
    { /* HDK_Element_PN_IPAddressLast */ 0, "IPAddressLast" },
    { /* HDK_Element_PN_InternalClient */ 0, "InternalClient" },
    { /* HDK_Element_PN_InternalPort */ 0, "InternalPort" },
    { /* HDK_Element_PN_IsDeviceReady */ 0, "IsDeviceReady" },
    { /* HDK_Element_PN_IsDeviceReadyResponse */ 0, "IsDeviceReadyResponse" },
    { /* HDK_Element_PN_IsDeviceReadyResult */ 0, "IsDeviceReadyResult" },
    { /* HDK_Element_PN_Key */ 0, "Key" },
    { /* HDK_Element_PN_KeyRenewal */ 0, "KeyRenewal" },
    { /* HDK_Element_PN_LeaseTime */ 0, "LeaseTime" },
    { /* HDK_Element_PN_Locale */ 0, "Locale" },
    { /* HDK_Element_PN_MTU */ 0, "MTU" },
    { /* HDK_Element_PN_MacAddress */ 0, "MacAddress" },
    { /* HDK_Element_PN_ManageRemote */ 0, "ManageRemote" },
    { /* HDK_Element_PN_ManageWireless */ 0, "ManageWireless" },
    { /* HDK_Element_PN_MaxIdleTime */ 0, "MaxIdleTime" },
    { /* HDK_Element_PN_Minutes */ 0, "Minutes" },
    { /* HDK_Element_PN_Mode */ 0, "Mode" },
    { /* HDK_Element_PN_ModelDescription */ 0, "ModelDescription" },
    { /* HDK_Element_PN_ModelName */ 0, "ModelName" },
    { /* HDK_Element_PN_Name */ 0, "Name" },
    { /* HDK_Element_PN_Password */ 0, "Password" },
    { /* HDK_Element_PN_PortMapping */ 0, "PortMapping" },
    { /* HDK_Element_PN_PortMappingDescription */ 0, "PortMappingDescription" },
    { /* HDK_Element_PN_PortMappingProtocol */ 0, "PortMappingProtocol" },
    { /* HDK_Element_PN_PortMappings */ 0, "PortMappings" },
    { /* HDK_Element_PN_PortName */ 0, "PortName" },
    { /* HDK_Element_PN_PresentationURL */ 0, "PresentationURL" },
    { /* HDK_Element_PN_Primary */ 0, "Primary" },
    { /* HDK_Element_PN_QoS */ 0, "QoS" },
    { /* HDK_Element_PN_RadioID */ 0, "RadioID" },
    { /* HDK_Element_PN_RadioInfo */ 0, "RadioInfo" },
    { /* HDK_Element_PN_RadioInfos */ 0, "RadioInfos" },
    { /* HDK_Element_PN_RadiusIP1 */ 0, "RadiusIP1" },
    { /* HDK_Element_PN_RadiusIP2 */ 0, "RadiusIP2" },
    { /* HDK_Element_PN_RadiusPort1 */ 0, "RadiusPort1" },
    { /* HDK_Element_PN_RadiusPort2 */ 0, "RadiusPort2" },
    { /* HDK_Element_PN_RadiusSecret1 */ 0, "RadiusSecret1" },
    { /* HDK_Element_PN_RadiusSecret2 */ 0, "RadiusSecret2" },
    { /* HDK_Element_PN_Reboot */ 0, "Reboot" },
    { /* HDK_Element_PN_RebootResponse */ 0, "RebootResponse" },
    { /* HDK_Element_PN_RebootResult */ 0, "RebootResult" },
    { /* HDK_Element_PN_RemotePort */ 0, "RemotePort" },
    { /* HDK_Element_PN_RemoteSSL */ 0, "RemoteSSL" },
    { /* HDK_Element_PN_RouterIPAddress */ 0, "RouterIPAddress" },
    { /* HDK_Element_PN_RouterSubnetMask */ 0, "RouterSubnetMask" },
    { /* HDK_Element_PN_SOAPActions */ 0, "SOAPActions" },
    { /* HDK_Element_PN_SSID */ 0, "SSID" },
    { /* HDK_Element_PN_SSIDBroadcast */ 0, "SSIDBroadcast" },
    { /* HDK_Element_PN_SSL */ 0, "SSL" },
    { /* HDK_Element_PN_Secondary */ 0, "Secondary" },
    { /* HDK_Element_PN_SecondaryChannel */ 0, "SecondaryChannel" },
    { /* HDK_Element_PN_SecondaryChannels */ 0, "SecondaryChannels" },
    { /* HDK_Element_PN_SecurityInfo */ 0, "SecurityInfo" },
    { /* HDK_Element_PN_SecurityType */ 0, "SecurityType" },
    { /* HDK_Element_PN_ServiceName */ 0, "ServiceName" },
    { /* HDK_Element_PN_SetBridgeConnect */ 0, "SetBridgeConnect" },
    { /* HDK_Element_PN_SetBridgeConnectResponse */ 0, "SetBridgeConnectResponse" },
    { /* HDK_Element_PN_SetBridgeConnectResult */ 0, "SetBridgeConnectResult" },
    { /* HDK_Element_PN_SetDeviceSettings */ 0, "SetDeviceSettings" },
    { /* HDK_Element_PN_SetDeviceSettings2 */ 0, "SetDeviceSettings2" },
    { /* HDK_Element_PN_SetDeviceSettings2Response */ 0, "SetDeviceSettings2Response" },
    { /* HDK_Element_PN_SetDeviceSettings2Result */ 0, "SetDeviceSettings2Result" },
    { /* HDK_Element_PN_SetDeviceSettingsResponse */ 0, "SetDeviceSettingsResponse" },
    { /* HDK_Element_PN_SetDeviceSettingsResult */ 0, "SetDeviceSettingsResult" },
    { /* HDK_Element_PN_SetRouterLanSettings2 */ 0, "SetRouterLanSettings2" },
    { /* HDK_Element_PN_SetRouterLanSettings2Response */ 0, "SetRouterLanSettings2Response" },
    { /* HDK_Element_PN_SetRouterLanSettings2Result */ 0, "SetRouterLanSettings2Result" },
    { /* HDK_Element_PN_SetRouterSettings */ 0, "SetRouterSettings" },
    { /* HDK_Element_PN_SetRouterSettingsResponse */ 0, "SetRouterSettingsResponse" },
    { /* HDK_Element_PN_SetRouterSettingsResult */ 0, "SetRouterSettingsResult" },
    { /* HDK_Element_PN_SetWLanRadioSecurity */ 0, "SetWLanRadioSecurity" },
    { /* HDK_Element_PN_SetWLanRadioSecurityResponse */ 0, "SetWLanRadioSecurityResponse" },
    { /* HDK_Element_PN_SetWLanRadioSecurityResult */ 0, "SetWLanRadioSecurityResult" },
    { /* HDK_Element_PN_SetWLanRadioSettings */ 0, "SetWLanRadioSettings" },
    { /* HDK_Element_PN_SetWLanRadioSettingsResponse */ 0, "SetWLanRadioSettingsResponse" },
    { /* HDK_Element_PN_SetWLanRadioSettingsResult */ 0, "SetWLanRadioSettingsResult" },
    { /* HDK_Element_PN_SubDeviceURLs */ 0, "SubDeviceURLs" },
    { /* HDK_Element_PN_SubnetMask */ 0, "SubnetMask" },
    { /* HDK_Element_PN_SupportedModes */ 0, "SupportedModes" },
    { /* HDK_Element_PN_SupportedSecurity */ 0, "SupportedSecurity" },
    { /* HDK_Element_PN_TaskExtension */ 0, "TaskExtension" },
    { /* HDK_Element_PN_Tasks */ 0, "Tasks" },
    { /* HDK_Element_PN_Tertiary */ 0, "Tertiary" },
    { /* HDK_Element_PN_TimeZone */ 0, "TimeZone" },
    { /* HDK_Element_PN_Type */ 0, "Type" },
    { /* HDK_Element_PN_URL */ 0, "URL" },
    { /* HDK_Element_PN_Username */ 0, "Username" },
    { /* HDK_Element_PN_VendorName */ 0, "VendorName" },
    { /* HDK_Element_PN_WPSPin */ 0, "WPSPin" },
    { /* HDK_Element_PN_WideChannel */ 0, "WideChannel" },
    { /* HDK_Element_PN_WideChannels */ 0, "WideChannels" },
    { /* HDK_Element_PN_WiredQoS */ 0, "WiredQoS" },
    { /* HDK_Element_PN_Wireless */ 0, "Wireless" },
    { /* HDK_Element_PN_int */ 0, "int" },
    { /* HDK_Element_PN_string */ 0, "string" },
    { /* HDK_Element__BODY__ */ 1, "Body" },
    { /* HDK_Element__ENVELOPE__ */ 1, "Envelope" },
    { /* HDK_Element__HEADER__ */ 1, "Header" },
};

#define s_elements_GetNode(element) \
    (&s_elements[element])

static HDK_Element s_elements_FindElement(char* pszNamespace, char* pszNamespaceEnd,
                                          char* pszElement, char* pszElementEnd)
{
    unsigned int ixElement;
    if (HDK_ElementNode_FindElement(pszNamespace, pszNamespaceEnd, pszElement, pszElementEnd,
                                    s_elements, sizeof(s_elements) / sizeof(*s_elements),
                                    &ixElement))
    {
        return (HDK_Element)ixElement;
    }

    return HDK_Element__UNKNOWN__;
}

static HDK_Element s_elements_FindElementFQ(char* pszElementFQ, char* pszElementFQEnd)
{
    unsigned int ixElement;
    if (HDK_ElementNode_FindElementFQ(pszElementFQ, pszElementFQEnd,
                                      s_elements, sizeof(s_elements) / sizeof(*s_elements),
                                      &ixElement))
    {
        return (HDK_Element)ixElement;
    }

    return HDK_Element__UNKNOWN__;
}


/*
 * HNAP element structure table
 */

typedef enum _HDK_ElementTreeProperties
{
    HDK_ElementTreeProp_Optional = 0x01,
    HDK_ElementTreeProp_Unbounded = 0x02,
    HDK_ElementTreeProp_OutputStruct = 0x04,
    HDK_ElementTreeProp_ErrorOutput = 0x08
} HDK_ElementTreeProperties;

typedef struct _HDK_ElementTreeNode
{
    unsigned int ixParent:8;
    unsigned int element:8;
    unsigned int type:6;
    unsigned int prop:4;
} HDK_ElementTreeNode;

static HDK_ElementTreeNode s_elementTree[] =
{
    { /* 0 */ 0, HDK_Element__ENVELOPE__, HDK_Type__UNKNOWN_ANY__, 0x00 },
    { /* 1 */ 0, HDK_Element__HEADER__, HDK_Type__UNKNOWN_ANY__, 0x00 },
    { /* 2 */ 0, HDK_Element__BODY__, HDK_Type__UNKNOWN__, 0x00 },
    { /* 3 */ 2, HDK_Element_PN_AddPortMapping, HDK_Type__STRUCT__, 0x00 },
    { /* 4 */ 2, HDK_Element_PN_AddPortMappingResponse, HDK_Type__STRUCT__, 0x04 },
    { /* 5 */ 2, HDK_Element_PN_DeletePortMapping, HDK_Type__STRUCT__, 0x00 },
    { /* 6 */ 2, HDK_Element_PN_DeletePortMappingResponse, HDK_Type__STRUCT__, 0x04 },
    { /* 7 */ 2, HDK_Element_PN_GetConnectedDevices, HDK_Type__STRUCT__, 0x00 },
    { /* 8 */ 2, HDK_Element_PN_GetConnectedDevicesResponse, HDK_Type__STRUCT__, 0x04 },
    { /* 9 */ 2, HDK_Element_PN_GetDeviceSettings, HDK_Type__STRUCT__, 0x00 },
    { /* 10 */ 2, HDK_Element_PN_GetDeviceSettingsResponse, HDK_Type__STRUCT__, 0x04 },
    { /* 11 */ 2, HDK_Element_PN_GetPortMappings, HDK_Type__STRUCT__, 0x00 },
    { /* 12 */ 2, HDK_Element_PN_GetPortMappingsResponse, HDK_Type__STRUCT__, 0x04 },
    { /* 13 */ 2, HDK_Element_PN_GetRouterLanSettings2, HDK_Type__STRUCT__, 0x00 },
    { /* 14 */ 2, HDK_Element_PN_GetRouterLanSettings2Response, HDK_Type__STRUCT__, 0x04 },
    { /* 15 */ 2, HDK_Element_PN_GetRouterSettings, HDK_Type__STRUCT__, 0x00 },
    { /* 16 */ 2, HDK_Element_PN_GetRouterSettingsResponse, HDK_Type__STRUCT__, 0x04 },
    { /* 17 */ 2, HDK_Element_PN_GetWLanRadioSecurity, HDK_Type__STRUCT__, 0x00 },
    { /* 18 */ 2, HDK_Element_PN_GetWLanRadioSecurityResponse, HDK_Type__STRUCT__, 0x04 },
    { /* 19 */ 2, HDK_Element_PN_GetWLanRadioSettings, HDK_Type__STRUCT__, 0x00 },
    { /* 20 */ 2, HDK_Element_PN_GetWLanRadioSettingsResponse, HDK_Type__STRUCT__, 0x04 },
    { /* 21 */ 2, HDK_Element_PN_GetWLanRadios, HDK_Type__STRUCT__, 0x00 },
    { /* 22 */ 2, HDK_Element_PN_GetWLanRadiosResponse, HDK_Type__STRUCT__, 0x04 },
    { /* 23 */ 2, HDK_Element_PN_GetWanSettings, HDK_Type__STRUCT__, 0x00 },
    { /* 24 */ 2, HDK_Element_PN_GetWanSettingsResponse, HDK_Type__STRUCT__, 0x04 },
    { /* 25 */ 2, HDK_Element_PN_IsDeviceReady, HDK_Type__STRUCT__, 0x00 },
    { /* 26 */ 2, HDK_Element_PN_IsDeviceReadyResponse, HDK_Type__STRUCT__, 0x04 },
    { /* 27 */ 2, HDK_Element_PN_Reboot, HDK_Type__STRUCT__, 0x00 },
    { /* 28 */ 2, HDK_Element_PN_RebootResponse, HDK_Type__STRUCT__, 0x04 },
    { /* 29 */ 2, HDK_Element_PN_SetBridgeConnect, HDK_Type__STRUCT__, 0x00 },
    { /* 30 */ 2, HDK_Element_PN_SetBridgeConnectResponse, HDK_Type__STRUCT__, 0x04 },
    { /* 31 */ 2, HDK_Element_PN_SetDeviceSettings, HDK_Type__STRUCT__, 0x00 },
    { /* 32 */ 2, HDK_Element_PN_SetDeviceSettingsResponse, HDK_Type__STRUCT__, 0x04 },
    { /* 33 */ 2, HDK_Element_PN_SetDeviceSettings2, HDK_Type__STRUCT__, 0x00 },
    { /* 34 */ 2, HDK_Element_PN_SetDeviceSettings2Response, HDK_Type__STRUCT__, 0x04 },
    { /* 35 */ 2, HDK_Element_PN_SetRouterLanSettings2, HDK_Type__STRUCT__, 0x00 },
    { /* 36 */ 2, HDK_Element_PN_SetRouterLanSettings2Response, HDK_Type__STRUCT__, 0x04 },
    { /* 37 */ 2, HDK_Element_PN_SetRouterSettings, HDK_Type__STRUCT__, 0x00 },
    { /* 38 */ 2, HDK_Element_PN_SetRouterSettingsResponse, HDK_Type__STRUCT__, 0x04 },
    { /* 39 */ 2, HDK_Element_PN_SetWLanRadioSecurity, HDK_Type__STRUCT__, 0x00 },
    { /* 40 */ 2, HDK_Element_PN_SetWLanRadioSecurityResponse, HDK_Type__STRUCT__, 0x04 },
    { /* 41 */ 2, HDK_Element_PN_SetWLanRadioSettings, HDK_Type__STRUCT__, 0x00 },
    { /* 42 */ 2, HDK_Element_PN_SetWLanRadioSettingsResponse, HDK_Type__STRUCT__, 0x04 },
    { /* 43 */ 3, HDK_Element_PN_PortMappingDescription, HDK_Type__STRING__, 0x00 },
    { /* 44 */ 3, HDK_Element_PN_InternalClient, HDK_Type__IPADDRESS__, 0x00 },
    { /* 45 */ 3, HDK_Element_PN_PortMappingProtocol, HDK_Type_PN_IPProtocol, 0x00 },
    { /* 46 */ 3, HDK_Element_PN_ExternalPort, HDK_Type__INT__, 0x00 },
    { /* 47 */ 3, HDK_Element_PN_InternalPort, HDK_Type__INT__, 0x00 },
    { /* 48 */ 4, HDK_Element_PN_AddPortMappingResult, HDK_Type__RESULT__, 0x08 },
    { /* 49 */ 5, HDK_Element_PN_PortMappingProtocol, HDK_Type_PN_IPProtocol, 0x00 },
    { /* 50 */ 5, HDK_Element_PN_ExternalPort, HDK_Type__INT__, 0x00 },
    { /* 51 */ 6, HDK_Element_PN_DeletePortMappingResult, HDK_Type__RESULT__, 0x08 },
    { /* 52 */ 8, HDK_Element_PN_GetConnectedDevicesResult, HDK_Type__RESULT__, 0x08 },
    { /* 53 */ 8, HDK_Element_PN_ConnectedClients, HDK_Type__STRUCT__, 0x00 },
    { /* 54 */ 10, HDK_Element_PN_GetDeviceSettingsResult, HDK_Type__RESULT__, 0x08 },
    { /* 55 */ 10, HDK_Element_PN_Type, HDK_Type_PN_DeviceType, 0x00 },
    { /* 56 */ 10, HDK_Element_PN_DeviceName, HDK_Type__STRING__, 0x00 },
    { /* 57 */ 10, HDK_Element_PN_VendorName, HDK_Type__STRING__, 0x00 },
    { /* 58 */ 10, HDK_Element_PN_ModelDescription, HDK_Type__STRING__, 0x00 },
    { /* 59 */ 10, HDK_Element_PN_ModelName, HDK_Type__STRING__, 0x00 },
    { /* 60 */ 10, HDK_Element_PN_FirmwareVersion, HDK_Type__STRING__, 0x00 },
    { /* 61 */ 10, HDK_Element_PN_PresentationURL, HDK_Type__STRING__, 0x00 },
    { /* 62 */ 10, HDK_Element_PN_SOAPActions, HDK_Type__STRUCT__, 0x00 },
    { /* 63 */ 10, HDK_Element_PN_SubDeviceURLs, HDK_Type__STRUCT__, 0x00 },
    { /* 64 */ 10, HDK_Element_PN_Tasks, HDK_Type__STRUCT__, 0x00 },
    { /* 65 */ 12, HDK_Element_PN_GetPortMappingsResult, HDK_Type__RESULT__, 0x08 },
    { /* 66 */ 12, HDK_Element_PN_PortMappings, HDK_Type__STRUCT__, 0x00 },
    { /* 67 */ 14, HDK_Element_PN_GetRouterLanSettings2Result, HDK_Type__RESULT__, 0x08 },
    { /* 68 */ 14, HDK_Element_PN_RouterIPAddress, HDK_Type__IPADDRESS__, 0x00 },
    { /* 69 */ 14, HDK_Element_PN_RouterSubnetMask, HDK_Type__IPADDRESS__, 0x00 },
    { /* 70 */ 14, HDK_Element_PN_DHCPServerEnabled, HDK_Type__BOOL__, 0x00 },
    { /* 71 */ 14, HDK_Element_PN_IPAddressFirst, HDK_Type__IPADDRESS__, 0x00 },
    { /* 72 */ 14, HDK_Element_PN_IPAddressLast, HDK_Type__IPADDRESS__, 0x00 },
    { /* 73 */ 14, HDK_Element_PN_LeaseTime, HDK_Type__INT__, 0x00 },
    { /* 74 */ 14, HDK_Element_PN_DHCPReservations, HDK_Type__STRUCT__, 0x00 },
    { /* 75 */ 16, HDK_Element_PN_GetRouterSettingsResult, HDK_Type__RESULT__, 0x08 },
    { /* 76 */ 16, HDK_Element_PN_ManageRemote, HDK_Type__BOOL__, 0x00 },
    { /* 77 */ 16, HDK_Element_PN_ManageWireless, HDK_Type__BOOL__, 0x00 },
    { /* 78 */ 16, HDK_Element_PN_RemotePort, HDK_Type__INT__, 0x00 },
    { /* 79 */ 16, HDK_Element_PN_RemoteSSL, HDK_Type__BOOL__, 0x00 },
    { /* 80 */ 16, HDK_Element_PN_DomainName, HDK_Type__STRING__, 0x00 },
    { /* 81 */ 16, HDK_Element_PN_WiredQoS, HDK_Type__BOOL__, 0x00 },
    { /* 82 */ 16, HDK_Element_PN_WPSPin, HDK_Type__STRING__, 0x00 },
    { /* 83 */ 17, HDK_Element_PN_RadioID, HDK_Type__STRING__, 0x00 },
    { /* 84 */ 18, HDK_Element_PN_GetWLanRadioSecurityResult, HDK_Type__RESULT__, 0x08 },
    { /* 85 */ 18, HDK_Element_PN_Enabled, HDK_Type__BOOL__, 0x00 },
    { /* 86 */ 18, HDK_Element_PN_Type, HDK_Type_PN_WiFiSecurity, 0x00 },
    { /* 87 */ 18, HDK_Element_PN_Encryption, HDK_Type_PN_WiFiEncryption, 0x00 },
    { /* 88 */ 18, HDK_Element_PN_Key, HDK_Type__STRING__, 0x00 },
    { /* 89 */ 18, HDK_Element_PN_KeyRenewal, HDK_Type__INT__, 0x00 },
    { /* 90 */ 18, HDK_Element_PN_RadiusIP1, HDK_Type__IPADDRESS__, 0x01 },
    { /* 91 */ 18, HDK_Element_PN_RadiusPort1, HDK_Type__INT__, 0x01 },
    { /* 92 */ 18, HDK_Element_PN_RadiusSecret1, HDK_Type__STRING__, 0x01 },
    { /* 93 */ 18, HDK_Element_PN_RadiusIP2, HDK_Type__IPADDRESS__, 0x01 },
    { /* 94 */ 18, HDK_Element_PN_RadiusPort2, HDK_Type__INT__, 0x01 },
    { /* 95 */ 18, HDK_Element_PN_RadiusSecret2, HDK_Type__STRING__, 0x01 },
    { /* 96 */ 19, HDK_Element_PN_RadioID, HDK_Type__STRING__, 0x00 },
    { /* 97 */ 20, HDK_Element_PN_GetWLanRadioSettingsResult, HDK_Type__RESULT__, 0x08 },
    { /* 98 */ 20, HDK_Element_PN_Enabled, HDK_Type__BOOL__, 0x00 },
    { /* 99 */ 20, HDK_Element_PN_Mode, HDK_Type_PN_WiFiMode, 0x00 },
    { /* 100 */ 20, HDK_Element_PN_MacAddress, HDK_Type__MACADDRESS__, 0x00 },
    { /* 101 */ 20, HDK_Element_PN_SSID, HDK_Type__STRING__, 0x00 },
    { /* 102 */ 20, HDK_Element_PN_SSIDBroadcast, HDK_Type__BOOL__, 0x00 },
    { /* 103 */ 20, HDK_Element_PN_ChannelWidth, HDK_Type__INT__, 0x00 },
    { /* 104 */ 20, HDK_Element_PN_Channel, HDK_Type__INT__, 0x00 },
    { /* 105 */ 20, HDK_Element_PN_SecondaryChannel, HDK_Type__INT__, 0x00 },
    { /* 106 */ 20, HDK_Element_PN_QoS, HDK_Type__BOOL__, 0x00 },
    { /* 107 */ 22, HDK_Element_PN_GetWLanRadiosResult, HDK_Type__RESULT__, 0x08 },
    { /* 108 */ 22, HDK_Element_PN_RadioInfos, HDK_Type__STRUCT__, 0x00 },
    { /* 109 */ 24, HDK_Element_PN_GetWanSettingsResult, HDK_Type__RESULT__, 0x08 },
    { /* 110 */ 24, HDK_Element_PN_Type, HDK_Type_PN_WANType, 0x00 },
    { /* 111 */ 24, HDK_Element_PN_Username, HDK_Type__STRING__, 0x00 },
    { /* 112 */ 24, HDK_Element_PN_Password, HDK_Type__STRING__, 0x00 },
    { /* 113 */ 24, HDK_Element_PN_MaxIdleTime, HDK_Type__INT__, 0x00 },
    { /* 114 */ 24, HDK_Element_PN_MTU, HDK_Type__INT__, 0x00 },
    { /* 115 */ 24, HDK_Element_PN_ServiceName, HDK_Type__STRING__, 0x00 },
    { /* 116 */ 24, HDK_Element_PN_AutoReconnect, HDK_Type__BOOL__, 0x00 },
    { /* 117 */ 24, HDK_Element_PN_IPAddress, HDK_Type__IPADDRESS__, 0x00 },
    { /* 118 */ 24, HDK_Element_PN_SubnetMask, HDK_Type__IPADDRESS__, 0x00 },
    { /* 119 */ 24, HDK_Element_PN_Gateway, HDK_Type__IPADDRESS__, 0x00 },
    { /* 120 */ 24, HDK_Element_PN_DNS, HDK_Type__STRUCT__, 0x00 },
    { /* 121 */ 24, HDK_Element_PN_MacAddress, HDK_Type__MACADDRESS__, 0x00 },
    { /* 122 */ 26, HDK_Element_PN_IsDeviceReadyResult, HDK_Type__RESULT__, 0x08 },
    { /* 123 */ 28, HDK_Element_PN_RebootResult, HDK_Type__RESULT__, 0x08 },
    { /* 124 */ 29, HDK_Element_PN_EthernetPort, HDK_Type__INT__, 0x00 },
    { /* 125 */ 29, HDK_Element_PN_Minutes, HDK_Type__INT__, 0x00 },
    { /* 126 */ 30, HDK_Element_PN_SetBridgeConnectResult, HDK_Type__RESULT__, 0x08 },
    { /* 127 */ 31, HDK_Element_PN_DeviceName, HDK_Type__STRING__, 0x00 },
    { /* 128 */ 31, HDK_Element_PN_AdminPassword, HDK_Type__STRING__, 0x00 },
    { /* 129 */ 32, HDK_Element_PN_SetDeviceSettingsResult, HDK_Type__RESULT__, 0x08 },
    { /* 130 */ 33, HDK_Element_PN_Username, HDK_Type__STRING__, 0x00 },
    { /* 131 */ 33, HDK_Element_PN_TimeZone, HDK_Type__STRING__, 0x00 },
    { /* 132 */ 33, HDK_Element_PN_AutoAdjustDST, HDK_Type__BOOL__, 0x00 },
    { /* 133 */ 33, HDK_Element_PN_Locale, HDK_Type__STRING__, 0x00 },
    { /* 134 */ 33, HDK_Element_PN_SSL, HDK_Type__BOOL__, 0x00 },
    { /* 135 */ 34, HDK_Element_PN_SetDeviceSettings2Result, HDK_Type__RESULT__, 0x08 },
    { /* 136 */ 35, HDK_Element_PN_RouterIPAddress, HDK_Type__IPADDRESS__, 0x00 },
    { /* 137 */ 35, HDK_Element_PN_RouterSubnetMask, HDK_Type__IPADDRESS__, 0x00 },
    { /* 138 */ 35, HDK_Element_PN_DHCPServerEnabled, HDK_Type__BOOL__, 0x00 },
    { /* 139 */ 35, HDK_Element_PN_IPAddressFirst, HDK_Type__IPADDRESS__, 0x00 },
    { /* 140 */ 35, HDK_Element_PN_IPAddressLast, HDK_Type__IPADDRESS__, 0x00 },
    { /* 141 */ 35, HDK_Element_PN_LeaseTime, HDK_Type__INT__, 0x00 },
    { /* 142 */ 35, HDK_Element_PN_DHCPReservations, HDK_Type__STRUCT__, 0x00 },
    { /* 143 */ 36, HDK_Element_PN_SetRouterLanSettings2Result, HDK_Type__RESULT__, 0x08 },
    { /* 144 */ 37, HDK_Element_PN_ManageRemote, HDK_Type__BOOL__, 0x00 },
    { /* 145 */ 37, HDK_Element_PN_ManageWireless, HDK_Type__BOOL__, 0x00 },
    { /* 146 */ 37, HDK_Element_PN_RemotePort, HDK_Type__INT__, 0x00 },
    { /* 147 */ 37, HDK_Element_PN_RemoteSSL, HDK_Type__BOOL__, 0x00 },
    { /* 148 */ 37, HDK_Element_PN_DomainName, HDK_Type__STRING__, 0x00 },
    { /* 149 */ 37, HDK_Element_PN_WiredQoS, HDK_Type__BOOL__, 0x00 },
    { /* 150 */ 38, HDK_Element_PN_SetRouterSettingsResult, HDK_Type__RESULT__, 0x08 },
    { /* 151 */ 39, HDK_Element_PN_RadioID, HDK_Type__STRING__, 0x00 },
    { /* 152 */ 39, HDK_Element_PN_Enabled, HDK_Type__BOOL__, 0x00 },
    { /* 153 */ 39, HDK_Element_PN_Type, HDK_Type_PN_WiFiSecurity, 0x00 },
    { /* 154 */ 39, HDK_Element_PN_Encryption, HDK_Type_PN_WiFiEncryption, 0x00 },
    { /* 155 */ 39, HDK_Element_PN_Key, HDK_Type__STRING__, 0x00 },
    { /* 156 */ 39, HDK_Element_PN_KeyRenewal, HDK_Type__INT__, 0x00 },
    { /* 157 */ 39, HDK_Element_PN_RadiusIP1, HDK_Type__IPADDRESS__, 0x00 },
    { /* 158 */ 39, HDK_Element_PN_RadiusPort1, HDK_Type__INT__, 0x00 },
    { /* 159 */ 39, HDK_Element_PN_RadiusSecret1, HDK_Type__STRING__, 0x00 },
    { /* 160 */ 39, HDK_Element_PN_RadiusIP2, HDK_Type__IPADDRESS__, 0x00 },
    { /* 161 */ 39, HDK_Element_PN_RadiusPort2, HDK_Type__INT__, 0x00 },
    { /* 162 */ 39, HDK_Element_PN_RadiusSecret2, HDK_Type__STRING__, 0x00 },
    { /* 163 */ 40, HDK_Element_PN_SetWLanRadioSecurityResult, HDK_Type__RESULT__, 0x08 },
    { /* 164 */ 41, HDK_Element_PN_RadioID, HDK_Type__STRING__, 0x00 },
    { /* 165 */ 41, HDK_Element_PN_Enabled, HDK_Type__BOOL__, 0x00 },
    { /* 166 */ 41, HDK_Element_PN_Mode, HDK_Type_PN_WiFiMode, 0x00 },
    { /* 167 */ 41, HDK_Element_PN_SSID, HDK_Type__STRING__, 0x00 },
    { /* 168 */ 41, HDK_Element_PN_SSIDBroadcast, HDK_Type__BOOL__, 0x00 },
    { /* 169 */ 41, HDK_Element_PN_ChannelWidth, HDK_Type__INT__, 0x00 },
    { /* 170 */ 41, HDK_Element_PN_Channel, HDK_Type__INT__, 0x00 },
    { /* 171 */ 41, HDK_Element_PN_SecondaryChannel, HDK_Type__INT__, 0x00 },
    { /* 172 */ 41, HDK_Element_PN_QoS, HDK_Type__BOOL__, 0x00 },
    { /* 173 */ 42, HDK_Element_PN_SetWLanRadioSettingsResult, HDK_Type__RESULT__, 0x08 },
    { /* 174 */ 53, HDK_Element_PN_ConnectedClient, HDK_Type__STRUCT__, 0x03 },
    { /* 175 */ 62, HDK_Element_PN_string, HDK_Type__STRING__, 0x03 },
    { /* 176 */ 63, HDK_Element_PN_string, HDK_Type__STRING__, 0x03 },
    { /* 177 */ 64, HDK_Element_PN_TaskExtension, HDK_Type__STRUCT__, 0x03 },
    { /* 178 */ 66, HDK_Element_PN_PortMapping, HDK_Type__STRUCT__, 0x03 },
    { /* 179 */ 74, HDK_Element_PN_DHCPReservation, HDK_Type__STRUCT__, 0x03 },
    { /* 180 */ 108, HDK_Element_PN_RadioInfo, HDK_Type__STRUCT__, 0x03 },
    { /* 181 */ 120, HDK_Element_PN_Primary, HDK_Type__IPADDRESS__, 0x00 },
    { /* 182 */ 120, HDK_Element_PN_Secondary, HDK_Type__IPADDRESS__, 0x00 },
    { /* 183 */ 120, HDK_Element_PN_Tertiary, HDK_Type__IPADDRESS__, 0x01 },
    { /* 184 */ 142, HDK_Element_PN_DHCPReservation, HDK_Type__STRUCT__, 0x03 },
    { /* 185 */ 174, HDK_Element_PN_ConnectTime, HDK_Type__DATETIME__, 0x00 },
    { /* 186 */ 174, HDK_Element_PN_MacAddress, HDK_Type__MACADDRESS__, 0x00 },
    { /* 187 */ 174, HDK_Element_PN_DeviceName, HDK_Type__STRING__, 0x00 },
    { /* 188 */ 174, HDK_Element_PN_PortName, HDK_Type_PN_LANConnection, 0x00 },
    { /* 189 */ 174, HDK_Element_PN_Wireless, HDK_Type__BOOL__, 0x00 },
    { /* 190 */ 174, HDK_Element_PN_Active, HDK_Type__BOOL__, 0x00 },
    { /* 191 */ 177, HDK_Element_PN_Name, HDK_Type__STRING__, 0x00 },
    { /* 192 */ 177, HDK_Element_PN_URL, HDK_Type__STRING__, 0x00 },
    { /* 193 */ 177, HDK_Element_PN_Type, HDK_Type_PN_TaskExtType, 0x00 },
    { /* 194 */ 178, HDK_Element_PN_PortMappingDescription, HDK_Type__STRING__, 0x00 },
    { /* 195 */ 178, HDK_Element_PN_InternalClient, HDK_Type__IPADDRESS__, 0x00 },
    { /* 196 */ 178, HDK_Element_PN_PortMappingProtocol, HDK_Type_PN_IPProtocol, 0x00 },
    { /* 197 */ 178, HDK_Element_PN_ExternalPort, HDK_Type__INT__, 0x00 },
    { /* 198 */ 178, HDK_Element_PN_InternalPort, HDK_Type__INT__, 0x00 },
    { /* 199 */ 179, HDK_Element_PN_DeviceName, HDK_Type__STRING__, 0x00 },
    { /* 200 */ 179, HDK_Element_PN_IPAddress, HDK_Type__IPADDRESS__, 0x00 },
    { /* 201 */ 179, HDK_Element_PN_MacAddress, HDK_Type__MACADDRESS__, 0x00 },
    { /* 202 */ 180, HDK_Element_PN_RadioID, HDK_Type__STRING__, 0x00 },
    { /* 203 */ 180, HDK_Element_PN_Frequency, HDK_Type__INT__, 0x00 },
    { /* 204 */ 180, HDK_Element_PN_SupportedModes, HDK_Type__STRUCT__, 0x00 },
    { /* 205 */ 180, HDK_Element_PN_Channels, HDK_Type__STRUCT__, 0x00 },
    { /* 206 */ 180, HDK_Element_PN_WideChannels, HDK_Type__STRUCT__, 0x00 },
    { /* 207 */ 180, HDK_Element_PN_SupportedSecurity, HDK_Type__STRUCT__, 0x00 },
    { /* 208 */ 184, HDK_Element_PN_DeviceName, HDK_Type__STRING__, 0x00 },
    { /* 209 */ 184, HDK_Element_PN_IPAddress, HDK_Type__IPADDRESS__, 0x00 },
    { /* 210 */ 184, HDK_Element_PN_MacAddress, HDK_Type__MACADDRESS__, 0x00 },
    { /* 211 */ 204, HDK_Element_PN_string, HDK_Type_PN_WiFiMode, 0x03 },
    { /* 212 */ 205, HDK_Element_PN_int, HDK_Type__INT__, 0x03 },
    { /* 213 */ 206, HDK_Element_PN_WideChannel, HDK_Type__STRUCT__, 0x03 },
    { /* 214 */ 207, HDK_Element_PN_SecurityInfo, HDK_Type__STRUCT__, 0x03 },
    { /* 215 */ 213, HDK_Element_PN_Channel, HDK_Type__INT__, 0x00 },
    { /* 216 */ 213, HDK_Element_PN_SecondaryChannels, HDK_Type__STRUCT__, 0x00 },
    { /* 217 */ 214, HDK_Element_PN_SecurityType, HDK_Type_PN_WiFiSecurity, 0x00 },
    { /* 218 */ 214, HDK_Element_PN_Encryptions, HDK_Type__STRUCT__, 0x00 },
    { /* 219 */ 216, HDK_Element_PN_int, HDK_Type__INT__, 0x03 },
    { /* 220 */ 218, HDK_Element_PN_string, HDK_Type_PN_WiFiEncryption, 0x03 },
};

#define s_elementTree_GetNode(ixNode) \
    (&s_elementTree[ixNode])

static HDK_ElementTreeNode* s_elementTree_GetChildNode(unsigned int ixParent, HDK_Element element, unsigned int* pixChild)
{
    unsigned int ix1 = 0;
    unsigned int ix2 = sizeof(s_elementTree) / sizeof(*s_elementTree) - 1;
    while (ix1 <= ix2)
    {
        unsigned int ix = (ix1 + ix2) / 2;
        HDK_ElementTreeNode* pNode = &s_elementTree[ix];
        if (ixParent == pNode->ixParent)
        {
            HDK_ElementTreeNode* pElementTreeEnd = s_elementTree + sizeof(s_elementTree) / sizeof(*s_elementTree);
            HDK_ElementTreeNode* pNode2;
            for (pNode2 = pNode; pNode2 >= s_elementTree && pNode2->ixParent == ixParent; pNode2--)
            {
                if ((HDK_Element)pNode2->element == element)
                {
                    *pixChild = pNode2 - s_elementTree;
                    return pNode2;
                }
            }
            for (pNode2 = pNode + 1; pNode2 < pElementTreeEnd && pNode2->ixParent == ixParent; pNode2++)
            {
                if ((HDK_Element)pNode2->element == element)
                {
                    *pixChild = pNode2 - s_elementTree;
                    return pNode2;
                }
            }
            break;
        }
        else if (ixParent < pNode->ixParent)
        {
            ix2 = ix - 1;
        }
        else
        {
            ix1 = ix + 1;
        }
    }

    return 0;
}

static HDK_ElementTreeNode* s_elementTree_GetChildNodes(unsigned int ixParent, unsigned int* pixChildBegin, unsigned int* pixChildEnd)
{
    unsigned int ix1 = 0;
    unsigned int ix2 = sizeof(s_elementTree) / sizeof(*s_elementTree) - 1;
    while (ix1 <= ix2)
    {
        unsigned int ix = (ix1 + ix2) / 2;
        HDK_ElementTreeNode* pNode = &s_elementTree[ix];
        if (ixParent == pNode->ixParent)
        {
            HDK_ElementTreeNode* pElementTreeEnd = s_elementTree + sizeof(s_elementTree) / sizeof(*s_elementTree);
            HDK_ElementTreeNode* pNodeBegin;
            HDK_ElementTreeNode* pNodeEnd;
            for (pNodeBegin = pNode - 1; pNodeBegin >= s_elementTree && pNodeBegin->ixParent == ixParent; pNodeBegin--) {}
            *pixChildBegin = pNodeBegin - s_elementTree + 1;
            for (pNodeEnd = pNode + 1; pNodeEnd < pElementTreeEnd && pNodeEnd->ixParent == ixParent; pNodeEnd++) {}
            *pixChildEnd = pNodeEnd - s_elementTree;
            return pNodeBegin + 1;
        }
        else if (ixParent < pNode->ixParent)
        {
            ix2 = ix - 1;
        }
        else
        {
            ix1 = ix + 1;
        }
    }
    return 0;
}


/*
 * HNAP type interface
 */

typedef HDK_Member* (*HDK_TypeFn_New)(void);
typedef void (*HDK_TypeFn_Free)(HDK_Member* pMember);
typedef int (*HDK_TypeFn_Serialize)(void* pDeviceCtx, int fNoWrite, HDK_Member* pMember);
typedef int (*HDK_TypeFn_Deserialize)(HDK_Member* pMember, char* pszValue);

typedef struct _HDK_TypeInfo
{
    HDK_TypeFn_New pfnNew;
    HDK_TypeFn_Free pfnFree;
    HDK_TypeFn_Serialize pfnSerialize;
    HDK_TypeFn_Deserialize pfnDeserialize;
} HDK_TypeInfo;

static HDK_TypeInfo* s_types_GetInfo(HDK_Type type);


/*
 * HNAP generic member functions
 */

HDK_Member* HDK_Copy_Member(HDK_Struct* pStructDst, HDK_Element elementDst,
                            HDK_Member* pMemberSrc, int fAppend)
{
    HDK_Member* pMemberDst = 0;

    switch (pMemberSrc->type)
    {
        case HDK_Type__STRUCT__:
            if (!fAppend)
            {
                pMemberDst = (HDK_Member*)HDK_Set_Struct(pStructDst, elementDst);
            }
            else
            {
                pMemberDst = (HDK_Member*)HDK_Append_Struct(pStructDst, elementDst);
            }
            if (pMemberDst)
            {
                HDK_Member* pChild;
                for (pChild = ((HDK_Struct*)pMemberSrc)->pHead; pChild; pChild = pChild->pNext)
                {
                    if (!HDK_Copy_Member((HDK_Struct*)pMemberDst, pChild->element, pChild, 1))
                    {
                        pMemberSrc = 0;
                        break;
                    }
                }
            }
            break;

        case HDK_Type__BLANK__:
            if (!fAppend)
            {
                pMemberDst = HDK_Set_Blank(pStructDst, elementDst);
            }
            else
            {
                pMemberDst = HDK_Append_Blank(pStructDst, elementDst);
            }
            break;

        case HDK_Type__IPADDRESS__:
            {
                HDK_IPAddress* pIPAddress = HDK_Get_IPAddressMember(pMemberSrc);
                if (!fAppend)
                {
                    pMemberDst = HDK_Set_IPAddress(pStructDst, elementDst, pIPAddress);
                }
                else
                {
                    pMemberDst = HDK_Append_IPAddress(pStructDst, elementDst, pIPAddress);
                }
            }
            break;

        case HDK_Type__MACADDRESS__:
            {
                HDK_MACAddress* pMACAddress = HDK_Get_MACAddressMember(pMemberSrc);
                if (!fAppend)
                {
                    pMemberDst = HDK_Set_MACAddress(pStructDst, elementDst, pMACAddress);
                }
                else
                {
                    pMemberDst = HDK_Append_MACAddress(pStructDst, elementDst, pMACAddress);
                }
            }
            break;

        case HDK_Type__BOOL__:
            {
                int fValue = *HDK_Get_BoolMember(pMemberSrc);
                if (!fAppend)
                {
                    pMemberDst = HDK_Set_Bool(pStructDst, elementDst, fValue);
                }
                else
                {
                    pMemberDst = HDK_Append_Bool(pStructDst, elementDst, fValue);
                }
            }
            break;

        case HDK_Type__DATETIME__:
            {
                time_t tValue = *HDK_Get_DateTimeMember(pMemberSrc);
                if (!fAppend)
                {
                    pMemberDst = HDK_Set_DateTime(pStructDst, elementDst, tValue);
                }
                else
                {
                    pMemberDst = HDK_Append_DateTime(pStructDst, elementDst, tValue);
                }
            }
            break;

        case HDK_Type__INT__:
            {
                int iValue = *HDK_Get_IntMember(pMemberSrc);
                if (!fAppend)
                {
                    pMemberDst = HDK_Set_Int(pStructDst, elementDst, iValue);
                }
                else
                {
                    pMemberDst = HDK_Append_Int(pStructDst, elementDst, iValue);
                }
            }
            break;

        case HDK_Type__STRING__:
            {
                char* pszValue = HDK_Get_StringMember(pMemberSrc);
                if (!fAppend)
                {
                    pMemberDst = HDK_Set_String(pStructDst, elementDst, pszValue);
                }
                else
                {
                    pMemberDst = HDK_Append_String(pStructDst, elementDst, pszValue);
                }
            }
            break;

        case HDK_Type__RESULT__:
            {
                HDK_Enum_Result eValue = *HDK_Get_ResultMember(pMemberSrc);
                if (!fAppend)
                {
                    pMemberDst = HDK_Set_Result(pStructDst, elementDst, eValue);
                }
                else
                {
                    pMemberDst = HDK_Append_Result(pStructDst, elementDst, eValue);
                }
            }
            break;

        case HDK_Type_PN_DeviceType:
            {
                HDK_Enum_PN_DeviceType eValue = *HDK_Get_PN_DeviceTypeMember(pMemberSrc);
                if (!fAppend)
                {
                    pMemberDst = HDK_Set_PN_DeviceType(pStructDst, elementDst, eValue);
                }
                else
                {
                    pMemberDst = HDK_Append_PN_DeviceType(pStructDst, elementDst, eValue);
                }
            }
            break;

        case HDK_Type_PN_IPProtocol:
            {
                HDK_Enum_PN_IPProtocol eValue = *HDK_Get_PN_IPProtocolMember(pMemberSrc);
                if (!fAppend)
                {
                    pMemberDst = HDK_Set_PN_IPProtocol(pStructDst, elementDst, eValue);
                }
                else
                {
                    pMemberDst = HDK_Append_PN_IPProtocol(pStructDst, elementDst, eValue);
                }
            }
            break;

        case HDK_Type_PN_LANConnection:
            {
                HDK_Enum_PN_LANConnection eValue = *HDK_Get_PN_LANConnectionMember(pMemberSrc);
                if (!fAppend)
                {
                    pMemberDst = HDK_Set_PN_LANConnection(pStructDst, elementDst, eValue);
                }
                else
                {
                    pMemberDst = HDK_Append_PN_LANConnection(pStructDst, elementDst, eValue);
                }
            }
            break;

        case HDK_Type_PN_TaskExtType:
            {
                HDK_Enum_PN_TaskExtType eValue = *HDK_Get_PN_TaskExtTypeMember(pMemberSrc);
                if (!fAppend)
                {
                    pMemberDst = HDK_Set_PN_TaskExtType(pStructDst, elementDst, eValue);
                }
                else
                {
                    pMemberDst = HDK_Append_PN_TaskExtType(pStructDst, elementDst, eValue);
                }
            }
            break;

        case HDK_Type_PN_WANType:
            {
                HDK_Enum_PN_WANType eValue = *HDK_Get_PN_WANTypeMember(pMemberSrc);
                if (!fAppend)
                {
                    pMemberDst = HDK_Set_PN_WANType(pStructDst, elementDst, eValue);
                }
                else
                {
                    pMemberDst = HDK_Append_PN_WANType(pStructDst, elementDst, eValue);
                }
            }
            break;

        case HDK_Type_PN_WiFiEncryption:
            {
                HDK_Enum_PN_WiFiEncryption eValue = *HDK_Get_PN_WiFiEncryptionMember(pMemberSrc);
                if (!fAppend)
                {
                    pMemberDst = HDK_Set_PN_WiFiEncryption(pStructDst, elementDst, eValue);
                }
                else
                {
                    pMemberDst = HDK_Append_PN_WiFiEncryption(pStructDst, elementDst, eValue);
                }
            }
            break;

        case HDK_Type_PN_WiFiMode:
            {
                HDK_Enum_PN_WiFiMode eValue = *HDK_Get_PN_WiFiModeMember(pMemberSrc);
                if (!fAppend)
                {
                    pMemberDst = HDK_Set_PN_WiFiMode(pStructDst, elementDst, eValue);
                }
                else
                {
                    pMemberDst = HDK_Append_PN_WiFiMode(pStructDst, elementDst, eValue);
                }
            }
            break;

        case HDK_Type_PN_WiFiSecurity:
            {
                HDK_Enum_PN_WiFiSecurity eValue = *HDK_Get_PN_WiFiSecurityMember(pMemberSrc);
                if (!fAppend)
                {
                    pMemberDst = HDK_Set_PN_WiFiSecurity(pStructDst, elementDst, eValue);
                }
                else
                {
                    pMemberDst = HDK_Append_PN_WiFiSecurity(pStructDst, elementDst, eValue);
                }
            }
            break;

        /* Note: Copying streams is not supported */
        default:
            break;
    }

    return pMemberDst;
}

HDK_Member* HDK_Get_Member(HDK_Struct* pStruct, HDK_Element element, HDK_Type type)
{
    if (pStruct)
    {
        HDK_Member* pMember;
        for (pMember = pStruct->pHead; pMember; pMember = pMember->pNext)
        {
            if (pMember->element == element && pMember->type == type)
            {
                return pMember;
            }
        }
    }
    return 0;
}

static void HDK_Append_Member(HDK_Struct* pStruct, HDK_Member* pMember)
{
    /* Find the list tail */
    if (!pStruct->pHead)
    {
        pStruct->pHead = pMember;
        pStruct->pTail = pMember;
    }
    else
    {
        pStruct->pTail->pNext = pMember;
        pStruct->pTail = pMember;
    }
}

/* Free a member node */
static void HDK_FreeMember(HDK_Member* pMember, int fStackStruct)
{
    if (pMember->type == HDK_Type__STRUCT__)
    {
        HDK_Member* pChildMember = ((HDK_Struct*)pMember)->pHead;
        while (pChildMember)
        {
            HDK_Member* pFree = pChildMember;
            pChildMember = pChildMember->pNext;
            HDK_FreeMember(pFree, 0);
        }
        if (!fStackStruct)
        {
            free(pMember);
        }
    }
    else
    {
        HDK_TypeInfo* pTypeInfo = s_types_GetInfo(pMember->type);
        if (pTypeInfo && pTypeInfo->pfnFree)
        {
            pTypeInfo->pfnFree(pMember);
        }
    }
}


/*
 * HNAP generic structure functions
 */

HDK_Struct* HDK_Set_Struct(HDK_Struct* pStruct, HDK_Element element)
{
    HDK_Struct* pMember = HDK_Get_Struct(pStruct, element);
    if (pMember)
    {
        HDK_Struct_Free(pMember);
        return (HDK_Struct*)pMember;
    }
    else
    {
        return HDK_Append_Struct(pStruct, element);
    }
}

HDK_Struct* HDK_Set_StructEx(HDK_Struct* pStructDst, HDK_Element element, HDK_Struct* pStruct)
{
    return (HDK_Struct*)HDK_Copy_Member(pStructDst, element, (HDK_Member*)pStruct, 0);
}

HDK_Struct* HDK_Append_Struct(HDK_Struct* pStruct, HDK_Element element)
{
    HDK_Struct* pMember = (HDK_Struct*)malloc(sizeof(HDK_Struct));
    if (pMember)
    {
        pMember->node.element = element;
        pMember->node.type = HDK_Type__STRUCT__;
        pMember->node.pNext = 0;
        pMember->pHead = 0;
        pMember->pTail = 0;
        HDK_Append_Member(pStruct, (HDK_Member*)pMember);
    }
    return (HDK_Struct*)pMember;
}

HDK_Struct* HDK_Append_StructEx(HDK_Struct* pStructDst, HDK_Element element, HDK_Struct* pStruct)
{
    return (HDK_Struct*)HDK_Copy_Member(pStructDst, element, (HDK_Member*)pStruct, 1);
}

HDK_Struct* HDK_Get_Struct(HDK_Struct* pStruct, HDK_Element element)
{
    return (HDK_Struct*)HDK_Get_Member(pStruct, element, HDK_Type__STRUCT__);
}

HDK_Struct* HDK_Get_StructMember(HDK_Member* pMember)
{
    return (pMember && pMember->type == HDK_Type__STRUCT__ ? (HDK_Struct*)pMember : 0);
}


/*
 * HNAP struct stack initialization/free
 */

void HDK_Struct_Init(HDK_Struct* pStruct)
{
    pStruct->node.element = HDK_Element__UNKNOWN__;
    pStruct->node.type = HDK_Type__STRUCT__;
    pStruct->node.pNext = 0;
    pStruct->pHead = 0;
    pStruct->pTail = 0;
}

void HDK_Struct_Free(HDK_Struct* pStruct)
{
    HDK_FreeMember((HDK_Member*)pStruct, 1);
    pStruct->pHead = 0;
    pStruct->pTail = 0;
}


/*
 * HNAP explicit blank element - use sparingly
 */

static void HDK_Type__BLANK__FREE__(HDK_Member* pMember)
{
    free(pMember);
}

HDK_Member* HDK_Set_Blank(HDK_Struct* pStruct, HDK_Element element)
{
    HDK_Member* pMember = HDK_Get_Member(pStruct, element, HDK_Type__BLANK__);
    if (pMember)
    {
        return pMember;
    }
    else
    {
        return HDK_Append_Blank(pStruct, element);
    }
}

HDK_Member* HDK_Append_Blank(HDK_Struct* pStruct, HDK_Element element)
{
    HDK_Member* pMember = (HDK_Member*)malloc(sizeof(HDK_Member));
    if (pMember)
    {
        pMember->element = element;
        pMember->type = HDK_Type__BLANK__;
        pMember->pNext = 0;
        HDK_Append_Member(pStruct, (HDK_Member*)pMember);
    }
    return pMember;
}


/*
 * HNAP int type
 */

typedef struct _HDK_Member__INT__
{
    HDK_Member node;
    int iValue;
} HDK_Member__INT__;

static HDK_Member* HDK_Type__INT__NEW__(void)
{
    HDK_Member__INT__* pMember = (HDK_Member__INT__*)malloc(sizeof(HDK_Member__INT__));
    if (pMember)
    {
        memset(pMember, 0, sizeof(*pMember));
    }
    return (HDK_Member*)pMember;
}

static void HDK_Type__INT__FREE__(HDK_Member* pMember)
{
    free(pMember);
}

static int HDK_Type__INT__DESERIALIZE__(HDK_Member* pMember, char* pszValue)
{
    char cEnd;
    return (sscanf(pszValue, "%d%c", &((HDK_Member__INT__*)pMember)->iValue, &cEnd) == 1);
}

static int HDK_Type__INT__SERIALIZE__(void* pDeviceCtx, int fNoWrite, HDK_Member* pMember)
{
    int iValue = ((HDK_Member__INT__*)pMember)->iValue;
    return HDK_Format(pDeviceCtx, fNoWrite, "%d", iValue);
}

static HDK_Member* HDK_Append_IntHelper(HDK_Struct* pStruct, HDK_Element element, HDK_Type type, int iValue)
{
    HDK_Member__INT__* pMember = (HDK_Member__INT__*)malloc(sizeof(HDK_Member__INT__));
    if (pMember)
    {
        pMember->node.element = element;
        pMember->node.type = type;
        pMember->node.pNext = 0;
        pMember->iValue = iValue;
        HDK_Append_Member(pStruct, (HDK_Member*)pMember);
    }
    return (HDK_Member*)pMember;
}

static HDK_Member* HDK_Set_IntHelper(HDK_Struct* pStruct, HDK_Element element, HDK_Type type, int iValue)
{
    HDK_Member* pMember = HDK_Get_Member(pStruct, element, type);
    if (pMember)
    {
        ((HDK_Member__INT__*)pMember)->iValue = iValue;
        return pMember;
    }
    else
    {
        return HDK_Append_IntHelper(pStruct, element, type, iValue);
    }
}

HDK_Member* HDK_Set_Int(HDK_Struct* pStruct, HDK_Element element, int iValue)
{
    return HDK_Set_IntHelper(pStruct, element, HDK_Type__INT__, iValue);
}

HDK_Member* HDK_Append_Int(HDK_Struct* pStruct, HDK_Element element, int iValue)
{
    return HDK_Append_IntHelper(pStruct, element, HDK_Type__INT__, iValue);
}

int* HDK_Get_Int(HDK_Struct* pStruct, HDK_Element element)
{
    return HDK_Get_IntMember(HDK_Get_Member(pStruct, element, HDK_Type__INT__));
}

int HDK_Get_IntEx(HDK_Struct* pStruct, HDK_Element element, int iDefault)
{
    int* piValue = HDK_Get_Int(pStruct, element);
    return (piValue ? *piValue : iDefault);
}

int* HDK_Get_IntMember(HDK_Member* pMember)
{
    return (pMember && pMember->type == HDK_Type__INT__ ? &((HDK_Member__INT__*)pMember)->iValue : 0);
}


/*
 * Bool member type
 */

static HDK_Member* HDK_Type__BOOL__NEW__(void)
{
    return HDK_Type__INT__NEW__();
}

static void HDK_Type__BOOL__FREE__(HDK_Member* pMember)
{
    HDK_Type__INT__FREE__(pMember);
}

static int HDK_Type__BOOL__DESERIALIZE__(HDK_Member* pMember, char* pszValue)
{
    if (strcmp(pszValue, "true") == 0)
    {
        ((HDK_Member__INT__*)pMember)->iValue = 1;
        return 1;
    }
    else if (strcmp(pszValue, "false") == 0)
    {
        ((HDK_Member__INT__*)pMember)->iValue = 0;
        return 1;
    }
    else
    {
        return 0;
    }
}

static int HDK_Type__BOOL__SERIALIZE__(void* pDeviceCtx, int fNoWrite, HDK_Member* pMember)
{
    int fValue = ((HDK_Member__INT__*)pMember)->iValue;
    return HDK_Write(pDeviceCtx, fNoWrite, fValue ? (char*)"true" : (char*)"false");
}

HDK_Member* HDK_Set_Bool(HDK_Struct* pStruct, HDK_Element element, int fValue)
{
    return HDK_Set_IntHelper(pStruct, element, HDK_Type__BOOL__, fValue);
}

HDK_Member* HDK_Append_Bool(HDK_Struct* pStruct, HDK_Element element, int fValue)
{
    return HDK_Append_IntHelper(pStruct, element, HDK_Type__BOOL__, fValue);
}

int* HDK_Get_Bool(HDK_Struct* pStruct, HDK_Element element)
{
    return HDK_Get_BoolMember(HDK_Get_Member(pStruct, element, HDK_Type__BOOL__));
}

int HDK_Get_BoolEx(HDK_Struct* pStruct, HDK_Element element, int fDefault)
{
    int* pfValue = HDK_Get_Bool(pStruct, element);
    return (pfValue ? *pfValue : fDefault);
}

int* HDK_Get_BoolMember(HDK_Member* pMember)
{
    return (pMember && pMember->type == HDK_Type__BOOL__ ? &((HDK_Member__INT__*)pMember)->iValue : 0);
}


/*
 * String member type
 */

typedef struct _HDK_Member__STRING__
{
    HDK_Member node;
    char* pszValue;
} HDK_Member__STRING__;

static HDK_Member* HDK_Type__STRING__NEW__(void)
{
    HDK_Member__STRING__* pMember = (HDK_Member__STRING__*)malloc(sizeof(HDK_Member__STRING__));
    if (pMember)
    {
        memset(pMember, 0, sizeof(*pMember));
    }
    return (HDK_Member*)pMember;
}

static void HDK_Type__STRING__FREE__(HDK_Member* pMember)
{
    free(((HDK_Member__STRING__*)pMember)->pszValue);
    free(pMember);
}

static int HDK_Type__STRING__DESERIALIZE__(HDK_Member* pMember, char* pszValue)
{
    int cchValue = strlen(pszValue) + 1;
    ((HDK_Member__STRING__*)pMember)->pszValue = (char*)malloc(cchValue);
    if (((HDK_Member__STRING__*)pMember)->pszValue)
    {
        strcpy(((HDK_Member__STRING__*)pMember)->pszValue, pszValue);
        return 1;
    }
    else
    {
        return 0;
    }
}

static int HDK_Type__STRING__SERIALIZE__(void* pDeviceCtx, int fNoWrite, HDK_Member* pMember)
{
    char* pBuf = ((HDK_Member__STRING__*)pMember)->pszValue;
    HDK_WriteBuf_EncodeContext encodeCtx;
    encodeCtx.pDeviceCtx = pDeviceCtx;
    encodeCtx.fNoWrite = fNoWrite;
    return HDK_EncodeString(HDK_WriteBuf_Encode, &encodeCtx, pBuf, strlen(pBuf));
}

HDK_Member* HDK_Set_String(HDK_Struct* pStruct, HDK_Element element, char* pszValue)
{
    size_t cchValue;
    HDK_Member* pMember = HDK_Get_Member(pStruct, element, HDK_Type__STRING__);
    if (pMember)
    {
        /* Free the old string */
        free(((HDK_Member__STRING__*)pMember)->pszValue);

        /* Duplicate the string */
        cchValue = strlen(pszValue);
        ((HDK_Member__STRING__*)pMember)->pszValue = (char*)malloc(cchValue + 1);
        if (((HDK_Member__STRING__*)pMember)->pszValue)
        {
            strcpy(((HDK_Member__STRING__*)pMember)->pszValue, pszValue);
        }

        return pMember;
    }
    else
    {
        return HDK_Append_String(pStruct, element, pszValue);
    }
}

HDK_Member* HDK_Append_String(HDK_Struct* pStruct, HDK_Element element, char* pszValue)
{
    size_t cchValue;
    HDK_Member__STRING__* pMember = (HDK_Member__STRING__*)malloc(sizeof(HDK_Member__STRING__));
    if (pMember)
    {
        pMember->node.element = element;
        pMember->node.type = HDK_Type__STRING__;
        pMember->node.pNext = 0;

        /* Duplicate the string */
        cchValue = strlen(pszValue);
        pMember->pszValue = (char*)malloc(cchValue + 1);
        if (pMember->pszValue)
        {
            strcpy(pMember->pszValue, pszValue);
            HDK_Append_Member(pStruct, (HDK_Member*)pMember);
        }
        else
        {
            free(pMember);
            pMember = 0;
        }
    }

    return (HDK_Member*)pMember;
}

char* HDK_Get_String(HDK_Struct* pStruct, HDK_Element element)
{
    return HDK_Get_StringMember(HDK_Get_Member(pStruct, element, HDK_Type__STRING__));
}

char* HDK_Get_StringEx(HDK_Struct* pStruct, HDK_Element element, char* pszDefault)
{
    char* pszValue = HDK_Get_String(pStruct, element);
    return (pszValue ? pszValue : pszDefault);
}

char* HDK_Get_StringMember(HDK_Member* pMember)
{
    return (pMember && pMember->type == HDK_Type__STRING__ ? ((HDK_Member__STRING__*)pMember)->pszValue : 0);
}


/*
 * DateTime member type
 */

typedef struct _HDK_Member__DATETIME__
{
    HDK_Member node;
    time_t tValue;
} HDK_Member__DATETIME__;

static HDK_Member* HDK_Type__DATETIME__NEW__(void)
{
    HDK_Member__DATETIME__* pMember = (HDK_Member__DATETIME__*)malloc(sizeof(HDK_Member__DATETIME__));
    if (pMember)
    {
        memset(pMember, 0, sizeof(*pMember));
    }
    return (HDK_Member*)pMember;
}

static void HDK_Type__DATETIME__FREE__(HDK_Member* pMember)
{
    free(pMember);
}

static int HDK_Type__DATETIME__DESERIALIZE__(HDK_Member* pMember, char* pszValue)
{
    int year, mon, mday, hour, min, sec, tzHour, tzMin;
    char cz, cEnd;
    int sscanfResult = sscanf(pszValue, "%04d-%02d-%02dT%02d:%02d:%02d%c%02d:%02d%c",
                              &year, &mon, &mday, &hour, &min, &sec, &cz, &tzHour, &tzMin, &cEnd);
    int fLocal = (sscanfResult == 6);
    int fUTCPlus = (sscanfResult == 9 && cz == '+');
    int fUTCMinus = (sscanfResult == 9 && cz == '-');
    int fUTC = fUTCPlus || fUTCMinus || (sscanfResult == 7 && cz == 'Z');
    if (fLocal || fUTC)
    {
        /* Compute result time_t (without UTC offset) */
        time_t result = HDK_mktime(year, mon, mday, hour, min, sec, fUTC);
        if (result == -1)
        {
            return 0;
        }

        /* Adjust for time zone */
        if (fUTCPlus)
        {
            result += tzHour * 60 * 60 + tzMin * 60;
        }
        else if (fUTCMinus)
        {
            result -= tzHour * 60 * 60 + tzMin * 60;
        }

        ((HDK_Member__DATETIME__*)pMember)->tValue = result;
        return 1;
    }
    else
    {
        return 0;
    }
}

static int HDK_Type__DATETIME__SERIALIZE__(void* pDeviceCtx, int fNoWrite, HDK_Member* pMember)
{
    struct tm t;
    HDK_gmtime(((HDK_Member__DATETIME__*)pMember)->tValue, &t);
    return HDK_Format(pDeviceCtx, fNoWrite, "%04d-%02d-%02dT%02d:%02d:%02dZ",
                      t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
}

HDK_Member* HDK_Set_DateTime(HDK_Struct* pStruct, HDK_Element element, time_t tValue)
{
    HDK_Member* pMember = HDK_Get_Member(pStruct, element, HDK_Type__DATETIME__);
    if (pMember)
    {
        ((HDK_Member__DATETIME__*)pMember)->tValue = tValue;
        return pMember;
    }
    else
    {
        return HDK_Append_DateTime(pStruct, element, tValue);
    }
}

HDK_Member* HDK_Append_DateTime(HDK_Struct* pStruct, HDK_Element element, time_t tValue)
{
    HDK_Member__DATETIME__* pMember = (HDK_Member__DATETIME__*)malloc(sizeof(HDK_Member__DATETIME__));
    if (pMember)
    {
        pMember->node.element = element;
        pMember->node.type = HDK_Type__DATETIME__;
        pMember->node.pNext = 0;
        pMember->tValue = tValue;
        HDK_Append_Member(pStruct, (HDK_Member*)pMember);
    }
    return (HDK_Member*)pMember;
}

time_t* HDK_Get_DateTime(HDK_Struct* pStruct, HDK_Element element)
{
    return HDK_Get_DateTimeMember(HDK_Get_Member(pStruct, element, HDK_Type__DATETIME__));
}

time_t HDK_Get_DateTimeEx(HDK_Struct* pStruct, HDK_Element element, time_t tDefault)
{
    time_t* ptValue = HDK_Get_DateTime(pStruct, element);
    return (ptValue ? *ptValue : tDefault);
}

time_t* HDK_Get_DateTimeMember(HDK_Member* pMember)
{
    return (pMember && pMember->type == HDK_Type__DATETIME__ ? &((HDK_Member__DATETIME__*)pMember)->tValue : 0);
}


/*
 * HNAP datetime helper functions
 */

time_t HDK_mktime(int year, int mon, int mday, int hour, int min, int sec, int fUTC)
{
    time_t result;
    struct tm t;

    /* Compute local time_t */
    t.tm_year = year - 1900;
    t.tm_mon = mon - 1;
    t.tm_mday = mday;
    t.tm_hour = hour;
    t.tm_min = min;
    t.tm_sec = sec;
    t.tm_isdst = -1;
    result = mktime(&t);

    /* If UTC time, translate to local time_t */
    if (result != -1 && fUTC)
    {
        /* Compute time_t difference between the local time and UTC time */
        time_t utc;
        struct tm tUTC;
        HDK_gmtime(result, &tUTC);
        tUTC.tm_isdst = t.tm_isdst;
        utc = mktime(&tUTC);
        if (utc == -1)
        {
            result = -1;
        }
        else
        {
            result += (result - utc);
        }
    }

    return result;
}

void HDK_localtime(time_t t, struct tm* ptm)
{
#ifdef _MSC_VER
    localtime_s(ptm, &t);
#else
    localtime_r(&t, ptm);
#endif
}

void HDK_gmtime(time_t t, struct tm* ptm)
{
#ifdef _MSC_VER
    gmtime_s(ptm, &t);
#else
    gmtime_r(&t, ptm);
#endif
}


/*
 * IPAddress member type
 */

typedef struct _HDK_Member_IPAddress
{
    HDK_Member node;
    HDK_IPAddress ipAddress;
} HDK_Member_IPAddress;

static HDK_Member* HDK_Type__IPADDRESS__NEW__(void)
{
    HDK_Member_IPAddress* pMember = (HDK_Member_IPAddress*)malloc(sizeof(HDK_Member_IPAddress));
    if (pMember)
    {
        memset(pMember, 0, sizeof(*pMember));
    }
    return (HDK_Member*)pMember;
}

static void HDK_Type__IPADDRESS__FREE__(HDK_Member* pMember)
{
    free(pMember);
}

static int HDK_Type__IPADDRESS__DESERIALIZE__(HDK_Member* pMember, char* pszValue)
{
    HDK_Member_IPAddress* pIPAddress;
    unsigned int a, b, c, d;
    char cEnd;

    /* Blank values are allowed... */
    if (!*pszValue)
    {
        a = b = c = d = 0;
    }
    else if (sscanf(pszValue, "%u.%u.%u.%u%c", &a, &b, &c, &d, &cEnd) != 4 ||
             a > 255 || b > 255 || c > 255 || d > 255)
    {
        return 0;
    }

    /* Add the IPAddress */
    pIPAddress = (HDK_Member_IPAddress*)pMember;
    pIPAddress->ipAddress.a = a;
    pIPAddress->ipAddress.b = b;
    pIPAddress->ipAddress.c = c;
    pIPAddress->ipAddress.d = d;
    return 1;
}

static int HDK_Type__IPADDRESS__SERIALIZE__(void* pDeviceCtx, int fNoWrite, HDK_Member* pMember)
{
    HDK_Member_IPAddress* pIPAddress = (HDK_Member_IPAddress*)pMember;
    unsigned int a = pIPAddress->ipAddress.a;
    unsigned int b = pIPAddress->ipAddress.b;
    unsigned int c = pIPAddress->ipAddress.c;
    unsigned int d = pIPAddress->ipAddress.d;
    return HDK_Format(pDeviceCtx, fNoWrite, "%u.%u.%u.%u", a, b, c, d);
}

HDK_Member* HDK_Set_IPAddress(HDK_Struct* pStruct, HDK_Element element, HDK_IPAddress* pIPAddress)
{
    HDK_Member* pMember = HDK_Get_Member(pStruct, element, HDK_Type__IPADDRESS__);
    if (pMember)
    {
        ((HDK_Member_IPAddress*)pMember)->ipAddress = *pIPAddress;
        return pMember;
    }
    else
    {
        return HDK_Append_IPAddress(pStruct, element, pIPAddress);
    }
}

HDK_Member* HDK_Append_IPAddress(HDK_Struct* pStruct, HDK_Element element, HDK_IPAddress* pIPAddress)
{
    HDK_Member_IPAddress* pMember = (HDK_Member_IPAddress*)malloc(sizeof(HDK_Member_IPAddress));
    if (pMember)
    {
        pMember->node.element = element;
        pMember->node.type = HDK_Type__IPADDRESS__;
        pMember->node.pNext = 0;
        pMember->ipAddress = *pIPAddress;
        HDK_Append_Member(pStruct, (HDK_Member*)pMember);
    }
    return (HDK_Member*)pMember;
}

HDK_IPAddress* HDK_Get_IPAddress(HDK_Struct* pStruct, HDK_Element element)
{
    return HDK_Get_IPAddressMember(HDK_Get_Member(pStruct, element, HDK_Type__IPADDRESS__));
}

HDK_IPAddress* HDK_Get_IPAddressEx(HDK_Struct* pStruct, HDK_Element element, HDK_IPAddress* pDefault)
{
    HDK_IPAddress* pValue = HDK_Get_IPAddress(pStruct, element);
    return (pValue ? pValue : pDefault);
}

HDK_IPAddress* HDK_Get_IPAddressMember(HDK_Member* pMember)
{
    return (pMember && pMember->type == HDK_Type__IPADDRESS__ ? &((HDK_Member_IPAddress*)pMember)->ipAddress : 0);
}


/*
 * MACAddress member type
 */

typedef struct _HDK_Member_MACAddress
{
    HDK_Member node;
    HDK_MACAddress macAddress;
} HDK_Member_MACAddress;

static HDK_Member* HDK_Type__MACADDRESS__NEW__(void)
{
    HDK_Member_MACAddress* pMember = (HDK_Member_MACAddress*)malloc(sizeof(HDK_Member_MACAddress));
    if (pMember)
    {
        memset(pMember, 0, sizeof(*pMember));
    }
    return (HDK_Member*)pMember;
}

static void HDK_Type__MACADDRESS__FREE__(HDK_Member* pMember)
{
    free(pMember);
}

static int HDK_Type__MACADDRESS__DESERIALIZE__(HDK_Member* pMember, char* pszValue)
{
    HDK_Member_MACAddress* pMACAddress;
    unsigned int a, b, c, d, e, f;
    char cEnd;

    /* Blank values are allowed... */
    if (!*pszValue)
    {
        a = b = c = d = e = f = 0;
    }
    else if (sscanf(pszValue, "%02X:%02X:%02X:%02X:%02X:%02X%c", &a, &b, &c, &d, &e, &f, &cEnd) != 6 ||
             a > 255 || b > 255 || c > 255 || d > 255 || e > 255 || f > 255)
    {
        return 0;
    }

    /* Add the MACAddress */
    pMACAddress = (HDK_Member_MACAddress*)pMember;
    pMACAddress->macAddress.a = a;
    pMACAddress->macAddress.b = b;
    pMACAddress->macAddress.c = c;
    pMACAddress->macAddress.d = d;
    pMACAddress->macAddress.e = e;
    pMACAddress->macAddress.f = f;
    return 1;
}

static int HDK_Type__MACADDRESS__SERIALIZE__(void* pDeviceCtx, int fNoWrite, HDK_Member* pMember)
{
    HDK_MACAddress* pMACAddress = &((HDK_Member_MACAddress*)pMember)->macAddress;
    unsigned int a = pMACAddress->a;
    unsigned int b = pMACAddress->b;
    unsigned int c = pMACAddress->c;
    unsigned int d = pMACAddress->d;
    unsigned int e = pMACAddress->e;
    unsigned int f = pMACAddress->f;
    return HDK_Format(pDeviceCtx, fNoWrite, "%02X:%02X:%02X:%02X:%02X:%02X", a, b, c, d, e, f);
}

HDK_Member* HDK_Set_MACAddress(HDK_Struct* pStruct, HDK_Element element, HDK_MACAddress* pMACAddress)
{
    HDK_Member* pMember = HDK_Get_Member(pStruct, element, HDK_Type__MACADDRESS__);
    if (pMember)
    {
        ((HDK_Member_MACAddress*)pMember)->macAddress = *pMACAddress;
        return pMember;
    }
    else
    {
        return HDK_Append_MACAddress(pStruct, element, pMACAddress);
    }
}

HDK_Member* HDK_Append_MACAddress(HDK_Struct* pStruct, HDK_Element element, HDK_MACAddress* pMACAddress)
{
    HDK_Member_MACAddress* pMember = (HDK_Member_MACAddress*)malloc(sizeof(HDK_Member_MACAddress));
    if (pMember)
    {
        pMember->node.element = element;
        pMember->node.type = HDK_Type__MACADDRESS__;
        pMember->node.pNext = 0;
        pMember->macAddress = *pMACAddress;
        HDK_Append_Member(pStruct, (HDK_Member*)pMember);
    }
    return (HDK_Member*)pMember;
}

HDK_MACAddress* HDK_Get_MACAddress(HDK_Struct* pStruct, HDK_Element element)
{
    return HDK_Get_MACAddressMember(HDK_Get_Member(pStruct, element, HDK_Type__MACADDRESS__));
}

HDK_MACAddress* HDK_Get_MACAddressEx(HDK_Struct* pStruct, HDK_Element element, HDK_MACAddress* pDefault)
{
    HDK_MACAddress* pValue = HDK_Get_MACAddress(pStruct, element);
    return (pValue ? pValue : pDefault);
}

HDK_MACAddress* HDK_Get_MACAddressMember(HDK_Member* pMember)
{
    return (pMember && pMember->type == HDK_Type__MACADDRESS__ ? &((HDK_Member_MACAddress*)pMember)->macAddress : 0);
}


/*
 * Enumeration deserialize and serialize helper functions
 */

static int HDK_Type__ENUM__DESERIALIZE__(HDK_Member* pMember, char* pszValue, char** ppszBegin, char** ppszEnd)
{
    char** ppszValue;
    for (ppszValue = ppszBegin; ppszValue != ppszEnd; ppszValue++)
    {
        if (*ppszValue && strcmp(*ppszValue, pszValue) == 0)
        {
            ((HDK_Member__INT__*)pMember)->iValue = ppszValue - ppszBegin;
            return 1;
        }
    }

    /* Unknown enumeration value */
    ((HDK_Member__INT__*)pMember)->iValue = 0;
    return 1;
}

static int HDK_Type__ENUM__SERIALIZE__(void* pDeviceCtx, int fNoWrite, HDK_Member* pMember, char** ppszBegin, char** ppszEnd)
{
    char** ppszValue = ppszBegin + ((HDK_Member__INT__*)pMember)->iValue;
    if (ppszValue >= ppszBegin && ppszValue < ppszEnd && *ppszValue)
    {
        return HDK_Write(pDeviceCtx, fNoWrite, *ppszValue);
    }
    return 0;
}


/*
 * HDK_Type__RESULT__ enumeration type
 */

static char* s_HDK_Enum_Result__VALUESTRINGS__[] =
{
    /* HDK_Enum_Result__UNKNOWN__ */ 0,
    /* HDK_Enum_Result_OK */ "OK",
    /* HDK_Enum_Result_REBOOT */ "REBOOT",
    /* HDK_Enum_Result_ERROR */ "ERROR",
    /* HDK_Enum_Result_ERROR_BAD_CHANNEL */ "ERROR_BAD_CHANNEL",
    /* HDK_Enum_Result_ERROR_BAD_CHANNEL_WIDTH */ "ERROR_BAD_CHANNEL_WIDTH",
    /* HDK_Enum_Result_ERROR_BAD_IP_ADDRESS */ "ERROR_BAD_IP_ADDRESS",
    /* HDK_Enum_Result_ERROR_BAD_IP_RANGE */ "ERROR_BAD_IP_RANGE",
    /* HDK_Enum_Result_ERROR_BAD_MODE */ "ERROR_BAD_MODE",
    /* HDK_Enum_Result_ERROR_BAD_RADIOID */ "ERROR_BAD_RADIOID",
    /* HDK_Enum_Result_ERROR_BAD_RADIUS_VALUES */ "ERROR_BAD_RADIUS_VALUES",
    /* HDK_Enum_Result_ERROR_BAD_RESERVATION */ "ERROR_BAD_RESERVATION",
    /* HDK_Enum_Result_ERROR_BAD_SECONDARY_CHANNEL */ "ERROR_BAD_SECONDARY_CHANNEL",
    /* HDK_Enum_Result_ERROR_BAD_SSID */ "ERROR_BAD_SSID",
    /* HDK_Enum_Result_ERROR_BAD_SUBNET */ "ERROR_BAD_SUBNET",
    /* HDK_Enum_Result_ERROR_DOMAIN_NOT_SUPPORTED */ "ERROR_DOMAIN_NOT_SUPPORTED",
    /* HDK_Enum_Result_ERROR_ENCRYPTION_NOT_SUPPORTED */ "ERROR_ENCRYPTION_NOT_SUPPORTED",
    /* HDK_Enum_Result_ERROR_ILLEGAL_KEY_VALUE */ "ERROR_ILLEGAL_KEY_VALUE",
    /* HDK_Enum_Result_ERROR_KEY_RENEWAL_BAD_VALUE */ "ERROR_KEY_RENEWAL_BAD_VALUE",
    /* HDK_Enum_Result_ERROR_QOS_NOT_SUPPORTED */ "ERROR_QOS_NOT_SUPPORTED",
    /* HDK_Enum_Result_ERROR_REMOTE_MANAGE_DEFAULT_PASSWORD */ "ERROR_REMOTE_MANAGE_DEFAULT_PASSWORD",
    /* HDK_Enum_Result_ERROR_REMOTE_MANAGE_MUST_BE_SSL */ "ERROR_REMOTE_MANAGE_MUST_BE_SSL",
    /* HDK_Enum_Result_ERROR_REMOTE_MANAGE_NOT_SUPPORTED */ "ERROR_REMOTE_MANAGE_NOT_SUPPORTED",
    /* HDK_Enum_Result_ERROR_REMOTE_SSL_NEEDS_SSL */ "ERROR_REMOTE_SSL_NEEDS_SSL",
    /* HDK_Enum_Result_ERROR_REMOTE_SSL_NOT_SUPPORTED */ "ERROR_REMOTE_SSL_NOT_SUPPORTED",
    /* HDK_Enum_Result_ERROR_RESERVATIONS_NOT_SUPPORTED */ "ERROR_RESERVATIONS_NOT_SUPPORTED",
    /* HDK_Enum_Result_ERROR_TIMEZONE_NOT_SUPPORTED */ "ERROR_TIMEZONE_NOT_SUPPORTED",
    /* HDK_Enum_Result_ERROR_TYPE_NOT_SUPPORTED */ "ERROR_TYPE_NOT_SUPPORTED",
    /* HDK_Enum_Result_ERROR_USERNAME_NOT_SUPPORTED */ "ERROR_USERNAME_NOT_SUPPORTED",
};

static HDK_Member* HDK_Type__RESULT__NEW__(void)
{
    return HDK_Type__INT__NEW__();
}

static void HDK_Type__RESULT__FREE__(HDK_Member* pMember)
{
    HDK_Type__INT__FREE__(pMember);
}

static int HDK_Type__RESULT__DESERIALIZE__(HDK_Member* pMember, char* pszValue)
{
    char** ppszBegin = s_HDK_Enum_Result__VALUESTRINGS__;
    char** ppszEnd = s_HDK_Enum_Result__VALUESTRINGS__ + sizeof(s_HDK_Enum_Result__VALUESTRINGS__) / sizeof(*s_HDK_Enum_Result__VALUESTRINGS__);
    return HDK_Type__ENUM__DESERIALIZE__(pMember, pszValue, ppszBegin, ppszEnd);
}

static int HDK_Type__RESULT__SERIALIZE__(void* pDeviceCtx, int fNoWrite, HDK_Member* pMember)
{
    char** ppszBegin = s_HDK_Enum_Result__VALUESTRINGS__;
    char** ppszEnd = s_HDK_Enum_Result__VALUESTRINGS__ + sizeof(s_HDK_Enum_Result__VALUESTRINGS__) / sizeof(*s_HDK_Enum_Result__VALUESTRINGS__);
    return HDK_Type__ENUM__SERIALIZE__(pDeviceCtx, fNoWrite, pMember, ppszBegin, ppszEnd);
}

HDK_Member* HDK_Set_Result(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_Result eValue)
{
    if (eValue != HDK_Enum_Result__UNKNOWN__)
    {
        return HDK_Set_IntHelper(pStruct, element, HDK_Type__RESULT__, eValue);
    }
    else
    {
        return 0;
    }
}

HDK_Member* HDK_Append_Result(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_Result eValue)
{
    if (eValue != HDK_Enum_Result__UNKNOWN__)
    {
        return HDK_Append_IntHelper(pStruct, element, HDK_Type__RESULT__, eValue);
    }
    else
    {
        return 0;
    }
}

HDK_Enum_Result* HDK_Get_Result(HDK_Struct* pStruct, HDK_Element element)
{
    return HDK_Get_ResultMember(HDK_Get_Member(pStruct, element, HDK_Type__RESULT__));
}

HDK_Enum_Result HDK_Get_ResultEx(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_Result eValue)
{
    HDK_Enum_Result* peDefault = HDK_Get_Result(pStruct, element);
    return (peDefault ? *peDefault : eValue);
}

HDK_Enum_Result* HDK_Get_ResultMember(HDK_Member* pMember)
{
    return (HDK_Enum_Result*)(pMember && pMember->type == HDK_Type__RESULT__ ? &((HDK_Member__INT__*)pMember)->iValue : 0);
}


/*
 * HDK_Type_PN_DeviceType enumeration type
 */

static char* s_HDK_Enum_PN_DeviceType__VALUESTRINGS__[] =
{
    /* HDK_Enum_PN_DeviceType__UNKNOWN__ */ 0,
    /* HDK_Enum_PN_DeviceType_Computer */ "Computer",
    /* HDK_Enum_PN_DeviceType_ComputerServer */ "ComputerServer",
    /* HDK_Enum_PN_DeviceType_WorkstationComputer */ "WorkstationComputer",
    /* HDK_Enum_PN_DeviceType_LaptopComputer */ "LaptopComputer",
    /* HDK_Enum_PN_DeviceType_Gateway */ "Gateway",
    /* HDK_Enum_PN_DeviceType_GatewayWithWiFi */ "GatewayWithWiFi",
    /* HDK_Enum_PN_DeviceType_DigitalDVR */ "DigitalDVR",
    /* HDK_Enum_PN_DeviceType_DigitalJukebox */ "DigitalJukebox",
    /* HDK_Enum_PN_DeviceType_MediaAdapter */ "MediaAdapter",
    /* HDK_Enum_PN_DeviceType_NetworkCamera */ "NetworkCamera",
    /* HDK_Enum_PN_DeviceType_NetworkDevice */ "NetworkDevice",
    /* HDK_Enum_PN_DeviceType_NetworkDrive */ "NetworkDrive",
    /* HDK_Enum_PN_DeviceType_NetworkGameConsole */ "NetworkGameConsole",
    /* HDK_Enum_PN_DeviceType_NetworkPDA */ "NetworkPDA",
    /* HDK_Enum_PN_DeviceType_NetworkPrinter */ "NetworkPrinter",
    /* HDK_Enum_PN_DeviceType_NetworkPrintServer */ "NetworkPrintServer",
    /* HDK_Enum_PN_DeviceType_PhotoFrame */ "PhotoFrame",
    /* HDK_Enum_PN_DeviceType_VOIPDevice */ "VOIPDevice",
    /* HDK_Enum_PN_DeviceType_WiFiAccessPoint */ "WiFiAccessPoint",
    /* HDK_Enum_PN_DeviceType_SetTopBox */ "SetTopBox",
    /* HDK_Enum_PN_DeviceType_WiFiBridge */ "WiFiBridge",
};

static HDK_Member* HDK_Type_PN_DeviceType__NEW__(void)
{
    return HDK_Type__INT__NEW__();
}

static void HDK_Type_PN_DeviceType__FREE__(HDK_Member* pMember)
{
    HDK_Type__INT__FREE__(pMember);
}

static int HDK_Type_PN_DeviceType__DESERIALIZE__(HDK_Member* pMember, char* pszValue)
{
    char** ppszBegin = s_HDK_Enum_PN_DeviceType__VALUESTRINGS__;
    char** ppszEnd = s_HDK_Enum_PN_DeviceType__VALUESTRINGS__ + sizeof(s_HDK_Enum_PN_DeviceType__VALUESTRINGS__) / sizeof(*s_HDK_Enum_PN_DeviceType__VALUESTRINGS__);
    return HDK_Type__ENUM__DESERIALIZE__(pMember, pszValue, ppszBegin, ppszEnd);
}

static int HDK_Type_PN_DeviceType__SERIALIZE__(void* pDeviceCtx, int fNoWrite, HDK_Member* pMember)
{
    char** ppszBegin = s_HDK_Enum_PN_DeviceType__VALUESTRINGS__;
    char** ppszEnd = s_HDK_Enum_PN_DeviceType__VALUESTRINGS__ + sizeof(s_HDK_Enum_PN_DeviceType__VALUESTRINGS__) / sizeof(*s_HDK_Enum_PN_DeviceType__VALUESTRINGS__);
    return HDK_Type__ENUM__SERIALIZE__(pDeviceCtx, fNoWrite, pMember, ppszBegin, ppszEnd);
}

HDK_Member* HDK_Set_PN_DeviceType(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_DeviceType eValue)
{
    if (eValue != HDK_Enum_PN_DeviceType__UNKNOWN__)
    {
        return HDK_Set_IntHelper(pStruct, element, HDK_Type_PN_DeviceType, eValue);
    }
    else
    {
        return 0;
    }
}

HDK_Member* HDK_Append_PN_DeviceType(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_DeviceType eValue)
{
    if (eValue != HDK_Enum_PN_DeviceType__UNKNOWN__)
    {
        return HDK_Append_IntHelper(pStruct, element, HDK_Type_PN_DeviceType, eValue);
    }
    else
    {
        return 0;
    }
}

HDK_Enum_PN_DeviceType* HDK_Get_PN_DeviceType(HDK_Struct* pStruct, HDK_Element element)
{
    return HDK_Get_PN_DeviceTypeMember(HDK_Get_Member(pStruct, element, HDK_Type_PN_DeviceType));
}

HDK_Enum_PN_DeviceType HDK_Get_PN_DeviceTypeEx(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_DeviceType eValue)
{
    HDK_Enum_PN_DeviceType* peDefault = HDK_Get_PN_DeviceType(pStruct, element);
    return (peDefault ? *peDefault : eValue);
}

HDK_Enum_PN_DeviceType* HDK_Get_PN_DeviceTypeMember(HDK_Member* pMember)
{
    return (HDK_Enum_PN_DeviceType*)(pMember && pMember->type == HDK_Type_PN_DeviceType ? &((HDK_Member__INT__*)pMember)->iValue : 0);
}


/*
 * HDK_Type_PN_IPProtocol enumeration type
 */

static char* s_HDK_Enum_PN_IPProtocol__VALUESTRINGS__[] =
{
    /* HDK_Enum_PN_IPProtocol__UNKNOWN__ */ 0,
    /* HDK_Enum_PN_IPProtocol_UDP */ "UDP",
    /* HDK_Enum_PN_IPProtocol_TCP */ "TCP",
};

static HDK_Member* HDK_Type_PN_IPProtocol__NEW__(void)
{
    return HDK_Type__INT__NEW__();
}

static void HDK_Type_PN_IPProtocol__FREE__(HDK_Member* pMember)
{
    HDK_Type__INT__FREE__(pMember);
}

static int HDK_Type_PN_IPProtocol__DESERIALIZE__(HDK_Member* pMember, char* pszValue)
{
    char** ppszBegin = s_HDK_Enum_PN_IPProtocol__VALUESTRINGS__;
    char** ppszEnd = s_HDK_Enum_PN_IPProtocol__VALUESTRINGS__ + sizeof(s_HDK_Enum_PN_IPProtocol__VALUESTRINGS__) / sizeof(*s_HDK_Enum_PN_IPProtocol__VALUESTRINGS__);
    return HDK_Type__ENUM__DESERIALIZE__(pMember, pszValue, ppszBegin, ppszEnd);
}

static int HDK_Type_PN_IPProtocol__SERIALIZE__(void* pDeviceCtx, int fNoWrite, HDK_Member* pMember)
{
    char** ppszBegin = s_HDK_Enum_PN_IPProtocol__VALUESTRINGS__;
    char** ppszEnd = s_HDK_Enum_PN_IPProtocol__VALUESTRINGS__ + sizeof(s_HDK_Enum_PN_IPProtocol__VALUESTRINGS__) / sizeof(*s_HDK_Enum_PN_IPProtocol__VALUESTRINGS__);
    return HDK_Type__ENUM__SERIALIZE__(pDeviceCtx, fNoWrite, pMember, ppszBegin, ppszEnd);
}

HDK_Member* HDK_Set_PN_IPProtocol(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_IPProtocol eValue)
{
    if (eValue != HDK_Enum_PN_IPProtocol__UNKNOWN__)
    {
        return HDK_Set_IntHelper(pStruct, element, HDK_Type_PN_IPProtocol, eValue);
    }
    else
    {
        return 0;
    }
}

HDK_Member* HDK_Append_PN_IPProtocol(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_IPProtocol eValue)
{
    if (eValue != HDK_Enum_PN_IPProtocol__UNKNOWN__)
    {
        return HDK_Append_IntHelper(pStruct, element, HDK_Type_PN_IPProtocol, eValue);
    }
    else
    {
        return 0;
    }
}

HDK_Enum_PN_IPProtocol* HDK_Get_PN_IPProtocol(HDK_Struct* pStruct, HDK_Element element)
{
    return HDK_Get_PN_IPProtocolMember(HDK_Get_Member(pStruct, element, HDK_Type_PN_IPProtocol));
}

HDK_Enum_PN_IPProtocol HDK_Get_PN_IPProtocolEx(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_IPProtocol eValue)
{
    HDK_Enum_PN_IPProtocol* peDefault = HDK_Get_PN_IPProtocol(pStruct, element);
    return (peDefault ? *peDefault : eValue);
}

HDK_Enum_PN_IPProtocol* HDK_Get_PN_IPProtocolMember(HDK_Member* pMember)
{
    return (HDK_Enum_PN_IPProtocol*)(pMember && pMember->type == HDK_Type_PN_IPProtocol ? &((HDK_Member__INT__*)pMember)->iValue : 0);
}


/*
 * HDK_Type_PN_LANConnection enumeration type
 */

static char* s_HDK_Enum_PN_LANConnection__VALUESTRINGS__[] =
{
    /* HDK_Enum_PN_LANConnection__UNKNOWN__ */ 0,
    /* HDK_Enum_PN_LANConnection_LAN */ "LAN",
    /* HDK_Enum_PN_LANConnection_WLAN_802_11a */ "WLAN 802.11a",
    /* HDK_Enum_PN_LANConnection_WLAN_802_11b */ "WLAN 802.11b",
    /* HDK_Enum_PN_LANConnection_WLAN_802_11g */ "WLAN 802.11g",
    /* HDK_Enum_PN_LANConnection_WLAN_802_11n */ "WLAN 802.11n",
    /* HDK_Enum_PN_LANConnection_WAN */ "WAN",
};

static HDK_Member* HDK_Type_PN_LANConnection__NEW__(void)
{
    return HDK_Type__INT__NEW__();
}

static void HDK_Type_PN_LANConnection__FREE__(HDK_Member* pMember)
{
    HDK_Type__INT__FREE__(pMember);
}

static int HDK_Type_PN_LANConnection__DESERIALIZE__(HDK_Member* pMember, char* pszValue)
{
    char** ppszBegin = s_HDK_Enum_PN_LANConnection__VALUESTRINGS__;
    char** ppszEnd = s_HDK_Enum_PN_LANConnection__VALUESTRINGS__ + sizeof(s_HDK_Enum_PN_LANConnection__VALUESTRINGS__) / sizeof(*s_HDK_Enum_PN_LANConnection__VALUESTRINGS__);
    return HDK_Type__ENUM__DESERIALIZE__(pMember, pszValue, ppszBegin, ppszEnd);
}

static int HDK_Type_PN_LANConnection__SERIALIZE__(void* pDeviceCtx, int fNoWrite, HDK_Member* pMember)
{
    char** ppszBegin = s_HDK_Enum_PN_LANConnection__VALUESTRINGS__;
    char** ppszEnd = s_HDK_Enum_PN_LANConnection__VALUESTRINGS__ + sizeof(s_HDK_Enum_PN_LANConnection__VALUESTRINGS__) / sizeof(*s_HDK_Enum_PN_LANConnection__VALUESTRINGS__);
    return HDK_Type__ENUM__SERIALIZE__(pDeviceCtx, fNoWrite, pMember, ppszBegin, ppszEnd);
}

HDK_Member* HDK_Set_PN_LANConnection(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_LANConnection eValue)
{
    if (eValue != HDK_Enum_PN_LANConnection__UNKNOWN__)
    {
        return HDK_Set_IntHelper(pStruct, element, HDK_Type_PN_LANConnection, eValue);
    }
    else
    {
        return 0;
    }
}

HDK_Member* HDK_Append_PN_LANConnection(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_LANConnection eValue)
{
    if (eValue != HDK_Enum_PN_LANConnection__UNKNOWN__)
    {
        return HDK_Append_IntHelper(pStruct, element, HDK_Type_PN_LANConnection, eValue);
    }
    else
    {
        return 0;
    }
}

HDK_Enum_PN_LANConnection* HDK_Get_PN_LANConnection(HDK_Struct* pStruct, HDK_Element element)
{
    return HDK_Get_PN_LANConnectionMember(HDK_Get_Member(pStruct, element, HDK_Type_PN_LANConnection));
}

HDK_Enum_PN_LANConnection HDK_Get_PN_LANConnectionEx(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_LANConnection eValue)
{
    HDK_Enum_PN_LANConnection* peDefault = HDK_Get_PN_LANConnection(pStruct, element);
    return (peDefault ? *peDefault : eValue);
}

HDK_Enum_PN_LANConnection* HDK_Get_PN_LANConnectionMember(HDK_Member* pMember)
{
    return (HDK_Enum_PN_LANConnection*)(pMember && pMember->type == HDK_Type_PN_LANConnection ? &((HDK_Member__INT__*)pMember)->iValue : 0);
}


/*
 * HDK_Type_PN_TaskExtType enumeration type
 */

static char* s_HDK_Enum_PN_TaskExtType__VALUESTRINGS__[] =
{
    /* HDK_Enum_PN_TaskExtType__UNKNOWN__ */ 0,
    /* HDK_Enum_PN_TaskExtType_Browser */ "Browser",
    /* HDK_Enum_PN_TaskExtType_MessageBox */ "MessageBox",
    /* HDK_Enum_PN_TaskExtType_PUI */ "PUI",
    /* HDK_Enum_PN_TaskExtType_Silent */ "Silent",
};

static HDK_Member* HDK_Type_PN_TaskExtType__NEW__(void)
{
    return HDK_Type__INT__NEW__();
}

static void HDK_Type_PN_TaskExtType__FREE__(HDK_Member* pMember)
{
    HDK_Type__INT__FREE__(pMember);
}

static int HDK_Type_PN_TaskExtType__DESERIALIZE__(HDK_Member* pMember, char* pszValue)
{
    char** ppszBegin = s_HDK_Enum_PN_TaskExtType__VALUESTRINGS__;
    char** ppszEnd = s_HDK_Enum_PN_TaskExtType__VALUESTRINGS__ + sizeof(s_HDK_Enum_PN_TaskExtType__VALUESTRINGS__) / sizeof(*s_HDK_Enum_PN_TaskExtType__VALUESTRINGS__);
    return HDK_Type__ENUM__DESERIALIZE__(pMember, pszValue, ppszBegin, ppszEnd);
}

static int HDK_Type_PN_TaskExtType__SERIALIZE__(void* pDeviceCtx, int fNoWrite, HDK_Member* pMember)
{
    char** ppszBegin = s_HDK_Enum_PN_TaskExtType__VALUESTRINGS__;
    char** ppszEnd = s_HDK_Enum_PN_TaskExtType__VALUESTRINGS__ + sizeof(s_HDK_Enum_PN_TaskExtType__VALUESTRINGS__) / sizeof(*s_HDK_Enum_PN_TaskExtType__VALUESTRINGS__);
    return HDK_Type__ENUM__SERIALIZE__(pDeviceCtx, fNoWrite, pMember, ppszBegin, ppszEnd);
}

HDK_Member* HDK_Set_PN_TaskExtType(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_TaskExtType eValue)
{
    if (eValue != HDK_Enum_PN_TaskExtType__UNKNOWN__)
    {
        return HDK_Set_IntHelper(pStruct, element, HDK_Type_PN_TaskExtType, eValue);
    }
    else
    {
        return 0;
    }
}

HDK_Member* HDK_Append_PN_TaskExtType(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_TaskExtType eValue)
{
    if (eValue != HDK_Enum_PN_TaskExtType__UNKNOWN__)
    {
        return HDK_Append_IntHelper(pStruct, element, HDK_Type_PN_TaskExtType, eValue);
    }
    else
    {
        return 0;
    }
}

HDK_Enum_PN_TaskExtType* HDK_Get_PN_TaskExtType(HDK_Struct* pStruct, HDK_Element element)
{
    return HDK_Get_PN_TaskExtTypeMember(HDK_Get_Member(pStruct, element, HDK_Type_PN_TaskExtType));
}

HDK_Enum_PN_TaskExtType HDK_Get_PN_TaskExtTypeEx(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_TaskExtType eValue)
{
    HDK_Enum_PN_TaskExtType* peDefault = HDK_Get_PN_TaskExtType(pStruct, element);
    return (peDefault ? *peDefault : eValue);
}

HDK_Enum_PN_TaskExtType* HDK_Get_PN_TaskExtTypeMember(HDK_Member* pMember)
{
    return (HDK_Enum_PN_TaskExtType*)(pMember && pMember->type == HDK_Type_PN_TaskExtType ? &((HDK_Member__INT__*)pMember)->iValue : 0);
}


/*
 * HDK_Type_PN_WANType enumeration type
 */

static char* s_HDK_Enum_PN_WANType__VALUESTRINGS__[] =
{
    /* HDK_Enum_PN_WANType__UNKNOWN__ */ 0,
    /* HDK_Enum_PN_WANType_BigPond */ "BigPond",
    /* HDK_Enum_PN_WANType_DHCP */ "DHCP",
    /* HDK_Enum_PN_WANType_DHCPPPPoE */ "DHCPPPPoE",
    /* HDK_Enum_PN_WANType_DynamicL2TP */ "DynamicL2TP",
    /* HDK_Enum_PN_WANType_DynamicPPTP */ "DynamicPPTP",
    /* HDK_Enum_PN_WANType_Static */ "Static",
    /* HDK_Enum_PN_WANType_StaticL2TP */ "StaticL2TP",
    /* HDK_Enum_PN_WANType_StaticPPPoE */ "StaticPPPoE",
    /* HDK_Enum_PN_WANType_StaticPPTP */ "StaticPPTP",
    /* HDK_Enum_PN_WANType_BridgedOnly */ "BridgedOnly",
    /* HDK_Enum_PN_WANType_Static1483Bridged */ "Static1483Bridged",
    /* HDK_Enum_PN_WANType_Dynamic1483Bridged */ "Dynamic1483Bridged",
    /* HDK_Enum_PN_WANType_Static1483Routed */ "Static1483Routed",
    /* HDK_Enum_PN_WANType_StaticPPPOA */ "StaticPPPOA",
    /* HDK_Enum_PN_WANType_DynamicPPPOA */ "DynamicPPPOA",
    /* HDK_Enum_PN_WANType_StaticIPOA */ "StaticIPOA",
    /* HDK_Enum_PN_WANType_UNKNOWN */ "UNKNOWN",
    /* HDK_Enum_PN_WANType_DETECTING */ "DETECTING",
};

static HDK_Member* HDK_Type_PN_WANType__NEW__(void)
{
    return HDK_Type__INT__NEW__();
}

static void HDK_Type_PN_WANType__FREE__(HDK_Member* pMember)
{
    HDK_Type__INT__FREE__(pMember);
}

static int HDK_Type_PN_WANType__DESERIALIZE__(HDK_Member* pMember, char* pszValue)
{
    char** ppszBegin = s_HDK_Enum_PN_WANType__VALUESTRINGS__;
    char** ppszEnd = s_HDK_Enum_PN_WANType__VALUESTRINGS__ + sizeof(s_HDK_Enum_PN_WANType__VALUESTRINGS__) / sizeof(*s_HDK_Enum_PN_WANType__VALUESTRINGS__);
    return HDK_Type__ENUM__DESERIALIZE__(pMember, pszValue, ppszBegin, ppszEnd);
}

static int HDK_Type_PN_WANType__SERIALIZE__(void* pDeviceCtx, int fNoWrite, HDK_Member* pMember)
{
    char** ppszBegin = s_HDK_Enum_PN_WANType__VALUESTRINGS__;
    char** ppszEnd = s_HDK_Enum_PN_WANType__VALUESTRINGS__ + sizeof(s_HDK_Enum_PN_WANType__VALUESTRINGS__) / sizeof(*s_HDK_Enum_PN_WANType__VALUESTRINGS__);
    return HDK_Type__ENUM__SERIALIZE__(pDeviceCtx, fNoWrite, pMember, ppszBegin, ppszEnd);
}

HDK_Member* HDK_Set_PN_WANType(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_WANType eValue)
{
    if (eValue != HDK_Enum_PN_WANType__UNKNOWN__)
    {
        return HDK_Set_IntHelper(pStruct, element, HDK_Type_PN_WANType, eValue);
    }
    else
    {
        return 0;
    }
}

HDK_Member* HDK_Append_PN_WANType(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_WANType eValue)
{
    if (eValue != HDK_Enum_PN_WANType__UNKNOWN__)
    {
        return HDK_Append_IntHelper(pStruct, element, HDK_Type_PN_WANType, eValue);
    }
    else
    {
        return 0;
    }
}

HDK_Enum_PN_WANType* HDK_Get_PN_WANType(HDK_Struct* pStruct, HDK_Element element)
{
    return HDK_Get_PN_WANTypeMember(HDK_Get_Member(pStruct, element, HDK_Type_PN_WANType));
}

HDK_Enum_PN_WANType HDK_Get_PN_WANTypeEx(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_WANType eValue)
{
    HDK_Enum_PN_WANType* peDefault = HDK_Get_PN_WANType(pStruct, element);
    return (peDefault ? *peDefault : eValue);
}

HDK_Enum_PN_WANType* HDK_Get_PN_WANTypeMember(HDK_Member* pMember)
{
    return (HDK_Enum_PN_WANType*)(pMember && pMember->type == HDK_Type_PN_WANType ? &((HDK_Member__INT__*)pMember)->iValue : 0);
}


/*
 * HDK_Type_PN_WiFiEncryption enumeration type
 */

static char* s_HDK_Enum_PN_WiFiEncryption__VALUESTRINGS__[] =
{
    /* HDK_Enum_PN_WiFiEncryption__UNKNOWN__ */ 0,
    /* HDK_Enum_PN_WiFiEncryption_WEP_64 */ "WEP-64",
    /* HDK_Enum_PN_WiFiEncryption_WEP_128 */ "WEP-128",
    /* HDK_Enum_PN_WiFiEncryption_AES */ "AES",
    /* HDK_Enum_PN_WiFiEncryption_TKIP */ "TKIP",
    /* HDK_Enum_PN_WiFiEncryption_TKIPORAES */ "TKIPORAES",
    /* HDK_Enum_PN_WiFiEncryption_ */ "",
};

static HDK_Member* HDK_Type_PN_WiFiEncryption__NEW__(void)
{
    return HDK_Type__INT__NEW__();
}

static void HDK_Type_PN_WiFiEncryption__FREE__(HDK_Member* pMember)
{
    HDK_Type__INT__FREE__(pMember);
}

static int HDK_Type_PN_WiFiEncryption__DESERIALIZE__(HDK_Member* pMember, char* pszValue)
{
    char** ppszBegin = s_HDK_Enum_PN_WiFiEncryption__VALUESTRINGS__;
    char** ppszEnd = s_HDK_Enum_PN_WiFiEncryption__VALUESTRINGS__ + sizeof(s_HDK_Enum_PN_WiFiEncryption__VALUESTRINGS__) / sizeof(*s_HDK_Enum_PN_WiFiEncryption__VALUESTRINGS__);
    return HDK_Type__ENUM__DESERIALIZE__(pMember, pszValue, ppszBegin, ppszEnd);
}

static int HDK_Type_PN_WiFiEncryption__SERIALIZE__(void* pDeviceCtx, int fNoWrite, HDK_Member* pMember)
{
    char** ppszBegin = s_HDK_Enum_PN_WiFiEncryption__VALUESTRINGS__;
    char** ppszEnd = s_HDK_Enum_PN_WiFiEncryption__VALUESTRINGS__ + sizeof(s_HDK_Enum_PN_WiFiEncryption__VALUESTRINGS__) / sizeof(*s_HDK_Enum_PN_WiFiEncryption__VALUESTRINGS__);
    return HDK_Type__ENUM__SERIALIZE__(pDeviceCtx, fNoWrite, pMember, ppszBegin, ppszEnd);
}

HDK_Member* HDK_Set_PN_WiFiEncryption(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_WiFiEncryption eValue)
{
    if (eValue != HDK_Enum_PN_WiFiEncryption__UNKNOWN__)
    {
        return HDK_Set_IntHelper(pStruct, element, HDK_Type_PN_WiFiEncryption, eValue);
    }
    else
    {
        return 0;
    }
}

HDK_Member* HDK_Append_PN_WiFiEncryption(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_WiFiEncryption eValue)
{
    if (eValue != HDK_Enum_PN_WiFiEncryption__UNKNOWN__)
    {
        return HDK_Append_IntHelper(pStruct, element, HDK_Type_PN_WiFiEncryption, eValue);
    }
    else
    {
        return 0;
    }
}

HDK_Enum_PN_WiFiEncryption* HDK_Get_PN_WiFiEncryption(HDK_Struct* pStruct, HDK_Element element)
{
    return HDK_Get_PN_WiFiEncryptionMember(HDK_Get_Member(pStruct, element, HDK_Type_PN_WiFiEncryption));
}

HDK_Enum_PN_WiFiEncryption HDK_Get_PN_WiFiEncryptionEx(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_WiFiEncryption eValue)
{
    HDK_Enum_PN_WiFiEncryption* peDefault = HDK_Get_PN_WiFiEncryption(pStruct, element);
    return (peDefault ? *peDefault : eValue);
}

HDK_Enum_PN_WiFiEncryption* HDK_Get_PN_WiFiEncryptionMember(HDK_Member* pMember)
{
    return (HDK_Enum_PN_WiFiEncryption*)(pMember && pMember->type == HDK_Type_PN_WiFiEncryption ? &((HDK_Member__INT__*)pMember)->iValue : 0);
}


/*
 * HDK_Type_PN_WiFiMode enumeration type
 */

static char* s_HDK_Enum_PN_WiFiMode__VALUESTRINGS__[] =
{
    /* HDK_Enum_PN_WiFiMode__UNKNOWN__ */ 0,
    /* HDK_Enum_PN_WiFiMode_802_11a */ "802.11a",
    /* HDK_Enum_PN_WiFiMode_802_11b */ "802.11b",
    /* HDK_Enum_PN_WiFiMode_802_11g */ "802.11g",
    /* HDK_Enum_PN_WiFiMode_802_11n */ "802.11n",
    /* HDK_Enum_PN_WiFiMode_802_11bg */ "802.11bg",
    /* HDK_Enum_PN_WiFiMode_802_11bn */ "802.11bn",
    /* HDK_Enum_PN_WiFiMode_802_11bgn */ "802.11bgn",
    /* HDK_Enum_PN_WiFiMode_802_11gn */ "802.11gn",
    /* HDK_Enum_PN_WiFiMode_802_11an */ "802.11an",
    /* HDK_Enum_PN_WiFiMode_ */ "",
};

static HDK_Member* HDK_Type_PN_WiFiMode__NEW__(void)
{
    return HDK_Type__INT__NEW__();
}

static void HDK_Type_PN_WiFiMode__FREE__(HDK_Member* pMember)
{
    HDK_Type__INT__FREE__(pMember);
}

static int HDK_Type_PN_WiFiMode__DESERIALIZE__(HDK_Member* pMember, char* pszValue)
{
    char** ppszBegin = s_HDK_Enum_PN_WiFiMode__VALUESTRINGS__;
    char** ppszEnd = s_HDK_Enum_PN_WiFiMode__VALUESTRINGS__ + sizeof(s_HDK_Enum_PN_WiFiMode__VALUESTRINGS__) / sizeof(*s_HDK_Enum_PN_WiFiMode__VALUESTRINGS__);
    return HDK_Type__ENUM__DESERIALIZE__(pMember, pszValue, ppszBegin, ppszEnd);
}

static int HDK_Type_PN_WiFiMode__SERIALIZE__(void* pDeviceCtx, int fNoWrite, HDK_Member* pMember)
{
    char** ppszBegin = s_HDK_Enum_PN_WiFiMode__VALUESTRINGS__;
    char** ppszEnd = s_HDK_Enum_PN_WiFiMode__VALUESTRINGS__ + sizeof(s_HDK_Enum_PN_WiFiMode__VALUESTRINGS__) / sizeof(*s_HDK_Enum_PN_WiFiMode__VALUESTRINGS__);
    return HDK_Type__ENUM__SERIALIZE__(pDeviceCtx, fNoWrite, pMember, ppszBegin, ppszEnd);
}

HDK_Member* HDK_Set_PN_WiFiMode(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_WiFiMode eValue)
{
    if (eValue != HDK_Enum_PN_WiFiMode__UNKNOWN__)
    {
        return HDK_Set_IntHelper(pStruct, element, HDK_Type_PN_WiFiMode, eValue);
    }
    else
    {
        return 0;
    }
}

HDK_Member* HDK_Append_PN_WiFiMode(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_WiFiMode eValue)
{
    if (eValue != HDK_Enum_PN_WiFiMode__UNKNOWN__)
    {
        return HDK_Append_IntHelper(pStruct, element, HDK_Type_PN_WiFiMode, eValue);
    }
    else
    {
        return 0;
    }
}

HDK_Enum_PN_WiFiMode* HDK_Get_PN_WiFiMode(HDK_Struct* pStruct, HDK_Element element)
{
    return HDK_Get_PN_WiFiModeMember(HDK_Get_Member(pStruct, element, HDK_Type_PN_WiFiMode));
}

HDK_Enum_PN_WiFiMode HDK_Get_PN_WiFiModeEx(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_WiFiMode eValue)
{
    HDK_Enum_PN_WiFiMode* peDefault = HDK_Get_PN_WiFiMode(pStruct, element);
    return (peDefault ? *peDefault : eValue);
}

HDK_Enum_PN_WiFiMode* HDK_Get_PN_WiFiModeMember(HDK_Member* pMember)
{
    return (HDK_Enum_PN_WiFiMode*)(pMember && pMember->type == HDK_Type_PN_WiFiMode ? &((HDK_Member__INT__*)pMember)->iValue : 0);
}


/*
 * HDK_Type_PN_WiFiSecurity enumeration type
 */

static char* s_HDK_Enum_PN_WiFiSecurity__VALUESTRINGS__[] =
{
    /* HDK_Enum_PN_WiFiSecurity__UNKNOWN__ */ 0,
    /* HDK_Enum_PN_WiFiSecurity_NONE */ "NONE",
    /* HDK_Enum_PN_WiFiSecurity_WEP_64 */ "WEP-64",
    /* HDK_Enum_PN_WiFiSecurity_WEP_128 */ "WEP-128",
    /* HDK_Enum_PN_WiFiSecurity_WPA_Personal */ "WPA-Personal",
    /* HDK_Enum_PN_WiFiSecurity_WPA2_Personal */ "WPA2-Personal",
    /* HDK_Enum_PN_WiFiSecurity_WPA_WPA2_Personal */ "WPA-WPA2-Personal",
    /* HDK_Enum_PN_WiFiSecurity_WPA_Enterprise */ "WPA-Enterprise",
    /* HDK_Enum_PN_WiFiSecurity_WPA2_Enterprise */ "WPA2-Enterprise",
    /* HDK_Enum_PN_WiFiSecurity_WPA_WPA2_Enterprise */ "WPA-WPA2-Enterprise",
    /* HDK_Enum_PN_WiFiSecurity_ */ "",
};

static HDK_Member* HDK_Type_PN_WiFiSecurity__NEW__(void)
{
    return HDK_Type__INT__NEW__();
}

static void HDK_Type_PN_WiFiSecurity__FREE__(HDK_Member* pMember)
{
    HDK_Type__INT__FREE__(pMember);
}

static int HDK_Type_PN_WiFiSecurity__DESERIALIZE__(HDK_Member* pMember, char* pszValue)
{
    char** ppszBegin = s_HDK_Enum_PN_WiFiSecurity__VALUESTRINGS__;
    char** ppszEnd = s_HDK_Enum_PN_WiFiSecurity__VALUESTRINGS__ + sizeof(s_HDK_Enum_PN_WiFiSecurity__VALUESTRINGS__) / sizeof(*s_HDK_Enum_PN_WiFiSecurity__VALUESTRINGS__);
    return HDK_Type__ENUM__DESERIALIZE__(pMember, pszValue, ppszBegin, ppszEnd);
}

static int HDK_Type_PN_WiFiSecurity__SERIALIZE__(void* pDeviceCtx, int fNoWrite, HDK_Member* pMember)
{
    char** ppszBegin = s_HDK_Enum_PN_WiFiSecurity__VALUESTRINGS__;
    char** ppszEnd = s_HDK_Enum_PN_WiFiSecurity__VALUESTRINGS__ + sizeof(s_HDK_Enum_PN_WiFiSecurity__VALUESTRINGS__) / sizeof(*s_HDK_Enum_PN_WiFiSecurity__VALUESTRINGS__);
    return HDK_Type__ENUM__SERIALIZE__(pDeviceCtx, fNoWrite, pMember, ppszBegin, ppszEnd);
}

HDK_Member* HDK_Set_PN_WiFiSecurity(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_WiFiSecurity eValue)
{
    if (eValue != HDK_Enum_PN_WiFiSecurity__UNKNOWN__)
    {
        return HDK_Set_IntHelper(pStruct, element, HDK_Type_PN_WiFiSecurity, eValue);
    }
    else
    {
        return 0;
    }
}

HDK_Member* HDK_Append_PN_WiFiSecurity(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_WiFiSecurity eValue)
{
    if (eValue != HDK_Enum_PN_WiFiSecurity__UNKNOWN__)
    {
        return HDK_Append_IntHelper(pStruct, element, HDK_Type_PN_WiFiSecurity, eValue);
    }
    else
    {
        return 0;
    }
}

HDK_Enum_PN_WiFiSecurity* HDK_Get_PN_WiFiSecurity(HDK_Struct* pStruct, HDK_Element element)
{
    return HDK_Get_PN_WiFiSecurityMember(HDK_Get_Member(pStruct, element, HDK_Type_PN_WiFiSecurity));
}

HDK_Enum_PN_WiFiSecurity HDK_Get_PN_WiFiSecurityEx(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_WiFiSecurity eValue)
{
    HDK_Enum_PN_WiFiSecurity* peDefault = HDK_Get_PN_WiFiSecurity(pStruct, element);
    return (peDefault ? *peDefault : eValue);
}

HDK_Enum_PN_WiFiSecurity* HDK_Get_PN_WiFiSecurityMember(HDK_Member* pMember)
{
    return (HDK_Enum_PN_WiFiSecurity*)(pMember && pMember->type == HDK_Type_PN_WiFiSecurity ? &((HDK_Member__INT__*)pMember)->iValue : 0);
}


/*
 * HNAP type info table
 */

static HDK_TypeInfo s_types[] =
{
    { /* HDK_Type__UNKNOWN__ */ 0, 0, 0, 0 },
    { /* HDK_Type__UNKNOWN_ANY__ */ 0, 0, 0, 0 },
    { /* HDK_Type__STRUCT__ */ 0, 0, 0, 0 },
    { /* HDK_Type__BLANK__ */ 0, HDK_Type__BLANK__FREE__, 0, 0 },
    { /* HDK_Type__IPADDRESS__ */ HDK_Type__IPADDRESS__NEW__, HDK_Type__IPADDRESS__FREE__, HDK_Type__IPADDRESS__SERIALIZE__, HDK_Type__IPADDRESS__DESERIALIZE__ },
    { /* HDK_Type__MACADDRESS__ */ HDK_Type__MACADDRESS__NEW__, HDK_Type__MACADDRESS__FREE__, HDK_Type__MACADDRESS__SERIALIZE__, HDK_Type__MACADDRESS__DESERIALIZE__ },
    { /* HDK_Type__RESULT__ */ HDK_Type__RESULT__NEW__, HDK_Type__RESULT__FREE__, HDK_Type__RESULT__SERIALIZE__, HDK_Type__RESULT__DESERIALIZE__ },
    { /* HDK_Type_PN_DeviceType */ HDK_Type_PN_DeviceType__NEW__, HDK_Type_PN_DeviceType__FREE__, HDK_Type_PN_DeviceType__SERIALIZE__, HDK_Type_PN_DeviceType__DESERIALIZE__ },
    { /* HDK_Type_PN_IPProtocol */ HDK_Type_PN_IPProtocol__NEW__, HDK_Type_PN_IPProtocol__FREE__, HDK_Type_PN_IPProtocol__SERIALIZE__, HDK_Type_PN_IPProtocol__DESERIALIZE__ },
    { /* HDK_Type_PN_LANConnection */ HDK_Type_PN_LANConnection__NEW__, HDK_Type_PN_LANConnection__FREE__, HDK_Type_PN_LANConnection__SERIALIZE__, HDK_Type_PN_LANConnection__DESERIALIZE__ },
    { /* HDK_Type_PN_TaskExtType */ HDK_Type_PN_TaskExtType__NEW__, HDK_Type_PN_TaskExtType__FREE__, HDK_Type_PN_TaskExtType__SERIALIZE__, HDK_Type_PN_TaskExtType__DESERIALIZE__ },
    { /* HDK_Type_PN_WANType */ HDK_Type_PN_WANType__NEW__, HDK_Type_PN_WANType__FREE__, HDK_Type_PN_WANType__SERIALIZE__, HDK_Type_PN_WANType__DESERIALIZE__ },
    { /* HDK_Type_PN_WiFiEncryption */ HDK_Type_PN_WiFiEncryption__NEW__, HDK_Type_PN_WiFiEncryption__FREE__, HDK_Type_PN_WiFiEncryption__SERIALIZE__, HDK_Type_PN_WiFiEncryption__DESERIALIZE__ },
    { /* HDK_Type_PN_WiFiMode */ HDK_Type_PN_WiFiMode__NEW__, HDK_Type_PN_WiFiMode__FREE__, HDK_Type_PN_WiFiMode__SERIALIZE__, HDK_Type_PN_WiFiMode__DESERIALIZE__ },
    { /* HDK_Type_PN_WiFiSecurity */ HDK_Type_PN_WiFiSecurity__NEW__, HDK_Type_PN_WiFiSecurity__FREE__, HDK_Type_PN_WiFiSecurity__SERIALIZE__, HDK_Type_PN_WiFiSecurity__DESERIALIZE__ },
    { /* HDK_Type__BOOL__ */ HDK_Type__BOOL__NEW__, HDK_Type__BOOL__FREE__, HDK_Type__BOOL__SERIALIZE__, HDK_Type__BOOL__DESERIALIZE__ },
    { /* HDK_Type__DATETIME__ */ HDK_Type__DATETIME__NEW__, HDK_Type__DATETIME__FREE__, HDK_Type__DATETIME__SERIALIZE__, HDK_Type__DATETIME__DESERIALIZE__ },
    { /* HDK_Type__INT__ */ HDK_Type__INT__NEW__, HDK_Type__INT__FREE__, HDK_Type__INT__SERIALIZE__, HDK_Type__INT__DESERIALIZE__ },
    { /* HDK_Type__STRING__ */ HDK_Type__STRING__NEW__, HDK_Type__STRING__FREE__, HDK_Type__STRING__SERIALIZE__, HDK_Type__STRING__DESERIALIZE__ },
};

static HDK_TypeInfo* s_types_GetInfo(HDK_Type type)
{
    return &s_types[type];
}


/*
 * Generic member node utilities
 */

/* Helper function for HDK_Serialize */
static int HDK_Serialize_Helper(void* pDeviceCtx, int fNoWrite, HDK_Member* pMember, HDK_Struct* pInputStruct,
                                unsigned int ixTreeMember, int fErrorOutput)
{
    int iContentLength = 0;

    /* Serialize the element open */
    HDK_ElementTreeNode* pMemberNode = s_elementTree_GetNode(ixTreeMember);
    HDK_ElementNode* pMemberElement = s_elements_GetNode(pMember->element);
    HDK_ElementTreeNode* pParentNode = s_elementTree_GetNode(pMemberNode->ixParent);
    HDK_ElementNode* pParentElement = s_elements_GetNode(pParentNode->element);
    iContentLength += HDK_Write(pDeviceCtx, fNoWrite, "<");
    iContentLength += HDK_Write(pDeviceCtx, fNoWrite, pMemberElement->pszElement);
    if (pParentElement->ixNamespace != pMemberElement->ixNamespace)
    {
        iContentLength += HDK_Write(pDeviceCtx, fNoWrite, " xmlns=\"");
        iContentLength += HDK_Write(pDeviceCtx, fNoWrite, s_namespaces_GetString(pMemberElement->ixNamespace));
        iContentLength += HDK_Write(pDeviceCtx, fNoWrite, "\"");
    }
    iContentLength += HDK_Write(pDeviceCtx, fNoWrite, ">");

    /* Serialize the element value */
    if (pMember->type == HDK_Type__STRUCT__)
    {
        unsigned int ixChildBegin;
        unsigned int ixChildEnd;

        /* Output a newline after the struct open tag */
        iContentLength += HDK_Write(pDeviceCtx, fNoWrite, "\n");

        /* Get the struct's child tree nodes */
        if (s_elementTree_GetChildNodes(ixTreeMember, &ixChildBegin, &ixChildEnd))
        {
            /* The result element guaranteed to be the first schema child of a response */
            HDK_Member* pChildBegin = ((HDK_Struct*)pMember)->pHead;

            /* Iterate the child tree nodes */
            unsigned int ixChild;
            for (ixChild = ixChildBegin; ixChild < ixChildEnd; ixChild++)
            {
                /* Output the matching members */
                HDK_ElementTreeNode* pChildNode = s_elementTree_GetNode(ixChild);
                HDK_Member* pChild;
                for (pChild = pChildBegin; pChild; pChild = pChild->pNext)
                {
                    if (pChild->element == (HDK_Element)pChildNode->element)
                    {
                        if (!fErrorOutput || (fErrorOutput && (pChildNode->prop & HDK_ElementTreeProp_ErrorOutput)))
                        {
                            iContentLength += HDK_Serialize_Helper(pDeviceCtx, fNoWrite, pChild, pInputStruct, ixChild,
                                                                   fErrorOutput && (pChildNode->prop & HDK_ElementTreeProp_OutputStruct));
                        }

                        /* Update child begin - we won't match this member again */
                        if (pChild == pChildBegin)
                        {
                            pChildBegin = pChild->pNext;
                        }

                        /* Stop if not unbounded */
                        if (!(pChildNode->prop & HDK_ElementTreeProp_Unbounded))
                        {
                            break;
                        }
                    }
                }
            }
        }
    }
    else
    {
        HDK_TypeInfo* pTypeInfo = s_types_GetInfo(pMember->type);
        if (pTypeInfo && pTypeInfo->pfnSerialize)
        {
            iContentLength += pTypeInfo->pfnSerialize(pDeviceCtx, fNoWrite, pMember);
        }
    }

    /* Serialize the element close */
    iContentLength += HDK_Write(pDeviceCtx, fNoWrite, "</");
    iContentLength += HDK_Write(pDeviceCtx, fNoWrite, pMemberElement->pszElement);
    iContentLength += HDK_Write(pDeviceCtx, fNoWrite, ">\n");

    return iContentLength;
}

/* Serialize an HNAP struct */
int HDK_Serialize(void* pDeviceCtx, int fNoWrite, HDK_Struct* pStruct, HDK_Struct* pInputStruct,
                  int fErrorOutput)
{
    /* Find the top level element in the XML element tree */
    unsigned int ixTreeStruct;
    if (!s_elementTree_GetChildNode(2, pStruct->node.element, &ixTreeStruct))
    {
        return 0;
    }

    return HDK_Serialize_Helper(pDeviceCtx, fNoWrite, (HDK_Member*)pStruct, pInputStruct,
                                ixTreeStruct, fErrorOutput);
}


/*
 * HNAP de-serialization
 */

/* Initialize XML parsing context */
void HDK_Parse_Init(HDK_ParseContext* pParseCtx, HDK_Struct* pStruct, void* pDecodeCtx)
{
    memset(pParseCtx, 0, sizeof(*pParseCtx));
    pParseCtx->pInputStack[0] = pStruct;
    pParseCtx->cbValueBuf = 128;
    pParseCtx->pDecodeCtx = pDecodeCtx;
}

/* Free XML parsing context */
void HDK_Parse_Free(HDK_ParseContext* pParseCtx)
{
    if (pParseCtx->pszValue)
    {
        free(pParseCtx->pszValue);
    }
}

/* Handle an XML element open */
void HDK_Parse_ElementOpen(HDK_ParseContext* pParseCtx, char* pszNamespace, char* pszNamespaceEnd,
                           char* pszElement, char* pszElementEnd)
{
    HDK_Element element;
    HDK_ElementTreeNode* pTreeNode;
    unsigned int ixElementOpen = 0;

    /* Already an error? */
    if (pParseCtx->parseError != HDK_ParseError_OK)
    {
        return;
    }

    /* Keep count of elements */
#ifndef HDK_MAX_ELEMENTS
#define HDK_MAX_ELEMENTS 1024
#endif
    if (++pParseCtx->cElements > HDK_MAX_ELEMENTS)
    {
        pParseCtx->parseError = HDK_ParseError_500_XMLInvalidRequest;
        return;
    }

    /* Search for the element enum */
    if (!pszNamespace || !pszElement ||
        (element = s_elements_FindElement(pszNamespace, pszNamespaceEnd, pszElement, pszElementEnd)) == HDK_Element__UNKNOWN__)
    {
        /* Unknown element - is any element allowed here? */
        HDK_ElementTreeNode* pTreeNodeParent = s_elementTree_GetNode(pParseCtx->ixElement);
        if (pTreeNodeParent->type == HDK_Type__UNKNOWN_ANY__)
        {
            pParseCtx->cAnyElement++;
            return;
        }
        else
        {
            pParseCtx->parseError = HDK_ParseError_500_XMLUnknownElement;
        }
        return;
    }

    /* Search for the element immediately under the current node */
    pTreeNode = s_elementTree_GetChildNode(pParseCtx->ixElement, element, &ixElementOpen);
    if (!pTreeNode)
    {
        /* Unknown element - is any element allowed here? */
        HDK_ElementTreeNode* pTreeNodeParent = s_elementTree_GetNode(pParseCtx->ixElement);
        if (pTreeNodeParent->type == HDK_Type__UNKNOWN_ANY__)
        {
            pParseCtx->cAnyElement++;
            return;
        }
        else
        {
            pParseCtx->parseError = HDK_ParseError_500_XMLUnexpectedElement;
            return;
        }
    }

    /* Create a new struct, if necessary */
    if (pTreeNode->type == HDK_Type__STRUCT__)
    {
        /* If we're at the top... */
        if (pParseCtx->ixStack == 0)
        {
            HDK_Struct* pStructCur = pParseCtx->pInputStack[0];

            /* Make sure we don't get multiple input structs */
            if (pParseCtx->fHaveInput)
            {
                pParseCtx->parseError = HDK_ParseError_500_XMLInvalid;
                return;
            }

            /* Validate the type of the input struct */
            if (pStructCur->node.element != (HDK_Element)pTreeNode->element)
            {
                pParseCtx->parseError = HDK_ParseError_500_XMLUnexpectedElement;
                return;
            }

            /* Increment the the input stack index - top is one less */
            pParseCtx->ixStack++;
            pParseCtx->fHaveInput = 1;
        }
        else if (pParseCtx->ixStack >= sizeof(pParseCtx->pInputStack) / sizeof(*pParseCtx->pInputStack))
        {
            /* Too deep... */
            pParseCtx->parseError = HDK_ParseError_500_OutOfMemory;
            return;
        }
        else
        {
            /* Create the new struct */
            HDK_Struct* pStructCur = pParseCtx->pInputStack[pParseCtx->ixStack - 1];
            HDK_Struct* pStructNew = HDK_Append_Struct(pStructCur, element);
            if (!pStructNew)
            {
                pParseCtx->parseError = HDK_ParseError_500_OutOfMemory;
                return;
            }
            pParseCtx->pInputStack[pParseCtx->ixStack++] = pStructNew;
        }
    }

    /* Update the current element index */
    pParseCtx->ixElement = (unsigned short)ixElementOpen;
    if (pParseCtx->pszValue)
    {
        pParseCtx->pszValue[0] = '\0';
        pParseCtx->cbValue = 0;
    }
}

/* Handle an XML element close */
void HDK_Parse_ElementClose(HDK_ParseContext* pParseCtx)
{
    HDK_ElementTreeNode* pTreeNode;
    HDK_TypeInfo* pTypeInfo;

    /* Already an error? */
    if (pParseCtx->parseError != HDK_ParseError_OK)
    {
        return;
    }

    /* Deserialize the type */
    pTreeNode = s_elementTree_GetNode(pParseCtx->ixElement);
    pTypeInfo = s_types_GetInfo((HDK_Type)pTreeNode->type);
    if (pTypeInfo && pTypeInfo->pfnNew && pTypeInfo->pfnDeserialize)
    {
        HDK_Member* pMember = pTypeInfo->pfnNew();
        if (pMember)
        {
            pMember->element = (HDK_Element)pTreeNode->element;
            pMember->type = (HDK_Type)pTreeNode->type;
            pMember->pNext = 0;
            if (pTypeInfo->pfnDeserialize(pMember, (pParseCtx->pszValue ? pParseCtx->pszValue : (char*)"")))
            {
                HDK_Append_Member(pParseCtx->pInputStack[pParseCtx->ixStack - 1], pMember);
            }
            else
            {
                HDK_FreeMember(pMember, 0);
                pParseCtx->parseError = (HDK_ParseError_500_XMLInvalidValue);
            }
        }
        else
        {
            pParseCtx->parseError = HDK_ParseError_500_OutOfMemory;
        }
    }

    /* Update the input stack index, if necessary */
    if (pTreeNode->type == HDK_Type__STRUCT__)
    {
        /* Too shallow? */
        if (pParseCtx->ixStack < 1)
        {
            pParseCtx->parseError = HDK_ParseError_500_XMLInvalid;
            return;
        }

        pParseCtx->ixStack--;
    }

    /* Update the current element index */
    if (pTreeNode->type == HDK_Type__UNKNOWN_ANY__ && pParseCtx->cAnyElement)
    {
        pParseCtx->cAnyElement--;
    }
    else
    {
        pParseCtx->ixElement = pTreeNode->ixParent;
    }
}

/* Handle an XML element value */
void HDK_Parse_ElementValue(HDK_ParseContext* pParseCtx, char* pszValue, int cbValue)
{
    HDK_ElementTreeNode* pTreeNode;
    unsigned int cbValueNew;

    /* Already an error? */
    if (pParseCtx->parseError != HDK_ParseError_OK)
    {
        return;
    }

    /* Does this element have a value? */
    pTreeNode = s_elementTree_GetNode(pParseCtx->ixElement);
    if (pTreeNode->type == HDK_Type__UNKNOWN__ ||
        pTreeNode->type == HDK_Type__UNKNOWN_ANY__ ||
        pTreeNode->type == HDK_Type__STRUCT__)
    {
        return;
    }

    /* Has the value grown too large? */
#ifndef HDK_MAX_VALUESIZE
#define HDK_MAX_VALUESIZE (16 * 1024)
#endif
    cbValueNew = pParseCtx->cbValue + cbValue + 1;
    if (cbValueNew > HDK_MAX_VALUESIZE)
    {
        pParseCtx->parseError = HDK_ParseError_500_XMLInvalidRequest;
        return;
    }

    /* Grow the buffer to fit the value, if needed */
    if (!pParseCtx->pszValue || cbValueNew > pParseCtx->cbValueBuf)
    {
        char* pValueRealloc;

        /* Calculate the new buffer size */
        if (pParseCtx->pszValue)
        {
            pParseCtx->cbValueBuf *= 2;
        }
        if (cbValueNew > pParseCtx->cbValueBuf)
        {
            pParseCtx->cbValueBuf = cbValueNew;
        }

        /* Allocate/reallocate the buffer */
        pValueRealloc = (char*)realloc(pParseCtx->pszValue, pParseCtx->cbValueBuf);
        if (pValueRealloc)
        {
            pParseCtx->pszValue = pValueRealloc;
        }
        else
        {
            pParseCtx->parseError = HDK_ParseError_500_OutOfMemory;
            return;
        }
    }

    /* Append to the value string */
    memcpy(pParseCtx->pszValue + pParseCtx->cbValue, pszValue, cbValue);
    pParseCtx->cbValue += cbValue;
    *(pParseCtx->pszValue + pParseCtx->cbValue) = '\0';
}


/*
 * HNAP request handling
 */

typedef enum _HDK_MethodProperties
{
    HDK_MethodProp_Hidden = 0x01,
    HDK_MethodProp_NoAuth = 0x02
} HDK_MethodProperties;

/* Array of soap request names and their handlers */
typedef struct _HDK_MethodInfo
{
    HDK_MethodFn pfnMethod;
    unsigned int inputElement:8;
    unsigned int outputElement:8;
    unsigned int resultElement:8;
    unsigned int prop:2;
} HDK_MethodInfo;

static HDK_MethodInfo s_methods[] =
{  // RDKB-9912
   // { HDK_Method_PN_AddPortMapping, HDK_Element_PN_AddPortMapping, HDK_Element_PN_AddPortMappingResponse, HDK_Element_PN_AddPortMappingResult, 0x00 },
   // { HDK_Method_PN_DeletePortMapping, HDK_Element_PN_DeletePortMapping, HDK_Element_PN_DeletePortMappingResponse, HDK_Element_PN_DeletePortMappingResult, 0x00 },
   // { HDK_Method_PN_GetConnectedDevices, HDK_Element_PN_GetConnectedDevices, HDK_Element_PN_GetConnectedDevicesResponse, HDK_Element_PN_GetConnectedDevicesResult, 0x00 },
    { HDK_Method_PN_GetDeviceSettings, HDK_Element_PN_GetDeviceSettings, HDK_Element_PN_GetDeviceSettingsResponse, HDK_Element_PN_GetDeviceSettingsResult, 0x00 },
   /* { HDK_Method_PN_GetPortMappings, HDK_Element_PN_GetPortMappings, HDK_Element_PN_GetPortMappingsResponse, HDK_Element_PN_GetPortMappingsResult, 0x00 },
    { HDK_Method_PN_GetRouterLanSettings2, HDK_Element_PN_GetRouterLanSettings2, HDK_Element_PN_GetRouterLanSettings2Response, HDK_Element_PN_GetRouterLanSettings2Result, 0x00 },
    { HDK_Method_PN_GetRouterSettings, HDK_Element_PN_GetRouterSettings, HDK_Element_PN_GetRouterSettingsResponse, HDK_Element_PN_GetRouterSettingsResult, 0x00 },
    { HDK_Method_PN_GetWLanRadioSecurity, HDK_Element_PN_GetWLanRadioSecurity, HDK_Element_PN_GetWLanRadioSecurityResponse, HDK_Element_PN_GetWLanRadioSecurityResult, 0x00 },
    { HDK_Method_PN_GetWLanRadioSettings, HDK_Element_PN_GetWLanRadioSettings, HDK_Element_PN_GetWLanRadioSettingsResponse, HDK_Element_PN_GetWLanRadioSettingsResult, 0x00 },
    { HDK_Method_PN_GetWLanRadios, HDK_Element_PN_GetWLanRadios, HDK_Element_PN_GetWLanRadiosResponse, HDK_Element_PN_GetWLanRadiosResult, 0x00 },
    { HDK_Method_PN_GetWanSettings, HDK_Element_PN_GetWanSettings, HDK_Element_PN_GetWanSettingsResponse, HDK_Element_PN_GetWanSettingsResult, 0x00 },
    { HDK_Method_PN_IsDeviceReady, HDK_Element_PN_IsDeviceReady, HDK_Element_PN_IsDeviceReadyResponse, HDK_Element_PN_IsDeviceReadyResult, 0x00 },
    { HDK_Method_PN_Reboot, HDK_Element_PN_Reboot, HDK_Element_PN_RebootResponse, HDK_Element_PN_RebootResult, 0x00 },*/
    { HDK_Method_PN_SetBridgeConnect, HDK_Element_PN_SetBridgeConnect, HDK_Element_PN_SetBridgeConnectResponse, HDK_Element_PN_SetBridgeConnectResult, 0x00 },
   /* { HDK_Method_PN_SetDeviceSettings, HDK_Element_PN_SetDeviceSettings, HDK_Element_PN_SetDeviceSettingsResponse, HDK_Element_PN_SetDeviceSettingsResult, 0x00 },
    { HDK_Method_PN_SetDeviceSettings2, HDK_Element_PN_SetDeviceSettings2, HDK_Element_PN_SetDeviceSettings2Response, HDK_Element_PN_SetDeviceSettings2Result, 0x00 },
    { HDK_Method_PN_SetRouterLanSettings2, HDK_Element_PN_SetRouterLanSettings2, HDK_Element_PN_SetRouterLanSettings2Response, HDK_Element_PN_SetRouterLanSettings2Result, 0x00 },
    { HDK_Method_PN_SetRouterSettings, HDK_Element_PN_SetRouterSettings, HDK_Element_PN_SetRouterSettingsResponse, HDK_Element_PN_SetRouterSettingsResult, 0x00 },
    { HDK_Method_PN_SetWLanRadioSecurity, HDK_Element_PN_SetWLanRadioSecurity, HDK_Element_PN_SetWLanRadioSecurityResponse, HDK_Element_PN_SetWLanRadioSecurityResult, 0x00 },
    { HDK_Method_PN_SetWLanRadioSettings, HDK_Element_PN_SetWLanRadioSettings, HDK_Element_PN_SetWLanRadioSettingsResponse, HDK_Element_PN_SetWLanRadioSettingsResult, 0x00 },*/
};

static HDK_MethodInfo* s_methods_FindFromElement(HDK_Element inputElement)
{
    HDK_MethodInfo* pMethod;
    HDK_MethodInfo* pMethodEnd;

    /* Search for the input (SOAPAction) element */
    pMethodEnd = s_methods + sizeof(s_methods) / sizeof(*s_methods);
    for (pMethod = s_methods; pMethod != pMethodEnd; pMethod++)
    {
        if (inputElement == (HDK_Element)pMethod->inputElement)
        {
            return pMethod;
        }
    }

    return 0;
}

static HDK_MethodInfo* s_methods_Find(char* pszSOAPAction)
{
    char* pszBegin;
    char* pszEnd;
    HDK_Element inputElement;

    /* Trim the SOAPAction header */
    for (pszBegin = pszSOAPAction; strchr(" \t\"", *pszBegin); pszBegin++) {}
    for (pszEnd = pszBegin + strlen(pszBegin) - 1; pszEnd >= pszBegin && strchr(" \t\"\r\n", *pszEnd); pszEnd--) {}
    pszEnd++;

    /* Find the input element */
    inputElement = s_elements_FindElementFQ(pszBegin, pszEnd);
    if (inputElement == HDK_Element__UNKNOWN__)
    {
        return 0;
    }

    return s_methods_FindFromElement(inputElement);
}


/* Helper function for GetDeviceSettings */
void HDK_Set_PN_GetDeviceSettings_SOAPActions(HDK_Struct* pStruct)
{
    HDK_Struct* pActions = HDK_Set_Struct(pStruct, HDK_Element_PN_SOAPActions);
    if (pActions)
    {
        HDK_MethodInfo* pMethod;
        HDK_MethodInfo* pMethodEnd = s_methods + sizeof(s_methods) / sizeof(*s_methods);
        for (pMethod = s_methods; pMethod != pMethodEnd; pMethod++)
        {
            /* Hidden? */
            if (!(pMethod->prop & HDK_MethodProp_Hidden))
            {
                /* Build the action URI */
                char szSOAPAction[64]; /* this buffer is guaranteed to be large enough for the longest action URI */
                HDK_ElementNode* pElemNode = s_elements_GetNode(pMethod->inputElement);
                strcpy(szSOAPAction, s_namespaces_GetString(pElemNode->ixNamespace));
                if (szSOAPAction[strlen(szSOAPAction) - 1] != '/')
                {
                    strcat(szSOAPAction, "/");
                }
                strcat(szSOAPAction, pElemNode->pszElement);

                /* Add the action URI string member */
                HDK_Append_String(pActions, HDK_Element_PN_string, szSOAPAction);
            }
        }
    }
}

/* Determine if an HNAP action requires authentication */
int HDK_Request_RequiresAuth(char* pszSOAPAction, int* pfRequiresAuth)
{
    HDK_MethodInfo* pMethod = s_methods_Find(pszSOAPAction);
    if (pMethod)
    {
        *pfRequiresAuth = !(pMethod->prop & HDK_MethodProp_NoAuth);
    }
    return pMethod != 0;
}

/* Helper function for HDK_Request_Init */
static HDK_MethodFn HDK_Request_InitHelper(HDK_MethodInfo* pMethod, int fAllowHidden,
                                           HDK_Struct* pInput, HDK_Struct* pOutput, HDK_Element* pResultElement)
{
    if (pMethod)
    {
        if (pInput)
        {
            pInput->node.element = (HDK_Element)pMethod->inputElement;
            pInput->node.type = HDK_Type__STRUCT__;
        }
        if (pOutput)
        {
            pOutput->node.element = (HDK_Element)pMethod->outputElement;
            pOutput->node.type = HDK_Type__STRUCT__;
            HDK_Set_Result(pOutput, (HDK_Element)pMethod->resultElement, HDK_Enum_Result_OK);
        }
        if (pResultElement)
        {
            *pResultElement = (HDK_Element)pMethod->resultElement;
        }
        return fAllowHidden || !(pMethod->prop & HDK_MethodProp_Hidden) ? pMethod->pfnMethod : 0;
    }
    else
    {
        return 0;
    }
}

/* Initialize an HNAP request input and output structs */
HDK_MethodFn HDK_Request_Init(char* pszSOAPAction, int fAllowHidden,
                              HDK_Struct* pInput, HDK_Struct* pOutput, HDK_Element* pResultElement)
{
    return HDK_Request_InitHelper(s_methods_Find(pszSOAPAction), fAllowHidden,
                                  pInput, pOutput, pResultElement);
}

/* Initialize an HNAP request input and output structs (from the element) */
HDK_MethodFn HDK_Request_InitFromElement(HDK_Element inputElement, int fAllowHidden,
                                         HDK_Struct* pInput, HDK_Struct* pOutput, HDK_Element* pResultElement)
{
    return HDK_Request_InitHelper(s_methods_FindFromElement(inputElement), fAllowHidden,
                                  pInput, pOutput, pResultElement);
}

/* Helper function for HDK_Request_Validate */
static int HDK_Request_Validate_Helper(HDK_Struct* pStruct, unsigned int ixTreeStruct, int fErrorOutput)
{
    /* Get the struct's child tree nodes */
    unsigned int ixChildBegin;
    unsigned int ixChildEnd;
    if (s_elementTree_GetChildNodes(ixTreeStruct, &ixChildBegin, &ixChildEnd))
    {
        unsigned int ixChild;
        HDK_Member* pMember;

        /* Iterate the child tree nodes */
        for (ixChild = ixChildBegin; ixChild < ixChildEnd; ixChild++)
        {
            HDK_ElementTreeNode* pChildNode = s_elementTree_GetNode(ixChild);

            /* Count the matching members */
            unsigned int nChild = 0;
            for (pMember = pStruct->pHead; pMember; pMember = pMember->pNext)
            {
                if (pMember->element == (HDK_Element)pChildNode->element &&
                    (pMember->type == (HDK_Type)pChildNode->type || pMember->type == HDK_Type__BLANK__))
                {
                    nChild++;
                }
            }

            /* Does the count match the element tree? */
            if ((nChild == 0 && !((pChildNode->prop & HDK_ElementTreeProp_Optional) ||
                                  (fErrorOutput && !(pChildNode->prop & HDK_ElementTreeProp_ErrorOutput)))) ||
                (nChild > 1 && !(pChildNode->prop & HDK_ElementTreeProp_Unbounded)))
            {
                return 0;
            }
        }

        /* Iterate the members */
        for (pMember = pStruct->pHead; pMember; pMember = pMember->pNext)
        {
            /* Ensure the member is allowed in this struct */
            int fAllowed = 0;
            for (ixChild = ixChildBegin; ixChild < ixChildEnd; ixChild++)
            {
                HDK_ElementTreeNode* pChildNode = s_elementTree_GetNode(ixChild);
                if (pMember->element == (HDK_Element)pChildNode->element &&
                    (pMember->type == (HDK_Type)pChildNode->type || pMember->type == HDK_Type__BLANK__))
                {
                    fAllowed = 1;

                    /* Validate struct members */
                    if (pMember->type == HDK_Type__STRUCT__)
                    {
                        if (!HDK_Request_Validate_Helper(HDK_Get_StructMember(pMember), ixChild,
                                                         fErrorOutput && (pChildNode->prop & HDK_ElementTreeProp_OutputStruct)))
                        {
                            return 0;
                        }
                    }

                    break;
                }
            }
            if (!fAllowed)
            {
                return 0;
            }
        }
    }
    else
    {
        /* No child tree nodes - ensure there are no members */
        if (pStruct->pHead)
        {
            return 0;
        }
    }

    return 1;
}

/* Validate a request input struct */
int HDK_Request_Validate(HDK_Struct* pStruct, HDK_Element topElement, int fErrorOutput)
{
    unsigned int ixTopElement;

    /* Ensure that the top element matches struct's element */
    if (pStruct->node.element != topElement)
    {
        return 0;
    }

    /* Find the top level element in the XML element tree */
    if (!s_elementTree_GetChildNode(2, topElement, &ixTopElement))
    {
        return 0;
    }

    /* Validate the struct */
    return HDK_Request_Validate_Helper(pStruct, ixTopElement, fErrorOutput);
}
