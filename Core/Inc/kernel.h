/*
 * kernel.h
 *
 *  Created on: Oct 18, 2023
 *      Author: bilal
 */

#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include <stdio.h>

#ifndef INC_KERNEL_H_
#define INC_KERNEL_H_
#define MAX_STACK_SIZE 0x4000
#define MAIN_STACK_SIZE 0x400
#define THREAD_STACK_SIZE 0x400
#define RUN_FIRST_THREAD 0x3
#define YIELD 0x4
#define SHPR2 *(uint32_t*)0xE000ED1C //for setting SVC priority, bits 31-24
#define SHPR3 *(uint32_t*)0xE000ED20 // PendSV is bits 23-16
#define _ICSR *(uint32_t*)0xE000ED04 //This lets us trigger PendSV
extern uint32_t* stackptr;

#endif /* INC_KERNEL_H_ */

// TCB

typedef struct k_thread{
	uint32_t* sp; // stack pointer
	void (*thread_function) (void*); // function pointer
	uint32_t timeslice;
	uint32_t runtime;
}thread;

void thread_function(void* args);

bool osCreateThread();

bool osCreateThreadWithDeadline();

void osKernelInitialize();

void osKernelStart();

void osYield();
