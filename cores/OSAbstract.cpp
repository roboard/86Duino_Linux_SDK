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

void spinLockInit(void) {
#if defined (DMP_LINUX)
	for(int i=0; i<GPIONUM; i++) pthread_spin_init(&gpio_lock_arary[i], PTHREAD_PROCESS_SHARED);
    pthread_spin_init(&LKADC, PTHREAD_PROCESS_SHARED);
    for(int i=0; i<MCM_MCMD_NUM; i++) pthread_spin_init(&mcm_lock_arary[i], PTHREAD_PROCESS_SHARED);
    pthread_spin_init(&LKMCGENAL, PTHREAD_PROCESS_SHARED);
    pthread_spin_init(&LKMCSIF, PTHREAD_PROCESS_SHARED);
	// ... Other spinlock
#endif
}

int lockGPIO(int n) {
#if defined (DMP_LINUX)
	return pthread_spin_lock(&gpio_lock_arary[n]);
#elif defined (DMP_DOS_DJGPP) || defined (DMP_DOS_WATCOM) || defined (DMP_DOS_BC30)
	io_DisableINT();
#endif
}

int tryLockGPIO(int n) {
#if defined (DMP_LINUX)
	return pthread_spin_trylock(&gpio_lock_arary[n]);
#endif
}

int unLockGPIO(int n) {
#if defined (DMP_LINUX)
	return pthread_spin_unlock(&gpio_lock_arary[n]);
#elif defined (DMP_DOS_DJGPP) || defined (DMP_DOS_WATCOM) || defined (DMP_DOS_BC30)
	io_RestoreINT();
#nedif
}

int lockADC(void) {
#if defined (DMP_LINUX)
	return pthread_spin_lock(&LKADC);
#elif defined (DMP_DOS_DJGPP) || defined (DMP_DOS_WATCOM) || defined (DMP_DOS_BC30)
	io_DisableINT();
#endif
}

int tryLockADC(void) {
#if defined (DMP_LINUX)
	return pthread_spin_trylock(&LKADC);
#endif
}

int unLockADC(void) {
#if defined (DMP_LINUX)
	return pthread_spin_unlock(&LKADC);
#elif defined (DMP_DOS_DJGPP) || defined (DMP_DOS_WATCOM) || defined (DMP_DOS_BC30)
	io_RestoreINT();
#nedif
}

int lockMCM(int mc, int md) {
#if defined (DMP_LINUX)
	int index;
	if(mc < 0 || mc > 3 || md < 0 || md > 2) return 0xffff;

	index = mc*3 + md;
	return pthread_spin_lock(&mcm_lock_arary[index]);
#elif defined (DMP_DOS_DJGPP) || defined (DMP_DOS_WATCOM) || defined (DMP_DOS_BC30)
	io_DisableINT();
#endif
}

int tryLockMCM(int mc, int md) {
#if defined (DMP_LINUX)
	int index;
	if(mc < 0 || mc > 3 || md < 0 || md > 2) return 0xffff;

	index = mc*3 + md;
	return pthread_spin_trylock(&mcm_lock_arary[index]);
#endif
}

int unLockMCM(int mc, int md) {
#if defined (DMP_LINUX)
    int index;
	if(mc < 0 || mc > 3 || md < 0 || md > 2) return 0xffff;

	index = mc*3 + md;
	return pthread_spin_unlock(&mcm_lock_arary[index]);
#elif defined (DMP_DOS_DJGPP) || defined (DMP_DOS_WATCOM) || defined (DMP_DOS_BC30)
	io_RestoreINT();
#nedif
}

int lockMCMGENAL(void) {
#if defined (DMP_LINUX)
	return pthread_spin_lock(&LKMCGENAL);
#elif defined (DMP_DOS_DJGPP) || defined (DMP_DOS_WATCOM) || defined (DMP_DOS_BC30)
	io_DisableINT();
#endif
}

int tryLockMCMGENAL(void) {
#if defined (DMP_LINUX)
	return pthread_spin_trylock(&LKMCGENAL);
#endif
}

int unLockMCMGENAL(void) {
#if defined (DMP_LINUX)
	return pthread_spin_unlock(&LKMCGENAL);
#elif defined (DMP_DOS_DJGPP) || defined (DMP_DOS_WATCOM) || defined (DMP_DOS_BC30)
	io_RestoreINT();
#nedif
}

int lockMCMSIF(void) {
#if defined (DMP_LINUX)
	return pthread_spin_lock(&LKMCSIF);
#elif defined (DMP_DOS_DJGPP) || defined (DMP_DOS_WATCOM) || defined (DMP_DOS_BC30)
	io_DisableINT();
#endif
}

int tryLockMCMSIF(void) {
#if defined (DMP_LINUX)
	return pthread_spin_trylock(&LKMCSIF);
#endif
}

int unLockMCMSIF(void) {
#if defined (DMP_LINUX)
	return pthread_spin_unlock(&LKMCSIF);
#elif defined (DMP_DOS_DJGPP) || defined (DMP_DOS_WATCOM) || defined (DMP_DOS_BC30)
	io_RestoreINT();
#nedif
}
