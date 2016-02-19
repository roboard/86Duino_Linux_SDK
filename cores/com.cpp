#define __COM_LIB

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

// Linux no used
//#define  USE_COMMON
//#include "common.h"

#include "io.h"
#include "com.h"

#if defined(DMP_MSVC_WIN32) || defined(DMP_MSVC_WINCE)
    #include <windows.h>
    #include <tchar.h>
#elif defined(DMP_LINUX)
    #include <unistd.h>
    #include <termios.h>
    #include <fcntl.h>
    #include <sys/ioctl.h>
    #include <sys/resource.h>
#endif


#define COM_TIMEOUT         (5000L)  // timeout = 5s
static unsigned char  COM_ctrlREG[6]  = {0x00, 0x04, 0x08, 0x0C, 0x10, 0x14};  // SB UART-CTRL registers
//static unsigned char  COM_addrREG[4]  = {0x54, 0xa0, 0xa4, 0xa8};  // SB UART-ADDR registers


#if defined(DMP_MSVC_WIN32) || defined(DMP_MSVC_WINCE)
    #ifdef RB_MSVC_WINCE
        static LPCTSTR COM_portname[6] = {_T("COM1:"), _T("COM2:"), _T("COM3:"), _T("COM4:", _T("COM5:"), _T("COM6"))};
    #else
        static LPCTSTR COM_portname[6] = {_T("\\\\.\\COM1"), _T("\\\\.\\COM2"), _T("\\\\.\\COM3"), _T("\\\\.\\COM4", _T("\\\\.\\COM5"), _T("\\\\.\\COM6"))};
    #endif
    typedef struct com_port {
        HANDLE fp;
        DCB newstate;
        DCB oldstate;
        COMMTIMEOUTS newtimeouts;
        COMMTIMEOUTS oldtimeouts;
    } COM_t;
#elif defined(DMP_LINUX)
    static char* COM_portname[6] = {"/dev/ttyS0", "/dev/ttyS1", "/dev/ttyS2", "/dev/ttyS3", "/dev/ttyS4", "/dev/ttyS5"};
	static char User_portname[128] = {'\0'};
    typedef struct com_port {
        int fp;
        termios newstate;
        termios oldstate;
    } COM_t;

#else
    // TODO ...
    typedef int* COM_t;
#endif
static COM_t COM_info[6];

static bool COM_oldTMode[6] = {false, false, false, false, false, false};  // old turbo-mode setting
static bool COM_oldFMode[6] = {false, false, false, false, false, false};  // old FIFO32-mode setting
static int  COM_duplex[6] = {COM_FDUPLEX, COM_FDUPLEX, COM_FDUPLEX, COM_FDUPLEX, COM_FDUPLEX, COM_FDUPLEX};  // duplex-mode stting
static unsigned short COM_baseaddr[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  // I/O base address of each UART


/****************************  Internal Functions  ****************************/
DMP_INLINE(bool) uart_isenabled(int com) {
    unsigned short com_config_base = sb_Read16(0x60) & 0xfffe;
    return ((io_inpdw(com_config_base + COM_ctrlREG[com]) & (0x01L << 23)) == 0L)? false : true;
}

DMP_INLINE(unsigned short) uart_getbaseaddr(int com) {
    unsigned short com_config_base = sb_Read16(0x60) & 0xfffe;
    return (unsigned short)(io_inpdw(com_config_base + COM_ctrlREG[com]) & 0xfff8L);
}

DMP_INLINE(void) clear_rts(int com) {
    #if defined(DMP_MSVC_WIN32) || defined(DMP_MSVC_WINCE)
        EscapeCommFunction(COM_info[com].fp, CLRRTS);
    #elif defined(DMP_LINUX)
        int cmd;

        ioctl(COM_info[com].fp, TIOCMGET, &cmd);
        cmd &= ~TIOCM_RTS;
        ioctl(COM_info[com].fp, TIOCMSET, &cmd);
    #endif
}

DMP_INLINE(void) set_rts(int com) {
    #if defined(DMP_MSVC_WIN32) || defined(DMP_MSVC_WINCE)
        EscapeCommFunction (COM_info[com].fp, SETRTS);
    #elif defined(DMP_LINUX)
        int cmd;

        ioctl(COM_info[com].fp, TIOCMGET, &cmd);
        cmd |= TIOCM_RTS;
        ioctl(COM_info[com].fp, TIOCMSET, &cmd);
    #endif
}

DMP_INLINE(int) check_rfifo(int com) {
    #if defined(DMP_MSVC_WIN32) || defined(DMP_MSVC_WINCE)
        COMSTAT stat;
        
        stat.cbInQue = 0;
        if (ClearCommError(COM_info[com].fp, NULL, &stat) == FALSE) return -1;
        return (int)(stat.cbInQue);
    #elif defined(DMP_LINUX)
        int numbytes = 0;

        if (ioctl(COM_info[com].fp, FIONREAD, &numbytes) < 0) return -1;
        return numbytes;
    #else
        // TODO ...
        return -1;
    #endif
}

// Only for EX CPU
DMPAPI(void) com_EnableHalfDuplex(int com) {
	if (com >= 0 && com <= 9) {
		unsigned short uart_baseAddr = sb_Read16(0x60) & 0xfffe;
		sb_Write16(0x60, sb_Read16(0x60) | 0x0001);

		io_outpdw(uart_baseAddr + com*4, io_inpdw(uart_baseAddr + com*4) | 0x02000000L);
	}
}

DMPAPI(void) com_EnableFullDuplex(int com) {
	if (com >= 0 && com <= 9) {
		unsigned short uart_baseAddr = sb_Read16(0x60) & 0xfffe;
		sb_Write16(0x60, sb_Read16(0x60) | 0x0001);

		io_outpdw(uart_baseAddr + com*4, io_inpdw(uart_baseAddr + com*4) & 0xFDFFFFFFL);
	}
}
/*-----------------------  end of Internal Functions  ------------------------*/

DMPAPI(bool) com_SetNewPortName(int com, char* portname) {
	int i;
	if(portname == NULL) return false;
	for(i=0; i<128; i++) User_portname[i] = '\0';
	for(i=0; i<128; i++) if(portname[i] == '\0') break;
	if(i == 128 || i == 0) return false;
	for(i=0; portname[i] != '\0'; i++) User_portname[i] = portname[i];
	COM_portname[com] = &User_portname[0];
	return true;
}

static int COM_ioSection[6] = {-1, -1, -1, -1, -1, -1};
DMPAPI(bool) com_InUse(int com) {
    if((com < 0) || (com > 5)) return false;
    if(COM_ioSection[com] != -1) return true; else return false;
}

DMPAPI(bool) com_Init(int com) {
    if (com_InUse(com) == true)
    {
	printf("COM%d was already opened", com);
	return false;
    }

    if((COM_ioSection[com] = io_Init()) == -1) return false;

    if(uart_isenabled(com) == false)
    {
        printf("COM%d isn't enabled in BIOS", com);
        goto COMINIT_FAIL;
    }
    COM_baseaddr[com] = uart_getbaseaddr(com);
    COM_oldTMode[com] = com_IsTurboMode(com);
    COM_oldFMode[com] = com_IsFIFO32Mode(com);

    #if defined(DMP_MSVC_WIN32) || defined(DMP_MSVC_WINCE)
        {
        #ifdef RB_MSVC_WINCE
            int idx = com;
        #else
            int i, idx;

            // find the device name of the COM port
            for (idx=0, i=0; i<com; i++) if (uart_isenabled(i) == true) idx++;
        #endif
        
        COM_info[com].fp = CreateFile(
                               COM_portname[idx],             // device name of COM port
                               GENERIC_READ | GENERIC_WRITE,  // access mode
                               0,                             // share mode
                               0,                             // security attributes
                               OPEN_EXISTING,                 // opens a device only if it exists
                               0,                             // non-overlapped
                               NULL);                         // NULL when opening an existing file

    	if (COM_info[com].fp == INVALID_HANDLE_VALUE)
    	{
            printf("cannot open COM%d device driver", com);
            goto COMINIT_FAIL;
    	}

        // backup the old DCB
	    if (GetCommState(COM_info[com].fp, &(COM_info[com].oldstate)) == FALSE)
        {
            printf("fail to get DCB settings");
            goto COMINIT_FAIL2;
	    }
	    memcpy(&(COM_info[com].newstate), &(COM_info[com].oldstate), sizeof(DCB));

        // set new DCB
        COM_info[com].newstate.fBinary         = TRUE;                 // binary mode
        COM_info[com].newstate.fOutxCtsFlow    = FALSE;                // no CTS output control
        COM_info[com].newstate.fOutxDsrFlow    = FALSE;                // no DSR output control
        COM_info[com].newstate.fDtrControl     = DTR_CONTROL_DISABLE;  // no DRT control
        COM_info[com].newstate.fDsrSensitivity = FALSE;                // no sensitive to DSR
        COM_info[com].newstate.fOutX           = FALSE;                // no S/W output flow control
        COM_info[com].newstate.fInX            = FALSE;                // no S/W input flow control
        COM_info[com].newstate.fErrorChar      = FALSE;                // no replace parity-error byte
        COM_info[com].newstate.fNull           = FALSE;                // no discard NULL byte
        COM_info[com].newstate.fRtsControl     = RTS_CONTROL_DISABLE;  // no S/W input flow control
        COM_info[com].newstate.fAbortOnError   = FALSE;                // no terminate on errors
	    if (SetCommState(COM_info[com].fp, &(COM_info[com].newstate)) == FALSE)
        {
            printf("fail to set DCB settings");
            goto COMINIT_FAIL2;
	    }

        // get old timeout parameters
        if (GetCommTimeouts(COM_info[com].fp, &(COM_info[com].oldtimeouts)) == FALSE)
        {
            printf("fail to get TIMEOUTS settings");
            goto COMINIT_FAIL3;
	    }

        // set timeout parameters (no waiting on read/write)
        COM_info[com].newtimeouts.ReadIntervalTimeout         = MAXDWORD;
    	COM_info[com].newtimeouts.ReadTotalTimeoutConstant    = 0;
    	COM_info[com].newtimeouts.ReadTotalTimeoutMultiplier  = 0;
    	COM_info[com].newtimeouts.WriteTotalTimeoutConstant   = 0;
    	COM_info[com].newtimeouts.WriteTotalTimeoutMultiplier = 0;
        if (SetCommTimeouts(COM_info[com].fp, &(COM_info[com].newtimeouts)) == FALSE)
        {
            printf("fail to set TIMEOUT parameters");
            goto COMINIT_FAIL3;
        }
        
        ClearCommBreak(COM_info[com].fp);
        ClearCommError(COM_info[com].fp, NULL, NULL);  // clear all communication errors
        SetupComm(COM_info[com].fp, 8192, 8192);          // set read/write FIFO to 8KB
        PurgeComm(COM_info[com].fp, PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR);  // clear all communication buffers
        }
    #elif defined(DMP_LINUX)
        if ((COM_info[com].fp = open(COM_portname[com], O_RDWR | O_NOCTTY | O_NONBLOCK)) < 0)
        {
            printf("cannot open COM%d device driver", com);
            goto COMINIT_FAIL;
        }
    	
        // backup the old termios settings
    	if (tcgetattr(COM_info[com].fp, &(COM_info[com].oldstate)) < 0)
        {
            printf("fail to get termios settings");
            goto COMINIT_FAIL2;
	    }
	    memcpy(&(COM_info[com].newstate), &(COM_info[com].oldstate), sizeof(termios));

        // set new termios settings
        COM_info[com].newstate.c_cflag     |= CLOCAL | CREAD;
        COM_info[com].newstate.c_cflag     &= ~CRTSCTS;                 // disable H/W flow control
    	COM_info[com].newstate.c_lflag     &= ~(ICANON |                // raw mode
                                                ISIG   |                // disable SIGxxxx signals
                                                IEXTEN |                // disable extended functions
                                                ECHO | ECHOE);          // disable all auto-echo functions
    	COM_info[com].newstate.c_iflag     &= ~(IXON | IXOFF | IXANY);  // disable S/W flow control
    	COM_info[com].newstate.c_oflag     &= ~OPOST;                   // raw output
    	COM_info[com].newstate.c_cc[VTIME]  = 0;                        // no waiting to read
        COM_info[com].newstate.c_cc[VMIN]   = 0;
    	if(tcsetattr(COM_info[com].fp, TCSANOW, &(COM_info[com].newstate)) < 0)
    	{
            printf("fail to set termios settings");
            goto COMINIT_FAIL2;
        }
        
        // clear input/output buffers
    	tcflush(COM_info[com].fp, TCIOFLUSH);

    #else
        // TODO ...
        printf("unsupported platform");
        goto COMINIT_FAIL;
    #endif
    
    com_SetFormat(com, BYTESIZE8|STOPBIT1|COM_NOPARITY);  // default data format: 8 bits, 1 stop bit, no parity
    com_SetBaud(com, COMBAUD_115200BPS);              // default baudrate: 115200 bps
    // com_EnableFIFO32(com);                                                                  // default FIFO: 16
    return true;


    #if defined(DMP_MSVC_WIN32) || defined(DMP_MSVC_WINCE)
    COMINIT_FAIL3:
        SetCommState(COM_info[com].fp, &(COM_info[com].oldstate));

    COMINIT_FAIL2:
        CloseHandle(COM_info[com].fp);
    #elif defined(DMP_LINUX)
    COMINIT_FAIL2:
        close(COM_info[com].fp);
    #endif

    COMINIT_FAIL:
    io_Close();
    COM_ioSection[com] = -1;
    return false;
}

DMPAPI(void) com_Close(int com) {
    if(com_InUse(com) == false) return;

    #if defined(DMP_MSVC_WIN32) || defined(DMP_MSVC_WINCE)
        SetCommTimeouts(COM_info[com].fp, &COM_info[com].oldtimeouts);
        SetCommState(COM_info[com].fp, &(COM_info[com].oldstate));
    	CloseHandle(COM_info[com].fp);
    #elif defined(DMP_LINUX)
        tcsetattr(COM_info[com].fp, TCSANOW, &(COM_info[com].oldstate));
        close(COM_info[com].fp);
    #endif

    if (COM_oldTMode[com] == true) com_EnableTurboMode(com); else com_DisableTurboMode(com);
    if (COM_oldFMode[com] == true) com_EnableFIFO32(com);    else com_DisableFIFO32(com);

    io_Close();
    COM_ioSection[com] = -1;
}

DMPAPI(bool) com_SetFlowControl(int com, bool xonxoff, bool rtscts) {
	int action = TCIOFF;
    
	if (xonxoff) action = TCION;
    if (tcflow(COM_info[com].fp, action)) return false;
    
	// rtscts
    struct termios termio;

    // get current modes
    if (tcgetattr(COM_info[com].fp, &termio))
	{
        printf("uart: tcgetattr() failed");
        return false;
    }

    if (rtscts)
        COM_info[com].newstate.c_cflag |= CRTSCTS;
    else
        COM_info[com].newstate.c_cflag &= ~CRTSCTS;

    if (tcsetattr(COM_info[com].fp, TCSAFLUSH, &(COM_info[com].newstate)) < 0) {
        printf("uart: tcsetattr() failed");
        return false;
    }
	
	return true;
}

DMPAPI(bool) com_SetFormat(int com, unsigned char format) {
    int bytesize, stopbit, parity;
    switch(format&0x03)
    {
        case BYTESIZE5: bytesize = COM_BYTESIZE5; break;
        case BYTESIZE6: bytesize = COM_BYTESIZE6; break;
        case BYTESIZE7: bytesize = COM_BYTESIZE7; break;
        case BYTESIZE8: bytesize = COM_BYTESIZE8; break;
    }
    
    stopbit = format&0x04;
    parity = format&0x18;
    
    #if defined(DMP_MSVC_WIN32) || defined(DMP_MSVC_WINCE)
        COM_info[com].newstate.ByteSize = bytesize;
        
        switch (stopbit)
        {
            case STOPBIT1: COM_info[com].newstate.StopBits = ONESTOPBIT;  break;
            case STOPBIT2: COM_info[com].newstate.StopBits = TWOSTOPBITS; break;
        }
    
        switch (parity)
        {
            case COM_NOPARITY:
                COM_info[com].newstate.Parity = NOPARITY;   break;
            case COM_ODDPARITY:
                COM_info[com].newstate.Parity = ODDPARITY;  break;
            case COM_EVENPARITY:
                COM_info[com].newstate.Parity = EVENPARITY; break;
        }

	    if (SetCommState(COM_info[com].fp, &(COM_info[com].newstate)) == FALSE)
        {
            printf("fail to set DCB settings");
            return false;
	    }
    #elif defined(DMP_LINUX)
        switch (bytesize)
        {
            case COM_BYTESIZE5:
                COM_info[com].newstate.c_cflag = (COM_info[com].newstate.c_cflag & ~CSIZE) | CS5; break;
            case COM_BYTESIZE6:
                COM_info[com].newstate.c_cflag = (COM_info[com].newstate.c_cflag & ~CSIZE) | CS6; break;
            case COM_BYTESIZE7:
                COM_info[com].newstate.c_cflag = (COM_info[com].newstate.c_cflag & ~CSIZE) | CS7; break;
            case COM_BYTESIZE8:
                COM_info[com].newstate.c_cflag = (COM_info[com].newstate.c_cflag & ~CSIZE) | CS8; break;
        }

        switch (stopbit)
        {
            case STOPBIT1: COM_info[com].newstate.c_cflag &= ~CSTOPB; break;
            case STOPBIT2: COM_info[com].newstate.c_cflag |= CSTOPB;  break;
        }

        switch (parity)
        {
            case COM_NOPARITY:
                COM_info[com].newstate.c_cflag &= ~PARENB;
                COM_info[com].newstate.c_iflag &= ~(INPCK | ISTRIP | PARMRK);
                break;
            case COM_ODDPARITY:
                COM_info[com].newstate.c_cflag |= (PARENB | PARODD);
                COM_info[com].newstate.c_iflag |= (INPCK | ISTRIP | PARMRK);
                break;
            case COM_EVENPARITY:
                COM_info[com].newstate.c_cflag |= PARENB;
                COM_info[com].newstate.c_cflag &= ~PARODD;
                COM_info[com].newstate.c_iflag |= (INPCK | ISTRIP | PARMRK);
                break;
        }

    	if(tcsetattr(COM_info[com].fp, TCSANOW, &(COM_info[com].newstate)) < 0)
    	{
            printf("fail to set termios settings");
            return false;
        }
    #else
        // TODO ...
        printf("unsupported platform");
        return false;
    #endif

	return true;
}

DMPAPI(bool) com_SetBaud(int com, unsigned int baudrate) {
    unsigned int baud = baudrate;
    /*
    switch (baudrate)
    {
        case COMBAUD_748800BPS: baud = COMBAUD_57600BPS; break;
        case COMBAUD_499200BPS: baud = COMBAUD_38400BPS; break;
        case COMBAUD_249600BPS: baud = COMBAUD_19200BPS; break;
    }
        */
    #if defined(DMP_MSVC_WIN32) || defined(DMP_MSVC_WINCE)
        {
        DWORD oldbaud = COM_info[com].newstate.BaudRate;

        COM_info[com].newstate.BaudRate = 9600;
        
        switch (baud)
        {
            case  COMBAUD_50BPS:     COM_info[com].newstate.BaudRate = 50;     break;
            case  COMBAUD_300BPS:    COM_info[com].newstate.BaudRate = 300;    break;
            case  COMBAUD_1200BPS:   COM_info[com].newstate.BaudRate = 1200;   break;
            case  COMBAUD_2400BPS:   COM_info[com].newstate.BaudRate = 2400;   break;
            case  COMBAUD_4800BPS:   COM_info[com].newstate.BaudRate = 4800;   break;
            case  COMBAUD_9600BPS:   COM_info[com].newstate.BaudRate = 9600;   break;
            case  COMBAUD_19200BPS:  COM_info[com].newstate.BaudRate = 19200;  break;
            case  COMBAUD_38400BPS:  COM_info[com].newstate.BaudRate = 38400;  break;
            case  COMBAUD_57600BPS:  COM_info[com].newstate.BaudRate = 57600;  break;
            case  COMBAUD_115200BPS: COM_info[com].newstate.BaudRate = 115200; break;
        }

	    if (SetCommState(COM_info[com].fp, &(COM_info[com].newstate)) == FALSE)
        {
            COM_info[com].newstate.BaudRate = oldbaud;
            printf("fail to set DCB settings");
            return false;
	    }
	    
	    }
    #elif defined(DMP_LINUX)
        {
        speed_t oldospeed = cfgetospeed(&(COM_info[com].newstate));
        speed_t oldispeed = cfgetispeed(&(COM_info[com].newstate));
        speed_t newspeed  = B9600; // default is 9600

        switch (baud)
        {
            case  COMBAUD_50BPS:     newspeed = B50;     break;
            case  COMBAUD_300BPS:    newspeed = B300;    break;
            case  COMBAUD_1200BPS:   newspeed = B1200;   break;
            case  COMBAUD_2400BPS:   newspeed = B2400;   break;
            case  COMBAUD_4800BPS:   newspeed = B4800;   break;
            case  COMBAUD_9600BPS:   newspeed = B9600;   break;
            case  COMBAUD_19200BPS:  newspeed = B19200;  break;
            case  COMBAUD_38400BPS:  newspeed = B38400;  break;
            case  COMBAUD_57600BPS:  newspeed = B57600;  break;
            case  COMBAUD_115200BPS: newspeed = B115200; break;
        }
        cfsetospeed(&(COM_info[com].newstate), newspeed);
        cfsetispeed(&(COM_info[com].newstate), newspeed);

    	if(tcsetattr(COM_info[com].fp, TCSANOW, &(COM_info[com].newstate)) < 0)
    	{
            cfsetospeed(&(COM_info[com].newstate), oldospeed);
            cfsetispeed(&(COM_info[com].newstate), oldispeed);
            printf("fail to set termios settings");
            return false;
        }
        
        unsigned char lcr;
        unsigned char lsb = (unsigned char)(((unsigned short)baud) & 0x00FF);
        unsigned char msb = (unsigned char)((((unsigned short)baud) & 0xFF00) >> 8);

        lcr = io_inpb(COM_baseaddr[com] + 3);  
        io_outpb(COM_baseaddr[com] + 3, 0x80); 
        
        do {
            io_outpb(COM_baseaddr[com], lsb);
        } while (io_inpb(COM_baseaddr[com]) != lsb);
        
        do {
            io_outpb(COM_baseaddr[com] + 1, msb);
        } while (io_inpb(COM_baseaddr[com] + 1) != msb);
        
        io_inpb(0x80); // do IO delay
        io_outpb(COM_baseaddr[com] + 3, lcr);  
		
        }
    #else
        // TODO ...
        printf("unsupported platform");
        return false;
    #endif
    
    // if ((baudrate & 0x8000) != 0) com_EnableTurboMode(com); else com_DisableTurboMode(com);

    return true;
}



#define MAXRWSIZE   (256)
DMPAPI(bool) com_Receive(int com, unsigned char* buf, int bsize) {
    int numbytes, bsize2;
    unsigned long nowtime;

    for (; bsize > 0; bsize -= bsize2, buf += bsize2)
    {
        bsize2 = (bsize <= MAXRWSIZE)? bsize : MAXRWSIZE;

        // wait enough bytes to read
        for (nowtime = timer_NowTime(); (numbytes = check_rfifo(com)) < bsize2; )
        {
            if (numbytes < 0)
            {
                printf("fail to check read FIFO");
                return false;
            }

            if ((timer_NowTime() - nowtime) > COM_TIMEOUT)
            {
                printf("time-out to read bytes");
                return false;
            }
        } // end for (nowtime...
    
        #if defined(DMP_MSVC_WIN32) || defined(DMP_MSVC_WINCE)
            if (ReadFile(COM_info[com].fp, buf, bsize2, (LPDWORD)&numbytes, NULL) == FALSE)
            {
                printf("ReadFile() fails");
                return false;
            }
        #elif defined(DMP_LINUX)
            if ((numbytes = read(COM_info[com].fp, buf, bsize2)) < 0)
            {
                printf("read() fails");
                return false;
            }
        #else
            // TODO ...
            printf("unsupported platform");
            return false;
        #endif
    
        if (numbytes != bsize2)
        {
            printf("cannot read enough bytes");
            return false;
        }
    } // end for (; bsize...
    
    return true;
}

DMPAPI(unsigned int) com_Read(int com) {
    #if defined(DMP_MSVC_WIN32) || defined(DMP_MSVC_WINCE) || defined(DMP_LINUX)
        unsigned char val;

        if (com_Receive(com, &val, 1) == false) return 0xffff;
        return (unsigned int)val;
    #else
        // TODO ...
        printf("unsupported platform");
        return 0xffff;
    #endif
}

DMPAPI(bool) com_ClearRFIFO(int com) {
    #if defined(DMP_MSVC_WIN32) || defined(DMP_MSVC_WINCE)
        if (PurgeComm(COM_info[com].fp, PURGE_RXCLEAR) == FALSE)
        {
            printf("fail to clear read FIFO");
            return false;
        }
    #elif defined(DMP_LINUX)
        if (tcflush(COM_info[com].fp, TCIFLUSH) < 0)
        {
            printf("fail to clear read FIFO");
            return false;
        }
	#endif

    return true;
}

DMPAPI(int) com_QueryRFIFO(int com) {
    int numbytes = check_rfifo(com);

    if (numbytes < 0) printf("fail to query read FIFO");
    return numbytes;
}

DMPAPI(bool) com_Send(int com, unsigned char* buf, int bsize) {
    unsigned long nowtime;
    int numbytes = 0, bsize2;

    while (bsize > 0)
    {
        bsize2 = (bsize <= MAXRWSIZE)? bsize : MAXRWSIZE;
        
        for (nowtime = timer_NowTime(); bsize2 > 0; buf += numbytes, bsize2 -= numbytes, bsize -= numbytes)
        {
            #if defined(DMP_MSVC_WIN32) || defined(DMP_MSVC_WINCE)
                if (WriteFile(COM_info[com].fp, buf, bsize2, (LPDWORD)&numbytes, NULL) == FALSE)
                {
                    printf("WriteFile() fails");
                    goto SEND_FAIL;
                }
            #elif defined(DMP_LINUX)
                if ((numbytes = write(COM_info[com].fp, buf, bsize2)) < 0)
                {
                    printf("write() fails");
                    goto SEND_FAIL;
                }
            #else
                // TODO ...
                printf("unsupported platform");
                goto SEND_FAIL;
            #endif
            
            if ((timer_NowTime() - nowtime) > COM_TIMEOUT)
            {
                printf("time-out to write bytes");
                goto SEND_FAIL;
            }
        } // for (nowtime...
    } // end while (bsize...

    return true;

SEND_FAIL:
    return false;
}

DMPAPI(bool) com_Write(int com, unsigned char val) {
    #if defined(DMP_MSVC_WIN32) || defined(DMP_MSVC_WINCE) || defined(DMP_LINUX)
        return com_Send(com, &val, 1);
    #else
        // TODO ...
        printf("unsupported platform");
        return false;
    #endif
}

DMPAPI(bool) com_FlushWFIFO(int com) {
    #if defined(DMP_MSVC_WIN32) || defined(DMP_MSVC_WINCE)
        if (FlushFileBuffers(COM_info[com].fp) == FALSE)
        {
            printf("FlushFileBuffers() fails");
            return false;
        }
    #elif defined(DMP_LINUX)
        if (COM_duplex[com] != COM_HDUPLEX_RTS)
        {
            if (tcdrain(COM_info[com].fp) < 0)
            {
                printf("tcdrain() fails");
                return false;
            }
        }
        else  // tcdrain() is too slow in the purpose of switching RTS to read servos in readtime
        {
            //use "ioctl(fd, FIONWRITE, &nBytes)" to wait write-FIFO empty...
        }
    #endif
    
    /*
    if (COM_duplex[com] == COM_HDUPLEX_RTS)  // ensure that all bytes in 16550 FIFO have been sent
        while((io_inpb(COM_baseaddr[com] + 5) & 0x60) != 0x60);
          */
    return true;
}

DMPAPI(bool) com_ClearWFIFO(int com) {
    #if defined(DMP_MSVC_WIN32) || defined(DMP_MSVC_WINCE)
        if (PurgeComm(COM_info[com].fp, PURGE_TXCLEAR) == FALSE)
        {
            printf("fail to clear write FIFO");
            return false;
        }
    #elif defined(DMP_LINUX)
        if (tcflush(COM_info[com].fp, TCOFLUSH) < 0)
        {
            printf("fail to clear write FIFO");
            return false;
        }
    #endif
    
    return true;
}


/*
#ifdef ROBOIO
    DMPAPI(bool) com_ServoTRX(int com, unsigned char* cmd, int csize, unsigned char* buf, int bsize) {
        com_ClearRFIFO(com);
    
        if (com_Send(com, cmd, csize) == false) return false;

        if (COM_duplex[com] == COM_HDUPLEX)  // discard the first-received csize bytes in case of TX/RX-short
        {
            int i; unsigned int tmpbyte;

            for (i=0; i<csize; i++)
            {
                if ((tmpbyte = com_Read(com)) == 0xffff) return false;
        
                if ((unsigned char)tmpbyte != cmd[i])
                {
                    printf("receive a wrong self-feedback byte");
                    return false;
                }
            }
        } // end if (COM_duplex[com] ...
    
        if (buf != NULL) //&&
        if (com_Receive(com, buf, bsize) == false) return false;

        return true;
    }
#endif
*/


/*************************  Isolated COM lib Functions  ***********************/
DMPAPI(void) com_EnableTurboMode(int com) {
    unsigned short com_config_base;
    if ((com < 0) || (com > 5)) return;

    com_config_base = sb_Read16(0x60) & 0xfffe;
    sb_Write(com_config_base + COM_ctrlREG[com], sb_Read(com_config_base + COM_ctrlREG[com]) | (1L<<22));
}

DMPAPI(void) com_DisableTurboMode(int com) {
    unsigned short com_config_base;
    if ((com < 0) || (com > 5)) return;

    com_config_base = sb_Read16(0x60) & 0xfffe;
    sb_Write(com_config_base + COM_ctrlREG[com], sb_Read(com_config_base + COM_ctrlREG[com]) & ~(1L<<22));
}

DMPAPI(bool) com_IsTurboMode(int com) {
    unsigned short com_config_base;
    if ((com < 0) || (com > 5)) return false;

    com_config_base = sb_Read16(0x60) & 0xfffe;
    if ((sb_Read(com_config_base + COM_ctrlREG[com]) & (1L<<22)) == 0L) return false;
    return true;
}

DMPAPI(void) com_EnableFIFO32(int com) {
    unsigned short com_config_base;
    if ((com < 0) || (com > 5)) return;

    com_config_base = sb_Read16(0x60) & 0xfffe;
    sb_Write(com_config_base + COM_ctrlREG[com], sb_Read(com_config_base + COM_ctrlREG[com]) | (1L<<21));
}

DMPAPI(void) com_DisableFIFO32(int com) {
    unsigned short com_config_base;
    if ((com < 0) || (com > 5)) return;

    com_config_base = sb_Read16(0x60) & 0xfffe;
    sb_Write(com_config_base + COM_ctrlREG[com], sb_Read(com_config_base + COM_ctrlREG[com]) & ~(1L<<21));
}

DMPAPI(bool) com_IsFIFO32Mode(int com) {
    unsigned short com_config_base;
    if ((com < 0) || (com > 5)) return false;
    
    com_config_base = sb_Read16(0x60) & 0xfffe;
    if ((sb_Read(com_config_base + COM_ctrlREG[com]) & (1L<<21)) == 0L) return false;
    return true;
}
/*--------------------  end of Isolated COM lib Functions  -------------------*/



/****************************  COM STDIO Functions  ***************************/
/*
#ifdef _MANAGED
	#pragma managed(push, off)
#endif
DMPAPI(bool) com_printf(int com, char* fmt, ...) {
    char buf[512];
    va_list args;

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);
	
	return com_Send(com, (unsigned char*)buf, (int)strlen(buf));
}

DMPAPI(bool) com1_printf(char* fmt, ...) {
    char buf[512];
    va_list args;

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);
	
	return com1_Send((unsigned char*)buf, (int)strlen(buf));
}

DMPAPI(bool) com2_printf(char* fmt, ...) {
    char buf[512];
    va_list args;

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);
	
	return com2_Send((unsigned char*)buf, (int)strlen(buf));
}

DMPAPI(bool) com3_printf(char* fmt, ...) {
    char buf[512];
    va_list args;

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);
	
	return com3_Send((unsigned char*)buf, (int)strlen(buf));
}

DMPAPI(bool) com4_printf(char* fmt, ...) {
    char buf[512];
    va_list args;

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);
	
	return com4_Send((unsigned char*)buf, (int)strlen(buf));
}
#ifdef _MANAGED
	#pragma managed(pop)
#endif
*/

DMPAPI(bool) com_kbhit(int com) {
    if (check_rfifo(com) > 0) return true;
	return false;
}

DMPAPI(unsigned int) com_getch(int com) {
    int numbytes;
    
    while ((numbytes = check_rfifo(com)) <= 0)
        if (numbytes < 0) return 0xffff;

    return (int)com_Read(com);
}
/*---------------------- end of COM STDIO Functions  -------------------------*/


