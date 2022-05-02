#pragma once
#include <stdint.h>

// TODO: is this really necessary?
// define for the entire project what data type memory and registers have!
typedef uint32_t MMREG_t;

#define MMREG_STK_CTRL (MMREG_t*) 0xE000E010
#define MMREG_STK_LOAD (MMREG_t*) 0xE000E014
#define MMREG_STK_VAL (MMREG_t*) 0xE000E018
#define MMREG_STK_CALIB (MMREG_t*) 0xE000E01C
#define FLASH_ACR (MMREG_t*) 0x40023C00
#define SPI1 (MMREG_t*) 0x40013000
#define SPI1_CR1 (MMREG_t*) 0x40013000
#define SPI1_CR2 (MMREG_t*) 0x40013004
#define SPI1_SR (MMREG_t*) 0x40013008
#define SPI1_DR (MMREG_t*) 0x4001300C
#define APB1ENR (MMREG_t*) 0x40023840
#define APB2ENR (MMREG_t*) 0x40023844
#define GPIOA_MODER (MMREG_t*) 0x40020000
#define GPIOA_OSPEEDR (MMREG_t*) 0x40020008
#define GPIOA_AFRL (MMREG_t*) 0x40020020
#define GPIOA_AFRH (MMREG_t*) 0x40020024
#define PWR_CR (MMREG_t*) 0x40007000

#define NVIC_IPR_BASE (MMREG_t*) 0xE000E400
#define NVIC_ICPR_BASE (MMREG_t*) 0xE000E280

#define SCB_SHRP1 (MMREG_t*) 0xE000ED18
#define SCB_SHRP2 (MMREG_t*) 0xE000ED1C
#define SCB_SHRP3 (MMREG_t*) 0xE000ED20

#define TIM2 (MMREG_t*) 0x40000000
#define TIM3 (MMREG_t*) 0x40000400

#define I2C1 (MMREG_t*) 0x40005400
#define GPIOC (MMREG_t*) 0x40020800
#define GPIOB (MMREG_t*) 0x40020400
#define GPIOA (MMREG_t*) 0x40020000

#define REG_WIDTH 4

// Da bei der Addition einer Ganzzahl mit einem Pointer der Pointer um die Größe des Datentyps auf den er zeigt verschoben wird,
// muss man hier den Offset (wie er im Refernce Manual zu finden ist) durch die Breite des Datentyps (also des Registers) teilen.
// TODO: Zur Zeit wird angenommen, dass alle Register 4 Byte groß sind, was aber nicht ganz stimmt. Manche Register sind nur 2 Byte
// groß, sind aber trotzdem 4-Byte aligned.

// Offsets
#define TIMx_CR1 0x00 / REG_WIDTH
#define TIMx_CR2 0x08 / REG_WIDTH
#define TIMx_DIER 0x0C / REG_WIDTH
#define TIMx_SR 0x10 / REG_WIDTH
#define TIMx_CNT 0x24 / REG_WIDTH
#define TIMx_PSC 0x28 / REG_WIDTH
#define TIMx_ARR 0x2C / REG_WIDTH

#define I2C_CR1 0x00 / REG_WIDTH
#define I2C_CR2 0x04 / REG_WIDTH
#define I2C_OAR1 0x08 / REG_WIDTH
#define I2C_OAR2 0x0C / REG_WIDTH
#define I2C_DR 0x10 / REG_WIDTH
#define I2C_SR1 0x14 / REG_WIDTH
#define I2C_SR2 0x18 / REG_WIDTH
#define I2C_CCR 0x1C / REG_WIDTH
#define I2C_TRISE 0x20 / REG_WIDTH

#define GPIO_MODER 0x00 / REG_WIDTH
#define GPIO_OTYPER 0x04 / REG_WIDTH
#define GPIO_OSPEEDR 0x08 / REG_WIDTH
#define GPIO_PUPDR 0x0C / REG_WIDTH
#define GPIO_IDR 0x10 / REG_WIDTH
#define GPIO_ODR 0x14 / REG_WIDTH
#define GPIO_AFRL 0x20 / REG_WIDTH
#define GPIO_AFRH 0x24 / REG_WIDTH

#define EXTI_IMR (MMREG_t*) 0x40013C00
#define EXTI_RTSR (MMREG_t*) 0x40013C08
#define EXTI_FTSR (MMREG_t*) 0x40013C0C

// GPIO
extern MMREG_t *GPIOA_BASE;
extern MMREG_t *GPIOB_BASE;
extern MMREG_t *GPIOC_BASE;
extern MMREG_t *GPIOA_ODR;
extern MMREG_t *GPIOB_ODR;
extern MMREG_t *GPIOC_ODR;
extern MMREG_t *GPIOA_IDR;
extern MMREG_t *GPIOB_IDR;
extern MMREG_t *GPIOC_IDR;

// RCC
extern MMREG_t *AHB1ENR;
extern MMREG_t *RCC_CFGR;
extern MMREG_t *RCC_PLLCFGR;
extern MMREG_t *RCC_CR;

// MEMORY
extern const MMREG_t *MMRAM;