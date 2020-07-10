;/*****************************************************************************
;*   startup.s: startup file for NXP LPC23xx Family Microprocessors
;*
;*   Copyright(C) 2006, NXP Semiconductor
;*   All rights reserved.
;*
;*   History
;*   2006.09.01  ver 1.00    Prelimnary version, first Release
;*

								PRESERVE8

;/*
; *  The STARTUP.S code is executed after CPU Reset. This file may be 
; *  translated with the following SET symbols. In uVision these SET 
; *  symbols are entered under Options - ASM - Define.
; *
; *  REMAP: when set the startup code initializes the register MEMMAP 
; *  which overwrites the settings of the CPU configuration pins. The 
; *  startup and interrupt vectors are remapped from:
; *     0x00000000  default setting (not remapped)
; *     0x40000000  when RAM_MODE is used
; *
; *  RAM_MODE: when set the device is configured for code execution
; *  from on-chip RAM starting at address 0x40000000. 
; */

; Standard definitions of Mode bits and Interrupt (I & F) flags in PSRs

Mode_USR        EQU     0x10
Mode_FIQ        EQU     0x11
Mode_IRQ        EQU     0x12
Mode_SVC        EQU     0x13
Mode_ABT        EQU     0x17
Mode_UND        EQU     0x1B
Mode_SYS        EQU     0x1F

I_Bit           EQU     0x80            ; when I bit is set, IRQ is disabled
F_Bit           EQU     0x40            ; when F bit is set, FIQ is disabled

;----------------------- Memory Definitions ------------------------------------

; Internal Memory Base Addresses
FLASH_BASE      EQU     0x00000000   
RAM_BASE        EQU     0x40000000
EXTMEM_BASE     EQU     0x80000000

; External Memory Base Addresses
STA_MEM0_BASE   EQU     0x80000000
STA_MEM1_BASE   EQU     0x81000000
STA_MEM2_BASE   EQU     0x82000000
STA_MEM3_BASE   EQU     0x83000000
DYN_MEM0_BASE   EQU     0xA0000000   
DYN_MEM1_BASE   EQU     0xB0000000   
DYN_MEM2_BASE   EQU     0xC0000000   
DYN_MEM3_BASE   EQU     0xD0000000   

;----------------------- External Memory Controller (EMC) Definitons -----------

EMC_BASE            EQU 0xFFE08000      ; EMC Base Address

EMC_CTRL_OFS        EQU 0x000
EMC_STAT_OFS        EQU 0x004
EMC_CONFIG_OFS      EQU 0x008
EMC_DYN_CTRL_OFS    EQU 0x020
EMC_DYN_RFSH_OFS    EQU 0x024
EMC_DYN_RD_CFG_OFS  EQU 0x028
EMC_DYN_RP_OFS      EQU 0x030
EMC_DYN_RAS_OFS     EQU 0x034
EMC_DYN_SREX_OFS    EQU 0x038
EMC_DYN_APR_OFS     EQU 0x03C
EMC_DYN_DAL_OFS     EQU 0x040
EMC_DYN_WR_OFS      EQU 0x044
EMC_DYN_RC_OFS      EQU 0x048
EMC_DYN_RFC_OFS     EQU 0x04C
EMC_DYN_XSR_OFS     EQU 0x050
EMC_DYN_RRD_OFS     EQU 0x054
EMC_DYN_MRD_OFS     EQU 0x058
EMC_DYN_CFG0_OFS    EQU 0x100
EMC_DYN_RASCAS0_OFS EQU 0x104
EMC_DYN_CFG1_OFS    EQU 0x140
EMC_DYN_RASCAS1_OFS EQU 0x144
EMC_DYN_CFG2_OFS    EQU 0x160
EMC_DYN_RASCAS2_OFS EQU 0x164
EMC_DYN_CFG3_OFS    EQU 0x180
EMC_DYN_RASCAS3_OFS EQU 0x184
EMC_STA_CFG0_OFS    EQU 0x200
EMC_STA_WWEN0_OFS   EQU 0x204
EMC_STA_WOEN0_OFS   EQU 0x208
EMC_STA_WRD0_OFS    EQU 0x20C
EMC_STA_WPAGE0_OFS  EQU 0x210
EMC_STA_WWR0_OFS    EQU 0x214
EMC_STA_WTURN0_OFS  EQU 0x218
EMC_STA_CFG1_OFS    EQU 0x220
EMC_STA_WWEN1_OFS   EQU 0x224
EMC_STA_WOEN1_OFS   EQU 0x228
EMC_STA_WRD1_OFS    EQU 0x22C
EMC_STA_WPAGE1_OFS  EQU 0x230
EMC_STA_WWR1_OFS    EQU 0x234
EMC_STA_WTURN1_OFS  EQU 0x238
EMC_STA_CFG2_OFS    EQU 0x240
EMC_STA_WWEN2_OFS   EQU 0x244
EMC_STA_WOEN2_OFS   EQU 0x248
EMC_STA_WRD2_OFS    EQU 0x24C
EMC_STA_WPAGE2_OFS  EQU 0x250
EMC_STA_WWR2_OFS    EQU 0x254
EMC_STA_WTURN2_OFS  EQU 0x258
EMC_STA_CFG3_OFS    EQU 0x260
EMC_STA_WWEN3_OFS   EQU 0x264
EMC_STA_WOEN3_OFS   EQU 0x268
EMC_STA_WRD3_OFS    EQU 0x26C
EMC_STA_WPAGE3_OFS  EQU 0x270
EMC_STA_WWR3_OFS    EQU 0x274
EMC_STA_WTURN3_OFS  EQU 0x278
EMC_STA_EXT_W_OFS   EQU 0x080

; Constants
NORMAL_CMD          EQU (0x0 << 7)      ; NORMAL        Command
MODE_CMD            EQU (0x1 << 7)      ; MODE          Command
PALL_CMD            EQU (0x2 << 7)      ; Precharge All Command
NOP_CMD             EQU (0x3 << 7)      ; NOP           Command

BUFEN_Const         EQU (1 << 19)       ; Buffer enable bit
EMC_PCONP_Const     EQU (1 << 11)       ; PCONP val to enable power for EMC

; External Memory Pins definitions
; pin functions for SDRAM, NOR and NAND flash interfacing
EMC_PINSEL5_Val     EQU 0x55010115      ; !CAS, !RAS, CLKOUT0, !DYCS0, DQMOUT0, DQMOUT1, DQMOUT2, DQMOUT3
EMC_PINSEL6_Val     EQU 0x55555555      ; D0 .. D15
EMC_PINSEL7_Val     EQU 0x55555555      ; D16..D31
EMC_PINSEL8_Val     EQU 0x55555555      ; A0 .. A15
EMC_PINSEL9_Val     EQU 0x50055555;     ; A16 .. A23, !OE, !WE, !CS0, !CS1

		INCLUDE	startupConfig.s
								
ISR_Stack_Size  EQU     (UND_Stack_Size + SVC_Stack_Size + ABT_Stack_Size + \
                         FIQ_Stack_Size + IRQ_Stack_Size)

                AREA    STACK, NOINIT, READWRITE, ALIGN=3

Stack_Mem       SPACE   USR_Stack_Size
__initial_sp    SPACE   ISR_Stack_Size

Stack_Top

                AREA    HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
Heap_Mem        SPACE   Heap_Size
__heap_limit

; Area Definition and Entry Point
;  Startup Code must be linked first at Address at which it expects to run.

                AREA    RESET, CODE, READONLY
                ARM

; Exception Vectors
;  Mapped to Address 0.
;  Absolute addressing mode must be used.
;  Dummy Handlers are implemented as infinite loops which can be modified.

Vectors         LDR     PC, Reset_Addr         
                LDR     PC, Undef_Addr
                LDR     PC, SWI_Addr
                LDR     PC, PAbt_Addr
                LDR     PC, DAbt_Addr
                NOP                            ; Reserved Vector 
;               LDR     PC, IRQ_Addr
                LDR     PC, [PC, #-0x0120]     ; Vector from VicVectAddr
                LDR     PC, FIQ_Addr

Reset_Addr      DCD     Reset_Handler
Undef_Addr      DCD     Undef_Handler

		IF      :DEF:USE_FREE_RTOS

								IMPORT  vPortYieldProcessor	
SWI_Addr        DCD     vPortYieldProcessor

		ELIF	:DEF:USE_USER_SWI

								IMPORT  SWI_Handler
SWI_Addr				DCD			SWI_Handler
								
		ELSE

SWI_Addr				DCD			SWI_Addr

		ENDIF

PAbt_Addr       DCD     PAbt_Handler
DAbt_Addr       DCD     DAbt_Handler
                DCD			0             ; Reserved Address 
IRQ_Addr        DCD     IRQ_Handler
FIQ_Addr        DCD     FIQ_Handler

Undef_Handler   B       Undef_Handler
PAbt_Handler    B       PAbt_Handler
DAbt_Handler    B       DAbt_Handler
IRQ_Handler     B       IRQ_Handler
FIQ_Handler     B       FIQ_Handler

; Reset Handler

                EXPORT  Reset_Handler
Reset_Handler   

; Setup External Memory Controller ---------------------------------------------

                IF      (:LNOT:(:DEF:NO_EMC_SETUP)):LAND:(EMC_SETUP != 0)
                LDR     R0, =EMC_BASE
                LDR     R1, =SCB_BASE
                LDR     R2, =PCB_BASE

                LDR     R4, =EMC_PCONP_Const      ; Enable EMC
                LDR     R3, [R1, #PCONP_OFS]
                ORR     R4, R4, R3
                STR     R4, [R1, #PCONP_OFS]

                LDR     R4, =EMC_CTRL_Val
                STR     R4, [R0, #EMC_CTRL_OFS]
                LDR     R4, =EMC_CONFIG_Val
                STR     R4, [R0, #EMC_CONFIG_OFS]

;  Setup pin functions for External Bus functionality
                LDR     R4, =EMC_PINSEL5_Val
                STR     R4, [R2, #PINSEL5_OFS]
                LDR     R4, =EMC_PINSEL6_Val
                STR     R4, [R2, #PINSEL6_OFS]
                LDR     R4, =EMC_PINSEL7_Val
                STR     R4, [R2, #PINSEL7_OFS]
                LDR     R4, =EMC_PINSEL8_Val
                STR     R4, [R2, #PINSEL8_OFS]
                LDR     R4, =EMC_PINSEL9_Val
                STR     R4, [R2, #PINSEL9_OFS]

;  Setup Dynamic Memory Interface
                IF      (EMC_DYNAMIC_SETUP != 0)

                LDR     R4, =EMC_DYN_RP_Val
                STR     R4, [R0, #EMC_DYN_RP_OFS]
                LDR     R4, =EMC_DYN_RAS_Val
                STR     R4, [R0, #EMC_DYN_RAS_OFS]
                LDR     R4, =EMC_DYN_SREX_Val
                STR     R4, [R0, #EMC_DYN_SREX_OFS]
                LDR     R4, =EMC_DYN_APR_Val
                STR     R4, [R0, #EMC_DYN_APR_OFS]
                LDR     R4, =EMC_DYN_DAL_Val
                STR     R4, [R0, #EMC_DYN_DAL_OFS]
                LDR     R4, =EMC_DYN_WR_Val
                STR     R4, [R0, #EMC_DYN_WR_OFS]
                LDR     R4, =EMC_DYN_RC_Val
                STR     R4, [R0, #EMC_DYN_RC_OFS]
                LDR     R4, =EMC_DYN_RFC_Val
                STR     R4, [R0, #EMC_DYN_RFC_OFS]
                LDR     R4, =EMC_DYN_XSR_Val
                STR     R4, [R0, #EMC_DYN_XSR_OFS]
                LDR     R4, =EMC_DYN_RRD_Val
                STR     R4, [R0, #EMC_DYN_RRD_OFS]
                LDR     R4, =EMC_DYN_MRD_Val
                STR     R4, [R0, #EMC_DYN_MRD_OFS]

                LDR     R4, =EMC_DYN_RD_CFG_Val
                STR     R4, [R0, #EMC_DYN_RD_CFG_OFS]

                IF      (EMC_DYNCS0_SETUP != 0)
                LDR     R4, =EMC_DYN_RASCAS0_Val
                STR     R4, [R0, #EMC_DYN_RASCAS0_OFS]
                LDR     R4, =EMC_DYN_CFG0_Val
                MVN     R5, #BUFEN_Const
                AND     R4, R4, R5
                STR     R4, [R0, #EMC_DYN_CFG0_OFS]
                ENDIF
                IF      (EMC_DYNCS1_SETUP != 0)
                LDR     R4, =EMC_DYN_RASCAS1_Val
                STR     R4, [R0, #EMC_DYN_RASCAS1_OFS]
                LDR     R4, =EMC_DYN_CFG1_Val
                MVN     R5, =BUFEN_Const
                AND     R4, R4, R5
                STR     R4, [R0, #EMC_DYN_CFG1_OFS]
                ENDIF
                IF      (EMC_DYNCS2_SETUP != 0)
                LDR     R4, =EMC_DYN_RASCAS2_Val
                STR     R4, [R0, #EMC_DYN_RASCAS2_OFS]
                LDR     R4, =EMC_DYN_CFG2_Val
                MVN     R5, =BUFEN_Const
                AND     R4, R4, R5
                STR     R4, [R0, #EMC_DYN_CFG2_OFS]
                ENDIF
                IF      (EMC_DYNCS3_SETUP != 0)
                LDR     R4, =EMC_DYN_RASCAS3_Val
                STR     R4, [R0, #EMC_DYN_RASCAS3_OFS]
                LDR     R4, =EMC_DYN_CFG3_Val
                MVN     R5, =BUFEN_Const
                AND     R4, R4, R5
                STR     R4, [R0, #EMC_DYN_CFG3_OFS]
                ENDIF

                LDR     R6, =1440000              ; Number of cycles to delay
Wait_0          SUBS    R6, R6, #1                ; Delay ~100 ms proc clk 57.6 MHz
                BNE     Wait_0                    ; BNE (3 cyc) + SUBS (1 cyc) = 4 cyc

                LDR     R4, =(NOP_CMD:OR:0x03)    ; Write NOP Command
                STR     R4, [R0, #EMC_DYN_CTRL_OFS]

                LDR     R6, =2880000              ; Number of cycles to delay
Wait_1          SUBS    R6, R6, #1                ; Delay ~200 ms proc clk 57.6 MHz
                BNE     Wait_1

                LDR     R4, =(PALL_CMD:OR:0x03)   ; Write Precharge All Command
                STR     R4, [R0, #EMC_DYN_CTRL_OFS]
  
                MOV     R4, #2
                STR     R4, [R0, #EMC_DYN_RFSH_OFS]

                MOV     R6, #64                   ; Number of cycles to delay
Wait_2          SUBS    R6, R6, #1                ; Delay
                BNE     Wait_2

                LDR     R4, =EMC_DYN_RFSH_Val
                STR     R4, [R0, #EMC_DYN_RFSH_OFS]

                LDR     R4, =(MODE_CMD:OR:0x03)   ; Write MODE Command
                STR     R4, [R0, #EMC_DYN_CTRL_OFS]
    
                ; Dummy read  (set SDRAM Mode register)
                IF      (EMC_DYNCS0_SETUP != 0)
                LDR     R4, =DYN_MEM0_BASE
				LDR     R5, =(EMC_DYN_RASCAS0_Val:AND:0x00000300)	; get CAS Latency
				LSR     R5, #4										; set CAS Latency    (Bit4..6)
                IF      ((EMC_DYN_CFG0_Val:AND:0x00004000) != 0)
				ORR     R5, R5, #0x02                               ; set burst length 4 (Bit0..2)
				LSL     R5, #11
				ELSE
				ORR     R5, R5, #0x03                               ; set burst length 8 (Bit0..2)
				LSL     R5, #12
				ENDIF
                ADD     R4, R4, R5
                LDR     R4, [R4, #0]
                ENDIF
                IF      (EMC_DYNCS1_SETUP != 0)
                LDR     R4, =DYN_MEM1_BASE
				LDR     R5, =(EMC_DYN_RASCAS1_Val:AND:0x00000300)	; get CAS Latency
				LSR     R5, #4										; set CAS Latency    (Bit4..6)
                IF      ((EMC_DYN_CFG1_Val:AND:0x00004000) != 0)
				ORR     R5, R5, #0x02                               ; set burst length 4 (Bit0..2)
				LSL     R5, #11
				ELSE
				ORR     R5, R5, #0x03                               ; set burst length 8 (Bit0..2)
				LSL     R5, #12
				ENDIF
                ADD     R4, R4, R5
                LDR     R4, [R4, #0]
                ENDIF
                IF      (EMC_DYNCS2_SETUP != 0)
                LDR     R4, =DYN_MEM2_BASE
				LDR     R5, =(EMC_DYN_RASCAS2_Val:AND:0x00000300)	; get CAS Latency
				LSR     R5, #4										; set CAS Latency    (Bit4..6)
                IF      ((EMC_DYN_CFG2_Val:AND:0x00004000) != 0)
				ORR     R5, R5, #0x02                               ; set burst length 4 (Bit0..2)
				LSL     R5, #11
				ELSE
				ORR     R5, R5, #0x03                               ; set burst length 8 (Bit0..2)
				LSL     R5, #12
				ENDIF
                ADD     R4, R4, R5
                LDR     R4, [R4, #0]
                ENDIF
                IF      (EMC_DYNCS3_SETUP != 0)
                LDR     R4, =DYN_MEM3_BASE
				LDR     R5, =(EMC_DYN_RASCAS3_Val:AND:0x00000300)	; get CAS Latency
				LSR     R5, #4										; set CAS Latency    (Bit4..6)
                IF      ((EMC_DYN_CFG3_Val:AND:0x00004000) != 0)
				ORR     R5, R5, #0x02                               ; set burst length 4 (Bit0..2)
				LSL     R5, #11
				ELSE
				ORR     R5, R5, #0x03                               ; set burst length 8 (Bit0..2)
				LSL     R5, #12
				ENDIF
                ADD     R4, R4, R5
                LDR     R4, [R4, #0]
                ENDIF

                LDR     R4, =NORMAL_CMD           ; Write NORMAL Command
                STR     R4, [R0, #EMC_DYN_CTRL_OFS]

                ; Enable buffer if requested by settings
                IF      (EMC_DYNCS0_SETUP != 0):LAND:((EMC_DYN_CFG0_Val:AND:BUFEN_Const) != 0)
                LDR     R4, =EMC_DYN_CFG0_Val
                STR     R4, [R0, #EMC_DYN_CFG0_OFS]
                ENDIF
                IF      (EMC_DYNCS1_SETUP != 0):LAND:((EMC_DYN_CFG1_Val:AND:BUFEN_Const) != 0)
                LDR     R4, =EMC_DYN_CFG1_Val
                STR     R4, [R0, #EMC_DYN_CFG1_OFS]
                ENDIF
                IF      (EMC_DYNCS2_SETUP != 0):LAND:((EMC_DYN_CFG2_Val:AND:BUFEN_Const) != 0)
                LDR     R4, =EMC_DYN_CFG2_Val
                STR     R4, [R0, #EMC_DYN_CFG2_OFS]
                ENDIF
                IF      (EMC_DYNCS3_SETUP != 0):LAND:((EMC_DYN_CFG3_Val:AND:BUFEN_Const) != 0)
                LDR     R4, =EMC_DYN_CFG3_Val
                STR     R4, [R0, #EMC_DYN_CFG3_OFS]
                ENDIF

                LDR     R6, =14400                ; Number of cycles to delay
Wait_3          SUBS    R6, R6, #1                ; Delay ~1 ms @ proc clk 57.6 MHz
                BNE     Wait_3

                ENDIF		; EMC_DYNAMIC_SETUP

;  Setup Static Memory Interface
                IF      (EMC_STATIC_SETUP != 0)

                LDR     R6, =1440000              ; Number of cycles to delay
Wait_4          SUBS    R6, R6, #1                ; Delay ~100 ms @ proc clk 57.6 MHz
                BNE     Wait_4

                IF      (EMC_STACS0_SETUP != 0)
                LDR     R4, =EMC_STA_CFG0_Val
                STR     R4, [R0, #EMC_STA_CFG0_OFS]
                LDR     R4, =EMC_STA_WWEN0_Val
                STR     R4, [R0, #EMC_STA_WWEN0_OFS]
                LDR     R4, =EMC_STA_WOEN0_Val
                STR     R4, [R0, #EMC_STA_WOEN0_OFS]
                LDR     R4, =EMC_STA_WRD0_Val
                STR     R4, [R0, #EMC_STA_WRD0_OFS]
                LDR     R4, =EMC_STA_WPAGE0_Val
                STR     R4, [R0, #EMC_STA_WPAGE0_OFS]
                LDR     R4, =EMC_STA_WWR0_Val
                STR     R4, [R0, #EMC_STA_WWR0_OFS]
                LDR     R4, =EMC_STA_WTURN0_Val
                STR     R4, [R0, #EMC_STA_WTURN0_OFS]
                ENDIF

                IF      (EMC_STACS1_SETUP != 0)
                LDR     R4, =EMC_STA_CFG1_Val
                STR     R4, [R0, #EMC_STA_CFG1_OFS]
                LDR     R4, =EMC_STA_WWEN1_Val
                STR     R4, [R0, #EMC_STA_WWEN1_OFS]
                LDR     R4, =EMC_STA_WOEN1_Val
                STR     R4, [R0, #EMC_STA_WOEN1_OFS]
                LDR     R4, =EMC_STA_WRD1_Val
                STR     R4, [R0, #EMC_STA_WRD1_OFS]
                LDR     R4, =EMC_STA_WPAGE1_Val
                STR     R4, [R0, #EMC_STA_WPAGE1_OFS]
                LDR     R4, =EMC_STA_WWR1_Val
                STR     R4, [R0, #EMC_STA_WWR1_OFS]
                LDR     R4, =EMC_STA_WTURN1_Val
                STR     R4, [R0, #EMC_STA_WTURN1_OFS]
                ENDIF

                IF      (EMC_STACS2_SETUP != 0)
                LDR     R4, =EMC_STA_CFG2_Val
                STR     R4, [R0, #EMC_STA_CFG2_OFS]
                LDR     R4, =EMC_STA_WWEN2_Val
                STR     R4, [R0, #EMC_STA_WWEN2_OFS]
                LDR     R4, =EMC_STA_WOEN2_Val
                STR     R4, [R0, #EMC_STA_WOEN2_OFS]
                LDR     R4, =EMC_STA_WRD2_Val
                STR     R4, [R0, #EMC_STA_WRD2_OFS]
                LDR     R4, =EMC_STA_WPAGE2_Val
                STR     R4, [R0, #EMC_STA_WPAGE2_OFS]
                LDR     R4, =EMC_STA_WWR2_Val
                STR     R4, [R0, #EMC_STA_WWR2_OFS]
                LDR     R4, =EMC_STA_WTURN2_Val
                STR     R4, [R0, #EMC_STA_WTURN2_OFS]
                ENDIF

                IF      (EMC_STACS3_SETUP != 0)
                LDR     R4, =EMC_STA_CFG3_Val
                STR     R4, [R0, #EMC_STA_CFG3_OFS]
                LDR     R4, =EMC_STA_WWEN3_Val
                STR     R4, [R0, #EMC_STA_WWEN3_OFS]
                LDR     R4, =EMC_STA_WOEN3_Val
                STR     R4, [R0, #EMC_STA_WOEN3_OFS]
                LDR     R4, =EMC_STA_WRD3_Val
                STR     R4, [R0, #EMC_STA_WRD3_OFS]
                LDR     R4, =EMC_STA_WPAGE3_Val
                STR     R4, [R0, #EMC_STA_WPAGE3_OFS]
                LDR     R4, =EMC_STA_WWR3_Val
                STR     R4, [R0, #EMC_STA_WWR3_OFS]
                LDR     R4, =EMC_STA_WTURN3_Val
                STR     R4, [R0, #EMC_STA_WTURN3_OFS]
                ENDIF

                LDR     R6, =144000               ; Number of cycles to delay
Wait_5          SUBS    R6, R6, #1                ; Delay ~10 ms @ proc clk 57.6 MHz
                BNE     Wait_5

                LDR     R4, =EMC_STA_EXT_W_Val
                LDR     R5, =EMC_STA_EXT_W_OFS
                ADD     R5, R5, R0
                STR     R4, [R5, #0]

                ENDIF   ; EMC_STATIC_SETUP 

                ENDIF   ; EMC_SETUP


; Copy Exception Vectors to Internal RAM ---------------------------------------

                IF      :DEF:RAM_INTVEC
                ADR     R8, Vectors         ; Source
                LDR     R9, =RAM_BASE       ; Destination
                LDMIA   R8!, {R0-R7}        ; Load Vectors 
                STMIA   R9!, {R0-R7}        ; Store Vectors 
                LDMIA   R8!, {R0-R7}        ; Load Handler Addresses 
                STMIA   R9!, {R0-R7}        ; Store Handler Addresses
                ENDIF


; Memory Mapping (when Interrupt Vectors are in RAM) ---------------------------

MEMMAP          EQU     0xE01FC040      ; Memory Mapping Control
                IF      :DEF:REMAP
                LDR     R0, =MEMMAP
                IF      :DEF:EXTMEM_MODE
                MOV     R1, #3
                ELIF    :DEF:RAM_MODE
                MOV     R1, #2
                ELSE
                MOV     R1, #1
                ENDIF
                STR     R1, [R0]
                ENDIF

; Enable interrupts in CPSR and start in Supervisor mode
								MSR			CPSR_c, #Mode_SVC

; Setup Stack for each mode
                LDR     R0, =Stack_Top

;  Enter Undefined Instruction Mode and set its Stack Pointer
                MSR     CPSR_c, #Mode_UND:OR:I_Bit:OR:F_Bit
                MOV     SP, R0
                SUB     R0, R0, #UND_Stack_Size

;  Enter Abort Mode and set its Stack Pointer
                MSR     CPSR_c, #Mode_ABT:OR:I_Bit:OR:F_Bit
                MOV     SP, R0
                SUB     R0, R0, #ABT_Stack_Size

;  Enter FIQ Mode and set its Stack Pointer
                MSR     CPSR_c, #Mode_FIQ:OR:I_Bit:OR:F_Bit
                MOV     SP, R0
                SUB     R0, R0, #FIQ_Stack_Size

;  Enter IRQ Mode and set its Stack Pointer
                MSR     CPSR_c, #Mode_IRQ:OR:I_Bit:OR:F_Bit
                MOV     SP, R0
                SUB     R0, R0, #IRQ_Stack_Size

;  Enter Supervisor Mode and set its Stack Pointer
                MSR     CPSR_c, #Mode_SVC:OR:I_Bit:OR:F_Bit
                MOV     SP, R0
                SUB     R0, R0, #SVC_Stack_Size

;  Enter System|User Mode and set its Stack Pointer
                MSR     CPSR_c, #Mode_SYS
                IF      :DEF:__MICROLIB

                EXPORT __initial_sp

                ELSE

                MOV     SP, R0
                SUB     SL, SP, #USR_Stack_Size
                ENDIF

		IF      :DEF:USE_FREE_RTOS
		
; Start in supervisor mode
                MSR     CPSR_c, #Mode_SVC|I_Bit|F_Bit
		
		ELSE

; Start in user mode
								MSR     CPSR_c, #Mode_USR ;|I_Bit|F_Bit
		
		ENDIF

; Enter the C code -------------------------------------------------------------

                IMPORT  __main
                LDR     R0, =__main
                BX      R0

                IF      :DEF:__MICROLIB

                EXPORT  __heap_base
                EXPORT  __heap_limit

                ELSE
; User Initial Stack & Heap
                AREA    |.text|, CODE, READONLY

                IMPORT  __use_two_region_memory
                EXPORT  __user_initial_stackheap
__user_initial_stackheap

                LDR     R0, =  Heap_Mem
                LDR     R1, =(Stack_Mem + USR_Stack_Size)
                LDR     R2, = (Heap_Mem +      Heap_Size)
                LDR     R3, = Stack_Mem
                BX      LR
                ENDIF


                END
