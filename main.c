/**
 * PLF (PWM LED Flasher)
 *
 * Author: Yoshifumi Shimono
 * Target device: PIC12F683
 */
#include <htc.h>

__CONFIG( \
	INTIO \		/* Use internal oscillator without CLKOUT */
	& PROTECT \	/* Protect program code */
	& WDTDIS \	/* Disable watch dog timer */
	& MCLRDIS \	/* Disable master clear reset */
);

#define PLF_ADC_MAX (512 - 1)
#define PLF_PWM_DUTY CCPR1L

static void
plf_init(void)
{
	GPIO = 0; /* first, set all output as Low */

	OSCCON = 0;
	IRCF2 = 1, IRCF1 = 1, IRCF0 = 0; /* 4MHz oscillation (default) */

	TRISIO	= 0;
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

	PR2	= PLF_ADC_MAX;	/* PWM period */
	CCPR1L 	= 0;		/* PWM duty */
	CCP1CON	= 0;
	CCP1M3	= 1, CCP1M2 = 1; /* PWM mode */
	TMR2ON	= 0; /* Stop TMR2 until all settings are done */
	T2CKPS1 = 1; /* TMR2 prescale rate 1:16 */
}

static void
plf_timer_start(void)
{
	TMR2ON = 1;
}

static unsigned int
plf_adc_read(void)
{
	GODONE = 1;
	while (GODONE) {
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
		PLF_PWM_DUTY = adc_val;
	}
}
