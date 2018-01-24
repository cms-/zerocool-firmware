/* ---------------------------------------------------------------------
 * PWM Fan speed control for ATtiny13.
 * Datasheet for ATtiny13: http://www.atmel.com/images/doc2535.pdf
 * 
 * Pin configuration -
 * PB0/OC0A: FAN p-chan FET output (Pin 5)
 * PB4/ADC2: NTC thermistor input (Pin 3)
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
 
int adc_read (void)
{
    // Start the conversion
    ADCSRA |= (1 << ADSC);
 
    // Wait for it to finish
    while (ADCSRA & (1 << ADSC));
 
    return ADCH;
}
 
void pwm_setup (void)
{
    // Set Timer 0 prescaler to clock/8.
    // At 9.6 MHz this is 1.2 MHz.
    // See ATtiny13 datasheet, Table 11.9.
    TCCR0B |= (1 << CS01);
 
    // Set to 'Fast PWM' mode
    TCCR0A |= (1 << WGM01) | (1 << WGM00);
 
    // Clear OC0A output on compare match, upwards counting.
    TCCR0A |= (1 << COM0A1);
}
 
void pwm_write (int val)
{
    OCR0A = val;
}
 
int main (void)
{
    int adc_in;
 
    // FAN is an output.
    DDRB |= (1 << FAN);  
 
    adc_setup();
    pwm_setup();
  
    while (1) {
        // Get the ADC value
        adc_in = adc_read();
        // Now write it to the PWM counter
        pwm_write(adc_in);
    }
}