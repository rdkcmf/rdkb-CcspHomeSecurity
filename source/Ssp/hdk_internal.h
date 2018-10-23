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

#ifndef __HDK_INTERNAL_H__
#define __HDK_INTERNAL_H__

#include "hdk_data.h"
#include "hdk_methods.h"

/* HNAP parsing error codes */
typedef enum _HDK_ParseError
{
    HDK_ParseError_OK = 0,
    HDK_ParseError_500_UnknownSOAPAction,
    HDK_ParseError_500_NoInput,
    HDK_ParseError_500_UnexpectedError,
    HDK_ParseError_500_OutOfMemory,
    HDK_ParseError_500_IOError,
    HDK_ParseError_500_XMLInvalid,
    HDK_ParseError_500_XMLUnknownElement,
    HDK_ParseError_500_XMLUnexpectedElement,
    HDK_ParseError_500_XMLInvalidValue,
    HDK_ParseError_500_XMLInvalidResponse,
    HDK_ParseError_500_XMLInvalidRequest
} HDK_ParseError;

/* HNAP XML parsing context */
typedef struct _HDK_ParseContext
{
    void* pXMLParser;
    unsigned short ixElement;             /* Element structure index */
    HDK_Struct* pInputStack[10];         /* Input struct stack */
    unsigned short ixStack;               /* Index one-past the current input structure */
    unsigned int cbValue;                 /* Accumulated value size */
    unsigned int cbValueBuf;              /* Accumulated value buffer size */
    char* pszValue;                       /* Accumulated primitive value text */
    int fHaveInput;                       /* 1 => we have seen the input struct */
    unsigned int cAnyElement;             /* Depth of any (unknown) elements */
    unsigned int cElements;               /* Total element count */
    unsigned int cbTotalValue;            /* Total value size */
    HDK_ParseError parseError;            /* Parse error code */
    void* pDecodeCtx;                     /* Decode callback context */
    int base64State;                      /* Base 64 decode "state" argument */
    int base64Prev;                       /* Base 64 decode "prev" argument */
} HDK_ParseContext;

/* HNAP XML de-serialization */
extern void HDK_Parse_Init(HDK_ParseContext* pParseCtx, HDK_Struct* pStruct, void* pDecodeCtx);
extern void HDK_Parse_Free(HDK_ParseContext* pParseCtx);
extern void HDK_Parse_ElementOpen(HDK_ParseContext* pParseCtx, char* pszNamespace, char* pszNamespaceEnd,
                                  char* pszElement, char* pszElementEnd);
extern void HDK_Parse_ElementClose(HDK_ParseContext* pParseCtx);
extern void HDK_Parse_ElementValue(HDK_ParseContext* pParseCtx, char* pszValue, int cchValue);
extern HDK_ParseError HDK_Parse(void* pDeviceCtx, HDK_Struct* pStruct, unsigned int cbContentLength);

/* HNAP XML serialization */
extern int HDK_WriteBuf(void* pDeviceCtx, int fNoWrite, char* pBuf, int cbBuf);
extern int HDK_WriteBuf_Encode(void* pEncodeCtx, char* pBuf, unsigned int cbBuf);
extern int HDK_Write(void* pDeviceCtx, int fNoWrite, char* pszStr);
extern int HDK_Format(void* pDeviceCtx, int fNoWrite, char* pszStr, ...);
extern int HDK_Serialize(void* pDeviceCtx, int fNoWrite, HDK_Struct* pStruct, HDK_Struct* pInputStruct,
                         int fErrorOutput);
extern int HDK_EncodeToBuffer(void* pEncodeCtx, char* pBuf, unsigned int cbBuf);

/* HDK_WriteBuf encoding context */
typedef struct _HDK_WriteBuf_EncodeContext
{
    void* pDeviceCtx;
    int fNoWrite;
} HDK_WriteBuf_EncodeContext;

/* HNAP request handling */
extern int HDK_Request_RequiresAuth(char* pszSOAPAction, int* pfRequiresAuth);
extern HDK_MethodFn HDK_Request_Init(char* pszSOAPAction, int fAllowHidden,
                                     HDK_Struct* pInput, HDK_Struct* pOutput, HDK_Element* pResultElement);
extern HDK_MethodFn HDK_Request_InitFromElement(HDK_Element inputElement, int fAllowHidden,
                                                HDK_Struct* pInput, HDK_Struct* pOutput, HDK_Element* pResultElement);
extern int HDK_Request_Validate(HDK_Struct* pStruct, HDK_Element topElement, int fErrorOutput);

#endif /* __HDK_INTERNAL_H__ */
