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

#ifndef __HDK_DEVICE_H__
#define __HDK_DEVICE_H__

#include "hdk_data.h"
#include "hdk_methods.h"
#include "hdk_ccsp_mbus.h"


/*
 * HNAP abstract device value enumeration.  Used by HDK_Device_CheckValue,
 * HDK_Device_GetValue, HDK_Device_SetValue, and HDK_Device_ValidateValue to
 * retrieve device information.
 */
typedef enum _HDK_DeviceValue
{
    HDK_DeviceValue__UNKNOWN__ = 0,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_DeviceName, HDK_Type__STRING__ }
     */
    HDK_DeviceValue_DeviceName,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_DeviceType, HDK_Type__STRING__ }
     */
    HDK_DeviceValue_DeviceType,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_VendorName, HDK_Type__STRING__ }
     */
    HDK_DeviceValue_VendorName,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_ModelDescription, HDK_Type__STRING__ }
     */
    HDK_DeviceValue_ModelDescription,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_ModelName, HDK_Type__STRING__ }
     */
    HDK_DeviceValue_ModelName,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_FirmwareVersion, HDK_Type__STRING__ }
     */
    HDK_DeviceValue_FirmwareVersion,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_PresentationURL, HDK_Type__STRING__ }
     */
    HDK_DeviceValue_PresentationURL,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_SubDeviceURLs, HDK_Type__STRUCT__ }
     *     { HDK_Element_string, HDK_Type__STRING__ }
     *     { ... }
     */
    HDK_DeviceValue_SubDeviceURLs,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_Tasks, HDK_Type__STRUCT__ }
     *     { HDK_Element_string, HDK_Type__STRING__ }
     *     { ... }
     */
    HDK_DeviceValue_Tasks,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (out):
     *
     * { HDK_Element__UNKNOWN__, HDK_Type__BOOLEAN__ }
     */
    HDK_DeviceValue_TimeZoneSetAllowed,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_TimeZone, HDK_Type__STRING__ }
     */
    HDK_DeviceValue_TimeZone,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_SerialNumber, HDK_Type__STRING__ }
     */
    HDK_DeviceValue_SerialNumber,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_Locale, HDK_Type__STRING__ }
     */
    HDK_DeviceValue_Locale,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_AutoAdjustDST, HDK_Type__BOOLEAN__ }
     */
    HDK_DeviceValue_AutoAdjustDST,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_SSL, HDK_Type__BOOLEAN__ }
     */
    HDK_DeviceValue_SSL,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_SupportedLocales, HDK_Type__STRUCT__ }
     *     { HDK_Element_string, HDK_Type__STRING__ }
     *     { ... }
     */
    HDK_DeviceValue_SupportedLocales,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_AdminPassword, HDK_Type__STRING__ }
     */
    HDK_DeviceValue_AdminPassword,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_ModelRevision, HDK_Type__STRING__ }
     */
    HDK_DeviceValue_ModelRevision,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_FirmwareDate, HDK_Type__STRING__ }
     */
    HDK_DeviceValue_FirmwareDate,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_UpdateMethods, HDK_Type__STRUCT__ }
     *     { HDK_Element_string, HDK_Type__STRING__ }
     *     { ... }
     */
    HDK_DeviceValue_UpdateMethods,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_IsDeviceReady, HDK_Type__BOOLEAN__ }
     */
    HDK_DeviceValue_IsDeviceReady,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (out):
     *
     * { HDK_Element__UNKNOWN__, HDK_Type__BOOLEAN__ }
     */
    HDK_DeviceValue_UsernameSetAllowed,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_Username, HDK_Type__STRING__ }
     */
    HDK_DeviceValue_Username,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_Type, HDK_Type_WANType }
     */
    HDK_DeviceValue_WanType,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_Status, HDK_Type_WANStatus }
     */
    HDK_DeviceValue_WanStatus,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_Username, HDK_Type__STRING__ }
     */
    HDK_DeviceValue_WanUsername,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_Password, HDK_Type__STRING__ }
     */
    HDK_DeviceValue_WanPassword,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_MaxIdleTime, HDK_Type__INT__ }
     */
    HDK_DeviceValue_WanMaxIdleTime,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_AutoReconnect, HDK_Type__BOOLEAN__ }
     */
    HDK_DeviceValue_WanAutoReconnect,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_ServiceName, HDK_Type__STRING__ }
     */
    HDK_DeviceValue_WanAuthService,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_ServiceName, HDK_Type__STRING__ }
     */
    HDK_DeviceValue_WanPPPoEService,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_ServiceName, HDK_Type__STRING__ }
     */
    HDK_DeviceValue_WanLoginService,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_IPAddress, HDK_Type_IPAddress }
     */
    HDK_DeviceValue_WanIP,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_Gateway, HDK_Type_IPAddress }
     */
    HDK_DeviceValue_WanGateway,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_SubnetMask, HDK_Type_IPAddress }
     */
    HDK_DeviceValue_WanSubnetMask,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_DNS, HDK_Type__STRUCT__ }
     *     { HDK_Element_Primary, HDK_Type_IPAddress },
     *     { HDK_Element_Secondary, HDK_Type_IPAddress },
     *     { HDK_Element_Tertiary, HDK_Type_IPAddress }
     */
    HDK_DeviceValue_WanDNSSettings,

    /*
     * Get:
     *     Return 1 for success, 0 for failure.
     *
     * HDK_Struct* pStruct (out):
     *
     * { HDK_Element__UNKNOWN__, HDK_Type__BOOL__ }
     */
    HDK_DeviceValue_AutoMTUAllowed,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_MTU, HDK_Type__INT__ }
     */
    HDK_DeviceValue_WanMTU,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_MacAddress, HDK_Type_MACAddress }
     */
    HDK_DeviceValue_MacAddress,
    HDK_DeviceValue_WANMacAddress,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_AutoDetectType, HDK_Type_WANType }
     */
    HDK_DeviceValue_WanAutoDetectType,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_SupportedTypes, HDK_Type__STRUCT__ }
     *     { HDK_Element_string, HDK_Type__STRING__ }
     *     { ... }
     */
    HDK_DeviceValue_WanSupportedTypes,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_ClientStats, HDK_Type__STRUCT__ }
     *     { HDK_Element_ClientStat, HDK_Type__STRUCT__ }
     *         { HDK_Element_MacAddress, HDK_Type_MACAddress }
     *         { HDK_Element_Wireless, HDK_Type__BOOLEAN__ }
     *         { HDK_Element_LinkSpeedIn, HDK_Type__INT__ }
     *         { HDK_Element_LinkSpeedOut, HDK_Type__INT__ }
     *         { HDK_Element_SignalStrength, HDK_Type__INT__ }
     *     { ... }
     */
    HDK_DeviceValue_ClientStats,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_ConnectedClients, HDK_Type__STRUCT__ }
     *     { HDK_Element_ConnectedClient, HDK_Type__STRUCT__ }
     *         { HDK_Element_ConnectTime, HDK_Type__DATETIME__ }
     *         { HDK_Element_MacAddress, HDK_Type_MACAddress_ }
     *         { HDK_Element_DeviceName, HDK_Type__STRING__ }
     *         { HDK_Element_PortName, HDK_Type_LANConnection }
     *         { HDK_Element_Wireless, HDK_Type__BOOLEAN__ }
     *         { HDK_Element_Active, HDK_Type__BOOLEAN__ }
     *     { ... }
     */
    HDK_DeviceValue_ConnectedClients,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_Enabled, HDK_Type__BOOLEAN__ }
     */
    HDK_DeviceValue_MACFilterEnabled,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_IsAllowList, HDK_Type__BOOLEAN__ }
     */
    HDK_DeviceValue_MACFilterAllowList,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_MACList, HDK_Type__STRUCT__ }
     *     { HDK_Element_MACInfo, HDK_Type__STRUCT__ }
     *         { HDK_Element_MacAddress, HDK_Type_MACAddress }
     *         { HDK_Element_DeviceName, HDK_Type__STRING__ }
     *     { ... }
     */
    HDK_DeviceValue_MACList,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_Stats, HDK_Type__STRUCT__ }
     *     { HDK_Element_NetworkStats, HDK_Type__STRUCT__ }
     *         { HDK_Element_PortName, HDK_Type_LANConnection }
     *         { HDK_Element_PacketsRecieved, HDK_Type__LONG__ }
     *         { HDK_Element_PacketsSent, HDK_Type__LONG__ }
     *         { HDK_Element_BytesRecieved, HDK_Type__LONG__ }
     *         { HDK_Element_BytesSent, HDK_Type__LONG__ }
     *     { ... }
     */
    HDK_DeviceValue_DeviceNetworkStats,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_PortMappings, HDK_Type__STRUCT__ }
     *     { HDK_Element_PortMapping, HDK_Type__STRUCT__ }
     *         { HDK_Element_PortMappingDescription, HDK_Type__STRING__ }
     *         { HDK_Element_InternalClient, HDK_Type_IPAddress }
     *         { HDK_Element_PortMappingProtocol, HDK_Type_IPProtocol }
     *         { HDK_Element_ExternalPort, HDK_Type__INT__ }
     *         { HDK_Element_InternalPort, HDK_Type__INT__ }
     *     { ... }
     */
    HDK_DeviceValue_PortMappings,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_RouterIPAddress, HDK_Type_IPAddress }
     */
    HDK_DeviceValue_LanIPAddress,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_RouterSubnetMask, HDK_Type_IPAddress }
     */
    HDK_DeviceValue_LanSubnetMask,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_DHCPServerEnabled, HDK_Type__BOOLEAN__ }
     */
    HDK_DeviceValue_DHCPEnabled,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_IPAddressFirst, HDK_Type_IPAddress }
     */
    HDK_DeviceValue_DHCPFirstIP,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_IPAddressLast, HDK_Type_IPAddress }
     */
    HDK_DeviceValue_DHCPLastIP,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_LeaseTime, HDK_Type__INT__ }
     */
    HDK_DeviceValue_DHCPLeaseTime,

    /*
     * Get:
     *     Return 1 for success, 0 for failure.
     *
     * HDK_Struct* pStruct (out):
     *
     * { HDK_Element__UNKNOWN__, HDK_Type__BOOL__ }
     */
    HDK_DeviceValue_DHCPReservationsAllowed,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_DHCPReservations, HDK_Type__STRUCT__ }
     *     { HDK_Element_DHCPReservation, HDK_Type__STRUCT__ }
     *         { HDK_Element_DeviceName, HDK_Type__STRING__ }
     *         { HDK_Element_IPAddress, HDK_Type_IPAddress }
     *         { HDK_Element_MacAddress, HDK_Type_MACAddress }
     *     { ... }
     */
    HDK_DeviceValue_DHCPReservations,

    /*
     * Get:
     *     Return 1 for success, 0 for failure.
     *
     * HDK_Struct* pStruct (out):
     *
     * { HDK_Element__UNKNOWN__, HDK_Type__BOOL__ }
     */
    HDK_DeviceValue_RemoteManagementAllowed,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_ManageRemote, HDK_Type__BOOLEAN__ }
     */
    HDK_DeviceValue_ManageRemote,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_ManageWireless, HDK_Type__BOOLEAN__ }
     */
    HDK_DeviceValue_ManageWireless,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_RemotePort, HDK_Type__INT__ }
     */
    HDK_DeviceValue_RemotePort,

    /*
     * Get:
     *     Return 1 for success, 0 for failure.
     *
     * HDK_Struct* pStruct (out):
     *
     * { HDK_Element__UNKNOWN__, HDK_Type__BOOL__ }
     */
    HDK_DeviceValue_ManageViaSSLAllowed,

    /*
     * Get:
     *     Return 1 for success, 0 for failure.
     *
     * HDK_Struct* pStruct (out):
     *
     * { HDK_Element__UNKNOWN__, HDK_Type__BOOL__ }
     */
    HDK_DeviceValue_ManageOnlyViaSSL,

    /*
     * Get:
     *     Return 1 for success, 0 for failure.
     *
     * HDK_Struct* pStruct (out):
     *
     * { HDK_Element__UNKNOWN__, HDK_Type__BOOL__ }
     */
    HDK_DeviceValue_RemoteSSLNeedsSSL,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_RemoteSSL, HDK_Type__BOOLEAN__ }
     */
    HDK_DeviceValue_RemoteSSL,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (out):
     *
     * { HDK_Element__UNKNOWN__, HDK_Type__STRING__ }
     */
    HDK_DeviceValue_AdminPasswordDefault,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (out):
     *
     * { HDK_Element__UNKNOWN__, HDK_Type__STRING__ }
     */
    HDK_DeviceValue_DomainNameChangeAllowed,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_DomainName, HDK_Type__STRING__ }
     */
    HDK_DeviceValue_DomainName,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (out):
     *
     * { HDK_Element__UNKNOWN__, HDK_Type__STRING__ }
     */
    HDK_DeviceValue_WiredQoSAllowed,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_WiredQoS, HDK_Type__BOOLEAN__ }
     */
    HDK_DeviceValue_WiredQoS,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (out):
     *
     * { HDK_Element_WPSPin, HDK_Type__STRING__ }
     */
    HDK_DeviceValue_WPSPin,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (out):
     *
     * { HDK_Element_RadioID, HDK_Type__STRING__ }
     */
    HDK_DeviceValue_WLanRadioID,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_Enabled, HDK_Type__BOOLEAN__ }
     */
    HDK_DeviceValue_WLanEnabled,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_Enabled, HDK_Type__BOOLEAN__ }
     */
    HDK_DeviceValue_WLanSecurityEnabled,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_Type, HDK_Type_WiFiSecurity }
     */
    HDK_DeviceValue_WLanType,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_Encryption, HDK_Type_WiFiEncryption }
     */
    HDK_DeviceValue_WLanEncryption,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_Key, HDK_Type__STRING__ }
     */
    HDK_DeviceValue_WLanKey,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_KeyRenewal, HDK_Type__STRING__ }
     */
    HDK_DeviceValue_WLanKeyRenewal,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_RadiusIP1, HDK_Type_IPAddress }
     */
    HDK_DeviceValue_WLanRadiusIP1,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_RadiusPort, HDK_Type__INT__ }
     */
    HDK_DeviceValue_WLanRadiusPort1,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_RadiusSecret1, HDK_Type__STRING__ }
     */
    HDK_DeviceValue_WLanRadiusSecret1,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_RadiusIP2, HDK_Type_IPAddress }
     */
    HDK_DeviceValue_WLanRadiusIP2,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_RadiusPort2, HDK_Type__INT__ }
     */
    HDK_DeviceValue_WLanRadiusPort2,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_RadiusSecret2, HDK_Type__STRING__ }
     */
    HDK_DeviceValue_WLanRadiusSecret2,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_Mode, HDK_Type_WiFiMode }
     */
    HDK_DeviceValue_WLanMode,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_SSID, HDK_Type__STRING__ }
     */
    HDK_DeviceValue_WLanSSID,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_SSIDBroadcast, HDK_Type__BOOLEAN__ }
     */
    HDK_DeviceValue_WLanSSIDBroadcast,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_ChannelWidth, HDK_Type__INT__ }
     */
    HDK_DeviceValue_WLanChannelWidth,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_Channel, HDK_Type__INT__ }
     */
    HDK_DeviceValue_WLanChannel,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_SecondaryChannel, HDK_Type__INT__ }
     */
    HDK_DeviceValue_WLanSecondaryChannel,

    /*
     * Get/Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_QoS, HDK_Type__BOOLEAN__ }
     */
    HDK_DeviceValue_WLanQoS,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (out):
     *
     * { HDK_Element_RadioInfos, HDK_Type__STRUCT__ }
     *     { HDK_Element_RadioInfo, HDK_Type__STRUCT__ }
     *         { HDK_Element_RadioID, HDK_Type__STRING__ }
     *         { HDK_Element_Frequency, HDK_Type__INT__ }
     *         { HDK_Element_SupportedModes, HDK_Type__STRUCT__ }
     *             { HDK_Element_string, HDK_Type_WiFiMode }
     *             { ... }
     *         { HDK_Element_Channels, HDK_Type__STRUCT__ }
     *             { HDK_Element_int, HDK_Type__INT__ }
     *             { ... }
     *         { HDK_Element_WideChannels, HDK_Type__STRUCT__ }
     *             { HDK_Element_WideChannel, HDK_Type__STRUCT__ }
     *                 { HDK_Element_Channel, HDK_Type__INT__ }
     *                 { HDK_Element_SecondaryChannels, HDK_Type__STRUCT__ }
     *                     { HDK_Element_int, HDK_Type__INT__ }
     *                     { ... }
     *             { ... }
     *         { HDK_Element_SupportedSecurity, HDK_Type__STRUCT__ }
     *             { HDK_Element_SecurityInfo, HDK_Type__STRUCT__ }
     *                 { HDK_Element_SecurityType, HDK_Type_WiFiSecurity }
     *                 { HDK_Element_Encryptions, HDK_Type__STRUCT__ }
     *                     { HDK_Element_string, HDK_Type_WiFiEncryption }
     *                     { ... }
     *             { ... }
     */
    HDK_DeviceValue_RadioInfos,

    /*
     * Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in):
     *
     * { HDK_Element_IsAccessPoint, HDK_Type__BOOLEAN__ }
     */
    HDK_DeviceValue_AccessPointMode,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (out):
     *
     * { HDK_Element_NewIPAddress, HDK_Type_IPAddress }
     */
    HDK_DeviceValue_NewIPAddress,

    /*
     * Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in):
     *
     * { HDK_Element_RenewTimeout, HDK_Type__INT__ }
     */
    HDK_DeviceValue_RenewWanTimeout,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (out):
     *
     * { HDK_Element_RadioFrequencyInfo, HDK_Type__STRUCT__ }
     *     { HDK_Element_RadioID, HDK_Type__STRING__ }
     *     { HDK_Element_Frequencies, HDK_Type__STRUCT__ }
     *         { HDK_Element_int, HDK_Type__INT__ }
     *         { ... }
     */
    HDK_DeviceValue_RadioFrequencyInfos,

    /*
     * Set/Validate:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in):
     *
     * { HDK_Element_RadioFrequency, HDK_Type__INT__ }
     */
    HDK_DeviceValue_RadioFrequency,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_SupportedNetworkType, HDK_Type__STRUCT__ }
     *     { HDK_Element_string, HDK_Type_WiFiNetworkType }
     *     { ... }
     */
    HDK_DeviceValue_WiFiClientSupportedNetworkTypes,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (in/out):
     *
     * { HDK_Element_SupportedSecurity, HDK_Type__STRUCT__ }
     *     { HDK_Element_SecurityInfo, HDK_Type__STRUCT__ }
     *         { HDK_Element_SecurityType, HDK_Type_WiFiClientSecurity }
     *         { HDK_Element_Encryptions, HDK_Type__STRUCT__ }
     *             { HDK_Element_string, HDK_Type_WiFiEncryption }
     *             { ... }
     *     { ... }
     */
    HDK_DeviceValue_WiFiClientSupportedSecurity,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (out):
     *
     * { HDK_Element_SSID, HDK_Type__STRING__ }
     */
    HDK_DeviceValue_WiFiClientSSID,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (out):
     *
     * { HDK_Element_Frequency, HDK_Type__INT__ }
     */
    HDK_DeviceValue_WiFiClientFrequency,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (out):
     *
     * { HDK_Element_Mode, HDK_Type_WiFiClientMode }
     */
    HDK_DeviceValue_WiFiClientMode,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (out):
     *
     * { HDK_Element_ChannelWidth, HDK_Type__INT__ }
     */
    HDK_DeviceValue_WiFiClientChannelWidth,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (out):
     *
     * { HDK_Element_Channel, HDK_Type__INT__ }
     */
    HDK_DeviceValue_WiFiClientChannel,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (out):
     *
     * { HDK_Element_SignalStrength, HDK_Type__INT__ }
     */
    HDK_DeviceValue_WiFiClientSignalStrength,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (out):
     *
     * { HDK_Element_Noise, HDK_Type__INT__ }
     */
    HDK_DeviceValue_WiFiClientNoise,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (out):
     *
     * { HDK_Element_LinkSpeedIn, HDK_Type__INT__ }
     */
    HDK_DeviceValue_WiFiClientLinkSpeedIn,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (out):
     *
     * { HDK_Element_LinkSpeedOut, HDK_Type__INT__ }
     */
    HDK_DeviceValue_WiFiClientLinkSpeedOut,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (out):
     *
     * { HDK_Element_WmmEnabled, HDK_Type__BOOL__ }
     */
    HDK_DeviceValue_WiFiClientWmmEnabled,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (out):
     *
     * { HDK_Element_NetworkType, HDK_Type_WiFiClientNetworkType }
     */
    HDK_DeviceValue_WiFiClientNetworkType,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (out):
     *
     * { HDK_Element_SecurityEnabled, HDK_Type__BOOL__ }
     */
    HDK_DeviceValue_WiFiClientSecurityEnabled,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (out):
     *
     * { HDK_Element_SecurityType, HDK_Type_WiFiClientSecurity }
     */
    HDK_DeviceValue_WiFiClientSecurityType,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (out):
     *
     * { HDK_Element_Encryption, HDK_Type_WiFiEncryption }
     */
    HDK_DeviceValue_WiFiClientEncryption,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (out):
     *
     * { HDK_Element_Key, HDK_Type__STRING__ }
     */
    HDK_DeviceValue_WiFiClientKey,

    /*
     * Get:
     *     Return 1 for success, 0 for failure
     *
     * HDK_Struct* pStruct (out):
     *
     * { HDK_Element_Connected, HDK_Type__BOOL__ }
     */
    HDK_DeviceValue_WiFiClientConnected,
    HDK_DeviceValue_BridgeConnect

} HDK_DeviceValue;

/*
 * Abstract device value get/set/validate functions.
 *
 * Return 1 upon success, 0 upon failure.
 */
extern int HDK_Device_GetValue(void* pDeviceCtx, HDK_Struct* pStruct, HDK_DeviceValue eValue, HDK_Struct* pInput);
extern int HDK_Device_SetValue(void* pDeviceCtx, HDK_DeviceValue eValue, HDK_Struct* pStruct);
extern int HDK_Device_ValidateValue(void* pDeviceCtx, HDK_DeviceValue eValue, HDK_Struct* pStruct);
extern int HDK_Start_HomeSecurity_Bridge(MBusObj_t *ccsp_bus);
extern int HDK_Stop_HomeSecurity_Bridge(MBusObj_t *ccsp_bus);


#endif /* __HDK_DEVICE_H__ */
