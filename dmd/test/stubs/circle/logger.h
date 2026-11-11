// Remplacement minimal de <circle/logger.h> pour les tests sous Windows
#ifndef _circle_logger_h
#define _circle_logger_h

#include <stdio.h>
#include <stdarg.h>

enum TLogSeverity
{
	LogPanic,
	LogError,
	LogWarning,
	LogNotice,
	LogDebug
};

class CLogger
{
public:
	static CLogger *Get (void)
	{
		static CLogger s_Logger;
		return &s_Logger;
	}

	void Write (const char *pSource, TLogSeverity Severity, const char *pMessage, ...)
	{
		va_list Args;
		va_start (Args, pMessage);
		fprintf (stderr, "[%s] ", pSource);
		vfprintf (stderr, pMessage, Args);
		fprintf (stderr, "\n");
		va_end (Args);
	}
};

#endif
