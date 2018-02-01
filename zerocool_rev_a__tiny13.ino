/* ---------------------------------------------------------------------
 * PWM Fan speed control for ATtiny13.
 * Datasheet for ATtiny13: http://www.atmel.com/images/doc2535.pdf
 * 
 * Pin configuration -
 * PB0/OC0A: FAN 4N25 gate driving pfet - output (Pin 5)
 * PB4/ADC2: NTC thermistor interface - input (Pin 3)
 *
 * Based on original work at https://gist.github.com/adnbr/9289235
 * -------------------------------------------------------------------*/
 
// 9.6 MHz, built in resonator
#define F_CPU 9600000
#define FAN PB0 
 
#include <avr/io.h>
 
void adc_setup (void)
{
    // Set the ADC input to PB4/ADC2
    ADMUX |= (1 << MUX1);
    ADMUX |= (1 << ADLAR);
 
    // Set the prescaler to clock/128 & enable ADC
    // At 9.6 MHz this is 75 kHz.
    // See ATtiny13 datasheet, Table 14.4.
    ADCSRA |= (1 << ADPS1) | (1 << ADPS0) | (1 << ADEN);
}
 
uint8_t adc_read (void)
{
    // Start the conversion
    ADCSRA |= (1 << ADSC);
 
    // Wait for it to finish
    while (ADCSRA & (1 << ADSC));
    
    // return the 8 bit value
    return ADCH;
}
 
void pwm_setup (void)
{

    // f_pwm = f_sysclock / Prescaler * Mode
    // 9.6*10^6 / 64 * 256
    // f_pwm = 586 Hz

    // Set Timer 0 prescaler to clock/64 (150kHz). 
    // P = 64
    // See ATtiny13 datasheet, Table 11.9.
    TCCR0B |= (1 << CS00) | (1 << CS01);
 
    // Set to 'Fast PWM' mode - TOP value: 0xFF
    // M = 256
    // See ATtiny13 datasheet, Table 11.8.
    TCCR0A |= (1 << WGM00) | (1 << WGM01);
 
    // Clear OC0A on Compare Match, set 0C0A at TOP
    // See ATtiny13 datasheet, Table 11.3.
    TCCR0A |= (1 << COM0A1);

}
 
void pwm_write ( uint8_t val )
{
    OCR0A = val;
}

void delay_nop( unsigned long delay ) 
{
  volatile unsigned long i = 0;
  for (i = 0; i < delay; i++) {
      __asm__ __volatile__ ("nop");
  }
}
 
int main (void)
{
    uint8_t adc_in;
 
    // FAN is an output.
    DDRB |= (1 << FAN);  
 
    adc_setup();
    pwm_setup();
  
    while (1) {

        /*
        Vcc = 4.8V
        pwm_write((uint8_t)0x45); // 2V
        pwm_write((uint8_t)0x75); // 2.7V
        pwm_write((uint8_t)0xA5); // 3.5V
        pwm_write((uint8_t)0xD5); // 4.1V
        pwm_write((uint8_t)0xFE); // 4.5V
        pwm_write((uint8_t)0x00); // 0V
        */

        // Get the ADC value
        adc_in = adc_read();
        // Now write it to the PWM counter
        pwm_write(adc_in);
    }
}