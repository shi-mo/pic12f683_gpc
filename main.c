/**
 * PLF (PWM LED Flasher)
 *
 * Author: Yoshifumi Shimono
 * Target device: PIC12F683
 */
#include <htc.h>

__CONFIG( \
	FOSC_INTOSCIO \	/* Use internal oscillator without CLKOUT */
	& CP_ON \	/* Protect program code */
	& WDTE_OFF \	/* Disable watch dog timer */
	& MCLRE_OFF \	/* Disable master clear reset */
);

#define PLF_REG_PWM_DUTY CCPR1L

static void
plf_clear_io(void)
{
	GPIO = 0;
	TRISIO	= 0;
}

#define PLF_OSC_4MHZ 0b110

static void
plf_init_osc(void)
{
	OSCCON = 0;
	OSCCONbits.IRCF = PLF_OSC_4MHZ;
}

#define PLF_ADC_MAX (512 - 1)
#define PLF_TMR2_PRESCALE_X16 0b11

static void
plf_init_pwm(void)
{
	PR2	= PLF_ADC_MAX;	/* PWM period */
	CCPR1L 	= 0;		/* PWM duty */
	CCP1CON	= 0;
	CCP1M3	= 1, CCP1M2 = 1; /* PWM mode */
	TMR2ON	= 0; /* Stop TMR2 until all settings are done */
	T2CONbits.T2CKPS = PLF_TMR2_PRESCALE_X16;
}

static void
plf_init(void)
{
	plf_clear_io();
	plf_init_osc();

	CMCON0	= 0;
	CM2 = 1, CM1 = 1, CM0 = 1; /* Disable comparator */

	TRISIO4	= 1; /* Use GP4 as input */
	ANSEL = 0;
	ANS3  = 1; /* Use AN3(GP4) as analog input */
	ADCS2 = 0, ADCS1 = 0, ADCS0 = 1; /* ADC clock = osc/8 =-> 2.0us */
	ADFM  = 1; /* Use right padding for ADC result */
	ADON  = 1; /* Enable ADC */
	VCFG  = 0; /* Use Vdd as Vref */
	CHS1  = 1, CHS0 = 1; /* Use ch.3 (AN3) */

	plf_init_pwm();
}

static void
plf_timer_start(void)
{
	TMR2ON = 1;
}

static unsigned int
plf_adc_read(void)
{
	ADCON0bits.GO_DONE = 1;
	while (ADCON0bits.GO_DONE) {
		/* LOOP: wait for conversion complete */
	}
	return (ADRESH<<6) & ADRESL;
}

static void
plf_truncate_adc_val(unsigned int *valp)
{
	if (*valp <= PLF_ADC_MAX) {
		/* Do nothing */
		return;
	}
	*valp = PLF_ADC_MAX;
}

int
main(int argc, char *argv[])
{
	plf_init();

	plf_timer_start();
	while (1) {
		unsigned int adc_val = 0;

		adc_val = plf_adc_read();
		plf_truncate_adc_val(&adc_val);
		PLF_REG_PWM_DUTY = adc_val;
	}
}
