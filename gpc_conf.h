/*
 * Configurations
 */

#ifndef GPC_CONF_H
#define	GPC_CONF_H

/* Oscillator */
#define GPC_OSC_125KHZ  0b001
#define GPC_OSC_500KHZ  0b011
#define GPC_OSC_1MHZ    0b100
#define GPC_OSC_2MHZ    0b101
#define GPC_OSC_4MHZ    0b110
#define GPC_OSC_8MHZ    0b111

#define GPC_CONF_OSC    GPC_OSC_4MHZ

/* Prescaler */
#define GPC_TMR2_PRESCALE_X1	0b00
#define GPC_TMR2_PRESCALE_X4	0b01
#define GPC_TMR2_PRESCALE_X16	0b11

#define GPC_CONF_PRESCALE   GPC_TMR2_PRESCALE_X1

/* PWM */
#define GPC_CONF_PR2    0xFF

#endif	/* GPC_CONF_H */

