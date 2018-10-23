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

#define HDK_LIBXML2

#include "hdk.h"
#include "hdk_methods.h"
#include "hdk_internal.h"
#include "hdk_interface.h"

#ifdef HDK_LIBXML2
#include <libxml/parser.h>
#else
#include <expat.h>
#endif

#include <stdarg.h>
#include <stdio.h>
#include <string.h>


/*
 * I/O helper functions
 */

/* Stream buffer-write function */
int HDK_WriteBuf(void* pDeviceCtx, int fNoWrite, char* pBuf, int cbBuf)
{
    if (!fNoWrite)
    {
        HDK_Device_Write(pDeviceCtx, pBuf, cbBuf);
    }
    return cbBuf;
}

/* Encoded write function */
int HDK_WriteBuf_Encode(void* pEncodeCtx, char* pBuf, unsigned int cbBuf)
{
    HDK_WriteBuf_EncodeContext* pCtx = (HDK_WriteBuf_EncodeContext*)pEncodeCtx;
    return HDK_WriteBuf(pCtx->pDeviceCtx, pCtx->fNoWrite, pBuf, cbBuf);
}

/* Stream null-terminated string write function */
int HDK_Write(void* pDeviceCtx, int fNoWrite, char* pszStr)
{
    int cbStr = strlen(pszStr);
    if (!fNoWrite)
    {
        HDK_Device_Write(pDeviceCtx, pszStr, cbStr);
    }
    return cbStr;
}

/* Stream formatted write function */
int HDK_Format(void* pDeviceCtx, int fNoWrite, char* pszStr, ...)
{
    va_list args;
    int cbBuf;
    char szBuf[128];

    va_start(args, pszStr);

    /* Format the string to the buffer */
    cbBuf = vsprintf(szBuf, pszStr, args);
    if (cbBuf < 0)
    {
        cbBuf = 0;
    }
    else if (!fNoWrite)
    {
        HDK_Device_Write(pDeviceCtx, szBuf, cbBuf);
    }

    va_end(args);

    return cbBuf;
}


/*
 * XML parsing
 */

#ifdef HDK_LIBXML2
static void ElementStartHandler(void* pDeviceCtx,
                                const xmlChar* pszElement,
                                const xmlChar* pszPrefix,
                                const xmlChar* pszNamespace,
                                int nNamespaces,
                                const xmlChar** ppszNamespaces,
                                int nAttributes,
                                int nDefaulted,
                                const xmlChar** ppszAttributes)
#else
static void ElementStartHandler(void* pDeviceCtx,
                                const char* pszElement,
                                const char** ppszAttributes)
#endif
{
    HDK_ParseContext* pParseCtx;
    char* pszNamespaceEnd = 0;

#ifdef HDK_LIBXML2
    /* Unused parameters */
    (void)pszPrefix;
    (void)nNamespaces;
    (void)ppszNamespaces;
    (void)nAttributes;
    (void)nDefaulted;
    (void)ppszAttributes;
#else
    char* pszNamespace;

    /* Unused parameters */
    (void)ppszAttributes;

    /* Locate the end of the namespace */
    pszNamespace = (char*)pszElement;
    for (pszNamespaceEnd = (char*)pszElement; *pszNamespaceEnd && *pszNamespaceEnd != '!'; ++pszNamespaceEnd) {}
    if (*pszNamespaceEnd)
    {
        pszElement = pszNamespaceEnd + 1;
    }
    else
    {
        pszNamespace = 0;
        pszNamespaceEnd = 0;
    }
#endif

    /* Handle the element open */
    pParseCtx = (HDK_ParseContext*)pDeviceCtx;
    HDK_Parse_ElementOpen(pParseCtx, (char*)pszNamespace, pszNamespaceEnd, (char*)pszElement, 0);

    /* Stop the parser on error */
    if (pParseCtx->parseError != HDK_ParseError_OK)
    {
#ifdef HDK_LIBXML2
        xmlStopParser((xmlParserCtxt*)pParseCtx->pXMLParser);
#else
        XML_StopParser((struct XML_ParserStruct*)pParseCtx->pXMLParser, XML_FALSE);
#endif
    }
}

#ifdef HDK_LIBXML2
static void ElementEndHandler(void* pDeviceCtx,
                              const xmlChar* pszElement,
                              const xmlChar* pszPrefix,
                              const xmlChar* pszNamespace)
#else
static void ElementEndHandler(void* pDeviceCtx,
                              const char* pszElement)
#endif
{
    HDK_ParseContext* pParseCtx;

#ifdef HDK_LIBXML2
    /* Unused parameters */
    (void)pszElement;
    (void)pszPrefix;
    (void)pszNamespace;
#else
    /* Unused parameters */
    (void)pszElement;
#endif

    /* Handle the element close */
    pParseCtx = (HDK_ParseContext*)pDeviceCtx;
    HDK_Parse_ElementClose(pParseCtx);

    /* Stop the parser on error */
    if (pParseCtx->parseError != HDK_ParseError_OK)
    {
#ifdef HDK_LIBXML2
        xmlStopParser((xmlParserCtxt*)pParseCtx->pXMLParser);
#else
        XML_StopParser((struct XML_ParserStruct*)pParseCtx->pXMLParser, XML_FALSE);
#endif
    }
}

#ifdef HDK_LIBXML2
static void ElementValueHandler(void* pDeviceCtx,
                                const xmlChar* pValue,
                                int cbValue)
#else
static void ElementValueHandler(void* pDeviceCtx,
                                const char* pValue,
                                int cbValue)
#endif
{
    /* Handle the element value (part) */
    HDK_ParseContext* pParseCtx = (HDK_ParseContext*)pDeviceCtx;
    HDK_Parse_ElementValue(pParseCtx, (char*)pValue, cbValue);

    /* Stop the parser on error */
    if (pParseCtx->parseError != HDK_ParseError_OK)
    {
#ifdef HDK_LIBXML2
        xmlStopParser((xmlParserCtxt*)pParseCtx->pXMLParser);
#else
        XML_StopParser((struct XML_ParserStruct*)pParseCtx->pXMLParser, XML_FALSE);
#endif
    }
}

/* Parse the HNAP request */
HDK_ParseError HDK_Parse(void* pDeviceCtx, HDK_Struct* pStruct, unsigned int cbContentLength)
{
    HDK_ParseError parseError = HDK_ParseError_OK;
    HDK_ParseContext parseContext;
    char buf[1024];
#ifdef HDK_LIBXML2
    xmlSAXHandler saxHandler;
    xmlParserCtxtPtr pXMLParser;
#else
    XML_Parser pXMLParser;
#endif

    /* "New" the parse context */
    HDK_Parse_Init(&parseContext, pStruct, pDeviceCtx);

    /* Create the XML parser */
#ifdef HDK_LIBXML2
    memset(&saxHandler, 0, sizeof(saxHandler));
    saxHandler.initialized = XML_SAX2_MAGIC;
    saxHandler.startElementNs = ElementStartHandler;
    saxHandler.endElementNs = ElementEndHandler;
    saxHandler.characters = ElementValueHandler;
    pXMLParser = xmlCreatePushParserCtxt(&saxHandler, &parseContext, 0, 0, 0);
#else
    pXMLParser = XML_ParserCreateNS(NULL, '!');
#endif
    if (pXMLParser)
    {
        parseContext.pXMLParser = pXMLParser;
#ifndef HDK_LIBXML2
        XML_SetUserData(pXMLParser, &parseContext);
        XML_SetElementHandler(pXMLParser, ElementStartHandler, ElementEndHandler);
        XML_SetCharacterDataHandler(pXMLParser, ElementValueHandler);
#endif
    }
    else
    {
        /* Error - out of memory */
        parseError = HDK_ParseError_500_OutOfMemory;
    }

    /* Parse the XML content */
    if (parseError == HDK_ParseError_OK)
    {
        /* Compute the bytes remaining to read of the content */
        unsigned int cbRemaining = cbContentLength;
        while (cbRemaining > 0)
        {
#ifdef HDK_LIBXML2
            xmlParserErrors xmlErrorCode;
#endif

            /* Read data... */
            int cbRead = (cbRemaining < sizeof(buf) ? cbRemaining : sizeof(buf));
            cbRead = HDK_Device_Read(pDeviceCtx, buf, cbRead);
            if (cbRead <= 0)
            {
                parseError = HDK_ParseError_500_IOError;
                break;
            }
            cbRemaining -= cbRead;

            /* Parse the XML in the buffer */
#ifdef HDK_LIBXML2
            if ((xmlErrorCode = (xmlParserErrors)xmlParseChunk(pXMLParser, buf, cbRead, cbRemaining <= 0)) != XML_ERR_OK)
#else
            if (!XML_Parse(pXMLParser, buf, cbRead, cbRemaining <= 0))
#endif
            {
#ifndef HDK_LIBXML2
                enum XML_Error xmlErrorCode = XML_GetErrorCode(pXMLParser);
#endif

                /* Error - XML error */
                switch (xmlErrorCode)
                {
#ifdef HDK_LIBXML2
                    case XML_ERR_NO_MEMORY:
#else
                    case XML_ERROR_NO_MEMORY:
#endif
                        parseError = HDK_ParseError_500_OutOfMemory;
                        break;

                    default:
                        if (parseContext.parseError != HDK_ParseError_OK)
                        {
                            parseError = parseContext.parseError;
                        }
                        else
                        {
                            parseError = HDK_ParseError_500_XMLInvalid;
                        }
                        break;
                }
                break;
            }

            /* Was parsing stopped? */
            else if (parseContext.parseError != HDK_ParseError_OK)
            {
                parseError = parseContext.parseError;
                break;
            }
        }
    }

    /* No input? */
    if (parseError == HDK_ParseError_OK && !parseContext.fHaveInput)
    {
        parseError = HDK_ParseError_500_NoInput;
    }

    /* Free the XML parser */
    if (pXMLParser)
    {
#ifdef HDK_LIBXML2
        xmlFreeParserCtxt(pXMLParser);
        xmlCleanupParser();
#else
        XML_ParserFree(pXMLParser);
#endif
    }

    /* Free the parse context */
    HDK_Parse_Free(&parseContext);

    return parseError;
}


/*
 * Request handling
 */

/* Send an HNAP response */
static void SendHNAPResponse(void* pDeviceCtx, HDK_Struct* pStruct, HDK_Struct* pInputStruct, int fErrorOutput)
{
    int iContentLength = 0;
    int fNoWrite;

#ifdef HDK_UNITTEST
    /* Output the input struct in a unittest build */
    if (pInputStruct)
    {
        HDK_Write(pDeviceCtx, 0, "*** BEGIN REQUEST ***\n\n");
        iContentLength += HDK_Serialize(pDeviceCtx, 0, pInputStruct, 0, 0);
        HDK_Write(pDeviceCtx, 0, "\n*** END REQUEST ***\n\n");
    }
#endif

    /* Write the response - the first iteration is to compute content length only */
    for (fNoWrite = 1; fNoWrite >= 0; fNoWrite--)
    {
        /* Response headers */
        if (!fNoWrite)
        {
            HDK_Write(pDeviceCtx, fNoWrite,
#ifndef HDK_CGI
                       "HTTP/1.1 "
#endif
                       "200 OK\r\n"
                       "Content-Type: text/xml; charset=utf-8\r\n"
                       "Connection: close\r\n"
                       "Content-Length: ");
            HDK_Format(pDeviceCtx, fNoWrite,
                        "%d\r\n\r\n", iContentLength);
        }

        /* Response content (and compute content length */
        iContentLength =  HDK_Write(pDeviceCtx, fNoWrite,
                                     "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                                     "<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">\n"
                                     "<soap:Body>\n");
        iContentLength += HDK_Serialize(pDeviceCtx, fNoWrite, pStruct, pInputStruct, fErrorOutput);
        iContentLength += HDK_Write(pDeviceCtx, fNoWrite,
                                     "</soap:Body>\n"
                                     "</soap:Envelope>\n");
    }
}

/* Send a SOAP error response */
static void SendErrorResponse(void* pDeviceCtx, HDK_ParseError parseError)
{
    char* pszCode;
    char* pszText;
    int iContentLength = 0;
    int fNoWrite;

    /* Determine the SOAP fault text */
    pszCode = "Client";
    switch (parseError)
    {
        case HDK_ParseError_500_UnknownSOAPAction:
            pszText = "Unknown SOAP action";
            break;
        case HDK_ParseError_500_NoInput:
            pszText = "No input";
            break;
        case HDK_ParseError_500_UnexpectedError:
            pszCode = "Server";
            pszText = "Unexpected error";
            break;
        case HDK_ParseError_500_OutOfMemory:
            pszCode = "Server";
            pszText = "Out of memory";
            break;
        case HDK_ParseError_500_IOError:
            pszCode = "Server";
            pszText = "I/O Error";
            break;
        case HDK_ParseError_500_XMLInvalid:
            pszText = "Invalid XML";
            break;
        case HDK_ParseError_500_XMLUnknownElement:
            pszText = "Unknown XML element";
            break;
        case HDK_ParseError_500_XMLUnexpectedElement:
            pszText = "Unexpected XML element";
            break;
        case HDK_ParseError_500_XMLInvalidValue:
            pszText = "Invalid XML value text";
            break;
        case HDK_ParseError_500_XMLInvalidResponse:
            pszCode = "Server";
            pszText = "Invalid response XML";
            break;
        case HDK_ParseError_500_XMLInvalidRequest:
            pszText = "Invalid request XML";
            break;
        default:
            pszText = "";
            break;
    }

    /* Write the response content - the first iteration is to compute content length only */
    for (fNoWrite = 1; fNoWrite >= 0; fNoWrite--)
    {
        /* Response headers */
        if (!fNoWrite)
        {
            HDK_Write(pDeviceCtx, fNoWrite,
#ifndef HDK_CGI
                       "HTTP/1.1 "
#endif
                       "500 Internal Server Error\r\n"
                       "Content-Type: text/xml; charset=utf-8\r\n"
                       "Connection: close\r\n"
                       "Content-Length: ");
            HDK_Format(pDeviceCtx, fNoWrite,
                        "%d\r\n\r\n", iContentLength);
        }

        /* Response content (and compute content length */
        iContentLength += HDK_Write(pDeviceCtx, fNoWrite,
                                     "<?xml version=\"1.0\" ?>\n"
                                     "<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">\n"
                                     "<soap:Body>\n"
                                     "<soap:Fault>\n"
                                     "<faultcode>soap:");
        iContentLength += HDK_Write(pDeviceCtx, fNoWrite, pszCode);
        iContentLength += HDK_Write(pDeviceCtx, fNoWrite,
                                     "</faultcode>\n"
                                     "<faultstring>");
        iContentLength += HDK_Write(pDeviceCtx, fNoWrite, pszText);
        iContentLength += HDK_Write(pDeviceCtx, fNoWrite,
                                     "</faultstring>\n"
                                     "</soap:Fault>\n"
                                     "</soap:Body>\n"
                                     "</soap:Envelope>\n");
    }
}


/*
 * hnap.h implementation
 */

/* Is this an HNAP request? */
int HDK_IsHNAPRequest(char* pszPath)
{
    return (strcmp(pszPath, "/HNAP1") == 0 ||
            strcmp(pszPath, "/HNAP1/") == 0);
}

/* Does this HNAP request require authentication? */
int HDK_RequiresAuth(int fGet, char* pszSoapAction)
{
    int fRequiresAuth;

    /* All GET requests are unauthenticated */
    if (fGet)
    {
        return 0;
    }

    /* Authentication required? */
    if (pszSoapAction && HDK_Request_RequiresAuth(pszSoapAction, &fRequiresAuth))
    {
        return fRequiresAuth;
    }
    else
    {
        return 1;
    }
}

/* Handle an HNAP request. */
HDK_Enum_Result HDK_HandleRequest(void* pDeviceCtx, int fGet, unsigned int cbContentLength, char* pszSoapAction)
{
    HDK_ParseError parseError = HDK_ParseError_OK;
    HDK_Struct sInput;
    HDK_Struct sOutput;
    HDK_Enum_Result result = HDK_Enum_Result_ERROR;
    HDK_Element resultElement = HDK_Element__UNKNOWN__;
    HDK_Element responseElement = HDK_Element__UNKNOWN__;
    HDK_MethodFn pMethod = 0;

    /* Init the input and output structs */
    HDK_Struct_Init(&sInput);
    HDK_Struct_Init(&sOutput);

    /* Get the HNAP action */
    if (0 && fGet)
    {
        pMethod = HDK_Request_Init("http://purenetworks.com/HNAP1/GetDeviceSettings", /*fAllowHidden=*/ 0,
                                   &sInput, &sOutput, &resultElement);
        responseElement = sOutput.node.element;
    }
    else if (pszSoapAction)
    {
        pMethod = HDK_Request_Init(pszSoapAction, /*fAllowHidden=*/ 0, &sInput, &sOutput, &resultElement);
        responseElement = sOutput.node.element;
    }
    if (!pMethod)
    {
        /* Unknown SOAPAction... */
        parseError = HDK_ParseError_500_UnknownSOAPAction;
    }

    /* Parse the POST data */
    if (parseError == HDK_ParseError_OK && !fGet)
    {
        parseError = HDK_Parse(pDeviceCtx, &sInput, cbContentLength);
    }

    /* Validate the input struct */
    if (parseError == HDK_ParseError_OK &&
        !HDK_Request_Validate(&sInput, sInput.node.element, 0))
    {
        parseError = HDK_ParseError_500_XMLInvalidRequest;
    }

    /* Handle request */
    if (parseError == HDK_ParseError_OK)
    {
        /* Handle the callback */
        pMethod(pDeviceCtx, &sInput, &sOutput);
        result = HDK_Get_ResultEx(&sOutput, resultElement, HDK_Enum_Result_OK);

        /* Prepare the device for the response */
        {
            HDK_Enum_Result resultOrig = result;
            HDK_Device_PrepareWrite(pDeviceCtx, &result);
            if (result != resultOrig)
            {
                HDK_Set_Result(&sOutput, resultElement, result);
            }
        }

        /* Validate the output struct */
        if (!HDK_Request_Validate(&sOutput, responseElement, HDK_FAILED(result)))
        {
            /* Send the error response */
            SendErrorResponse(pDeviceCtx, HDK_ParseError_500_XMLInvalidResponse);
        }
        else
        {
            /* Send the response */
            SendHNAPResponse(pDeviceCtx, &sOutput, &sInput, HDK_FAILED(result));
        }
    }
    else
    {
        /* Prepare the device for the error response */
        HDK_Enum_Result resultTmp = result;
        HDK_Device_PrepareWrite(pDeviceCtx, &resultTmp);

        /* Send the error response */
        SendErrorResponse(pDeviceCtx, parseError);
    }

    /* Free the input and output structs */
    HDK_Struct_Free(&sInput);
    HDK_Struct_Free(&sOutput);

    return result;
}
