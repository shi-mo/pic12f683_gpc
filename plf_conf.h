/*
 * Configurations
 */

#ifndef PLF_CONF_H
#define	PLF_CONF_H

/* Oscillator */
#define PLF_OSC_125KHZ  0b001
#define PLF_OSC_500KHZ  0b011
#define PLF_OSC_1MHZ    0b100
#define PLF_OSC_2MHZ    0b101
#define PLF_OSC_4MHZ    0b110
#define PLF_OSC_8MHZ    0b111

#define PLF_CONF_OSC    PLF_OSC_1MHZ

/* Prescaler */
#define PLF_TMR2_PRESCALE_X1	0b00
#define PLF_TMR2_PRESCALE_X4	0b01
#define PLF_TMR2_PRESCALE_X16	0b11

#define PLF_CONF_PRESCALE   PLF_TMR2_PRESCALE_X16

/* PWM */
#define PLF_CONF_PR2    0x20

#endif	/* PLF_CONF_H */

