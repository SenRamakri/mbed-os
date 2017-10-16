/* mbed Microcontroller Library
 * Copyright (c) 2006-2016 ARM Limited
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

#include "rtx_os.h"
#include "rtx_evr.h"
#include "mbed_rtx.h"
#include "mbed_error.h"
#include "mbed_fault_handler.h"

__NO_RETURN void mbedFaultHandler (void *mbedFaultContextInfo, void *osRtxInfoIn)
{
    osRtxFaultContext_t *mbedFaultContext = (osRtxFaultContext_t *)mbedFaultContextInfo;
    error_print("\n====================="); 
    error_print("\nMbedOS Fault Handler");
    error_print("\n====================="); 
  
    error_print("\n\nRegister Info:");
    error_print("\nR0:0x%08X R1:0x%08X R2:0x%08X R3:0x%08X", mbedFaultContext->R0, mbedFaultContext->R1, mbedFaultContext->R2, mbedFaultContext->R3 );
    error_print("\nR4:0x%08X R5:0x%08X R6:0x%08X R7:0x%08X", mbedFaultContext->R4, mbedFaultContext->R5, mbedFaultContext->R6, mbedFaultContext->R7 );
    error_print("\nR8:0x%08X R9:0x%08X R10:0x%08X R11:0x%08X", mbedFaultContext->R8, mbedFaultContext->R9, mbedFaultContext->R10, mbedFaultContext->R11 );
    error_print("\nR12:0x%08X SP:0x%08X LR:0x%08X PC:0x%08X", mbedFaultContext->R12, mbedFaultContext->SP, mbedFaultContext->LR, mbedFaultContext->PC );
        
    error_print("\n\nThread Info:");
    
    
    error_print("\n\nMemPool Info:");
    
    /* Just spin */
    for (;;) {}
}

