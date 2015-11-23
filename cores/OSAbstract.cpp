#include <pthread.h>

#define GPIONUM    (10)
pthread_spinlock_t LKGPIO0;
pthread_spinlock_t LKGPIO1;
pthread_spinlock_t LKGPIO2;
pthread_spinlock_t LKGPIO3;
pthread_spinlock_t LKGPIO4;
pthread_spinlock_t LKGPIO5;
pthread_spinlock_t LKGPIO6;
pthread_spinlock_t LKGPIO7;
pthread_spinlock_t LKGPIO8;
pthread_spinlock_t LKGPIO9;

pthread_spinlock_t gpio_lock_arary[GPIONUM] = {LKGPIO0, LKGPIO1, LKGPIO2, LKGPIO3, LKGPIO4,
                                               LKGPIO5, LKGPIO6, LKGPIO7, LKGPIO8, LKGPIO9};

void spinLockInit(void) {
	for(int i=0; i<GPIONUM; i++) pthread_spin_init(&gpio_lock_arary[i], PTHREAD_PROCESS_SHARED);
	// ... Other spinlock
}

int lockGPIO(int n) {
	return pthread_spin_lock(&gpio_lock_arary[n]);
}

int tryLockGPIO(int n) {
	return pthread_spin_trylock(&gpio_lock_arary[n]);
}

int unLockGPIO(int n) {
	return pthread_spin_unlock(&gpio_lock_arary[n]);
}
