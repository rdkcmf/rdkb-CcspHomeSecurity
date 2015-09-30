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
 * Copyright (c) 2008-2009 Cisco Systems, Inc. All rights reserved.
 *
 * Cisco Systems, Inc. retains all right, title and interest (including all
 * intellectual property rights) in and to this computer program, which is
 * protected by applicable intellectual property laws.  Unless you have obtained
 * a separate written license from Cisco Systems, Inc., you are not authorized
 * to utilize all or a part of this computer program for any purpose (including
 * reproduction, distribution, modification, and compilation into object code),
 * and you must immediately destroy or return to Cisco Systems, Inc. all copies
 * of this computer program.  If you are licensed by Cisco Systems, Inc., your
 * rights to utilize this computer program are limited by the terms of that
 * license.  To obtain a license, please contact Cisco Systems, Inc.
 *
 * This computer program contains trade secrets owned by Cisco Systems, Inc.
 * and, unless unauthorized by Cisco Systems, Inc. in writing, you agree to
 * maintain the confidentiality of this computer program and related information
 * and to not disclose this computer program and related information to any
 * other person or entity.
 *
 * THIS COMPUTER PROGRAM IS PROVIDED AS IS WITHOUT ANY WARRANTIES, AND CISCO
 * SYSTEMS, INC. EXPRESSLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED,
 * INCLUDING THE WARRANTIES OF MERCHANTIBILITY, FITNESS FOR A PARTICULAR
 * PURPOSE, TITLE, AND NONINFRINGEMENT.
 */

/*
 * hnapd.c - Stand-alone HNAP server
 */

#include "hdk.h"
#include "hdk_context.h"
#include "hdk_encode.h"
#include "chs_log.h"
#include "syscfg/syscfg.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <signal.h>


/*
 * HTTP_ParseHeaders - Parse the HTTP request headers
 */
static void HTTP_ParseHeaders(FILE* pfh, char* pszBuf, unsigned int cbBuf, int* pGet, char** ppszLocation,
                              unsigned int* pContentLength, char** ppszSoapAction, char** ppszAuth)
{
    char* pBuf;
    char* pBufEnd;
    int cNewline;
    int fHaveContent;
    char* pszMethod = 0;
    char* pszLocation = 0;
    char* pszContentLength = 0;
    char* pszSoapAction = 0;
    char* pszAuth = 0;

    /* Search for the blank line preceding the content */
    pBufEnd = pszBuf + cbBuf;
    cNewline = 0;
    fHaveContent = 0;
    for (pBuf = pszBuf; pBuf < pBufEnd; ++pBuf)
    {
        /* Read 1 byte into the buffer */
        if (fread(pBuf, 1, 1, pfh) <= 0)
        {
            break;
        }

        /* Count consecutive newlines */
        if (*pBuf == '\n')
        {
            if (++cNewline == 2)
            {
                fHaveContent = 1;
                break;
            }
        }
        else if (cNewline > 0 && *pBuf != '\r')
        {
            cNewline = 0;
        }
    }

    /* Parse HTTP headers */
    if (fHaveContent)
    {
        char* pszName = 0;
        char* pszValue = 0;
        pBufEnd = pBuf;

        /* Find the HTTP method and location */
        for (pBuf = pszBuf; pBuf < pBufEnd; ++pBuf)
        {
            if (*pBuf == '\n' || *pBuf == '\r')
            {
                break;
            }
            else if (!pszMethod && *pBuf == ' ')
            {
                pszMethod = pszBuf;
                *pBuf = '\0';
            }
            else if (pszMethod && !pszValue && *pBuf != ' ')
            {
                pszValue = pBuf;
            }
            else if (pszMethod && pszValue && *pBuf == ' ')
            {
                pszLocation = pszValue;
                *pBuf = '\0';
                break;
            }
        }

        /* Parse the HTTP header name/value pairs */
        pszValue = 0;
        for ( ; pBuf < pBufEnd; ++pBuf)
        {
            if (*pBuf == '\n' || *pBuf == '\r')
            {
                /* Found name/value pair? */
                if (pszName && pszValue)
                {
                    /* Skip whitespace & quotes*/
                    for ( ; *(pBuf - 1) == ' ' && pBuf >= pszValue; pBuf--) {}
                    for ( ; *(pBuf - 1) == '"' && pBuf >= pszValue; pBuf--) {}

                    /* Null-terminate the value string */
                    *pBuf = '\0';

                    /* Handle HTTP headers... */
                    if (strcasecmp(pszName, "Content-Length") == 0)
                    {
                        pszContentLength = pszValue;
                    }
                    else if (strcasecmp(pszName, "Authorization") == 0)
                    {
                        pszAuth = pszValue;
                    }
                    else if (strcasecmp(pszName, "SOAPAction") == 0)
                    {
                        pszSoapAction = pszValue;
                    }
                }

                /* Start searching for the next name/value pair */
                pszName = pBuf + 1;
                pszValue = 0;
            }
            else if (!pszValue && *pBuf == ':')
            {
                /* Null-terminate the name string */
                *pBuf++ = '\0';

                /* Skip whitespace & quotes*/
                for ( ; *pBuf == ' ' && pBuf < pBufEnd; pBuf++) {}
                for ( ; *pBuf == '"' && pBuf < pBufEnd; pBuf++) {}
                pszValue = pBuf;
            }
        }
    }

    /* Return the results */
    *pGet = (pszMethod ? strcasecmp(pszMethod, "GET") == 0 : 0);
    *ppszLocation = pszLocation;
    *pContentLength = (pszContentLength ? atoi(pszContentLength) : 0);
    *ppszSoapAction = pszSoapAction;
    *ppszAuth = pszAuth;
}

/*
 * HTTP_AuthDecode - Decode the HTTP Authorization header
 */
static int HTTP_AuthDecode(char* pszHTTPAuth, char* pszAuthBuf, unsigned int cbAuthBuf,
                           char** ppszUsername, char** ppszPassword)
{
    int fResult = 0;

    /* Locate the start of the authorization data */
    char* pszBasicAuth = pszHTTPAuth;
    for ( ; *pszBasicAuth == ' ' || *pszBasicAuth == '\t'; ++pszBasicAuth) {}
    if ((*pszBasicAuth == 'B' || *pszBasicAuth == 'b') &&
        (*(pszBasicAuth + 1) == 'A' || *(pszBasicAuth + 1) == 'a') &&
        (*(pszBasicAuth + 2) == 'S' || *(pszBasicAuth + 2) == 's') &&
        (*(pszBasicAuth + 3) == 'I' || *(pszBasicAuth + 3) == 'i') &&
        (*(pszBasicAuth + 4) == 'C' || *(pszBasicAuth + 4) == 'c') &&
        (*(pszBasicAuth + 5) == ' ' || *(pszBasicAuth + 5) == '\t'))
    {
        int iState = 0;
        int iPrev = 0;
        unsigned int cbBasicAuth;
        int cbDecoded;

        /* Move beyond 'Basic ' */
        pszBasicAuth += sizeof("Basic ") - 1;
        cbBasicAuth = strlen(pszBasicAuth);

        /* Ensure the buffer is big enough for the base64-decoded auth data */
        if ((cbDecoded = HDK_DecodeBase64(0, 0, pszBasicAuth, cbBasicAuth, &iState, &iPrev)) >= 0 &&
            HDK_DecodeBase64Done(iState) >= 0 &&
            (unsigned int)cbDecoded < cbAuthBuf)
        {
            /* Base64-decode the auth data */
            char* pEncode = pszAuthBuf;
            iState = 0;
            iPrev = 0;
            if (HDK_DecodeBase64(HDK_EncodeToBuffer, &pEncode, pszBasicAuth, cbBasicAuth, &iState, &iPrev) >= 0 &&
                HDK_DecodeBase64Done(iState) >= 0)
            {
                char* pszColon;

                /* Null-terminate the decoded output */
                pszAuthBuf[cbDecoded] = 0;

                /* Split into user and password */
                pszColon = strchr(pszAuthBuf, ':');
                if (pszColon)
                {
                    *pszColon = 0;
                    *ppszUsername = pszAuthBuf;
                    *ppszPassword = pszColon + 1;
                    fResult = 1;
                }
            }
        }
    }

    return fResult;
}

/* Helper macro for static content responses */
#define FPRINTF_CONTENT(pfh, s) fprintf(pfh, "Content-Length: %d\r\n\r\n" s, (int)sizeof(s) - 1)

/*
 * HTTP_HandleRequest - Handle the HTTP request
 */
static void HTTP_HandleRequest(FILE* pfhRead, FILE* pfhWrite)
{
    void* pCtx;

    /* Parse the HTTP headers */
    char szHeaders[1024];
    int fGet = 0;
    char* pszLocation = 0;
    unsigned int cbContentLength = 0;
    char* pszSoapAction = 0;
    char* pszAuth = 0;
    HTTP_ParseHeaders(pfhRead, szHeaders, sizeof(szHeaders), &fGet, &pszLocation,
                      &cbContentLength, &pszSoapAction, &pszAuth);

	if (pszSoapAction == NULL)
	{
		pszSoapAction = "http://purenetworks.com/HNAP1/GetDeviceSettings";
	}

    /* Is this not an HNAP call? */
    if (!pszLocation || !HDK_IsHNAPRequest(pszLocation))
    {
        fprintf(
            pfhWrite,
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/html\r\n"
            "Connection: close\r\n");
        FPRINTF_CONTENT(
            pfhWrite,
            "<html>\n"
            "<head>\n"
            "<title>404 Not Found</title>\n"
            "</head>\n"
            "<body>\n"
            "404 Not Found\n"
            "</body>\n"
            "</html>\n");
    }
    /* Init the device context */
    else if (HDK_Context_Init(&pCtx, pfhRead, pfhWrite))
    {
        HDK_Enum_Result result = HDK_Enum_Result_ERROR;
        char szAuth[256];
        char* pszUsername;
        char* pszPassword;
		
        /* Authenticate, if necessary */
        if (!HDK_RequiresAuth(fGet, pszSoapAction) ||
            (pszAuth &&
             HTTP_AuthDecode(pszAuth, szAuth, sizeof(szAuth), &pszUsername, &pszPassword) &&
             HDK_Context_Authenticate(pCtx, pszUsername, pszPassword)))
        {
            result = HDK_HandleRequest(pCtx, fGet, cbContentLength, pszSoapAction);
        }
        else
        {
            fprintf(
                pfhWrite,
                "HTTP/1.1 401 Authorization Required\r\n"
                "WWW-Authenticate: Basic realm=\"HNAP1\"\r\n"
                "Content-Type: text/html\r\n"
                "Connection: close\r\n");
            FPRINTF_CONTENT(
                pfhWrite,
                "<html>\n"
                "<head>\n"
                "<title>401 Authorization Required</title>\n"
                "</head>\n"
                "<body>\n"
                "401 Authorization Required\n"
                "</body>\n"
                "</html>\n");
        }

        /* Free the device context */
        HDK_Context_Free(pCtx, HDK_SUCCEEDED(result));
    }
    else
    {
        fprintf(
            pfhWrite,
            "HTTP/1.1 500 Internal Server Error\r\n"
            "Content-Type: text/html\r\n"
            "Connection: close\r\n");
        FPRINTF_CONTENT(
            pfhWrite,
            "<html>\n"
            "<head>\n"
            "<title>500 Internal Server Error</title>\n"
            "</head>\n"
            "<body>\n"
            "500 Internal Server Error\n"
            "</body>\n"
            "</html>\n");
    }
}


void alarm_handler(int signal)
{
	char buf[4];
	memset(buf, 0, sizeof(buf));
	syscfg_get(NULL, "HomeSecurityEthernet4Flag", buf, sizeof(buf));

	if(strcmp(buf, "1") && signal == SIGALRM)
	{
		//system("date");
		homesecurity_timeout_handler();
		log_printf(LOG_WARNING, "received alarm signal, stopping eth4 Home Security\n");
	}
}
/*
 * hnapsrv main entry point
 */
int main(int argc, char *argv[])
{
    int port;
    int fdSock, ret;
    struct sockaddr_in addrServer;
	struct ifreq ifr;
	char buf[4];

    openlog ("hs_log", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

    /* Command line */
    if (argc < 2)
    {
        perror("Error: No port provided\n");
        exit(1);
    }
    port = atoi(argv[1]);	
	syscfg_init();
	/* Bind to the socket */
    fdSock = socket(AF_INET, SOCK_STREAM, 0);
    if (fdSock < 0)
    {
        perror("Error: Failure opening socket\n");
        exit(1);
    }
    memset(&addrServer, 0, sizeof(addrServer));
    addrServer.sin_family = AF_INET;
    addrServer.sin_addr.s_addr = INADDR_ANY;
    addrServer.sin_port = htons(port);

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, "brlan1", IFNAMSIZ);
	//binding interface 
	while (setsockopt(fdSock, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) < 0) {
		log_printf(LOG_WARNING, "Failure binding interface");
		sleep(1);
	}
    if (bind(fdSock, (struct sockaddr*)&addrServer, sizeof(addrServer)) < 0)
    {
        perror("Error: Failure binding socket");
        exit(1);
    }
	mbus_init();

	//If HomeSecurityEthernet4Flag was not set, need toggle Ethernet port 4 again to aviod
	//unexpected reboot during previous SetBridgeConnect method
	memset(buf, 0, sizeof(buf));
	ret = syscfg_get(NULL, "HomeSecurityEthernet4Flag", buf, sizeof(buf));

	//printf("*************************** %s\n", buf);
	if(ret == -1 || strcmp(buf, "1"))
	{
		homesecurity_timeout_handler();
	}

	//For SetBridgeConnect timer 
	signal( SIGALRM, alarm_handler);

    /* Loop forever... */
    while (1)
    {
        int fdRequest;
        socklen_t cbRequest;
        struct sockaddr_in addrRequest;

        /* Listen on the socket */
        listen(fdSock, 5);
        cbRequest = sizeof(addrRequest);
        fdRequest = accept(fdSock, (struct sockaddr*)&addrRequest, &cbRequest);
        if (fdRequest >= 0)
        {
            /* Create the request file streams */
            FILE* pfhRead = fdopen(fdRequest, "r+b");
            FILE* pfhWrite = fdopen(fdRequest, "w+b");
            if (pfhRead && pfhWrite)
            {
                HTTP_HandleRequest(pfhRead, pfhWrite);
            }

            /* Close the socket */
            if (pfhWrite)
            {
                fflush(pfhWrite);
                fclose(pfhWrite);
            }
            if (pfhRead)
            {
                fclose(pfhRead);
            }
        }
    }

    return 0;
}
