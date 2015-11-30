#include <Arduino.h>
#include <pthread.h>
#include "OSAbstract.h"
#include "mcm.h"

#define MCPFAU_CAP_LEVEL0 (0x08L << 16)
#define MCPFAU_CAP_LEVEL1 (0x09L << 16)
#define INTERRUPTS 13

#if defined(__86DUINO_ZERO)
    #define MAX_INTR_NUM 2
#elif defined(__86DUINO_EDUCAKE)
    #define MAX_INTR_NUM 5
#else
    #define MAX_INTR_NUM 11
#endif

struct interrupt {
    bool used;
    void (*callback)(void);
    uint32_t mode;
    uint32_t start;
    uint32_t timeout;
};

struct interrupt_desc {
    pthread_t thread;
    pthread_spinlock_t spinlock;
	struct interrupt intr[INTERRUPTS];
};

static void (*sifIntMode[3])(int, int, unsigned long) = {mcpfau_SetCapMode1, mcpfau_SetCapMode2, mcpfau_SetCapMode3}; 
static void (*sifSetPol[3])(int, int, unsigned long) = {mcpfau_SetPolarity1, mcpfau_SetPolarity2, mcpfau_SetPolarity3}; 
static void (*sifSetRelease[3])(int, int, unsigned long) = {mcpfau_SetFAU1RELS, mcpfau_SetFAU2RELS, mcpfau_SetFAU3RELS}; 
static void (*sifClearStat[3])(int, int) = {mcpfau_ClearFAU1STAT, mcpfau_ClearFAU2STAT, mcpfau_ClearFAU3STAT}; 
static void (*sifSetMask[3])(int, int, unsigned long) = {mcpfau_SetMask1, mcpfau_SetMask2, mcpfau_SetMask3}; 
static unsigned long (*readCapStat[3])(int, int) = {mcpfau_ReadCAPSTAT1, mcpfau_ReadCAPSTAT2, mcpfau_ReadCAPSTAT3};
static unsigned long (*readCapFIFO[3])(int, int, unsigned long*) = {mcpfau_ReadCAPFIFO1, mcpfau_ReadCAPFIFO2, mcpfau_ReadCAPFIFO3};
static volatile bool mcm_init[4] = {false, false, false, false};

static interrupt_desc idc;
bool do_callback[INTERRUPTS] = {0};

void* intrMain(void* pargs)
{
	unsigned long capdata;
	int32_t m, n;
    while(true)
    {
        pthread_spin_lock(&idc.spinlock);
        for(int i = 0; i < INTERRUPTS; i++)
        {
			if(idc.intr[i].used == false)
				continue;
            if(i <= MAX_INTR_NUM)
            {
				m = i/3;
				n = i%3;
				lockMCMSIF();
                switch(idc.intr[i].mode)
                {
                case LOW:
                    if(digitalRead(pin_interrupt[i]) == LOW)
                        do_callback[i] = true;
                    break;
                case HIGH:
                    if(digitalRead(pin_interrupt[i]) == HIGH)
						do_callback[i] = true;
                    break;
                case CHANGE:
					if(readCapStat[n](m, MCSIF_MODULEB) != MCENC_CAPFIFO_EMPTY)
						if(readCapFIFO[n](m, MCSIF_MODULEB, &capdata) != MCPFAU_CAP_CAPCNT_OVERFLOW)
							do_callback[i] = true;
                    break;
                case FALLING:
					if(readCapStat[n](m, MCSIF_MODULEB)!= MCENC_CAPFIFO_EMPTY)
          				if(readCapFIFO[n](m, MCSIF_MODULEB, &capdata) == MCPFAU_CAP_1TO0EDGE)
                       		do_callback[i] = true;
                    break;
                case RISING:
					if(readCapStat[n](m, MCSIF_MODULEB)!= MCENC_CAPFIFO_EMPTY)
          				if(readCapFIFO[n](m, MCSIF_MODULEB, &capdata) == MCPFAU_CAP_0TO1EDGE)
                        	do_callback[i] = true;
                    break;
                default:
                    break;
                }
				unLockMCMSIF();
            }
            else
            {
                if(micros() - idc.intr[i].start > idc.intr[i].timeout)
				{
                    do_callback[i] = true;
					idc.intr[i].start = micros();
				}
            }
        }
        pthread_spin_unlock(&idc.spinlock);
		pthread_spin_lock(&idc.spinlock);
		for(int i = 0; i < INTERRUPTS; i++)
		{
			if(do_callback[i] && idc.intr[i].used)
				idc.intr[i].callback();
			do_callback[i] = false;
		}
		pthread_spin_unlock(&idc.spinlock);
	}
    pthread_exit(NULL);
}

static void mcmsif_init(int32_t mc)
{	
    mcsif_SetInputFilter(mc, MCSIF_MODULEB, 20L);
    mcsif_SetSWDeadband(mc, MCSIF_MODULEB, 0L);
    mcsif_SetSWPOL(mc, MCSIF_MODULEB, MCSIF_SWPOL_REMAIN);
    mcsif_SetSamplWin(mc, MCSIF_MODULEB, MCSIF_SWSTART_DISABLE + MCSIF_SWEND_NOW);
    mcsif_SetSamplWin(mc, MCSIF_MODULEB, MCSIF_SWSTART_NOW + MCSIF_SWEND_DISABLE);

	mcsif_SetMode(mc, MCSIF_MODULEB, MCSIF_PFAU);               // enable MC2 SIFA
    
    mcpfau_SetCapMode1(mc, MCSIF_MODULEB, MCPFAU_CAP_DISABLE);
    mcpfau_SetCapInterval1(mc, MCSIF_MODULEB, 1L);              // pin1 for FAUTRIG
    mcpfau_SetCap1INT(mc, MCSIF_MODULEB, 1L);
    mcpfau_SetPolarity1(mc, MCSIF_MODULEB, MCPFAU_POL_NORMAL);
    mcpfau_SetMask1(mc, MCSIF_MODULEB, MCPFAU_MASK_NONE);
    mcpfau_SetRLDTRIG1(mc, MCSIF_MODULEB, MCPFAU_RLDTRIG_DISABLE);
    mcpfau_SetFAU1TRIG(mc, MCSIF_MODULEB, MCPFAU_FAUTRIG_INPUT1);
    mcpfau_SetFAU1RELS(mc, MCSIF_MODULEB, MCPFAU_FAURELS_INPUT0);
	
    mcpfau_SetCapMode2(mc, MCSIF_MODULEB, MCPFAU_CAP_DISABLE);     // pin2 for RLDTRIG
    mcpfau_SetCapInterval2(mc, MCSIF_MODULEB, 1L);
    mcpfau_SetCap2INT(mc, MCSIF_MODULEB, 1L);
    mcpfau_SetPolarity2(mc, MCSIF_MODULEB, MCPFAU_POL_NORMAL);
    mcpfau_SetMask2(mc, MCSIF_MODULEB, MCPFAU_MASK_NONE);
    mcpfau_SetRLDTRIG2(mc, MCSIF_MODULEB, MCPFAU_RLDTRIG_DISABLE);
    mcpfau_SetFAU2TRIG(mc, MCSIF_MODULEB, MCPFAU_FAUTRIG_INPUT1);
    mcpfau_SetFAU2RELS(mc, MCSIF_MODULEB, MCPFAU_FAURELS_INPUT0);

    mcpfau_SetCapMode3(mc, MCSIF_MODULEB, MCPFAU_CAP_DISABLE);
    mcpfau_SetCapInterval3(mc, MCSIF_MODULEB, 1L);              // pin3 for FAUTRIG 
    mcpfau_SetCap3INT(mc, MCSIF_MODULEB, 1L);
    mcpfau_SetPolarity3(mc, MCSIF_MODULEB, MCPFAU_POL_NORMAL);
    mcpfau_SetMask3(mc, MCSIF_MODULEB, MCPFAU_MASK_NONE);
    mcpfau_SetRLDTRIG3(mc, MCSIF_MODULEB, MCPFAU_RLDTRIG_DISABLE);          
	mcpfau_SetFAU3TRIG(mc, MCSIF_MODULEB, MCPFAU_FAUTRIG_INPUT1);
	mcpfau_SetFAU3RELS(mc, MCSIF_MODULEB, MCPFAU_FAURELS_INPUT0);
	
	mcm_init[mc] = true;
}

static int addIRQEntry(uint8_t interruptNum, void (*callback)(void), int mode, uint32_t timeout)
{
	pthread_spin_lock(&idc.spinlock);

	idc.intr[interruptNum].used = true;
    idc.intr[interruptNum].callback = callback;
    idc.intr[interruptNum].mode = mode;
    idc.intr[interruptNum].start = micros();
    idc.intr[interruptNum].timeout = timeout;
	pinMode(pin_interrupt[interruptNum], INPUT);

	if(mode != LOW && mode != HIGH && timeout == 0)
	{
		uint16_t crossbar_ioaddr;
		int32_t mc = interruptNum/3;
		int32_t md = MCSIF_MODULEB;
		lockMCMSIF();
		if(mcm_init[mc] == false)
			mcmsif_init(mc);
		crossbar_ioaddr = sb_Read16(0x64)&0xfffe;
    	if (mc == 0)
			io_outpb(crossbar_ioaddr + 2, 0x01); // GPIO port2: 0A, 0B, 0C, 3A
		else if (mc == 1)
    		io_outpb(crossbar_ioaddr + 3, 0x02); // GPIO port3: 1A, 1B, 1C, 3B
		else if(mc == 2)
			io_outpb(crossbar_ioaddr, 0x03); // GPIO port0: 2A, 2B, 2C, 3C
		else if(mc == 3)
		{
			io_outpb(crossbar_ioaddr + 2, 0x01);
			io_outpb(crossbar_ioaddr + 3, 0x02);
			io_outpb(crossbar_ioaddr, 0x03);
		}
		mcsif_Disable(mc, md);

		switch (mode) 
		{
		case CHANGE:
			sifIntMode[interruptNum%3](mc, md, MCPFAU_CAP_BOTH);
			break;
		case FALLING:
			sifIntMode[interruptNum%3](mc, md, MCPFAU_CAP_1TO0);
			break;	
		case RISING:
			sifIntMode[interruptNum%3](mc, md, MCPFAU_CAP_0TO1);
			break;
		default:
			break;
		}
		// switch crossbar to MCM_SIF_PIN
		io_outpb(crossbar_ioaddr + 0x90 + pin_offset[interruptNum], 0x08);//RICH IO
		mcsif_Enable(mc, md);
		unLockMCMSIF();
	}
	
	pthread_spin_unlock(&idc.spinlock);
}

void attachInterrupt(uint8_t interruptNum, void (*userFunc)(void), int mode)
{
    if( mode < 0 || mode > 4 )
    {
        printf("Not support this mode\n");
        return;
    }

    if( interruptNum > MAX_INTR_NUM )
    {
        printf("Not support this interruptNum\n");
        return;
    }

	if( idc.intr[interruptNum].used == true )
	{
		printf("This pin was attached before\n");
		return;
	}

    addIRQEntry(interruptNum, userFunc, mode, 0);
}

static void mcmsif_close(int32_t mc)
{
	mcsif_Disable(mc, MCSIF_MODULEB);
	mcm_init[mc] = false;
}

void detachInterrupt(uint8_t interruptNum)
{
	if(interruptNum > MAX_INTR_NUM + 1)
		return;

	pthread_spin_lock(&idc.spinlock);
	idc.intr[interruptNum].used = false;
	uint8_t mc = interruptNum/3;
	lockMCMSIF();
	if(mc < 4 && !(idc.intr[mc*3].used) && !(idc.intr[mc*3 + 1].used) && !(idc.intr[mc*3 + 2].used))
		mcmsif_close(mc);
	unLockMCMSIF();
	pthread_spin_unlock(&idc.spinlock);
}

void attachTimerInterrupt(uint8_t interruptNum, void (*callback)(void), uint32_t microseconds)
{
    if(interruptNum != 12)
        return;
    addIRQEntry(interruptNum, callback, 0, microseconds);
}

void detachTimerInterrupt(void)
{
	detachInterrupt(12);
}

int interrupt_init(void)
{
    pthread_spin_init(&idc.spinlock, 0);
    for(int i = 0; i < INTERRUPTS; i++)
		idc.intr[i].used = false;
    int err = pthread_create(&idc.thread, NULL, intrMain, NULL);
	if(err != 0)
		printf("failed to create the thread\n");
	return 0;
}
