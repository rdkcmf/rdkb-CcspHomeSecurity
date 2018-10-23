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

#include "hdk_encode.h"


/* Encode-to-buffer encoding function */
int HDK_EncodeToBuffer(void* pEncodeCtx, char* pBuf, unsigned int cbBuf)
{
    char** ppBuf = (char**)pEncodeCtx;
    int i = cbBuf;
    while (i--)
    {
        *((*ppBuf)++) = *(pBuf++);
    }
    return cbBuf;
}


/*
 * XML string encoding - returns encoded length.
 */

int HDK_EncodeString(HDK_EncodeFn pfnEncode, void* pEncodeCtx, char* pBuf, unsigned int cbBuf)
{
    int cchLen = 0;

    char* pszValue = pBuf;
    char* pszValueEnd = pBuf + cbBuf;
    while (pszValue != pszValueEnd)
    {
        /* Search for XML entities */
        char* pszEntity = 0;
        char* p;
        for (p = pszValue; p != pszValueEnd; p++)
        {
            if (*p == '\"' || *p =='&' || *p == '\'' || *p == '<' || *p == '>')
            {
                pszEntity = p;
                break;
            }
        }

        /* Encode XML entities, if any */
        if (pszEntity)
        {
            /* Output the preceding text */
            cchLen += pszEntity - pszValue;
            if (pfnEncode)
            {
                pfnEncode(pEncodeCtx, pszValue, pszEntity - pszValue);
            }

            /* Output the XML entity */
            switch (*pszEntity)
            {
                case '"':
                    cchLen += sizeof("&quot;") - 1;
                    if (pfnEncode)
                    {
                        pfnEncode(pEncodeCtx, "&quot;", sizeof("&quot;") - 1);
                    }
                    break;
                case '&':
                    cchLen += sizeof("&amp;") - 1;
                    if (pfnEncode)
                    {
                        pfnEncode(pEncodeCtx, "&amp;", sizeof("&amp;") - 1);
                    }
                    break;
                case '\'':
                    cchLen += sizeof("&apos;") - 1;
                    if (pfnEncode)
                    {
                        pfnEncode(pEncodeCtx, "&apos;", sizeof("&apos;") - 1);
                    }
                    break;
                case '<':
                    cchLen += sizeof("&lt;") - 1;
                    if (pfnEncode)
                    {
                        pfnEncode(pEncodeCtx, "&lt;", sizeof("&lt;") - 1);
                    }
                    break;
                case '>':
                    cchLen += sizeof("&gt;") - 1;
                    if (pfnEncode)
                    {
                        pfnEncode(pEncodeCtx, "&gt;", sizeof("&gt;") - 1);
                    }
                    break;
                default:
                    break;
            }

            /* Increment past the entity */
            pszValue = pszEntity + 1;
        }
        else
        {
            /* No more entities */
            cchLen += pszValueEnd - pszValue;
            if (pfnEncode)
            {
                pfnEncode(pEncodeCtx, pszValue, pszValueEnd - pszValue);
            }
            break;
        }
    }

    return cchLen;
}


/*
 * Base 64 encoding - functions return encoded length.
 */

static char s_szBase64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int HDK_EncodeBase64(HDK_EncodeFn pfnEncode, void* pEncodeCtx, char* pBuf, unsigned int cbBuf, int* pState, int* pPrev)
{
    int cchLen = 0;

    char* p;
    char* pBufEnd = pBuf + cbBuf;
    for (p = pBuf; p != pBufEnd; p++)
    {
        if (*pState == 0)
        {
            if (pfnEncode)
            {
                pfnEncode(pEncodeCtx, &s_szBase64[(*p & 0xFC) >> 2], 1);
            }
            cchLen += 1;
            *pState = 1;
            *pPrev = ((*p & 0x03) << 4);
        }
        else if (*pState == 1)
        {
            if (pfnEncode)
            {
                pfnEncode(pEncodeCtx, &s_szBase64[*pPrev + ((*p & 0xF0) >> 4)], 1);
            }
            cchLen += 1;
            *pState = 2;
            *pPrev = ((*p & 0x0F) << 2);
        }
        else if (*pState == 2)
        {
            char c[2];
            c[0] = s_szBase64[*pPrev + ((*p & 0xC0) >> 6)];
            c[1] = s_szBase64[(*p & 0x3F)];
            if (pfnEncode)
            {
                pfnEncode(pEncodeCtx, c, sizeof(c));
            }
            cchLen += sizeof(c);
            *pState = 0;
        }
    }

    return cchLen;
}

int HDK_EncodeBase64Done(HDK_EncodeFn pfnEncode, void* pEncodeCtx, int state, int prev)
{
    int cchLen = 0;

    if (state)
    {
        char c[3];
        c[0] = s_szBase64[prev];
        c[1] = '=';
        c[2] = '=';
        cchLen = sizeof(c) + 1 - state;
        if (pfnEncode)
        {
            pfnEncode(pEncodeCtx, c, cchLen);
        }
    }

    return cchLen;
}


/*
 * Base64 decoding - functions return <0 on failure, decoded length on success.
 */

int HDK_DecodeBase64(HDK_EncodeFn pfnEncode, void* pEncodeCtx, char* pBuf, unsigned int cbBuf, int* pState, int* pPrev)
{
    int cchLen = 0;

    char* p;
    char* pBufEnd = pBuf + cbBuf;
    for (p = pBuf; p != pBufEnd; p++)
    {
        int bits = (*p >= 'A' && *p <= 'Z' ? *p - 'A' :
                    (*p >= 'a' && *p <= 'z' ? *p - 'a' + 26 :
                     (*p >= '0' && *p <= '9' ? *p - '0' + 52 :
                      (*p == '+' ? 62 :
                       (*p == '/' ? 63 :
                        (*p == '=' ? -1 :
                         (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' ? -2 : -3)))))));
        if (bits == -2)
        {
            continue;
        }
        if (bits == -3)
        {
            return -1;
        }

        if (*pState == 0)
        {
            if (bits == -1)
            {
                return -1;
            }
            *pState = 1;
            *pPrev = bits;
        }
        else if (*pState == 1)
        {
            char byte;
            if (bits == -1)
            {
                return -1;
            }
            byte = (*pPrev << 2) | ((bits & 0x30) >> 4);
            if (pfnEncode)
            {
                pfnEncode(pEncodeCtx, &byte, 1);
            }
            cchLen += 1;
            *pState = 2;
            *pPrev = bits;
        }
        else if (*pState == 2)
        {
            int nextState = 3;
            if (bits == -1)
            {
                nextState = 4; /* == */
                bits = 0;
            }
            else
            {
                char byte = ((*pPrev & 0x0F) << 4) | ((bits & 0x3C) >> 2);
                if (pfnEncode)
                {
                    pfnEncode(pEncodeCtx, &byte, 1);
                }
                cchLen += 1;
            }
            *pState = nextState;
            *pPrev = bits;
        }
        else if (*pState == 3)
        {
            int nextState = 0;
            if (bits == -1)
            {
                nextState = 5; /* = */
                bits = 0;
            }
            else
            {
                char byte = ((*pPrev & 0x03) << 6) | bits;
                if (pfnEncode)
                {
                    pfnEncode(pEncodeCtx, &byte, 1);
                }
                cchLen += 1;
            }
            *pState = nextState;
            *pPrev = bits;
        }
        else if (*pState == 4)
        {
            if (bits != -1)
            {
                return -1;
            }
            *pState = 5;
        }
        else if (*pState == 5)
        {
            return -1;
        }
    }

    return cchLen;
}

int HDK_DecodeBase64Done(int state)
{
    return (state == 0 || state == 5 ? 0 : -1);
}
