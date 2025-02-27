

#ifndef SAMPLER_H_
#define SAMPLER_H_

#define MAX_STR_LENGTH 32
#define NUM_SAMPLES         1
#define SAMPLE_SIZE         4
#define NUM_BLOCKS          1
#define BUF_LENGTH (NUM_SAMPLES * SAMPLE_SIZE * NUM_BLOCKS)

#define SAMPLER_VECTOR 		TIMER0_A0_VECTOR
#define SAMPLER_CCR0		TA0CCR0
#define SAMPLER_EX0			TA0EX0
#define SAMPLER_CTL			TA0CTL
#define SAMPLER_CCTL0		TA0CCTL0
#define SAMPLER_IS_RUNNING  (SAMPLER_CCTL0 & CCIE)

void sampler_init(uint16_t);    // initialize timer to generate interrupt every sec
void sampler_process();
void sampler_start();
void sampler_stop();
uint8_t sampler_isDone();

#endif
