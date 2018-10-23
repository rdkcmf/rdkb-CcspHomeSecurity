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

#ifndef __HDK_ENCODE_H__
#define __HDK_ENCODE_H__

/* Encoding output functions */
typedef int (*HDK_EncodeFn)(void* pEncodeCtx, char* pBuf, unsigned int cbBuf);
extern int HDK_EncodeToBuffer(void* pEncodeCtx, char* pBuf, unsigned int cbBuf);

/* Encoding/decoding utilities */
extern int HDK_EncodeString(HDK_EncodeFn pfnEncode, void* pEncodeCtx, char* pBuf, unsigned int cbBuf);
extern int HDK_EncodeBase64(HDK_EncodeFn pfnEncode, void* pEncodeCtx, char* pBuf, unsigned int cbBuf, int* pState, int* pPrev);
extern int HDK_EncodeBase64Done(HDK_EncodeFn pfnEncode, void* pEncodeCtx, int state, int prev);
extern int HDK_DecodeBase64(HDK_EncodeFn pfnEncode, void* pEncodeCtx, char* pBuf, unsigned int cbBuf, int* pState, int* pPrev);
extern int HDK_DecodeBase64Done(int state);

#endif /* __HDK_ENCODE_H__ */
