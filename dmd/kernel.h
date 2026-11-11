//
// kernel.h
//
#ifndef _kernel_h
#define _kernel_h

#include <circle/actled.h>
#include <circle/koptions.h>
#include <circle/devicenameservice.h>
#include <circle/screen.h>
#include <circle/serial.h>
#include <circle/exceptionhandler.h>
#include <circle/interrupt.h>
#include <circle/timer.h>
#include <circle/logger.h>
#include <circle/sched/scheduler.h>
#include <circle/cputhrottle.h>
#include <circle/i2cmaster.h>
#include <SDCard/emmc.h>
#include <fatfs/ff.h>
#include <circle/types.h>
#include "config.h"
#include "matrix.h"
#include "rtc.h"
#include "clock.h"
#include "remote.h"
#include "network.h"
#include "dmdapp.h"

// destination du journal qui ne fait rien : le journal reste lisible sur la page web
// (/journal), sans le cout du dessin sur l'ecran HDMI a chaque ligne
class CNullDevice : public CDevice
{
public:
	int Write (const void *pBuffer, size_t nCount)	{ return (int) nCount; }
};

enum TShutdownMode
{
	ShutdownNone,
	ShutdownHalt,
	ShutdownReboot
};

class CKernel
{
public:
	CKernel (void);
	~CKernel (void);

	boolean Initialize (void);

	TShutdownMode Run (void);

private:
	// do not change this order
	CActLED			m_ActLED;
	CKernelOptions		m_Options;
	CDeviceNameService	m_DeviceNameService;
	CScreenDevice		m_Screen;
	CSerialDevice		m_Serial;
	CNullDevice		m_NullDevice;
	CExceptionHandler	m_ExceptionHandler;
	CInterruptSystem	m_Interrupt;
	CTimer			m_Timer;
	CLogger			m_Logger;
	CScheduler		m_Scheduler;
	CCPUThrottle		m_CPUThrottle;
	CEMMCDevice		m_EMMC;
	CI2CMaster		m_I2CMaster;
	FATFS			m_FileSystem;

	CConfig			m_Config;
	CMatrix			m_Matrix;
	CRtcDS1307		m_RTC;
	CClock			m_Clock;
	CRemoteControl		m_Remote;
	CNetwork		m_Network;
	CDmdApp			m_App;
};

#endif
