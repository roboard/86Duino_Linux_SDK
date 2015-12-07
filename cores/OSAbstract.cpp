#include "dmpcfg.h"
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

pthread_spinlock_t LKADC;

#define MCM_MCMD_NUM    (12)
pthread_spinlock_t LKMC0MD0;
pthread_spinlock_t LKMC0MD1;
pthread_spinlock_t LKMC0MD2;
pthread_spinlock_t LKMC1MD0;
pthread_spinlock_t LKMC1MD1;
pthread_spinlock_t LKMC1MD2;
pthread_spinlock_t LKMC2MD0;
pthread_spinlock_t LKMC2MD1;
pthread_spinlock_t LKMC2MD2;
pthread_spinlock_t LKMC3MD0;
pthread_spinlock_t LKMC3MD1;
pthread_spinlock_t LKMC3MD2;
pthread_spinlock_t LKMCGENAL;
pthread_spinlock_t LKMCSIF;

pthread_spinlock_t mcm_lock_arary[MCM_MCMD_NUM] = {LKMC0MD0, LKMC0MD1, LKMC0MD2, LKMC1MD0, LKMC1MD1, LKMC1MD2, LKMC2MD0,
                                              LKMC2MD1, LKMC2MD2, LKMC3MD0, LKMC3MD1, LKMC3MD2};

pthread_spinlock_t LKSPI;
											  
void spinLockInit(void) {
	for(int i=0; i<GPIONUM; i++) pthread_spin_init(&gpio_lock_arary[i], PTHREAD_PROCESS_SHARED);
    pthread_spin_init(&LKADC, PTHREAD_PROCESS_SHARED);
    for(int i=0; i<MCM_MCMD_NUM; i++) pthread_spin_init(&mcm_lock_arary[i], PTHREAD_PROCESS_SHARED);
    pthread_spin_init(&LKMCGENAL, PTHREAD_PROCESS_SHARED);
    pthread_spin_init(&LKMCSIF, PTHREAD_PROCESS_SHARED);
	pthread_spin_init(&LKSPI, PTHREAD_PROCESS_SHARED);
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

int lockADC(void) {
	return pthread_spin_lock(&LKADC);
}

int tryLockADC(void) {
	return pthread_spin_trylock(&LKADC);
}

int unLockADC(void) {
	return pthread_spin_unlock(&LKADC);
}

int lockMCM(int mc, int md) {
	int index;
	if(mc < 0 || mc > 3 || md < 0 || md > 2) return 0xffff;

	index = mc*3 + md;
	return pthread_spin_lock(&mcm_lock_arary[index]);
}

int tryLockMCM(int mc, int md) {
	int index;
	if(mc < 0 || mc > 3 || md < 0 || md > 2) return 0xffff;

	index = mc*3 + md;
	return pthread_spin_trylock(&mcm_lock_arary[index]);
}

int unLockMCM(int mc, int md) {
    int index;
	if(mc < 0 || mc > 3 || md < 0 || md > 2) return 0xffff;

	index = mc*3 + md;
	return pthread_spin_unlock(&mcm_lock_arary[index]);
}

int lockMCMGENAL(void) {
	return pthread_spin_lock(&LKMCGENAL);
}

int tryLockMCMGENAL(void) {
	return pthread_spin_trylock(&LKMCGENAL);
}

int unLockMCMGENAL(void) {
	return pthread_spin_unlock(&LKMCGENAL);
}

int lockMCMSIF(void) {
	return pthread_spin_lock(&LKMCSIF);
}

int tryLockMCMSIF(void) {
	return pthread_spin_trylock(&LKMCSIF);
}

int unLockMCMSIF(void) {
	return pthread_spin_unlock(&LKMCSIF);
}

int lockSPI(void) {
	return pthread_spin_lock(&LKSPI);
}

int tryLockSPI(void) {
	return pthread_spin_trylock(&LKSPI);
}

int unLockSPI(void) {
	return pthread_spin_unlock(&LKSPI);
}
