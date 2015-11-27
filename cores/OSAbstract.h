#ifndef _OSABSTRACT_h
#define _OSABSTRACT_h

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

#endif
