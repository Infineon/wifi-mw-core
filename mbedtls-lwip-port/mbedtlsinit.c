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
#include "mbedtls/config.h"
#include <cy_result.h>
#include <cy_pdl.h>
#include "cyhal.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include <time.h>
#include "stdio.h"
#include "mbedtls/platform_time.h"

// Connect MBEDTLS to the PSoC 6 real time clock

mbedtls_time_t get_current_time(mbedtls_time_t *t)
{
    cy_stc_rtc_config_t now ;
    struct tm tm ;
    mbedtls_time_t ret ;

    Cy_RTC_GetDateAndTime(&now) ;

    memset(&tm, 0, sizeof(tm)) ;
    tm.tm_sec = now.sec ;
    tm.tm_min = now.min ;
    tm.tm_hour = now.hour ;
    tm.tm_mon = now.month ;
    tm.tm_mday = now.date ;
    tm.tm_year = now.year + 100 ;

    ret = mktime(&tm) ;
    if (t != NULL)
        *t = ret ;
    return ret ;
}

/* A default date has been chosen. Preferred method would be to use NTP or other time source */
cy_stc_rtc_config_t cfg = 
{
    .sec = 0,
    .min = 0,
    .hour = 0,
    .amPm = CY_RTC_AM,
    .hrFormat = CY_RTC_24_HOURS,
    .dayOfWeek = CY_RTC_SATURDAY,
    .date = 1,
    .month = 12,
    .year = 19
} ;

cy_rslt_t mbedtls_init()
{
    cy_rslt_t res ;

    res = Cy_RTC_Init(&cfg) ;
    if (res != CY_RSLT_SUCCESS)
        return res ;

    mbedtls_platform_set_time(get_current_time) ;
    return CY_RSLT_SUCCESS ;
}

/* This function generates true random number using TRNG HW engine.
*
* Parameters:
*  cyhal_trng_t *obj:        cyhal RNG object
*  uint8_t *output:          output buffer holding the random number
*  size_t length:            Requested random number length
*  size_t *output_length:    Actual generated random number length
* Return:
*  int    zero on success, negative value on failure
*/
static int trng_get_bytes(cyhal_trng_t *obj, uint8_t *output, size_t length, size_t *output_length)
{
    uint32_t offset = 0;
    /* If output is not word-aligned, write partial word */
    uint32_t prealign = (uint32_t)((uintptr_t)output % sizeof(uint32_t));
    if (prealign != 0) {
        uint32_t value = cyhal_trng_generate(obj);
        uint32_t count = sizeof(uint32_t) - prealign;
        memmove(&output[0], &value, count);
        offset += count;
    }
    /* Write aligned full words */
    for (; offset < length - (sizeof(uint32_t) - 1u); offset += sizeof(uint32_t)) {
        *(uint32_t *)(&output[offset]) = cyhal_trng_generate(obj);
    }
    /* Write partial trailing word if requested */
    if (offset < length) {
        uint32_t value = cyhal_trng_generate(obj);
        uint32_t count = length - offset;
        memmove(&output[offset], &value, count);
        offset += count;
    }
    *output_length = offset;
    return 0;
}

/*
* This function is the entropy source function. It generates true random number
* using HW TRNG engine. mbedtls random number module calls this function
* to get the entropy from HW TRGN engine.
*
* Parameters:
*  cyhal_trng_t *obj:        cyhal RNG object
*  uint8_t *output:          output buffer holding the random number
*  size_t length:            Requested random number length
*  size_t *output_length:    Actual generated random number length
* Return:
*  int    zero on success, negative value on failure
*/
int mbedtls_hardware_poll( void * data,
                           unsigned char * output,
                           size_t len,
                           size_t * olen )
{
    cyhal_trng_t obj;
    int ret;
    cy_rslt_t result;

    result = cyhal_trng_init(&obj);
    if( result != CY_RSLT_SUCCESS)
    {
        return -1;
    }

    ret = trng_get_bytes(&obj, output, len, olen);
    if( ret != 0)
    {
        return -1;
    }

    cyhal_trng_free(&obj);
    return 0;
}

