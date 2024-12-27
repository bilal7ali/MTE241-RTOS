#include <kernel.h>

extern void runFirstThread(void);
thread global_thread;
thread threadArray[15];
uint32_t MSP;
uint32_t num_threads = MAX_STACK_SIZE / THREAD_STACK_SIZE;
uint32_t numThreadsRunning = 0;
uint32_t currentThread = 0;

void SVC_Handler_Main( unsigned int *svc_args )
{
	unsigned int svc_number;
/*
* Stack contains:
* r0, r1, r2, r3, r12, r14, the return address and xPSR
* First argument (r0) is svc_args[0]
*/
	svc_number = ( ( char * )svc_args[ 6 ] )[ -2 ] ;
	switch( svc_number )
	{
	case 0:
		printf("Success\r\n");
	break;
	case 1:
		printf("Fail\r\n");
	break;
	case RUN_FIRST_THREAD:
		__set_PSP(threadArray[currentThread].sp);
		runFirstThread();
	break;
	case YIELD:
		//Pend an interrupt to do the context switch
		_ICSR |= 1<<28;
		__asm("isb");
		break;
	default: /* unknown SVC */
		break;
	}
}

uint32_t* stack_allocator(void) {
	static int j = 1;
	if (j > (MAX_STACK_SIZE / THREAD_STACK_SIZE) - 1) {
		return NULL;
	}
	j++;

	return (uint32_t*)(MSP - MAIN_STACK_SIZE - (j-1)*(THREAD_STACK_SIZE));;
}

bool osCreateThread(void* function_pointer, void* args) {
	uint32_t* stack_availability = stack_allocator();
	if (stack_availability == NULL) {
		return false;
	} else {
		threadArray[numThreadsRunning].sp = stack_availability;
		threadArray[numThreadsRunning].thread_function = function_pointer;
		*(--threadArray[numThreadsRunning].sp) = 1<<24;
		*(--threadArray[numThreadsRunning].sp) = (uint32_t)threadArray[numThreadsRunning].thread_function;
		for (int i = 0; i < 14; i++) {
			if (i == 5) {
				*(--threadArray[numThreadsRunning].sp) = (uint32_t)args;
			} else {
				*(--threadArray[numThreadsRunning].sp) = 0xA;
			}
		}
		threadArray[numThreadsRunning].runtime = 5;
		threadArray[numThreadsRunning].timeslice = 5;
	}
	numThreadsRunning++;
	return true;
}

bool osCreateThreadWithDeadline(void* function_pointer, void* args, uint32_t input) {
	uint32_t* stack_availability = stack_allocator();
	if (stack_availability == NULL) {
		return false;
	} else {
		threadArray[numThreadsRunning].sp = stack_availability;
		threadArray[numThreadsRunning].thread_function = function_pointer;
		*(--threadArray[numThreadsRunning].sp) = 1<<24;
		*(--threadArray[numThreadsRunning].sp) = (uint32_t)threadArray[numThreadsRunning].thread_function;
		for (int i = 0; i < 14; i++) {
			if (i == 5) {
				*(--threadArray[numThreadsRunning].sp) = (uint32_t)args;
			} else {
				*(--threadArray[numThreadsRunning].sp) = 0xA;
			}
		}
		threadArray[numThreadsRunning].runtime = input;
		threadArray[numThreadsRunning].timeslice = input;
	}
	numThreadsRunning++;
	return true;
}


void osKernelInitialize() {
	uint32_t* MSP_INIT_VAL = *(uint32_t**)0x0;
	MSP = (uint32_t)MSP_INIT_VAL;

	//set the priority of PendSV to almost the weakest
	SHPR3 |= 0xFE << 16; //shift the constant 0xFE 16 bits to set PendSV priority
	SHPR2 |= 0xFDU << 24; //Set the priority of SVC higher than PendSV
}

void osKernelStart() {
	__asm("SVC #3");
}

void osSched() {
	threadArray[currentThread].sp = (uint32_t*)(__get_PSP() - 8*4);
	currentThread = (currentThread+1)%numThreadsRunning;
	__set_PSP(threadArray[currentThread].sp);
}

void osYield(void) {
	__asm("SVC #4");
}
