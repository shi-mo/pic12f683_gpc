/**
 * GPC (General purpose PWM Controller)
 *
 * Author: shi-mo
 * Target device: PIC12F683
 */
#include <htc.h>
#include "gpc_conf.h"

__CONFIG( \
	FOSC_INTOSCIO \	/* Use internal oscillator without CLKOUT */
	& CP_ON \	/* Protect program code */
	& WDTE_OFF \	/* Disable watch dog timer */
	& MCLRE_OFF \	/* Disable master clear reset */
);

static void
gpc_init_clear_io(void)
{
	GPIO = 0;
	TRISIO	= 0;
}

static void
gpc_init_osc(void)
{
	OSCCON = 0;
	OSCCONbits.IRCF = GPC_CONF_OSC;
}

#define GPC_CM_DISABLE 0b111

static void
gpc_init_cm(void)
{
	CMCON0	= 0;
	CMCON0bits.CM = GPC_CM_DISABLE;
}

#define GPC_ADC_CLK_OSC8 0b001

static void
gpc_init_adc(void)
{
	ANSELbits.ADCS = GPC_ADC_CLK_OSC8;
	ADFM  = 1; /* Use right padding for ADC result */
	ADON  = 1; /* Enable ADC */
	VCFG  = 0; /* Use Vdd as Vref */
}

static void
gpc_init_an3_input(void)
{
	TRISIO4	= 1; /* Use GP4 as input */
	ANSEL = 0;
	ANS3  = 1; /* Use AN3(GP4) as analog input */
	CHS1  = 1, CHS0 = 1; /* Use ch.3 (AN3) */
}

#define GPC_REG_PWM_WIDTH	CCPR1L
#define GPC_CCP_MODE_PWM	0b1100

static void
gpc_init_pwm(void)
{
	PR2 = GPC_CONF_PR2;
	GPC_REG_PWM_WIDTH = 0;

	CCP1CON	= 0;
	CCP1CONbits.CCP1M = GPC_CCP_MODE_PWM;
	TMR2ON = 0; /* Stop TMR2 until all settings are done */
	T2CONbits.T2CKPS = GPC_CONF_PRESCALE;
}

static void
gpc_init(void)
{
	gpc_init_clear_io();
	gpc_init_osc();
	gpc_init_cm();
	gpc_init_adc();
	gpc_init_an3_input();
	gpc_init_pwm();
}

static void
gpc_timer_start(void)
{
	TMR2ON = 1;
}

#define GPC_ADC_VAL_ADRESH ((ADRESH & 0b11) << 8)

static unsigned long
gpc_adc_read(void)
{
	ADCON0bits.GO_DONE = 1;
	while (ADCON0bits.GO_DONE) {
		/* LOOP: wait for conversion complete */
	}
	return GPC_ADC_VAL_ADRESH + ADRESL;
}

#define GPC_ADC_VAL_MAX	0x3FF

static double
gpc_adc_get_rate_from(unsigned long adc_val)
{
	return (double)adc_val / GPC_ADC_VAL_MAX;
}

#define GPC_RATE_LIMIT_LOWER	0.03
#define GPC_RATE_LIMIT_HIGHER	0.97

static double
gpc_rate_correct(double rate)
{
	double corrected = 0;

	if (rate < 0) {
		/* must not happen */
		return 0;
	}

	if (corrected <= GPC_RATE_LIMIT_LOWER) {
		return 0.0;
	}
	if (GPC_RATE_LIMIT_HIGHER <= corrected) {
		return 1.0;
	}
	return corrected;
}

static void
gpc_update(void)
{
	unsigned long	adc_val	= 0;
	double		rate	= 0;

	adc_val	= gpc_adc_read();
	rate	= gpc_adc_get_rate_from(adc_val);
	rate	= gpc_rate_correct(rate);

	GPC_REG_PWM_WIDTH = (unsigned char)(rate * GPC_CONF_PR2);
}

int
main(int argc, char *argv[])
{
	gpc_init();

	gpc_timer_start();
	while (1) {
		gpc_update();
	}
}
