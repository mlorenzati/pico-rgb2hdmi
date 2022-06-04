#ifndef _SYSTEM_H
#define _SYSTEM_H
#include "pico.h"

// System Handler Control and State Register
#define SYSHND_CTRL (*(volatile unsigned int*) (0xE000ED24u))
// Memory Management Fault Status Register
#define NVIC_MFSR (*(volatile unsigned char*)  (0xE000ED28u))
// Bus Fault Status Register
#define NVIC_BFSR (*(volatile unsigned char*)  (0xE000ED29u))
// Usage Fault Status Register
#define NVIC_UFSR (*(volatile unsigned short*) (0xE000ED2Au))
// Hard Fault Status Register
#define NVIC_HFSR (*(volatile unsigned int*)   (0xE000ED2Cu))
// Debug Fault Status Register
#define NVIC_DFSR (*(volatile unsigned int*)   (0xE000ED30u))
// Bus Fault Manage Address Register
#define NVIC_BFAR (*(volatile unsigned int*)   (0xE000ED38u))
// Auxiliary Fault Status Register
#define NVIC_AFSR (*(volatile unsigned int*)   (0xE000ED3Cu))

//CMSIS
#define __IM                               volatile const      /*! Defines 'read only' structure member permissions */
#define __OM                               volatile            /*! Defines 'write only' structure member permissions */
#define __IOM                              volatile            /*! Defines 'read / write' structure member permissions */

typedef struct
{
  __IM  uint32_t CPUID;                  /*!< Offset: 0x000 (R/ )  CPUID Base Register */
  __IOM uint32_t ICSR;                   /*!< Offset: 0x004 (R/W)  Interrupt Control and State Register */
        uint32_t RESERVED0;
  __IOM uint32_t AIRCR;                  /*!< Offset: 0x00C (R/W)  Application Interrupt and Reset Control Register */
  __IOM uint32_t SCR;                    /*!< Offset: 0x010 (R/W)  System Control Register */
  __IOM uint32_t CCR;                    /*!< Offset: 0x014 (R/W)  Configuration Control Register */
        uint32_t RESERVED1;
  __IOM uint32_t SHP[2U];                /*!< Offset: 0x01C (R/W)  System Handlers Priority Registers. [0] is RESERVED */
  __IOM uint32_t SHCSR;                  /*!< Offset: 0x024 (R/W)  System Handler Control and State Register */
} SCB_Type;

typedef struct
{
        uint32_t RESERVED0[2U];
  __IOM uint32_t ACTLR;                  /*!< Offset: 0x008 (R/W)  Auxiliary Control Register */
} SCnSCB_Type;

#define SCB_AIRCR_VECTKEY_Pos              16U                                  /*!< SCB AIRCR: VECTKEY Position */
#define SCB_AIRCR_SYSRESETREQ_Pos          2U                                  /*!< SCB AIRCR: SYSRESETREQ Position */
#define SCB_ACTLR_DISDEFWBUF_MASK          0x2u
#define SCB_AIRCR_SYSRESETREQ_Msk          (1UL << SCB_AIRCR_SYSRESETREQ_Pos)      
#define SCB_AIRCR_VECTKEY_Msk              (0xFFFFUL << SCB_AIRCR_VECTKEY_Pos)  /*!< SCB AIRCR: VECTKEY Mask */
#define SCS_BASE                           (0xE000E000UL)                       /*!< System Control Space Base Address */
#define SCB_BASE                           (SCS_BASE +  0x0D00UL)               /*!< System Control Block Base Address */
#define SCB                                ((SCB_Type       *)     SCB_BASE)    /*!< SCB configuration struct */
#define SCnSCB                             ((SCnSCB_Type    *)     SCS_BASE)    /*!< System control Register not in SCB */
static inline void __DSB(void)
{
  __asm volatile ("dsb 0xF":::"memory");
}

static inline void system_delayed_write_disable() {
    SCnSCB->ACTLR = SCB_ACTLR_DISDEFWBUF_MASK;
}
#endif