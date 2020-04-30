/*
 * Copyright 2020 Cypress Semiconductor Corporation
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif


#ifndef LIBS_WCM_INCLUDE_CY_LWIP_DEBUG_H_
#define LIBS_WCM_INCLUDE_CY_LWIP_DEBUG_H_

/* Enable Macros for WCM Info, debug, error */
#define WPRINT_ENABLE_LWIP_INFO
//#define WPRINT_ENABLE_LWIP_DEBUG
#define WPRINT_ENABLE_LWIP_ERROR

#ifdef WPRINT_ENABLE_LWIP_INFO
#define CY_LWIP_INFO( X )         printf X
#else
#define CY_LWIP_INFO( X )
#endif

#ifdef WPRINT_ENABLE_LWIP_DEBUG
#define CY_LWIP_DEBUG( X )        printf X
#else
#define CY_LWIP_DEBUG( X )
#endif

#ifdef WPRINT_ENABLE_LWIP_ERROR
#define CY_LWIP_ERROR( X )        printf X
#else
CY_LWIP_ERROR( X )
#endif

#endif /* LIBS_WCM_INCLUDE_CY_LWIP_DEBUG_H_ */

#ifdef __cplusplus
} /* extern C */
#endif
