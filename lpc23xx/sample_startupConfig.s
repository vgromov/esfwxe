;/*****************************************************************************/
;/* <<< Use Configuration Wizard in Context Menu >>>                          */ 
;/*****************************************************************************/
;*****************************************************************************/

;// <h> Stack Configuration (Stack Sizes in Bytes)
;//   <o0> Undefined Mode      <0x0-0xFFFFFFFF:8>
;//   <o1> Supervisor Mode     <0x0-0xFFFFFFFF:8>
;//   <o2> Abort Mode          <0x0-0xFFFFFFFF:8>
;//   <o3> Fast Interrupt Mode <0x0-0xFFFFFFFF:8>
;//   <o4> Interrupt Mode      <0x0-0xFFFFFFFF:8>
;//   <o5> User/System Mode    <0x0-0xFFFFFFFF:8>
;// </h>

UND_Stack_Size  EQU     0x00000000
SVC_Stack_Size  EQU     0x00000080
ABT_Stack_Size  EQU     0x00000000
FIQ_Stack_Size  EQU     0x00000000
IRQ_Stack_Size  EQU     0x00000100
USR_Stack_Size  EQU     0x00000400

;// <h> Heap Configuration
;//   <o>  Heap Size (in Bytes) <0x0-0xFFFFFFFF>
;// </h>

Heap_Size       EQU     0x00000000

;//     External Memory Controller Setup (EMC) ---------------------------------
;// <e> External Memory Controller Setup (EMC)
EMC_SETUP           EQU 0

;//   <h> EMC Control Register (EMCControl)
;//     <i> Controls operation of the memory controller
;//     <o0.2> L: Low-power mode enable
;//     <o0.1> M: Address mirror enable
;//     <o0.0> E: EMC enable
;//   </h>
EMC_CTRL_Val        EQU 0x00000001

;//   <h> EMC Configuration Register (EMCConfig)
;//     <i> Configures operation of the memory controller
;//     <o0.8> CCLK: CLKOUT ratio
;//       <0=> 1:1
;//       <1=> 1:2
;//     <o0.0> Endian mode
;//       <0=> Little-endian
;//       <1=> Big-endian
;//   </h>
EMC_CONFIG_Val      EQU 0x00000000

;//       Dynamic Memory Interface Setup ---------------------------------------
;//   <e> Dynamic Memory Interface Setup
EMC_DYNAMIC_SETUP   EQU 1

;//     <h> Dynamic Memory Refresh Timer Register (EMCDynamicRefresh)
;//       <i> Configures dynamic memory refresh operation
;//       <o0.0..10> REFRESH: Refresh timer <0x000-0x7FF>
;//         <i> 0 = refresh disabled, 0x01-0x7FF: value * 16 CCLKS
;//     </h>
EMC_DYN_RFSH_Val    EQU 0x0000001C

;//     <h> Dynamic Memory Read Configuration Register (EMCDynamicReadConfig)
;//       <i> Configures the dynamic memory read strategy
;//       <o0.0..1> RD: Read data strategy
;//                  <0=> Clock out delayed strategy
;//         <1=> Command delayed strategy
;//         <2=> Command delayed strategy plus one clock cycle
;//         <3=> Command delayed strategy plus two clock cycles
;//     </h>
EMC_DYN_RD_CFG_Val  EQU 0x00000001

;//     <h> Dynamic Memory Timings
;//       <h> Dynamic Memory Percentage Command Period Register (EMCDynamictRP)
;//         <o0.0..3> tRP: Precharge command period <1-16> <#-1>
;//           <i> The delay is in EMCCLK cycles
;//           <i> This value is normally found in SDRAM data sheets as tRP
;//       </h>
;//       <h> Dynamic Memory Active to Precharge Command Period Register (EMCDynamictRAS)
;//         <o1.0..3> tRAS: Active to precharge command period <1-16> <#-1>
;//           <i> The delay is in EMCCLK cycles
;//           <i> This value is normally found in SDRAM data sheets as tRAS
;//       </h>
;//       <h> Dynamic Memory Self-refresh Exit Time Register (EMCDynamictSREX)
;//         <o2.0..3> tSREX: Self-refresh exit time <1-16> <#-1>
;//           <i> The delay is in CCLK cycles
;//           <i> This value is normally found in SDRAM data sheets as tSREX, 
;//           <i> for devices without this parameter you use the same value as tXSR
;//       </h>
;//       <h> Dynamic Memory Last Data Out to Active Time Register (EMCDynamictAPR)
;//         <o3.0..3> tAPR: Last-data-out to active command time <1-16> <#-1>
;//           <i> The delay is in CCLK cycles
;//           <i> This value is normally found in SDRAM data sheets as tAPR
;//       </h>
;//       <h> Dynamic Memory Data-in to Active Command Time Register (EMCDynamictDAL)
;//         <o4.0..3> tDAL: Data-in to active command time <1-16> <#-1>
;//           <i> The delay is in CCLK cycles
;//           <i> This value is normally found in SDRAM data sheets as tDAL or tAPW
;//       </h>
;//       <h> Dynamic Memory Write Recovery Time Register (EMCDynamictWR)
;//         <o5.0..3> tWR: Write recovery time <1-16> <#-1>
;//           <i> The delay is in CCLK cycles
;//           <i> This value is normally found in SDRAM data sheets as tWR, tDPL, tRWL, or tRDL
;//       </h>
;//       <h> Dynamic Memory Active to Active Command Period Register (EMCDynamictRC)
;//         <o6.0..4> tRC: Active to active command period <1-32> <#-1>
;//           <i> The delay is in CCLK cycles
;//           <i> This value is normally found in SDRAM data sheets as tRC
;//       </h>
;//       <h> Dynamic Memory Auto-refresh Period Register (EMCDynamictRFC)
;//         <o7.0..4> tRFC: Auto-refresh period and auto-refresh to active command period <1-32> <#-1>
;//           <i> The delay is in CCLK cycles
;//           <i> This value is normally found in SDRAM data sheets as tRFC or  tRC
;//       </h>
;//       <h> Dynamic Memory Exit Self-refresh Register (EMCDynamictXSR)
;//         <o8.0..4> tXSR: Exit self-refresh to active command time <1-32> <#-1>
;//           <i> The delay is in CCLK cycles
;//           <i> This value is normally found in SDRAM data sheets as tXSR
;//       </h>
;//       <h> Dynamic Memory Active Bank A to Active Bank B Time Register (EMCDynamicRRD)
;//         <o9.0..3> tRRD: Active bank A to active bank B latency <1-16> <#-1>
;//           <i> The delay is in CCLK cycles
;//           <i> This value is normally found in SDRAM data sheets as tRRD
;//       </h>
;//       <h> Dynamic Memory Load Mode Register to Active Command Time (EMCDynamictMRD)
;//         <o10.0..3> tMRD: Load mode register to active command time <1-16> <#-1>
;//           <i> The delay is in CCLK cycles
;//           <i> This value is normally found in SDRAM data sheets as tMRD or tRSA
;//       </h>
;//     </h>
EMC_DYN_RP_Val      EQU 0x00000002
EMC_DYN_RAS_Val     EQU 0x00000003
EMC_DYN_SREX_Val    EQU 0x00000007
EMC_DYN_APR_Val     EQU 0x00000002
EMC_DYN_DAL_Val     EQU 0x00000005
EMC_DYN_WR_Val      EQU 0x00000001
EMC_DYN_RC_Val      EQU 0x00000005
EMC_DYN_RFC_Val     EQU 0x00000005
EMC_DYN_XSR_Val     EQU 0x00000007
EMC_DYN_RRD_Val     EQU 0x00000001
EMC_DYN_MRD_Val     EQU 0x00000002

;//     <e> Configure External Bus Behaviour for Dynamic CS0 Area
EMC_DYNCS0_SETUP    EQU 1

;//       <h> Dynamic Memory Configuration Register (EMCDynamicConfig0)
;//         <i> Defines the configuration information for the dynamic memory CS0
;//         <o0.20> P: Write protect
;//         <o0.19> B: Buffer enable
;//         <o0.14> AM 14: External bus data width
;//           <0=> 16 bit
;//           <1=> 32 bit
;//         <o0.12> AM 12: External bus memory type
;//           <0=> High-performance
;//           <1=> Low-power SDRAM
;//         <o0.7..11> AM 11..7: External bus address mapping (Row, Bank, Column)
;//           <0x00=> 16 Mb = 2MB (2Mx8), 2 banks, row length = 11, column length = 9
;//           <0x01=> 16 Mb = 2MB (1Mx16), 2 banks, row length = 11, column length = 8
;//           <0x04=> 64 Mb = 8MB (8Mx8), 4 banks, row length = 12, column length = 9
;//           <0x05=> 64 Mb = 8MB (4Mx16), 4 banks, row length = 12, column length = 8
;//           <0x08=> 128 Mb = 16MB (16Mx8), 4 banks, row length = 12, column length = 10
;//           <0x09=> 128 Mb = 16MB (8Mx16), 4 banks, row length = 12, column length = 9
;//           <0x0C=> 256 Mb = 32MB (32Mx8), 4 banks, row length = 13, column length = 10
;//           <0x0D=> 256 Mb = 32MB (16Mx16), 4 banks, row length = 13, column length = 9
;//           <0x10=> 512 Mb = 64MB (64Mx8), 4 banks, row length = 13, column length = 11
;//           <0x11=> 512 Mb = 64MB (32Mx16), 4 banks, row length = 13, column length = 10
;//         <o0.3..4> MD: Memory device
;//           <0=> SDRAM
;//           <1=> Low-power SDRAM
;//           <2=> Micron SyncFlash
;//       </h>
EMC_DYN_CFG0_Val    EQU 0x00080680

;//       <h> Dynamic Memory RAS & CAS Delay register (EMCDynamicRASCAS0)
;//         <i> Controls the RAS and CAS latencies for the dynamic memory CS0
;//         <o0.8..9> CAS: CAS latency
;//           <1=> One CCLK cycle
;//           <2=> Two CCLK cycles
;//           <3=> Three CCLK cycles
;//         <o0.0..1> RAS: RAS latency (active to read/write delay)
;//           <1=> One CCLK cycle
;//           <2=> Two CCLK cycles
;//           <3=> Three CCLK cycles
;//       </h>
EMC_DYN_RASCAS0_Val EQU 0x00000303

;//     </e> End of Dynamic Setup for CS0 Area


;//     <e> Configure External Bus Behaviour for Dynamic CS1 Area
EMC_DYNCS1_SETUP    EQU 0

;//       <h> Dynamic Memory Configuration Register (EMCDynamicConfig1)
;//         <i> Defines the configuration information for the dynamic memory CS1
;//         <o0.20> P: Write protect
;//         <o0.19> B: Buffer enable
;//         <o0.14> AM 14: External bus data width
;//           <0=> 16 bit
;//           <1=> 32 bit
;//         <o0.12> AM 12: External bus memory type
;//           <0=> High-performance
;//           <1=> Low-power SDRAM
;//         <o0.7..11> AM 11..7: External bus address mapping (Row, Bank, Column)
;//           <0x00=> 16 Mb = 2MB (2Mx8), 2 banks, row length = 11, column length = 9
;//           <0x01=> 16 Mb = 2MB (1Mx16), 2 banks, row length = 11, column length = 8
;//           <0x04=> 64 Mb = 8MB (8Mx8), 4 banks, row length = 12, column length = 9
;//           <0x05=> 64 Mb = 8MB (4Mx16), 4 banks, row length = 12, column length = 8
;//           <0x08=> 128 Mb = 16MB (16Mx8), 4 banks, row length = 12, column length = 10
;//           <0x09=> 128 Mb = 16MB (8Mx16), 4 banks, row length = 12, column length = 9
;//           <0x0C=> 256 Mb = 32MB (32Mx8), 4 banks, row length = 13, column length = 10
;//           <0x0D=> 256 Mb = 32MB (16Mx16), 4 banks, row length = 13, column length = 9
;//           <0x10=> 512 Mb = 64MB (64Mx8), 4 banks, row length = 13, column length = 11
;//           <0x11=> 512 Mb = 64MB (32Mx16), 4 banks, row length = 13, column length = 10
;//         <o0.3..4> MD: Memory device
;//           <0=> SDRAM
;//           <1=> Low-power SDRAM
;//           <2=> Micron SyncFlash
;//       </h>
EMC_DYN_CFG1_Val    EQU 0x00000000

;//       <h> Dynamic Memory RAS & CAS Delay register (EMCDynamicRASCAS1)
;//         <i> Controls the RAS and CAS latencies for the dynamic memory CS1
;//         <o0.8..9> CAS: CAS latency
;//           <1=> One CCLK cycle
;//           <2=> Two CCLK cycles
;//           <3=> Three CCLK cycles
;//         <o0.0..1> RAS: RAS latency (active to read/write delay)
;//           <1=> One CCLK cycle
;//           <2=> Two CCLK cycles
;//           <3=> Three CCLK cycles
;//       </h>
EMC_DYN_RASCAS1_Val EQU 0x00000303

;//     </e> End of Dynamic Setup for CS1 Area

;//     <e> Configure External Bus Behaviour for Dynamic CS2 Area
EMC_DYNCS2_SETUP    EQU 0

;//       <h> Dynamic Memory Configuration Register (EMCDynamicConfig2)
;//         <i> Defines the configuration information for the dynamic memory CS2
;//         <o0.20> P: Write protect
;//         <o0.19> B: Buffer enable
;//         <o0.14> AM 14: External bus data width
;//           <0=> 16 bit
;//           <1=> 32 bit
;//         <o0.12> AM 12: External bus memory type
;//           <0=> High-performance
;//           <1=> Low-power SDRAM
;//         <o0.7..11> AM 11..7: External bus address mapping (Row, Bank, Column)
;//           <0x00=> 16 Mb = 2MB (2Mx8), 2 banks, row length = 11, column length = 9
;//           <0x01=> 16 Mb = 2MB (1Mx16), 2 banks, row length = 11, column length = 8
;//           <0x04=> 64 Mb = 8MB (8Mx8), 4 banks, row length = 12, column length = 9
;//           <0x05=> 64 Mb = 8MB (4Mx16), 4 banks, row length = 12, column length = 8
;//           <0x08=> 128 Mb = 16MB (16Mx8), 4 banks, row length = 12, column length = 10
;//           <0x09=> 128 Mb = 16MB (8Mx16), 4 banks, row length = 12, column length = 9
;//           <0x0C=> 256 Mb = 32MB (32Mx8), 4 banks, row length = 13, column length = 10
;//           <0x0D=> 256 Mb = 32MB (16Mx16), 4 banks, row length = 13, column length = 9
;//           <0x10=> 512 Mb = 64MB (64Mx8), 4 banks, row length = 13, column length = 11
;//           <0x11=> 512 Mb = 64MB (32Mx16), 4 banks, row length = 13, column length = 10
;//         <o0.3..4> MD: Memory device
;//           <0=> SDRAM
;//           <1=> Low-power SDRAM
;//           <2=> Micron SyncFlash
;//       </h>
EMC_DYN_CFG2_Val    EQU 0x00000000

;//       <h> Dynamic Memory RAS & CAS Delay register (EMCDynamicRASCAS2)
;//         <i> Controls the RAS and CAS latencies for the dynamic memory CS2
;//         <o0.8..9> CAS: CAS latency
;//           <1=> One CCLK cycle
;//           <2=> Two CCLK cycles
;//           <3=> Three CCLK cycles
;//         <o0.0..1> RAS: RAS latency (active to read/write delay)
;//           <1=> One CCLK cycle
;//           <2=> Two CCLK cycles
;//           <3=> Three CCLK cycles
;//       </h>
EMC_DYN_RASCAS2_Val EQU 0x00000303

;//     </e> End of Dynamic Setup for CS2 Area

;//     <e> Configure External Bus Behaviour for Dynamic CS3 Area
EMC_DYNCS3_SETUP    EQU 0

;//       <h> Dynamic Memory Configuration Register (EMCDynamicConfig3)
;//         <i> Defines the configuration information for the dynamic memory CS3
;//         <o0.20> P: Write protect
;//         <o0.19> B: Buffer enable
;//         <o0.14> AM 14: External bus data width
;//           <0=> 16 bit
;//           <1=> 32 bit
;//         <o0.12> AM 12: External bus memory type
;//           <0=> High-performance
;//           <1=> Low-power SDRAM
;//         <o0.7..11> AM 11..7: External bus address mapping (Row, Bank, Column)
;//           <0x00=> 16 Mb = 2MB (2Mx8), 2 banks, row length = 11, column length = 9
;//           <0x01=> 16 Mb = 2MB (1Mx16), 2 banks, row length = 11, column length = 8
;//           <0x04=> 64 Mb = 8MB (8Mx8), 4 banks, row length = 12, column length = 9
;//           <0x05=> 64 Mb = 8MB (4Mx16), 4 banks, row length = 12, column length = 8
;//           <0x08=> 128 Mb = 16MB (16Mx8), 4 banks, row length = 12, column length = 10
;//           <0x09=> 128 Mb = 16MB (8Mx16), 4 banks, row length = 12, column length = 9
;//           <0x0C=> 256 Mb = 32MB (32Mx8), 4 banks, row length = 13, column length = 10
;//           <0x0D=> 256 Mb = 32MB (16Mx16), 4 banks, row length = 13, column length = 9
;//           <0x10=> 512 Mb = 64MB (64Mx8), 4 banks, row length = 13, column length = 11
;//           <0x11=> 512 Mb = 64MB (32Mx16), 4 banks, row length = 13, column length = 10
;//         <o0.3..4> MD: Memory device
;//           <0=> SDRAM
;//           <1=> Low-power SDRAM
;//           <2=> Micron SyncFlash
;//       </h>
EMC_DYN_CFG3_Val    EQU 0x00000000

;//       <h> Dynamic Memory RAS & CAS Delay register (EMCDynamicRASCAS3)
;//         <i> Controls the RAS and CAS latencies for the dynamic memory CS3
;//         <o0.8..9> CAS: CAS latency
;//           <1=> One CCLK cycle
;//           <2=> Two CCLK cycles
;//           <3=> Three CCLK cycles
;//         <o0.0..1> RAS: RAS latency (active to read/write delay)
;//           <1=> One CCLK cycle
;//           <2=> Two CCLK cycles
;//           <3=> Three CCLK cycles
;//       </h>
EMC_DYN_RASCAS3_Val EQU 0x00000303

;//     </e> End of Dynamic Setup for CS3 Area

;//   </e> End of Dynamic Setup

;//       Static Memory Interface Setup ----------------------------------------
;//   <e> Static Memory Interface Setup
EMC_STATIC_SETUP    EQU 1

;//         Configure External Bus Behaviour for Static CS0 Area ---------------
;//     <e> Configure External Bus Behaviour for Static CS0 Area
EMC_STACS0_SETUP    EQU 1

;//       <h> Static Memory Configuration Register (EMCStaticConfig0)
;//         <i> Defines the configuration information for the static memory CS0
;//         <o0.20> WP: Write protect
;//         <o0.19> B: Buffer enable
;//         <o0.8> EW: Extended wait enable
;//         <o0.7> PB: Byte lane state
;//           <0=> For reads BLSn are HIGH, for writes BLSn are LOW
;//           <1=> For reads BLSn are LOW, for writes BLSn are LOW
;//         <o0.6> PC: Chip select polarity
;//           <0=> Active LOW chip select
;//           <1=> Active HIGH chip select
;//         <o0.3> PM: Page mode enable
;//         <o0.0..1> MW: Memory width
;//           <0=> 8 bit
;//           <1=> 16 bit
;//           <2=> 32 bit
;//       </h>
EMC_STA_CFG0_Val    EQU 0x00000081

;//       <h> Static Memory Write Enable Delay Register (EMCStaticWaitWen0)
;//         <i> Selects the delay from CS0 to write enable
;//         <o.0..3> WAITWEN: Wait write enable <1-16> <#-1>
;//           <i> The delay is in CCLK cycles
;//       </h>
EMC_STA_WWEN0_Val   EQU 0x00000002

;//       <h> Static Memory Output Enable Delay register (EMCStaticWaitOen0)
;//         <i> Selects the delay from CS0 or address change, whichever is later, to output enable
;//         <o.0..3> WAITOEN: Wait output enable <0-15>
;//           <i> The delay is in CCLK cycles
;//       </h>
EMC_STA_WOEN0_Val   EQU 0x00000002
                                      
;//       <h> Static Memory Read Delay Register (EMCStaticWaitRd0)
;//         <i> Selects the delay from CS0 to a read access
;//         <o.0..4> WAITRD: Non-page mode read wait states or asynchronous page mode read first access wait states <1-32> <#-1>
;//           <i> The delay is in CCLK cycles
;//       </h>
EMC_STA_WRD0_Val    EQU 0x0000001F

;//       <h> Static Memory Page Mode Read Delay Register (EMCStaticWaitPage0)
;//         <i> Selects the delay for asynchronous page mode sequential accesses for CS0
;//         <o.0..4> WAITPAGE: Asynchronous page mode read after the first read wait states <1-32> <#-1>
;//           <i> The delay is in CCLK cycles
;//       </h>
EMC_STA_WPAGE0_Val  EQU 0x0000001F

;//       <h> Static Memory Write Delay Register (EMCStaticWaitWr0)
;//         <i> Selects the delay from CS0 to a write access
;//         <o.0..4> WAITWR: Write wait states <2-33> <#-2>
;//           <i> The delay is in CCLK cycles
;//       </h>
EMC_STA_WWR0_Val    EQU 0x0000001F

;//       <h> Static Memory Turn Round Delay Register (EMCStaticWaitTurn0)
;//         <i> Selects the number of bus turnaround cycles for CS0
;//         <o.0..4> WAITTURN: Bus turnaround cycles <1-16> <#-1>
;//           <i> The delay is in CCLK cycles
;//       </h>
EMC_STA_WTURN0_Val  EQU 0x0000000F

;//     </e> End of Static Setup for Static CS0 Area

;//         Configure External Bus Behaviour for Static CS1 Area ---------------
;//     <e> Configure External Bus Behaviour for Static CS1 Area
EMC_STACS1_SETUP    EQU 0

;//       <h> Static Memory Configuration Register (EMCStaticConfig1)
;//         <i> Defines the configuration information for the static memory CS1
;//         <o0.20> WP: Write protect
;//         <o0.19> B: Buffer enable
;//         <o0.8> EW: Extended wait enable
;//         <o0.7> PB: Byte lane state
;//           <0=> For reads BLSn are HIGH, for writes BLSn are LOW
;//           <1=> For reads BLSn are LOW, for writes BLSn are LOW
;//         <o0.6> PC: Chip select polarity
;//           <0=> Active LOW chip select
;//           <1=> Active HIGH chip select
;//         <o0.3> PM: Page mode enable
;//         <o0.0..1> MW: Memory width
;//           <0=> 8 bit
;//           <1=> 16 bit
;//           <2=> 32 bit
;//       </h>
EMC_STA_CFG1_Val    EQU 0x00000000

;//       <h> Static Memory Write Enable Delay Register (EMCStaticWaitWen1)
;//         <i> Selects the delay from CS1 to write enable
;//         <o.0..3> WAITWEN: Wait write enable <1-16> <#-1>
;//           <i> The delay is in CCLK cycles
;//       </h>
EMC_STA_WWEN1_Val   EQU 0x00000000

;//       <h> Static Memory Output Enable Delay register (EMCStaticWaitOen1)
;//         <i> Selects the delay from CS1 or address change, whichever is later, to output enable
;//         <o.0..3> WAITOEN: Wait output enable <0-15>
;//           <i> The delay is in CCLK cycles
;//       </h>
EMC_STA_WOEN1_Val   EQU 0x00000000
                                      
;//       <h> Static Memory Read Delay Register (EMCStaticWaitRd1)
;//         <i> Selects the delay from CS1 to a read access
;//         <o.0..4> WAITRD: Non-page mode read wait states or asynchronous page mode read first access wait states <1-32> <#-1>
;//           <i> The delay is in CCLK cycles
;//       </h>
EMC_STA_WRD1_Val    EQU 0x0000001F

;//       <h> Static Memory Page Mode Read Delay Register (EMCStaticWaitPage0)
;//         <i> Selects the delay for asynchronous page mode sequential accesses for CS1
;//         <o.0..4> WAITPAGE: Asynchronous page mode read after the first read wait states <1-32> <#-1>
;//           <i> The delay is in CCLK cycles
;//       </h>
EMC_STA_WPAGE1_Val  EQU 0x0000001F

;//       <h> Static Memory Write Delay Register (EMCStaticWaitWr1)
;//         <i> Selects the delay from CS1 to a write access
;//         <o.0..4> WAITWR: Write wait states <2-33> <#-2>
;//           <i> The delay is in CCLK cycles
;//       </h>
EMC_STA_WWR1_Val    EQU  0x0000001F

;//       <h> Static Memory Turn Round Delay Register (EMCStaticWaitTurn1)
;//         <i> Selects the number of bus turnaround cycles for CS1
;//         <o.0..4> WAITTURN: Bus turnaround cycles <1-16> <#-1>
;//           <i> The delay is in CCLK cycles
;//       </h>
EMC_STA_WTURN1_Val  EQU 0x0000000F

;//     </e> End of Static Setup for Static CS1 Area

;//         Configure External Bus Behaviour for Static CS2 Area ---------------
;//     <e> Configure External Bus Behaviour for Static CS2 Area
EMC_STACS2_SETUP    EQU 0

;//       <h> Static Memory Configuration Register (EMCStaticConfig2)
;//         <i> Defines the configuration information for the static memory CS2
;//         <o0.20> WP: Write protect
;//         <o0.19> B: Buffer enable
;//         <o0.8> EW: Extended wait enable
;//         <o0.7> PB: Byte lane state
;//           <0=> For reads BLSn are HIGH, for writes BLSn are LOW
;//           <1=> For reads BLSn are LOW, for writes BLSn are LOW
;//         <o0.6> PC: Chip select polarity
;//           <0=> Active LOW chip select
;//           <1=> Active HIGH chip select
;//         <o0.3> PM: Page mode enable
;//         <o0.0..1> MW: Memory width
;//           <0=> 8 bit
;//           <1=> 16 bit
;//           <2=> 32 bit
;//       </h>
EMC_STA_CFG2_Val    EQU 0x00000000

;//       <h> Static Memory Write Enable Delay Register (EMCStaticWaitWen2)
;//         <i> Selects the delay from CS2 to write enable
;//         <o.0..3> WAITWEN: Wait write enable <1-16> <#-1>
;//           <i> The delay is in CCLK cycles
;//       </h>
EMC_STA_WWEN2_Val   EQU 0x00000000

;//       <h> Static Memory Output Enable Delay register (EMCStaticWaitOen2)
;//         <i> Selects the delay from CS2 or address change, whichever is later, to output enable
;//         <o.0..3> WAITOEN: Wait output enable <0-15>
;//           <i> The delay is in CCLK cycles
;//       </h>
EMC_STA_WOEN2_Val   EQU 0x00000000
                                      
;//       <h> Static Memory Read Delay Register (EMCStaticWaitRd2)
;//         <i> Selects the delay from CS2 to a read access
;//         <o.0..4> WAITRD: Non-page mode read wait states or asynchronous page mode read first access wait states <1-32> <#-1>
;//           <i> The delay is in CCLK cycles
;//       </h>
EMC_STA_WRD2_Val    EQU 0x0000001F

;//       <h> Static Memory Page Mode Read Delay Register (EMCStaticWaitPage2)
;//         <i> Selects the delay for asynchronous page mode sequential accesses for CS2
;//         <o.0..4> WAITPAGE: Asynchronous page mode read after the first read wait states <1-32> <#-1>
;//           <i> The delay is in CCLK cycles
;//       </h>
EMC_STA_WPAGE2_Val  EQU 0x0000001F

;//       <h> Static Memory Write Delay Register (EMCStaticWaitWr2)
;//         <i> Selects the delay from CS2 to a write access
;//         <o.0..4> WAITWR: Write wait states <2-33> <#-2>
;//           <i> The delay is in CCLK cycles
;//       </h>
EMC_STA_WWR2_Val    EQU 0x0000001F

;//       <h> Static Memory Turn Round Delay Register (EMCStaticWaitTurn2)
;//         <i> Selects the number of bus turnaround cycles for CS2
;//         <o.0..4> WAITTURN: Bus turnaround cycles <1-16> <#-1>
;//           <i> The delay is in CCLK cycles
;//       </h>
EMC_STA_WTURN2_Val  EQU 0x0000000F

;//     </e> End of Static Setup for Static CS2 Area

;//         Configure External Bus Behaviour for Static CS3 Area ---------------
;//     <e> Configure External Bus Behaviour for Static CS3 Area
EMC_STACS3_SETUP    EQU 0

;//       <h> Static Memory Configuration Register (EMCStaticConfig3)
;//         <i> Defines the configuration information for the static memory CS3
;//         <o0.20> WP: Write protect
;//         <o0.19> B: Buffer enable
;//         <o0.8> EW: Extended wait enable
;//         <o0.7> PB: Byte lane state
;//           <0=> For reads BLSn are HIGH, for writes BLSn are LOW
;//           <1=> For reads BLSn are LOW, for writes BLSn are LOW
;//         <o0.6> PC: Chip select polarity
;//           <0=> Active LOW chip select
;//           <1=> Active HIGH chip select
;//         <o0.3> PM: Page mode enable
;//         <o0.0..1> MW: Memory width
;//           <0=> 8 bit
;//           <1=> 16 bit
;//           <2=> 32 bit
;//       </h>
EMC_STA_CFG3_Val    EQU 0x00000000

;//       <h> Static Memory Write Enable Delay Register (EMCStaticWaitWen3)
;//         <i> Selects the delay from CS3 to write enable
;//         <o.0..3> WAITWEN: Wait write enable <1-16> <#-1>
;//           <i> The delay is in CCLK cycles
;//       </h>
EMC_STA_WWEN3_Val   EQU 0x00000000

;//       <h> Static Memory Output Enable Delay register (EMCStaticWaitOen3)
;//         <i> Selects the delay from CS3 or address change, whichever is later, to output enable
;//         <o.0..3> WAITOEN: Wait output enable <0-15>
;//           <i> The delay is in CCLK cycles
;//       </h>
EMC_STA_WOEN3_Val   EQU 0x00000000
                                      
;//       <h> Static Memory Read Delay Register (EMCStaticWaitRd3)
;//         <i> Selects the delay from CS3 to a read access
;//         <o.0..4> WAITRD: Non-page mode read wait states or asynchronous page mode read first access wait states <1-32> <#-1>
;//           <i> The delay is in CCLK cycles
;//       </h>
EMC_STA_WRD3_Val    EQU 0x0000001F

;//       <h> Static Memory Page Mode Read Delay Register (EMCStaticWaitPage3)
;//         <i> Selects the delay for asynchronous page mode sequential accesses for CS3
;//         <o.0..4> WAITPAGE: Asynchronous page mode read after the first read wait states <1-32> <#-1>
;//           <i> The delay is in CCLK cycles
;//       </h>
EMC_STA_WPAGE3_Val  EQU 0x0000001F

;//       <h> Static Memory Write Delay Register (EMCStaticWaitWr3)
;//         <i> Selects the delay from CS3 to a write access
;//         <o.0..4> WAITWR: Write wait states <2-33> <#-2>
;//           <i> The delay is in CCLK cycles
;//       </h>
EMC_STA_WWR3_Val    EQU 0x0000001F

;//       <h> Static Memory Turn Round Delay Register (EMCStaticWaitTurn3)
;//         <i> Selects the number of bus turnaround cycles for CS3
;//         <o.0..4> WAITTURN: Bus turnaround cycles <1-16> <#-1>
;//           <i> The delay is in CCLK cycles
;//       </h>
EMC_STA_WTURN3_Val  EQU 0x0000000F

;//     </e> End of Static Setup for Static CS3 Area

;//     <h> Static Memory Extended Wait Register (EMCStaticExtendedWait)
;//       <i> Time long static memory read and write transfers
;//       <o.0..9> EXTENDEDWAIT: Extended wait time out <0-1023>
;//         <i> The delay is in (16 * CCLK) cycles
;//     </h>
EMC_STA_EXT_W_Val   EQU 0x00000000

;//   </e> End of Static Setup

;// </e> End of EMC Setup

								END
