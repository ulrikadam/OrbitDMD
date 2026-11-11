//
// kernel.cpp
//
#include "kernel.h"
#include "common.h"
#include "webserver.h"
#include <circle/memory.h>
#include <circle/util.h>

static const char FromKernel[] = "kernel";

CKernel::CKernel (void)
:	m_Screen (m_Options.GetWidth (), m_Options.GetHeight ()),
	m_Timer (&m_Interrupt),
	m_Logger (m_Options.GetLogLevel (), &m_Timer),
	m_CPUThrottle (CPUSpeedMaximum),
	m_EMMC (&m_Interrupt, &m_Timer, &m_ActLED),
	m_I2CMaster (1),
	m_RTC (&m_I2CMaster),
	m_Clock (&m_RTC, &m_Config),
	m_Network (&m_Config),
	m_App (CMemorySystem::Get (), &m_Matrix, &m_Config, &m_Network)
{
	m_ActLED.Blink (3);	// signe de vie
}

CKernel::~CKernel (void)
{
}

boolean CKernel::Initialize (void)
{
	// ecran HDMI facultatif : sans ecran, le journal part sur le port serie
	boolean bScreenOK = m_Screen.Initialize ();
	m_Serial.Initialize (115200);

	CDevice *pTarget = m_DeviceNameService.GetDevice (m_Options.GetLogDevice (), FALSE);
	if (pTarget == 0)
	{
		pTarget = bScreenOK ? (CDevice *) &m_Screen : (CDevice *) &m_Serial;
	}

	if (   !m_Logger.Initialize (pTarget)
	    || !m_Interrupt.Initialize ()
	    || !m_Timer.Initialize ())
	{
		return FALSE;
	}

	m_Logger.Write (FromKernel, LogNotice, "OrbitDMD %s, compile le %s", GetVersion (), GetBuildDate ());

	// carte SD
	boolean bFileSystemOK = FALSE;
	if (!m_EMMC.Initialize ())
	{
		m_Logger.Write (FromKernel, LogError, "Carte SD non detectee");
	}
	else if (f_mount (&m_FileSystem, DRIVE, 1) != FR_OK)
	{
		m_Logger.Write (FromKernel, LogError, "Impossible de monter %s", DRIVE);
	}
	else
	{
		bFileSystemOK = TRUE;
		m_Config.Load (CONFIG_FILE);
	}
	m_Config.Log ();

	// journal sur l'ecran HDMI seulement si demande : chaque ligne y fait defiler toute
	// l'image, ce qui occupe le coeur 0 (celui du Wi-Fi) ; le journal reste lisible sur /journal
	if (!m_Config.LogToScreen)
	{
		m_Logger.Write (FromKernel, LogNotice, "Journal : page web /journal (ecran HDMI desactive)");
		m_Logger.SetNewTarget (&m_NullDevice);
	}

	// horloge a pile sur I2C1 (GPIO 2/3), inutilisable si 3 sorties en parallele
	if (m_Config.Parallel < 3 && m_I2CMaster.Initialize ())
	{
		m_RTC.SetAvailable (TRUE);
	}
	m_Clock.Initialize ();

	if (!m_Remote.Initialize ())
	{
		m_Logger.Write (FromKernel, LogPanic, "Memoire insuffisante");
		return FALSE;
	}

	// panneaux
	TMatrixOptions Options;
	Options.nRows = m_Config.PanelRows;
	Options.nCols = m_Config.PanelCols;
	Options.nChain = m_Config.Chain;
	Options.nParallel = m_Config.Parallel;
	Options.nSlowdown = m_Config.Slowdown;
	Options.nPWMBits = m_Config.PWMBits;
	Options.nPWMLSBNanos = m_Config.PWMLSBNanos;
	strncpy (Options.RGBOrder, m_Config.RGBOrder, sizeof Options.RGBOrder - 1);
	Options.RGBOrder[sizeof Options.RGBOrder - 1] = '\0';

	if (!m_Matrix.Initialize (Options))
	{
		// reglages refuses : on demarre quand meme (2 x 64x32), pour rester joignable
		m_Logger.Write (FromKernel, LogError, "Reglages des panneaux refuses : 2 x 64x32 par defaut");
		Options.nRows = 32;
		Options.nCols = 64;
		Options.nChain = 2;
		Options.nParallel = 1;
		m_Config.PanelRows = 32;
		m_Config.PanelCols = 64;
		m_Config.Chain = 2;
		m_Config.Parallel = 1;

		if (!m_Matrix.Initialize (Options))
		{
			m_Logger.Write (FromKernel, LogPanic, "Initialisation des panneaux impossible");
			return FALSE;
		}
	}

	if (!m_App.Prepare (bFileSystemOK))
	{
		m_Logger.Write (FromKernel, LogPanic, "Memoire insuffisante");
		return FALSE;
	}

	return m_App.Initialize ();	// en dernier : demarre les autres coeurs
}

TShutdownMode CKernel::Run (void)
{
	m_App.Run (0);		// ne revient pas (le Wi-Fi est demarre par l'application)

	return ShutdownHalt;
}
