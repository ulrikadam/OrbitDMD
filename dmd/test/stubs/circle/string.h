// Remplacement minimal de <circle/string.h> (CString) pour les tests sous Windows
#ifndef _circle_string_h
#define _circle_string_h

#include <stdarg.h>
#include <stdio.h>
#include <string>

class CString
{
public:
	CString (void) {}
	CString (const char *pText) : m_Text (pText) {}

	operator const char * (void) const	{ return m_Text.c_str (); }
	CString &operator = (const char *pText)	{ m_Text = pText; return *this; }
	size_t GetLength (void) const		{ return m_Text.size (); }

	void Append (const char *pText)		{ m_Text += pText; }
	void Append (char c)			{ m_Text += c; }

	void Format (const char *pFormat, ...)
	{
		char Buffer[4096];
		va_list Args;
		va_start (Args, pFormat);
		vsnprintf (Buffer, sizeof Buffer, pFormat, Args);
		va_end (Args);
		m_Text = Buffer;
	}

private:
	std::string m_Text;
};

#endif
