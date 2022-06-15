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

/**************************************************************************

    module: chs_log.h

        For CHS development

    -------------------------------------------------------------------
    description:

        This file implements the log mechanism for HNAP control on Home Security.

    -------------------------------------------------------------------


    author:

        Rongwei

    -------------------------------------------------------------------

    revision:

        4/03/2013    initial revision.

**************************************************************************/

#ifndef _CHS_LOG_H
#define _CHS_LOG_H

#ifdef __cplusplus
extern "C" {
#endif


#include "syslog.h"

#ifdef LOG_DEBUG
#define log_printf(level, fmt, ...)	syslog(level, "[%s:%d]" fmt, __FILE__, __LINE__, ##__VA_ARGS__); 

#else
#define log_printf(level, fmt, ...)	syslog(level, fmt, ##__VA_ARGS__); 

#endif


#ifdef __cplusplus
};
#endif

#endif

