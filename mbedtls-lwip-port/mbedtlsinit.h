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
/**
* \addtogroup group_mbedtls_lwip_port mbedTLS and lwIP port
* \{
* Functions for dealing with linking the mbedTLS security stack with lwIP TCP/IP stack
*
* \defgroup group_mbedtls_lwip_port_functions Functions
* \defgroup group_mbedtls_lwip_port_macros Macros
* \defgroup group_mbedtls_lwip_port_structures Structures
*/
#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

/**
* \addtogroup group_mbedtls_lwip_port_functions
* \{
*/

/**
 * Function that performs core initialization of mbedTLS
 *
 * @return CY_RESULT_SUCCESS for successful initialization or error otherwise
 */
extern cy_rslt_t mbedtls_init() ;

/** \} group_mbedtls_lwip_port_functions */

#if defined(__cplusplus)
}
#endif
/** \} group_mbedtls_lwip_port */

