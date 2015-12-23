#ifndef _OSABSTRACT_h
#define _OSABSTRACT_h

#if defined (DMP_LINUX)
	#include <pthread.h>
	#define OSSPIN pthread_spinlock_t
	#define OSSPININIT(var) pthread_spin_init(&var, PTHREAD_PROCESS_SHARED)
	#define OSSPINLOCK(var) pthread_spin_lock(&var)
	#define OSSPINTRYLOCK(var) pthread_spin_trylock(&var)
	#define OSSPINUNLOCK(var) pthread_spin_unlock(&var)
#else
	#define OSSPIN (int)
	#define OSSPININIT(var)
	#define OSSPINLOCK(var)
	#define OSSPINTRYLOCK(var)
	#define OSSPINUNLOCK(var)
#endif

void spinLockInit(void);

int lockGPIO(int n);
int tryLockGPIO(int n);
int unLockGPIO(int n);

int lockADC(void);
int tryLockADC(void);
int unLockADC(void);

int lockMCM(int mc, int md);
int tryLockMCM(int mc, int md);
int unLockMCM(int mc, int md);

int lockMCMGENAL(void);
int tryLockMCMGENAL(void);
int unLockMCMGENAL(void);

int lockMCMSIF(void);
int tryLockMCMSIF(void);
int unLockMCMSIF(void);

#define MCGENAL    (12)
#define MCSIF      (13)

int lockSPI(void);
int tryLockSPI(void);
int unLockSPI(void);

#endif
