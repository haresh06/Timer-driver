#ifndef TIMER1_H
#define TIMER1_H

#include <stdint.h>
#include <avr/interrupt.h>

//====================== TIMER1 REGISTERS ======================

#define TCCR1A (*(volatile uint8_t*)0x80)
#define TCCR1B (*(volatile uint8_t*)0x81)
#define TCNT1  (*(volatile uint16_t*)0x84)
#define OCR1A  (*(volatile uint16_t*)0x88)
#define TIMSK1 (*(volatile uint8_t*)0x6F)
#define TIFR1  (*(volatile uint8_t*)0x36)
#define ICR1A  (*(volatile uint16_t*)0x86)

//====================== BIT POSITIONS ======================

// Waveform Generation Mode bits
#define WGM10 0
#define WGM11 1
#define WGM12 3
#define WGM13 4


#define COM1A1 7
#define COM0A0 6

// Clock Select bits
#define CS10 0
#define CS11 1
#define CS12 2

// Timer Flags
#define TOV1  0
#define OCF1A 1

// Interrupt Enable bits
#define TOIE1  0
#define OCIE1A 1

//====================== PRESCALER ENUM ======================

typedef enum
{
    TIMER1_NOCLK = 0,
    TIMER1_PRES1,
    TIMER1_PRES8,
    TIMER1_PRES64,
    TIMER1_PRES256,
    TIMER1_PRES1024

} timer1_clk;

//====================== TIMER CONTROL ======================

// Stop Timer1
void timer1_stop()
{
    TCCR1B &= ~((1<<CS10) | (1<<CS11) | (1<<CS12));
}

// Start Timer1 with selected prescaler
void timer1_start(timer1_clk clock)
{
    timer1_stop();   // Clear previous clock settings

    switch(clock)
    {
        case TIMER1_PRES1:
            TCCR1B |= (1<<CS10);                 // Prescaler = 1
            break;

        case TIMER1_PRES8:
            TCCR1B |= (1<<CS11);                 // Prescaler = 8
            break;

        case TIMER1_PRES64:
            TCCR1B |= ((1<<CS10)|(1<<CS11));     // Prescaler = 64
            break;

        case TIMER1_PRES256:
            TCCR1B |= (1<<CS12);                 // Prescaler = 256
            break;

        case TIMER1_PRES1024:
            TCCR1B |= ((1<<CS10)|(1<<CS12));     // Prescaler = 1024
            break;

        default:
            break;
    }
}

// Reset Timer Counter
void timer1_reset()
{
    TCNT1 = 0;
}

//====================== REGISTER SET FUNCTIONS ======================

// Load value into TCNT1
void set_count1(uint16_t val)
{
    TCNT1 = val;
}

// Load value into OCR1A
void set_compare1(uint16_t val)
{
    OCR1A = val;
}

//====================== NORMAL MODE ======================

// Configure Timer1 in Normal Mode
void timer1_normal_mode(timer1_clk val)
{
    timer1_stop();      // Stop timer

    TCCR1A = 0;         // Clear control register A
    TCCR1B = 0;         // Clear control register B

    timer1_reset();     // TCNT1 = 0

    timer1_start(val);  // Start timer
}

//====================== CTC MODE ======================

// Configure Timer1 in CTC Mode
void timer1_ctc_mode(uint16_t value, timer1_clk val)
{
    timer1_stop();      // Stop timer

    TCCR1A = 0;         // Clear control register A
    TCCR1B = 0;         // Clear control register B

    timer1_reset();     // TCNT1 = 0

    OCR1A = value;      // Load compare value

    TCCR1B |= (1<<WGM12); // Select CTC Mode

    timer1_start(val);  // Start timer
}

//====================== POLLING DELAYS ======================

// Wait for x overflows
void normal_delay(uint16_t x)
{
    for(uint16_t i=0; i<x; i++)
    {
        // Wait until overflow occurs
        while(!(TIFR1 & (1<<TOV1)));

        // Clear overflow flag
        TIFR1 |= (1<<TOV1);
    }
}

// Wait for x compare matches
void ctc_delay(uint16_t x)
{
    for(uint16_t i=0; i<x; i++)
    {
        // Wait until compare match occurs
        while(!(TIFR1 & (1<<OCF1A)));

        // Clear compare match flag
        TIFR1 |= (1<<OCF1A);
    }
}

//====================== INTERRUPTS ======================

// Enable Timer1 Overflow Interrupt
void en_ovf_intr()
{
    sei();                  // Enable global interrupts
    TIMSK1 |= (1<<TOIE1);   // Enable overflow interrupt
}

// Disable Timer1 Overflow Interrupt
void dis_ovf_intr()
{
    TIMSK1 &= ~(1<<TOIE1);
}

// Enable Timer1 Compare Match Interrupt
void en_com_intr()
{
    sei();                  // Enable global interrupts
    TIMSK1 |= (1<<OCIE1A);  // Enable compare interrupt
}

// Disable Timer1 Compare Match Interrupt
void dis_com_intr()
{
    TIMSK1 &= ~(1<<OCIE1A);
}

//====================== USER DELAY FUNCTIONS ======================

// Delay in milliseconds
// OCR1A = 249
// Prescaler = 64
// 1 Compare Match = 1 ms
void delay_ms(uint16_t value)
{
    timer1_ctc_mode(249, TIMER1_PRES64);

    ctc_delay(value);

    timer1_stop();
}

// Delay in seconds
// OCR1A = 15624
// Prescaler = 1024
// 1 Compare Match = 1 second
void delay_s(uint16_t value)
{
    timer1_ctc_mode(15624, TIMER1_PRES1024);

    ctc_delay(value);

    timer1_stop();
}

//====================== PWM ======================

void pwm_init(timer1_clk prescaler,uint8_t top)
{
    TCCR1A |= ((1<<WGM11) | (1<<COM1A1));      // PWM Mode + Non-Inverting
    TCCR1B |= ((1<<WGM12) | (1<<WGM13) | (prescaler & 0x07)); // PWM Mode + Prescaler
    ICR1A = top;      // Set TOP
    OCR1A = 0;        // 0% Duty
}

void duty_cycle(uint8_t duty_percent)
{
    uint16_t duty_value = (duty_percent * ICR1A)/100; // Calculate Duty
    OCR1A = duty_value;      // Update Duty
}
#endif