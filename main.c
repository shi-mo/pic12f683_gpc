/**
 * PLF (PWM LED Flasher)
 *
 * Author: Yoshifumi Shimono
 * Target device: PIC12F683
 */
#include <htc.h>
#include "plf_conf.h"

__CONFIG( \
	FOSC_INTOSCIO \	/* Use internal oscillator without CLKOUT */
	& CP_ON \	/* Protect program code */
	& WDTE_OFF \	/* Disable watch dog timer */
	& MCLRE_OFF \	/* Disable master clear reset */
);

static void
plf_init_clear_io(void)
{
	GPIO = 0;
	TRISIO	= 0;
}

static void
plf_init_osc(void)
{
	OSCCON = 0;
	OSCCONbits.IRCF = PLF_CONF_OSC;
}

#define PLF_CM_DISABLE 0b111

static void
plf_init_cm(void)
{
	CMCON0	= 0;
	CMCON0bits.CM = PLF_CM_DISABLE;
}

#define PLF_ADC_CLK_OSC8 0b001

static void
plf_init_adc(void)
{
	ANSELbits.ADCS = PLF_ADC_CLK_OSC8;
	ADFM  = 1; /* Use right padding for ADC result */
	ADON  = 1; /* Enable ADC */
	VCFG  = 0; /* Use Vdd as Vref */
}

static void
plf_init_an3_input(void)
{
	TRISIO4	= 1; /* Use GP4 as input */
	ANSEL = 0;
	ANS3  = 1; /* Use AN3(GP4) as analog input */
	CHS1  = 1, CHS0 = 1; /* Use ch.3 (AN3) */
}

#define PLF_REG_PWM_WIDTH	CCPR1L
#define PLF_CCP_MODE_PWM	0b1100

static void
plf_init_pwm(void)
{
	PR2 = PLF_CONF_PR2;
	PLF_REG_PWM_WIDTH = 0;

	CCP1CON	= 0;
	CCP1CONbits.CCP1M = PLF_CCP_MODE_PWM;
	TMR2ON = 0; /* Stop TMR2 until all settings are done */
	T2CONbits.T2CKPS = PLF_CONF_PRESCALE;
}

static void
plf_init(void)
{
	plf_init_clear_io();
	plf_init_osc();
	plf_init_cm();
	plf_init_adc();
	plf_init_an3_input();
	plf_init_pwm();
}

static void
plf_timer_start(void)
{
	TMR2ON = 1;
}

#define PLF_ADC_VAL_ADRESH ((ADRESH & 0b11) << 8)

static unsigned long
plf_adc_read(void)
{
	ADCON0bits.GO_DONE = 1;
	while (ADCON0bits.GO_DONE) {
		/* LOOP: wait for conversion complete */
	}
	return PLF_ADC_VAL_ADRESH + ADRESL;
}

#define PLF_ADC_VAL_MAX	0x3FF

static double
plf_adc_get_rate_from(unsigned long adc_val)
{
	return (double)adc_val / PLF_ADC_VAL_MAX;
}

#define PLF_RATE_LIMIT_LOWER	0.03
#define PLF_RATE_LIMIT_HIGHER	0.97

static double
plf_rate_correct(double rate)
{
	double corrected = 0;

	if (rate < 0) {
		/* must not happen */
		return 0;
	}

	if (corrected <= PLF_RATE_LIMIT_LOWER) {
		return 0.0;
	}
	if (PLF_RATE_LIMIT_HIGHER <= corrected) {
		return 1.0;
	}
	return corrected;
}

static void
plf_update(void)
{
	unsigned long	adc_val	= 0;
	double		rate	= 0;

	adc_val	= plf_adc_read();
	rate	= plf_adc_get_rate_from(adc_val);
	rate	= plf_rate_correct(rate);

	PLF_REG_PWM_WIDTH = (unsigned char)(rate * PLF_CONF_PR2);
}

int
main(int argc, char *argv[])
{
	plf_init();

	plf_timer_start();
	while (1) {
		plf_update();
	}
}
