#ifndef _OSABSTRACT_h
#define _OSABSTRACT_h

void spinLockInit(void);
int lockGPIO(int n);
int tryLockGPIO(int n);
int unLockGPIO(int n);

#endif
