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

void printThreadsInfo(osRtxThread_t *);

__NO_RETURN void mbedFaultHandler (void *mbedFaultContextIn, void *osRtxInfoIn)
{
    osRtxFaultContext_t *mbedFaultContext = (osRtxFaultContext_t *)mbedFaultContextIn;
    osRtxInfo_t *osRtxInfo = (osRtxInfo_t *)osRtxInfoIn;
  
    error_print("\n===================="); 
    error_print("\nMbedOS Fault Handler");
    error_print("\n===================="); 
  
    error_print("\n\nRegister Info:");
    error_print("\nR0:0x%08X R1:0x%08X R2:0x%08X R3:0x%08X", mbedFaultContext->R0, mbedFaultContext->R1, mbedFaultContext->R2, mbedFaultContext->R3 );
    error_print("\nR4:0x%08X R5:0x%08X R6:0x%08X R7:0x%08X", mbedFaultContext->R4, mbedFaultContext->R5, mbedFaultContext->R6, mbedFaultContext->R7 );
    error_print("\nR8:0x%08X R9:0x%08X R10:0x%08X R11:0x%08X", mbedFaultContext->R8, mbedFaultContext->R9, mbedFaultContext->R10, mbedFaultContext->R11 );
    error_print("\nR12:0x%08X SP:0x%08X LR:0x%08X PC:0x%08X", mbedFaultContext->R12, mbedFaultContext->SP, mbedFaultContext->LR, mbedFaultContext->PC );
    error_print("\nxPSR:0x%08X MMFSR:0x%02X BFSR:0x%02X UFSR:0x%04X ", mbedFaultContext->xPSR, mbedFaultContext->MMFSR, mbedFaultContext->BFSR, mbedFaultContext->UFSR );  
    error_print("\nHFSR:0x%08X DFSR:0x%08X AFSR:0x%08X MMFAR:0x%08X ", mbedFaultContext->HFSR, mbedFaultContext->DFSR, mbedFaultContext->AFSR, mbedFaultContext->MMFAR );  
    error_print("\nBFAR:0x%08X SHCSR:0x%08X ", mbedFaultContext->BFAR, mbedFaultContext->SHCSR );  
        
    error_print("\n\nThread Info:");
    error_print("\nCurrent Thread:");
    osRtxThread_t *threads = osRtxInfo->thread.run.curr;
    printThreadsInfo(threads);
  
    error_print("\nNext Thread:");
    threads = osRtxInfo->thread.run.next;
    printThreadsInfo(threads);
    
    error_print("\nWait Threads:");
    threads = osRtxInfo->thread.wait_list;
    printThreadsInfo(threads);
    
    error_print("\nDelay Threads:");
    threads = osRtxInfo->thread.delay_list;
    printThreadsInfo(threads);
    
    error_print("\nIdle Thread:");
    threads = osRtxInfo->thread.idle;
    printThreadsInfo(threads);
    
    error_print("\n==========================="); 
    error_print("\nMbedOS Fault Handler - Done");
    error_print("\n===========================\n"); 
    
    /* Just spin */
    for (;;) {}
}

void printThreadsInfo(osRtxThread_t *threads)
{
    int count = 1;
    while(threads != NULL) {
        error_print("\n[%3d] State:%d Entry:0x%08X Stack Size:0x%08X Mem:0x%08X SP:0x%08X",
            count++, 
            threads->state,
            threads->thread_addr,
            threads->stack_size,
            threads->stack_mem,
            threads->sp      
            );
        threads = threads->thread_next;
    }
}
