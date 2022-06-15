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

#ifndef __HDK_DATA_H__
#define __HDK_DATA_H__

#include <time.h>

/* HNAP element enumeration */
typedef enum _HDK_Element
{
    HDK_Element__UNKNOWN__ = 0,
    HDK_Element_PN_Active,
    HDK_Element_PN_AddPortMapping,
    HDK_Element_PN_AddPortMappingResponse,
    HDK_Element_PN_AddPortMappingResult,
    HDK_Element_PN_AdminPassword,
    HDK_Element_PN_AutoAdjustDST,
    HDK_Element_PN_AutoReconnect,
    HDK_Element_PN_Channel,
    HDK_Element_PN_ChannelWidth,
    HDK_Element_PN_Channels,
    HDK_Element_PN_ConnectTime,
    HDK_Element_PN_ConnectedClient,
    HDK_Element_PN_ConnectedClients,
    HDK_Element_PN_DHCPReservation,
    HDK_Element_PN_DHCPReservations,
    HDK_Element_PN_DHCPServerEnabled,
    HDK_Element_PN_DNS,
    HDK_Element_PN_DeletePortMapping,
    HDK_Element_PN_DeletePortMappingResponse,
    HDK_Element_PN_DeletePortMappingResult,
    HDK_Element_PN_DeviceName,
    HDK_Element_PN_DomainName,
    HDK_Element_PN_Enabled,
    HDK_Element_PN_Encryption,
    HDK_Element_PN_Encryptions,
    HDK_Element_PN_EthernetPort,
    HDK_Element_PN_ExternalPort,
    HDK_Element_PN_FirmwareVersion,
    HDK_Element_PN_Frequency,
    HDK_Element_PN_Gateway,
    HDK_Element_PN_GetConnectedDevices,
    HDK_Element_PN_GetConnectedDevicesResponse,
    HDK_Element_PN_GetConnectedDevicesResult,
    HDK_Element_PN_GetDeviceSettings,
    HDK_Element_PN_GetDeviceSettingsResponse,
    HDK_Element_PN_GetDeviceSettingsResult,
    HDK_Element_PN_GetPortMappings,
    HDK_Element_PN_GetPortMappingsResponse,
    HDK_Element_PN_GetPortMappingsResult,
    HDK_Element_PN_GetRouterLanSettings2,
    HDK_Element_PN_GetRouterLanSettings2Response,
    HDK_Element_PN_GetRouterLanSettings2Result,
    HDK_Element_PN_GetRouterSettings,
    HDK_Element_PN_GetRouterSettingsResponse,
    HDK_Element_PN_GetRouterSettingsResult,
    HDK_Element_PN_GetWLanRadioSecurity,
    HDK_Element_PN_GetWLanRadioSecurityResponse,
    HDK_Element_PN_GetWLanRadioSecurityResult,
    HDK_Element_PN_GetWLanRadioSettings,
    HDK_Element_PN_GetWLanRadioSettingsResponse,
    HDK_Element_PN_GetWLanRadioSettingsResult,
    HDK_Element_PN_GetWLanRadios,
    HDK_Element_PN_GetWLanRadiosResponse,
    HDK_Element_PN_GetWLanRadiosResult,
    HDK_Element_PN_GetWanSettings,
    HDK_Element_PN_GetWanSettingsResponse,
    HDK_Element_PN_GetWanSettingsResult,
    HDK_Element_PN_IPAddress,
    HDK_Element_PN_IPAddressFirst,
    HDK_Element_PN_IPAddressLast,
    HDK_Element_PN_InternalClient,
    HDK_Element_PN_InternalPort,
    HDK_Element_PN_IsDeviceReady,
    HDK_Element_PN_IsDeviceReadyResponse,
    HDK_Element_PN_IsDeviceReadyResult,
    HDK_Element_PN_Key,
    HDK_Element_PN_KeyRenewal,
    HDK_Element_PN_LeaseTime,
    HDK_Element_PN_Locale,
    HDK_Element_PN_MTU,
    HDK_Element_PN_MacAddress,
    HDK_Element_PN_ManageRemote,
    HDK_Element_PN_ManageWireless,
    HDK_Element_PN_MaxIdleTime,
    HDK_Element_PN_Minutes,
    HDK_Element_PN_Mode,
    HDK_Element_PN_ModelDescription,
    HDK_Element_PN_ModelName,
    HDK_Element_PN_Name,
    HDK_Element_PN_Password,
    HDK_Element_PN_PortMapping,
    HDK_Element_PN_PortMappingDescription,
    HDK_Element_PN_PortMappingProtocol,
    HDK_Element_PN_PortMappings,
    HDK_Element_PN_PortName,
    HDK_Element_PN_PresentationURL,
    HDK_Element_PN_Primary,
    HDK_Element_PN_QoS,
    HDK_Element_PN_RadioID,
    HDK_Element_PN_RadioInfo,
    HDK_Element_PN_RadioInfos,
    HDK_Element_PN_RadiusIP1,
    HDK_Element_PN_RadiusIP2,
    HDK_Element_PN_RadiusPort1,
    HDK_Element_PN_RadiusPort2,
    HDK_Element_PN_RadiusSecret1,
    HDK_Element_PN_RadiusSecret2,
    HDK_Element_PN_Reboot,
    HDK_Element_PN_RebootResponse,
    HDK_Element_PN_RebootResult,
    HDK_Element_PN_RemotePort,
    HDK_Element_PN_RemoteSSL,
    HDK_Element_PN_RouterIPAddress,
    HDK_Element_PN_RouterSubnetMask,
    HDK_Element_PN_SOAPActions,
    HDK_Element_PN_SSID,
    HDK_Element_PN_SSIDBroadcast,
    HDK_Element_PN_SSL,
    HDK_Element_PN_Secondary,
    HDK_Element_PN_SecondaryChannel,
    HDK_Element_PN_SecondaryChannels,
    HDK_Element_PN_SecurityInfo,
    HDK_Element_PN_SecurityType,
    HDK_Element_PN_ServiceName,
    HDK_Element_PN_SetBridgeConnect,
    HDK_Element_PN_SetBridgeConnectResponse,
    HDK_Element_PN_SetBridgeConnectResult,
    HDK_Element_PN_SetDeviceSettings,
    HDK_Element_PN_SetDeviceSettings2,
    HDK_Element_PN_SetDeviceSettings2Response,
    HDK_Element_PN_SetDeviceSettings2Result,
    HDK_Element_PN_SetDeviceSettingsResponse,
    HDK_Element_PN_SetDeviceSettingsResult,
    HDK_Element_PN_SetRouterLanSettings2,
    HDK_Element_PN_SetRouterLanSettings2Response,
    HDK_Element_PN_SetRouterLanSettings2Result,
    HDK_Element_PN_SetRouterSettings,
    HDK_Element_PN_SetRouterSettingsResponse,
    HDK_Element_PN_SetRouterSettingsResult,
    HDK_Element_PN_SetWLanRadioSecurity,
    HDK_Element_PN_SetWLanRadioSecurityResponse,
    HDK_Element_PN_SetWLanRadioSecurityResult,
    HDK_Element_PN_SetWLanRadioSettings,
    HDK_Element_PN_SetWLanRadioSettingsResponse,
    HDK_Element_PN_SetWLanRadioSettingsResult,
    HDK_Element_PN_SubDeviceURLs,
    HDK_Element_PN_SubnetMask,
    HDK_Element_PN_SupportedModes,
    HDK_Element_PN_SupportedSecurity,
    HDK_Element_PN_TaskExtension,
    HDK_Element_PN_Tasks,
    HDK_Element_PN_Tertiary,
    HDK_Element_PN_TimeZone,
    HDK_Element_PN_Type,
    HDK_Element_PN_URL,
    HDK_Element_PN_Username,
    HDK_Element_PN_VendorName,
    HDK_Element_PN_WPSPin,
    HDK_Element_PN_WideChannel,
    HDK_Element_PN_WideChannels,
    HDK_Element_PN_WiredQoS,
    HDK_Element_PN_Wireless,
    HDK_Element_PN_int,
    HDK_Element_PN_string,
    HDK_Element__BODY__,
    HDK_Element__ENVELOPE__,
    HDK_Element__HEADER__
} HDK_Element;

/* HNAP type defines */
#define HDK_TYPE__IPADDRESS__
#define HDK_TYPE__MACADDRESS__
#define HDK_TYPE__RESULT__
#define HDK_TYPE_PN_DEVICETYPE
#define HDK_TYPE_PN_IPPROTOCOL
#define HDK_TYPE_PN_LANCONNECTION
#define HDK_TYPE_PN_TASKEXTTYPE
#define HDK_TYPE_PN_WANTYPE
#define HDK_TYPE_PN_WIFIENCRYPTION
#define HDK_TYPE_PN_WIFIMODE
#define HDK_TYPE_PN_WIFISECURITY
#define HDK_TYPE__BOOL__
#define HDK_TYPE__DATETIME__
#define HDK_TYPE__INT__
#define HDK_TYPE__STRING__

/* HNAP type enumeration */
typedef enum _HDK_Type
{
    HDK_Type__UNKNOWN__ = 0,
    HDK_Type__UNKNOWN_ANY__,
    HDK_Type__STRUCT__,
    HDK_Type__BLANK__,
    HDK_Type__IPADDRESS__,
    HDK_Type__MACADDRESS__,
    HDK_Type__RESULT__,
    HDK_Type_PN_DeviceType,
    HDK_Type_PN_IPProtocol,
    HDK_Type_PN_LANConnection,
    HDK_Type_PN_TaskExtType,
    HDK_Type_PN_WANType,
    HDK_Type_PN_WiFiEncryption,
    HDK_Type_PN_WiFiMode,
    HDK_Type_PN_WiFiSecurity,
    HDK_Type__BOOL__,
    HDK_Type__DATETIME__,
    HDK_Type__INT__,
    HDK_Type__STRING__
} HDK_Type;

/* HNAP generic structure member node */
typedef struct _HDK_Member
{
    HDK_Element element;               /* XML element */
    HDK_Type type;                     /* Element type */
    struct _HDK_Member* pNext;
} HDK_Member;

/* HNAP struct */
typedef struct _HDK_Member_Struct
{
    HDK_Member node;
    HDK_Member* pHead;
    HDK_Member* pTail;
} HDK_Struct;

/* HNAP generic member functions */
extern HDK_Member* HDK_Copy_Member(HDK_Struct* pStructDst, HDK_Element elementDst,
                                   HDK_Member* pMemberSrc, int fAppend);
extern HDK_Member* HDK_Get_Member(HDK_Struct* pStruct, HDK_Element element, HDK_Type type);

/* HNAP struct type */
extern HDK_Struct* HDK_Set_Struct(HDK_Struct* pStruct, HDK_Element element);
extern HDK_Struct* HDK_Set_StructEx(HDK_Struct* pStructDst, HDK_Element element, HDK_Struct* pStruct);
extern HDK_Struct* HDK_Append_Struct(HDK_Struct* pStruct, HDK_Element element);
extern HDK_Struct* HDK_Append_StructEx(HDK_Struct* pStructDst, HDK_Element element, HDK_Struct* pStruct);
extern HDK_Struct* HDK_Get_Struct(HDK_Struct* pStruct, HDK_Element element);
extern HDK_Struct* HDK_Get_StructMember(HDK_Member* pMember);

/* HNAP struct stack initialization/free */
void HDK_Struct_Init(HDK_Struct* pStruct);
void HDK_Struct_Free(HDK_Struct* pStruct);

/* HNAP explicit blank element - use sparingly */
extern HDK_Member* HDK_Set_Blank(HDK_Struct* pStruct, HDK_Element element);
extern HDK_Member* HDK_Append_Blank(HDK_Struct* pStruct, HDK_Element element);

/* HNAP bool type */
extern HDK_Member* HDK_Set_Bool(HDK_Struct* pStruct, HDK_Element element, int fValue);
extern HDK_Member* HDK_Append_Bool(HDK_Struct* pStruct, HDK_Element element, int fValue);
extern int* HDK_Get_Bool(HDK_Struct* pStruct, HDK_Element element);
extern int HDK_Get_BoolEx(HDK_Struct* pStruct, HDK_Element element, int fDefault);
extern int* HDK_Get_BoolMember(HDK_Member* pMember);

/* HNAP int type */
extern HDK_Member* HDK_Set_Int(HDK_Struct* pStruct, HDK_Element element, int iValue);
extern HDK_Member* HDK_Append_Int(HDK_Struct* pStruct, HDK_Element element, int iValue);
extern int* HDK_Get_Int(HDK_Struct* pStruct, HDK_Element element);
extern int HDK_Get_IntEx(HDK_Struct* pStruct, HDK_Element element, int iDefault);
extern int* HDK_Get_IntMember(HDK_Member* pMember);

/* HNAP string type */
extern HDK_Member* HDK_Set_String(HDK_Struct* pStruct, HDK_Element element, char* pszValue);
extern HDK_Member* HDK_Append_String(HDK_Struct* pStruct, HDK_Element element, char* pszValue);
extern char* HDK_Get_String(HDK_Struct* pStruct, HDK_Element element);
extern char* HDK_Get_StringEx(HDK_Struct* pStruct, HDK_Element element, char* pszDefault);
extern char* HDK_Get_StringMember(HDK_Member* pMember);

/* HNAP datetime type */
extern HDK_Member* HDK_Set_DateTime(HDK_Struct* pStruct, HDK_Element element, time_t tValue);
extern HDK_Member* HDK_Append_DateTime(HDK_Struct* pStruct, HDK_Element element, time_t tValue);
extern time_t* HDK_Get_DateTime(HDK_Struct* pStruct, HDK_Element element);
extern time_t HDK_Get_DateTimeEx(HDK_Struct* pStruct, HDK_Element element, time_t tDefault);
extern time_t* HDK_Get_DateTimeMember(HDK_Member* pMember);

/* HNAP datetime helper functions */
time_t HDK_mktime(int year, int mon, int mday, int hour, int min, int sec, int fUTC);   /* Returns -1 for failure */
void HDK_localtime(time_t t, struct tm* ptm);
void HDK_gmtime(time_t t, struct tm* ptm);

/* HNAP IPAddress type */
typedef struct _HDK_IPAddress
{
    unsigned char a;
    unsigned char b;
    unsigned char c;
    unsigned char d;
} HDK_IPAddress;

extern HDK_Member* HDK_Set_IPAddress(HDK_Struct* pStruct, HDK_Element element, HDK_IPAddress* pIPAddress);
extern HDK_Member* HDK_Append_IPAddress(HDK_Struct* pStruct, HDK_Element element, HDK_IPAddress* pIPAddress);
extern HDK_IPAddress* HDK_Get_IPAddress(HDK_Struct* pStruct, HDK_Element element);
extern HDK_IPAddress* HDK_Get_IPAddressEx(HDK_Struct* pStruct, HDK_Element element, HDK_IPAddress* pDefault);
extern HDK_IPAddress* HDK_Get_IPAddressMember(HDK_Member* pMember);

/* HNAP MACAddress type */
typedef struct _HDK_MACAddress
{
    unsigned char a;
    unsigned char b;
    unsigned char c;
    unsigned char d;
    unsigned char e;
    unsigned char f;
} HDK_MACAddress;

extern HDK_Member* HDK_Set_MACAddress(HDK_Struct* pStruct, HDK_Element element, HDK_MACAddress* pMACAddress);
extern HDK_Member* HDK_Append_MACAddress(HDK_Struct* pStruct, HDK_Element element, HDK_MACAddress* pMACAddress);
extern HDK_MACAddress* HDK_Get_MACAddress(HDK_Struct* pStruct, HDK_Element element);
extern HDK_MACAddress* HDK_Get_MACAddressEx(HDK_Struct* pStruct, HDK_Element element, HDK_MACAddress* pDefault);
extern HDK_MACAddress* HDK_Get_MACAddressMember(HDK_Member* pMember);

/* HDK_Type__RESULT__ enumeration type */
typedef enum _HDK_Enum_Result
{
    HDK_Enum_Result__UNKNOWN__ = 0,
    HDK_Enum_Result_OK,
    HDK_Enum_Result_REBOOT,
    HDK_Enum_Result_ERROR,
    HDK_Enum_Result_ERROR_BAD_CHANNEL,
    HDK_Enum_Result_ERROR_BAD_CHANNEL_WIDTH,
    HDK_Enum_Result_ERROR_BAD_IP_ADDRESS,
    HDK_Enum_Result_ERROR_BAD_IP_RANGE,
    HDK_Enum_Result_ERROR_BAD_MODE,
    HDK_Enum_Result_ERROR_BAD_RADIOID,
    HDK_Enum_Result_ERROR_BAD_RADIUS_VALUES,
    HDK_Enum_Result_ERROR_BAD_RESERVATION,
    HDK_Enum_Result_ERROR_BAD_SECONDARY_CHANNEL,
    HDK_Enum_Result_ERROR_BAD_SSID,
    HDK_Enum_Result_ERROR_BAD_SUBNET,
    HDK_Enum_Result_ERROR_DOMAIN_NOT_SUPPORTED,
    HDK_Enum_Result_ERROR_ENCRYPTION_NOT_SUPPORTED,
    HDK_Enum_Result_ERROR_ILLEGAL_KEY_VALUE,
    HDK_Enum_Result_ERROR_KEY_RENEWAL_BAD_VALUE,
    HDK_Enum_Result_ERROR_QOS_NOT_SUPPORTED,
    HDK_Enum_Result_ERROR_REMOTE_MANAGE_DEFAULT_PASSWORD,
    HDK_Enum_Result_ERROR_REMOTE_MANAGE_MUST_BE_SSL,
    HDK_Enum_Result_ERROR_REMOTE_MANAGE_NOT_SUPPORTED,
    HDK_Enum_Result_ERROR_REMOTE_SSL_NEEDS_SSL,
    HDK_Enum_Result_ERROR_REMOTE_SSL_NOT_SUPPORTED,
    HDK_Enum_Result_ERROR_RESERVATIONS_NOT_SUPPORTED,
    HDK_Enum_Result_ERROR_TIMEZONE_NOT_SUPPORTED,
    HDK_Enum_Result_ERROR_TYPE_NOT_SUPPORTED,
    HDK_Enum_Result_ERROR_USERNAME_NOT_SUPPORTED
} HDK_Enum_Result;

#define HDK_SUCCEEDED(result) ((result) > HDK_Enum_Result__UNKNOWN__ && (result) < HDK_Enum_Result_ERROR)
#define HDK_FAILED(result) (!HDK_SUCCEEDED(result))

extern HDK_Member* HDK_Set_Result(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_Result eValue);
extern HDK_Member* HDK_Append_Result(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_Result eValue);
extern HDK_Enum_Result* HDK_Get_Result(HDK_Struct* pStruct, HDK_Element element);
extern HDK_Enum_Result HDK_Get_ResultEx(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_Result eDefault);
extern HDK_Enum_Result* HDK_Get_ResultMember(HDK_Member* pMember);

/* HDK_Type_PN_DeviceType enumeration type */
typedef enum _HDK_Enum_PN_DeviceType
{
    HDK_Enum_PN_DeviceType__UNKNOWN__ = 0,
    HDK_Enum_PN_DeviceType_Computer,
    HDK_Enum_PN_DeviceType_ComputerServer,
    HDK_Enum_PN_DeviceType_WorkstationComputer,
    HDK_Enum_PN_DeviceType_LaptopComputer,
    HDK_Enum_PN_DeviceType_Gateway,
    HDK_Enum_PN_DeviceType_GatewayWithWiFi,
    HDK_Enum_PN_DeviceType_DigitalDVR,
    HDK_Enum_PN_DeviceType_DigitalJukebox,
    HDK_Enum_PN_DeviceType_MediaAdapter,
    HDK_Enum_PN_DeviceType_NetworkCamera,
    HDK_Enum_PN_DeviceType_NetworkDevice,
    HDK_Enum_PN_DeviceType_NetworkDrive,
    HDK_Enum_PN_DeviceType_NetworkGameConsole,
    HDK_Enum_PN_DeviceType_NetworkPDA,
    HDK_Enum_PN_DeviceType_NetworkPrinter,
    HDK_Enum_PN_DeviceType_NetworkPrintServer,
    HDK_Enum_PN_DeviceType_PhotoFrame,
    HDK_Enum_PN_DeviceType_VOIPDevice,
    HDK_Enum_PN_DeviceType_WiFiAccessPoint,
    HDK_Enum_PN_DeviceType_SetTopBox,
    HDK_Enum_PN_DeviceType_WiFiBridge
} HDK_Enum_PN_DeviceType;

extern HDK_Member* HDK_Set_PN_DeviceType(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_DeviceType eValue);
extern HDK_Member* HDK_Append_PN_DeviceType(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_DeviceType eValue);
extern HDK_Enum_PN_DeviceType* HDK_Get_PN_DeviceType(HDK_Struct* pStruct, HDK_Element element);
extern HDK_Enum_PN_DeviceType HDK_Get_PN_DeviceTypeEx(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_DeviceType eDefault);
extern HDK_Enum_PN_DeviceType* HDK_Get_PN_DeviceTypeMember(HDK_Member* pMember);

/* HDK_Type_PN_IPProtocol enumeration type */
typedef enum _HDK_Enum_PN_IPProtocol
{
    HDK_Enum_PN_IPProtocol__UNKNOWN__ = 0,
    HDK_Enum_PN_IPProtocol_UDP,
    HDK_Enum_PN_IPProtocol_TCP
} HDK_Enum_PN_IPProtocol;

extern HDK_Member* HDK_Set_PN_IPProtocol(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_IPProtocol eValue);
extern HDK_Member* HDK_Append_PN_IPProtocol(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_IPProtocol eValue);
extern HDK_Enum_PN_IPProtocol* HDK_Get_PN_IPProtocol(HDK_Struct* pStruct, HDK_Element element);
extern HDK_Enum_PN_IPProtocol HDK_Get_PN_IPProtocolEx(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_IPProtocol eDefault);
extern HDK_Enum_PN_IPProtocol* HDK_Get_PN_IPProtocolMember(HDK_Member* pMember);

/* HDK_Type_PN_LANConnection enumeration type */
typedef enum _HDK_Enum_PN_LANConnection
{
    HDK_Enum_PN_LANConnection__UNKNOWN__ = 0,
    HDK_Enum_PN_LANConnection_LAN,
    HDK_Enum_PN_LANConnection_WLAN_802_11a,
    HDK_Enum_PN_LANConnection_WLAN_802_11b,
    HDK_Enum_PN_LANConnection_WLAN_802_11g,
    HDK_Enum_PN_LANConnection_WLAN_802_11n,
    HDK_Enum_PN_LANConnection_WAN
} HDK_Enum_PN_LANConnection;

extern HDK_Member* HDK_Set_PN_LANConnection(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_LANConnection eValue);
extern HDK_Member* HDK_Append_PN_LANConnection(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_LANConnection eValue);
extern HDK_Enum_PN_LANConnection* HDK_Get_PN_LANConnection(HDK_Struct* pStruct, HDK_Element element);
extern HDK_Enum_PN_LANConnection HDK_Get_PN_LANConnectionEx(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_LANConnection eDefault);
extern HDK_Enum_PN_LANConnection* HDK_Get_PN_LANConnectionMember(HDK_Member* pMember);

/* HDK_Type_PN_TaskExtType enumeration type */
typedef enum _HDK_Enum_PN_TaskExtType
{
    HDK_Enum_PN_TaskExtType__UNKNOWN__ = 0,
    HDK_Enum_PN_TaskExtType_Browser,
    HDK_Enum_PN_TaskExtType_MessageBox,
    HDK_Enum_PN_TaskExtType_PUI,
    HDK_Enum_PN_TaskExtType_Silent
} HDK_Enum_PN_TaskExtType;

extern HDK_Member* HDK_Set_PN_TaskExtType(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_TaskExtType eValue);
extern HDK_Member* HDK_Append_PN_TaskExtType(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_TaskExtType eValue);
extern HDK_Enum_PN_TaskExtType* HDK_Get_PN_TaskExtType(HDK_Struct* pStruct, HDK_Element element);
extern HDK_Enum_PN_TaskExtType HDK_Get_PN_TaskExtTypeEx(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_TaskExtType eDefault);
extern HDK_Enum_PN_TaskExtType* HDK_Get_PN_TaskExtTypeMember(HDK_Member* pMember);

/* HDK_Type_PN_WANType enumeration type */
typedef enum _HDK_Enum_PN_WANType
{
    HDK_Enum_PN_WANType__UNKNOWN__ = 0,
    HDK_Enum_PN_WANType_BigPond,
    HDK_Enum_PN_WANType_DHCP,
    HDK_Enum_PN_WANType_DHCPPPPoE,
    HDK_Enum_PN_WANType_DynamicL2TP,
    HDK_Enum_PN_WANType_DynamicPPTP,
    HDK_Enum_PN_WANType_Static,
    HDK_Enum_PN_WANType_StaticL2TP,
    HDK_Enum_PN_WANType_StaticPPPoE,
    HDK_Enum_PN_WANType_StaticPPTP,
    HDK_Enum_PN_WANType_BridgedOnly,
    HDK_Enum_PN_WANType_Static1483Bridged,
    HDK_Enum_PN_WANType_Dynamic1483Bridged,
    HDK_Enum_PN_WANType_Static1483Routed,
    HDK_Enum_PN_WANType_StaticPPPOA,
    HDK_Enum_PN_WANType_DynamicPPPOA,
    HDK_Enum_PN_WANType_StaticIPOA,
    HDK_Enum_PN_WANType_UNKNOWN,
    HDK_Enum_PN_WANType_DETECTING
} HDK_Enum_PN_WANType;

extern HDK_Member* HDK_Set_PN_WANType(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_WANType eValue);
extern HDK_Member* HDK_Append_PN_WANType(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_WANType eValue);
extern HDK_Enum_PN_WANType* HDK_Get_PN_WANType(HDK_Struct* pStruct, HDK_Element element);
extern HDK_Enum_PN_WANType HDK_Get_PN_WANTypeEx(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_WANType eDefault);
extern HDK_Enum_PN_WANType* HDK_Get_PN_WANTypeMember(HDK_Member* pMember);

/* HDK_Type_PN_WiFiEncryption enumeration type */
typedef enum _HDK_Enum_PN_WiFiEncryption
{
    HDK_Enum_PN_WiFiEncryption__UNKNOWN__ = 0,
    HDK_Enum_PN_WiFiEncryption_WEP_64,
    HDK_Enum_PN_WiFiEncryption_WEP_128,
    HDK_Enum_PN_WiFiEncryption_AES,
    HDK_Enum_PN_WiFiEncryption_TKIP,
    HDK_Enum_PN_WiFiEncryption_TKIPORAES,
    HDK_Enum_PN_WiFiEncryption_
} HDK_Enum_PN_WiFiEncryption;

extern HDK_Member* HDK_Set_PN_WiFiEncryption(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_WiFiEncryption eValue);
extern HDK_Member* HDK_Append_PN_WiFiEncryption(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_WiFiEncryption eValue);
extern HDK_Enum_PN_WiFiEncryption* HDK_Get_PN_WiFiEncryption(HDK_Struct* pStruct, HDK_Element element);
extern HDK_Enum_PN_WiFiEncryption HDK_Get_PN_WiFiEncryptionEx(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_WiFiEncryption eDefault);
extern HDK_Enum_PN_WiFiEncryption* HDK_Get_PN_WiFiEncryptionMember(HDK_Member* pMember);

/* HDK_Type_PN_WiFiMode enumeration type */
typedef enum _HDK_Enum_PN_WiFiMode
{
    HDK_Enum_PN_WiFiMode__UNKNOWN__ = 0,
    HDK_Enum_PN_WiFiMode_802_11a,
    HDK_Enum_PN_WiFiMode_802_11b,
    HDK_Enum_PN_WiFiMode_802_11g,
    HDK_Enum_PN_WiFiMode_802_11n,
    HDK_Enum_PN_WiFiMode_802_11bg,
    HDK_Enum_PN_WiFiMode_802_11bn,
    HDK_Enum_PN_WiFiMode_802_11bgn,
    HDK_Enum_PN_WiFiMode_802_11gn,
    HDK_Enum_PN_WiFiMode_802_11an,
    HDK_Enum_PN_WiFiMode_
} HDK_Enum_PN_WiFiMode;

extern HDK_Member* HDK_Set_PN_WiFiMode(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_WiFiMode eValue);
extern HDK_Member* HDK_Append_PN_WiFiMode(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_WiFiMode eValue);
extern HDK_Enum_PN_WiFiMode* HDK_Get_PN_WiFiMode(HDK_Struct* pStruct, HDK_Element element);
extern HDK_Enum_PN_WiFiMode HDK_Get_PN_WiFiModeEx(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_WiFiMode eDefault);
extern HDK_Enum_PN_WiFiMode* HDK_Get_PN_WiFiModeMember(HDK_Member* pMember);

/* HDK_Type_PN_WiFiSecurity enumeration type */
typedef enum _HDK_Enum_PN_WiFiSecurity
{
    HDK_Enum_PN_WiFiSecurity__UNKNOWN__ = 0,
    HDK_Enum_PN_WiFiSecurity_NONE,
    HDK_Enum_PN_WiFiSecurity_WEP_64,
    HDK_Enum_PN_WiFiSecurity_WEP_128,
    HDK_Enum_PN_WiFiSecurity_WPA_Personal,
    HDK_Enum_PN_WiFiSecurity_WPA2_Personal,
    HDK_Enum_PN_WiFiSecurity_WPA_WPA2_Personal,
    HDK_Enum_PN_WiFiSecurity_WPA_Enterprise,
    HDK_Enum_PN_WiFiSecurity_WPA2_Enterprise,
    HDK_Enum_PN_WiFiSecurity_WPA_WPA2_Enterprise,
    HDK_Enum_PN_WiFiSecurity_
} HDK_Enum_PN_WiFiSecurity;

extern HDK_Member* HDK_Set_PN_WiFiSecurity(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_WiFiSecurity eValue);
extern HDK_Member* HDK_Append_PN_WiFiSecurity(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_WiFiSecurity eValue);
extern HDK_Enum_PN_WiFiSecurity* HDK_Get_PN_WiFiSecurity(HDK_Struct* pStruct, HDK_Element element);
extern HDK_Enum_PN_WiFiSecurity HDK_Get_PN_WiFiSecurityEx(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_WiFiSecurity eDefault);
extern HDK_Enum_PN_WiFiSecurity* HDK_Get_PN_WiFiSecurityMember(HDK_Member* pMember);

/* Helper function for GetDeviceSettings */
extern void HDK_Set_PN_GetDeviceSettings_SOAPActions(HDK_Struct* pStruct);

#endif /* __HDK_DATA_H__ */
