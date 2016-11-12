/*
 * gpio.h
 *
 *  Created on: 24.10.2016
 *      Author: Bernd Kreuss
 */

#ifndef SRC_GPIO_H_
#define SRC_GPIO_H_

#include <MKL25Z4.h>

#define PORT_SIM_SCG_LIST                                               \
    X(SIM_SCGC5_PORTB_MASK)                                             \
    X(SIM_SCGC5_PORTD_MASK)                                             \

#define PIN_OUTPUT_LIST                                                 \

#define PIN_OUTPUT_HIGH_LIST                                            \
    X(LED_GN,       B, 19)                                              \
    X(LED_RD,       B, 18)                                              \
    X(LED_BL,       D, 1)                                               \

#define PIN_ALL_LIST                                                    \
    PIN_OUTPUT_LIST                                                     \
    PIN_OUTPUT_HIGH_LIST                                                \

/////////////////////////////////////////////////////////////////////////

#define X(NAME, LETTER, NUMBER)                                         \
                                                                        \
    static inline void NAME ## _as_output() {                           \
        PORT##LETTER->PCR[NUMBER] = PORT_PCR_MUX(1);                    \
        FPT##LETTER->PDDR |= (1UL << NUMBER);                           \
    }                                                                   \
                                                                        \
    static inline void NAME ## _as_input() {                            \
        PORT##LETTER->PCR[NUMBER] = PORT_PCR_MUX(1);                    \
        FPT##LETTER->PDDR &= ~(1UL << NUMBER);                          \
    }                                                                   \
                                                                        \
    static inline void NAME ## _high() {                                \
        FPT##LETTER->PSOR = (1UL << NUMBER);                            \
    }                                                                   \
                                                                        \
    static inline void NAME ## _low() {                                 \
        FPT##LETTER->PCOR = (1UL << NUMBER);                            \
    }                                                                   \
                                                                        \
    static inline void NAME ## _toggle() {                              \
        FPT##LETTER->PTOR = (1UL << NUMBER);                            \
    }                                                                   \
                                                                        \

    PIN_ALL_LIST
#undef X


static inline void gpio_init(void) {
    #define X(MASK) MASK |
    SIM->SCGC5 |= PORT_SIM_SCG_LIST 0;
    #undef X

    #define X(NAME, LETTER, NUMBER) NAME##_high();
    PIN_OUTPUT_HIGH_LIST
    #undef X

    #define X(NAME, LETTER, NUMBER) NAME##_as_output();
    PIN_OUTPUT_LIST
    PIN_OUTPUT_HIGH_LIST
    #undef X

}

#endif /* SRC_GPIO_H_ */
