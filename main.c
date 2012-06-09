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

static void
plf_init_clear_io(void)
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

static void
plf_init_cm(void)
{
	CMCON0	= 0;
	CM2 = 1, CM1 = 1, CM0 = 1; /* Disable comparator */
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

#define PLF_PWM_PERIOD		0x100
#define PLF_REG_PWM_DUTY	CCPR1L
#define PLF_CCP_MODE_PWM	0b1100
#define PLF_TMR2_PRESCALE_X1	0b00
#define PLF_TMR2_PRESCALE_X16	0b11

static void
plf_init_pwm(void)
{
	PR2 = PLF_PWM_PERIOD - 1;
	PLF_REG_PWM_DUTY = 0;

	CCP1CON	= 0;
	CCP1CONbits.CCP1M = PLF_CCP_MODE_PWM;
	TMR2ON = 0; /* Stop TMR2 until all settings are done */
	T2CONbits.T2CKPS = PLF_TMR2_PRESCALE_X1;
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

#define PLF_ADC_RESOLUTION 0x400

static unsigned char
plf_pwm_get_duty_from(unsigned long adc_val)
{
	double rate = 0.0;
	unsigned char duty = 0;

	rate = (1.0 + adc_val) / PLF_ADC_RESOLUTION;
	duty = rate * PLF_PWM_PERIOD;
	if (1 < duty) {
		duty--;
	}
	return duty;
}

int
main(int argc, char *argv[])
{
	plf_init();

	plf_timer_start();
	while (1) {
		unsigned long adc_val	= 0;
		unsigned char duty	= 0;

		adc_val	= plf_adc_read();
		duty	= plf_pwm_get_duty_from(adc_val);
		PLF_REG_PWM_DUTY = duty;
	}
}
