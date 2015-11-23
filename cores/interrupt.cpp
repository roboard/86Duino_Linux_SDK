#include <Arduino.h>
#include <pthread.h>

struct interrupt {
    uint8_t pin;
    void (*callback)(void);
    uint32_t mode;
    uint32_t status[2];
    uint32_t start;
    uint32_t timeout;
    struct interrupt* next;
};

struct interrupt_desc {
    pthread_t thread;
    pthread_spinlock_t spinlock;
    struct interrupt* head;
};

static interrupt_desc idc;

#if defined(__86DUINO_ZERO)
    #define MAX_INTR_NUM 2
#elif defined(__86DUINO_EDUCAKE)
    #define MAX_INTR_NUM 5
#else
    #define MAX_INTR_NUM 11
#endif

void* intrMain(void* pargs)
{
    struct interrupt* pIntr;
    while(true)
    {
        pthread_spin_lock(&idc.spinlock);
        pIntr = idc.head;
        while(pIntr != NULL)
        {
            if(pIntr->pin < 128)
            {
                pIntr->status[1] = digitalRead(pIntr->pin);
                switch(pIntr->mode)
                {
                    case LOW:
                        if(pIntr->status[1] == LOW)
                            pIntr->callback();
                        break;
                    case HIGH:
                        if(pIntr->status[1] == HIGH)
                            pIntr->callback();
                        break;
                    case CHANGE:
                        if(pIntr->status[0] ^ pIntr->status[1])
                            pIntr->callback();
                        break;
                    case FALLING:
                        if(pIntr->status[0] > pIntr->status[1])
                            pIntr->callback();
                        break;
                    case RISING:
                        if(pIntr->status[0] < pIntr->status[1])
                            pIntr->callback();
                        break;
                    default:
                        break;
                }
                pIntr->status[0] = pIntr->status[1];
            }
            else
            {
                if(micros() - pIntr->start > pIntr->timeout)
				{
                    pIntr->callback();
					pIntr->start = micros();
				}
            }
            pIntr = pIntr->next;
        }
        pthread_spin_unlock(&idc.spinlock);
	}
    pthread_exit(NULL);
}

static int addIRQEntry(uint8_t pin, void (*callback)(void), int mode, uint32_t timeout)
{
    struct interrupt* intr = NULL;
    intr = (struct interrupt*)malloc(sizeof(struct interrupt));
    if(intr == NULL)
    {
        printf("Out of memory\n");
        return -1;
    }
    pinMode(pin, INPUT);
    intr->pin = pin;
    intr->callback = callback;
    intr->mode = mode;
    intr->status[0] = digitalRead(pin);
    intr->start = micros();
    intr->timeout = timeout;
    intr->next = NULL;

    struct interrupt* tmp = idc.head;
    if(idc.head == NULL)
    {
        idc.head = intr;
    }
    else
    {
        while(tmp->next != NULL)
        {
            tmp = tmp->next;
        }
        tmp->next = intr;
    }
}

void attachInterrupt(uint8_t interruptNum, void (*userFunc)(void), int mode)
{
    uint8_t pin = pin_interrupt[interruptNum];

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

    struct interrupt* tmp = idc.head;
    while(tmp != NULL)
    {
        if(pin == tmp->pin)
            return;
        tmp = tmp->next;
    }

    pthread_spin_lock(&idc.spinlock);
    addIRQEntry(pin, userFunc, mode, 0);
    pthread_spin_unlock(&idc.spinlock);
}

void detachInterrupt(uint8_t interruptNum)
{
    if(idc.head == NULL)
        return;
    struct interrupt* pIntr = NULL;
    pIntr = (struct interrupt*)malloc(sizeof(struct interrupt));
    if(pIntr == NULL)
    {
        printf("Out of memory\n");
        return;
    }

    pthread_spin_lock(&idc.spinlock);
    pIntr->next = idc.head;
    while(pIntr->next != NULL)
    {
        if(pIntr->next->pin == pin_interrupt[interruptNum])
            break;
        pIntr = pIntr->next;
    }
    if(pIntr->next != NULL)
    {
        if(pIntr->next == idc.head)
        {
            idc.head = pIntr->next->next;
            free(pIntr->next);
        }
        else
        {
            struct interrupt* target = pIntr->next;
            pIntr->next = pIntr->next->next;
            free(target);
        }
    }
    pthread_spin_unlock(&idc.spinlock);
}

void attachTimerInterrupt(uint8_t pin, void (*callback)(void), uint32_t microseconds)
{
    if(!(pin == 128 || pin == 129))
        return;
    microseconds = microseconds > 0 ? microseconds : 1;
    pthread_spin_lock(&idc.spinlock);
    addIRQEntry(pin, callback, 0, microseconds);
    pthread_spin_unlock(&idc.spinlock);
}

int interrupt_init(void)
{
    pthread_spin_init(&idc.spinlock, 0);
    idc.head = NULL;
    int err = pthread_create(&idc.thread, NULL, intrMain, NULL);
	if(err != 0)
		printf("failed to create the thread\n");
	return 0;
}
