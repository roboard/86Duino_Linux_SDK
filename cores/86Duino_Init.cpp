#include "Arduino.h"

static __attribute__((constructor(101))) void _f_init()
{
	init();
	interrupt_init();
}

static __attribute__((destructor(101))) void _f_final()
{
	io_Close();
}
