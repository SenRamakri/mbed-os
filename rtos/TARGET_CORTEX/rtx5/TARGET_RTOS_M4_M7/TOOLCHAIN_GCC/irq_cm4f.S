/*
 * Copyright (c) 2013-2017 ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * -----------------------------------------------------------------------------
 *
 * Project:     CMSIS-RTOS RTX
 * Title:       Cortex-M4F Exception handlers
 *
 * -----------------------------------------------------------------------------
 */


        .file    "irq_cm4f.S"
        .syntax  unified

        .equ     I_T_RUN_OFS, 28        // osRtxInfo.thread.run offset
        .equ     I_T_CB_OFS,  172       // osRtxInfo.error_handler_callback offset            
        .equ     TCB_SP_OFS,  56        // TCB.SP offset
        .equ     TCB_SF_OFS,  34        // TCB.stack_frame offset
        .equ     SHCSR_ADDR,  0xE000ED24 // Address of System Handler and Control Status Register 
        .equ     CFSR_ADDR,   0xE000ED28 // Address of Configurable Fault Status Register 
        .equ     HFSR_ADDR,   0xE000ED2C // Address of Hard Fault Status Register
        .equ     DFSR_ADDR,   0xE000ED30 // Address of Debug Fault Status Register
        .equ     AFSR_ADDR,   0xE000ED3C // Address of Auxiliary Fault Status Register
        .equ     MMFAR_ADDR,  0xE000ED34 // Address of MemManage Fault Address Register
        .equ     BFAR_ADDR,   0xE000ED38 // Address of Bus Fault Status Register

        .section ".rodata"
        .global  irqRtxLib              // Non weak library reference
irqRtxLib:
        .byte    0


        .thumb
        .section ".text"
        .align   2

        .thumb_func
        .type    HardFault_Handler, %function
        .global  HardFault_Handler
        .fnstart
        .cantunwind
HardFault_Handler:
        B        Fault_Handler

        .fnend
        .size    HardFault_Handler, .-HardFault_Handler

        .thumb_func
        .type    MemManage_Handler, %function
        .global  MemManage_Handler
        .fnstart
        .cantunwind
MemManage_Handler:
        B        Fault_Handler

        .fnend
        .size    MemManage_Handler, .-MemManage_Handler


        .thumb_func
        .type    BusFault_Handler, %function
        .global  BusFault_Handler
        .fnstart
        .cantunwind
BusFault_Handler:
        B        Fault_Handler

        .fnend
        .size    BusFault_Handler, .-BusFault_Handler
        
        .thumb_func
        .type    UsageFault_Handler, %function
        .global  UsageFault_Handler
        .fnstart
        .cantunwind
UsageFault_Handler:
        B        Fault_Handler

        .fnend
        .size    UsageFault_Handler, .-UsageFault_Handler        

        .thumb_func
        .type    Fault_Handler, %function
        .global  Fault_Handler
        .fnstart
        .cantunwind
Fault_Handler:
        TST      LR,#0x4                  // Check EXC_RETURN for bit 2
        ITE      EQ
        MRSEQ    R0,MSP
        MRSNE    R0,PSP
                
        LDR      R1,=osRtxFaultContext     
        LDR      R2,[R0]                          // Capture R0
        STR      R2,[R1],#4
        LDR      R2,[R0,#4]                       // Capture R1
        STR      R2,[R1],#4
        LDR      R2,[R0,#8]               // Capture R2
        STR      R2,[R1],#4
        LDR      R2,[R0,#12]              // Capture R3
        STR      R2,[R1],#4
        STMIA    R1!,{R4-R11}             // Capture R4..R11
        LDR      R2,[R0,#16]              // Capture R12
        STR      R2,[R1],#4
        STR      R3,[R1],#4               // Capture SP
        LDR      R2,[R0,#20]              // Capture LR
        STR      R2,[R1],#4
        LDR      R2,[R0,#24]              // Capture PC
        STR      R2,[R1],#4
        LDR      R2,[R0,#28]              // Capture xPSR
        STR      R2,[R1],#4
        // Adjust stack pointer to its original value and capture it
        MOV      R3,R0
        TST      LR,#0x10                 // Check for if FP context was saved  
        ITE      EQ
        ADDEQ    R3,#0x68
        ADDNE    R3,#0x20
        TST      R2,#0x200                // Check for if STK was aligned
        ITE      EQ
        ADDEQ    R2,#0x4
        ADDNE    R2,#0x0
        MOV      R4,R1
        SUB      R4,#0x10                 // Set the location of SP
        STR      R3,[R4]                  // Capture the adjusted SP
        LDR      R0,=CFSR_ADDR            // Capture CFSR
        LDR      R2,[R0]
        STR      R2,[R1],#4
        LDR      R0,=HFSR_ADDR            // Capture HFSR
        LDR      R2,[R0]
        STR      R2,[R1],#4
        LDR      R0,=DFSR_ADDR            // Capture DFSR
        LDR      R2,[R0]
        STR      R2,[R1],#4
        LDR      R0,=AFSR_ADDR            // Capture AFSR
        LDR      R2,[R0]
        STR      R2,[R1],#4
        LDR      R0,=SHCSR_ADDR           // Capture SHCSR
        LDR      R2,[R0]
        STR      R2,[R1],#4
        LDR      R0,=MMFAR_ADDR           // Capture MMFAR
        LDR      R2,[R0]
        STR      R2,[R1],#4
        LDR      R0,=BFAR_ADDR            // Capture BFAR
        LDR      R2,[R0]
        STR      R2,[R1],#4
        LDR      R2,=osRtxInfo+I_T_CB_OFS // Load address of osRtxInfo.error_handler_callback
        LDR      R3,[R2]                  // Load the address of error_handler_callback                
        CMP      R3,#0
        BNE      ErrorHandler_CB                            
        B        .
ErrorHandler_CB: 
        LDR      R0, =osRtxFaultContext
        LDR      R1, =osRtxInfo
        BLX      R3

        .fnend
        .size    Fault_Handler, .-Fault_Handler

        .thumb_func
        .type    SVC_Handler, %function
        .global  SVC_Handler
        .fnstart
        .cantunwind
SVC_Handler:

        MRS      R0,PSP                 // Get PSP
        LDR      R1,[R0,#24]            // Load saved PC from stack
        LDRB     R1,[R1,#-2]            // Load SVC number
        CBNZ     R1,SVC_User            // Branch if not SVC 0

        PUSH     {R0,LR}                // Save PSP and EXC_RETURN
        LDM      R0,{R0-R3,R12}         // Load function parameters and address from stack
        BLX      R12                    // Call service function
        POP      {R12,LR}               // Restore PSP and EXC_RETURN
        STM      R12,{R0-R1}            // Store function return values

SVC_Context:
        LDR      R3,=osRtxInfo+I_T_RUN_OFS // Load address of osRtxInfo.run
        LDM      R3,{R1,R2}             // Load osRtxInfo.thread.run: curr & next
        CMP      R1,R2                  // Check if thread switch is required
        IT       EQ
        BXEQ     LR                     // Exit when threads are the same

        CBNZ     R1,SVC_ContextSave     // Branch if running thread is not deleted
        TST      LR,#0x10               // Check if extended stack frame
        BNE      SVC_ContextSwitch
#ifdef __FPU_PRESENT
        LDR      R1,=0xE000EF34         // FPCCR Address
        LDR      R0,[R1]                // Load FPCCR
        BIC      R0,#1                  // Clear LSPACT (Lazy state)
        STR      R0,[R1]                // Store FPCCR
        B        SVC_ContextSwitch
#endif

SVC_ContextSave:
        STMDB    R12!,{R4-R11}          // Save R4..R11

#ifdef __FPU_PRESENT
        TST      LR,#0x10               // Check if extended stack frame
        IT       EQ
        VSTMDBEQ R12!,{S16-S31}         //  Save VFP S16.S31
#endif

        STR      R12,[R1,#TCB_SP_OFS]   // Store SP
        STRB     LR, [R1,#TCB_SF_OFS]   // Store stack frame information

SVC_ContextSwitch:
#ifdef FEATURE_UVISOR
        CPSID I // The call to the thread switch helper and PSP loading must be atomic.
#endif
        /* The call to thread_switch_helper can clobber R2 and R3, but we don't
         * want to clobber R2 or R3. We can't save R2 and R3 to the stack (as
         * the stack we save them onto is likely to be inaccessible after the
         * call to thread_switch_helper). So, we just re-obtain the values from
         * osRtxInfo again. */
        BL       thread_switch_helper
        LDR      R3,=osRtxInfo+I_T_RUN_OFS // Load address of osRtxInfo.run
        LDM      R3,{R1,R2}             // Load osRtxInfo.thread.run: curr & next

        STR      R2,[R3]                // osRtxInfo.thread.run: curr = next

SVC_ContextRestore:
        LDRB     R1,[R2,#TCB_SF_OFS]    // Load stack frame information
        LDR      R0,[R2,#TCB_SP_OFS]    // Load SP
        ORR      LR,R1,#0xFFFFFF00      // Set EXC_RETURN

#ifdef __FPU_PRESENT
        TST      LR,#0x10               // Check if extended stack frame
        IT       EQ
        VLDMIAEQ R0!,{S16-S31}          //  Restore VFP S16..S31
#endif
        LDMIA    R0!,{R4-R11}           // Restore R4..R11
        MSR      PSP,R0                 // Set PSP
#ifdef FEATURE_UVISOR
        CPSIE I                         // The PSP has been set. Re-enable interrupts.
#endif

SVC_Exit:
        BX       LR                     // Exit from handler

SVC_User:
        PUSH     {R4,LR}                // Save registers
        LDR      R2,=osRtxUserSVC       // Load address of SVC table
        LDR      R3,[R2]                // Load SVC maximum number
        CMP      R1,R3                  // Check SVC number range
        BHI      SVC_Done               // Branch if out of range

        LDR      R4,[R2,R1,LSL #2]      // Load address of SVC function

        LDM      R0,{R0-R3}             // Load function parameters from stack
        BLX      R4                     // Call service function
        MRS      R4,PSP                 // Get PSP
        STR      R0,[R4]                // Store function return value

SVC_Done:
        POP     {R4,PC}                 // Return from handler

        .fnend
        .size    SVC_Handler, .-SVC_Handler


        .thumb_func
        .type    PendSV_Handler, %function
        .global  PendSV_Handler
        .fnstart
        .cantunwind
PendSV_Handler:

        PUSH     {R4,LR}                // Save EXC_RETURN
        BL       osRtxPendSV_Handler    // Call osRtxPendSV_Handler
        POP      {R4,LR}                // Restore EXC_RETURN
        MRS      R12,PSP
        B        SVC_Context

        .fnend
        .size    PendSV_Handler, .-PendSV_Handler


        .thumb_func
        .type    SysTick_Handler, %function
        .global  SysTick_Handler
        .fnstart
        .cantunwind
SysTick_Handler:

        PUSH     {R4,LR}                // Save EXC_RETURN
        BL       osRtxTick_Handler      // Call osRtxTick_Handler
        POP      {R4,LR}                // Restore EXC_RETURN
        MRS      R12,PSP
        B        SVC_Context

        .fnend
        .size    SysTick_Handler, .-SysTick_Handler


        .end
