#include <stdint.h>
#include<avr/interrupt
#define TCCR1A (*(volatile uint8_t*)0x80)
#define TCCR1B (*(volatile uint8_t*)0x81)
#define TCNT1 (*(volatile uint16_t*)0x84)
#define OCR1A (*(volatile uint16_t*)0x88)
#define TIMSK1 (*(volatile uint8_t*)0x6F)
#define TIFR1 (*(volatile uint8_t*)0x36)

#define WGM10 0
#define WGM11 1
#define WGM12 3
#define WGM13 4

#define CS10 0
#define CS11 1
#define CS12 2

#define TOV1 0
#define OCF1A 1

#define TOIE1 0
#define OCIE1A 1

typedef enum{
    TIMER1_NOCLK=0,
    TIMER1_PRES1,
    TIMER1_PRES8,
    TIMER1_PRES64,
    TIMER1_PRES256,
    TIMER1_PRES1024,
}timer1_clk;

void timer1_stop()
{
    TCCR1B &= ~((1<<CS10) |(1<<CS11) | (1<<CS12));
}

void timer1_start(timer1_clk clock)
{
    timer1_stop();
    switch(clock){
        case TIMER1_PRES1:
        TCCR1B |= (1<<CS10);
        break;
        case TIMER1_PRES8:
        TCCR1B |= (1<<CS11);
        break;
        case TIMER1_PRES64:
        TCCR1B |= ((1<<CS10) | (1<<CS11));
        break;
        case TIMER1_PRES256:
        TCCR1B |= (1<<CS12);
        break;
        case TIMER1_PRES1024:
        TCCR1B |= ((1<<CS10) | (1<<CS12));
        break;
        default:
        break;
    }
}

void timer1_reset()
{
    TCNT1=0;
}

void set_count1(uint16_t val)
{
    TCNT1=val;
}

void set_compare1(uint16_t val)
{
    OCR1A=val;
}

//NORMAL MODE
 void timer1_normal_mode(timer1_clk val)
{
    timer1_stop();
    TCCR1A=0;
    TCCR1B=0;
    timer1_reset();
    timer1_start(val);
}

//CTC MODE
void timer1_ctc_mode(uint16_t value,timer1_clk val)
{
    timer1_stop();
    TCCR1A=0;
    TCCR1B=0;
    timer1_reset();
    OCR1A=value;
    TCCR1B |= (1<<WGM12);
    timer1_start(val);
}

void normal_delay(uint16_t x){
    for(int i=0;i<x;i++)
    {
       while(!(TIFR1 & (1<<TOV1)));
       TIFR1|=(1<<TOV1);
    }
}

void ctc_delay(uint16_t x){
    for(int i=0;i<x;i++)
    {
       while(!(TIFR1 & (1<<OCF1A)));
       TIFR1|=(1<<OCF1A);
    }
}
void en_ovf_intr()
{
    sei();
    TIMSK1|=(1<<TOIE1);
}
void dis_ovf_intr()
{
    TIMSK1 &= ~(1<<TOIE1);
}

void en_com_intr()
{
    sei();
    TIMSK1 |= (1<<OCIE1A);
}

void dis_com_intr()
{
    TIMSK1 &= ~(1<<OCIE1A);
}

void delay_ms(uint16_t value)
{
    timer1_ctc_mode(249,TIMER1_PRES64);
    ctc_delay(value);
    timer1_stop();
}

void delay_s(uint16_t value)
{
    timer1_ctc_mode(15624,TIMER1_PRES1024);
    ctc_delay(value);
    timer1_stop();
}