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

#ifndef LIBS_WIFI_MW_CORE_LWIP_WHD_PORT_CY_LWIP_ERROR_H_
#define LIBS_WIFI_MW_CORE_LWIP_WHD_PORT_CY_LWIP_ERROR_H_

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "cy_result.h"
#include "cy_result_mw.h"

/**
 * @defgroup generic_lwip_whd_port_defines CY generic LwIP WHD glue results/error codes
 * @ingroup group_utils_enums
 * Cypress middleware APIs return results of type cy_rslt_t and comprise of three parts:
 * - module base
 * - type
 * - error code
 *
 * \par Result Format
 *
   \verbatim
              Module base                   Type    Library specific error code
      +-----------------------------------+------+------------------------------+
      |CY_RSLT_MODULE_LWIP_WHD_PORT_BASE  | 0x2  |           Error Code         |
      +-----------------------------------+------+------------------------------+
                14-bits                    2-bits            16-bits

   Refer to the macro section of this document for library specific error codes.
   \endverbatim
 *
 * The data structure cy_rslt_t is part of cy_result.h located in <core_lib/include>
 *
 * Module base: This base is derived from CY_RSLT_MODULE_MIDDLEWARE_BASE (defined in cy_result.h) and is an offset of the CY_RSLT_MODULE_MIDDLEWARE_BASE
 *              The details of the offset and the middleware base are defined in cy_result_mw.h, that is part of [Github connectivity-utilities] (https://github.com/cypresssemiconductorco/connectivity-utilities)
 *              For instance, LwIP WHD PORT (CY_LWP_WHD_PORT) uses CY_RSLT_MODULE_LWIP_WHD_PORT_BASE as the module base
 *
 * Type: This type is defined in cy_result.h and can be one of CY_RSLT_TYPE_FATAL, CY_RSLT_TYPE_ERROR, CY_RSLT_TYPE_WARNING or CY_RSLT_TYPE_INFO. AWS library error codes are of type CY_RSLT_TYPE_ERROR
 *
 * Library specific error code: These error codes are library specific and defined in macro section
 *
 * Helper macros used for creating the library specific result are provided as part of cy_result.h
 * \{
 */
/** Generic CY LWIP WHD port base error code */
#define CY_RSLT_LWIP_WHD_PORT_ERR_BASE                     CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_LWIP_WHD_PORT_BASE, 0)

/** CY LWIP error */
#define CY_RSLT_LWIP_INTERFACE_EXISTS                      (CY_RSLT_LWIP_WHD_PORT_ERR_BASE + 1)  /**< Denotes interface already exists   */
#define CY_RSLT_LWIP_ERROR_ADDING_INTERFACE                (CY_RSLT_LWIP_WHD_PORT_ERR_BASE + 2)  /**< Denotes adding interface failed    */
#define CY_RSLT_LWIP_ERROR_STARTING_DHCP                   (CY_RSLT_LWIP_WHD_PORT_ERR_BASE + 3)  /**< Denotes failure to start DHCP      */
#define CY_RSLT_LWIP_INTERFACE_DOES_NOT_EXIST              (CY_RSLT_LWIP_WHD_PORT_ERR_BASE + 4)  /**< Denotes interface does not exist   */
/**
 * \}
 */

#ifdef __cplusplus
} /* extern C */
#endif

#endif /* LIBS_WIFI_MW_CORE_LWIP_WHD_PORT_CY_LWIP_ERROR_H_ */

