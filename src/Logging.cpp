#include "Common.h"
#include "Logging.h"

#include <stdarg.h>
#include <time.h>

VINYL_NS_BEGIN;

namespace Log
{
	FormatVarInt::FormatVarInt(int i)
	{
		m_value = i;
	}

	FormatVarInt::~FormatVarInt()
	{
		if (m_cache != nullptr) {
			delete m_cache;
		}
	}

	const char* FormatVarInt::GetString()
	{
		if (m_cache != nullptr) {
			delete m_cache;
		}
		m_cache = (char*)malloc(14);
		snprintf(m_cache, 14, "%d", m_value);
		return m_cache;
	}
	
	FormatVarString::FormatVarString(const char* str)
	{
		m_value = str;
	}

	const char* FormatVarString::GetString()
	{
		return m_value;
	}

	s::String Format(const char* format, va_list vl)
	{
		s::StackArray<FormatVar> vars;
		vars.sa_bOnlyPop = true;

		int numHighest = 0;

		int bufferLen = 128;
		s::String ret;

		int len = strlen(format);
		for (int i = 0; i < len; i++) {
			char c = format[i];
			if (c == '%') {
				char cn = format[++i];
				char str[5];
				int strc = 0;
				while (cn >= '0' && cn <= '9' && strc < 4 && i < len) {
					str[strc] = cn;
					strc++;
					cn = format[++i];
				}
				i--;
				str[strc] = '\0';
				int num = atoi(str);
				if (num == 0) {
					continue;
				}
				if (num > numHighest) {
					for (int j = numHighest; j < num; j++) {
						FormatVar &newvar = va_arg(vl, FormatVar);
						vars.Push(&newvar);
					}
					numHighest = num;
				}
				FormatVar &var = vars[num - 1];
				ret += var.GetString();
				continue;
			}

			ret += c;
		}

		return ret;
	}

	LogLevel Level = LogLevel_Info;

	static s::Mutex _mutex;

	static void LogLine(const char* keyword, const s::String &str, int16_t color_windows, const char* color_linux)
	{
		_mutex.Lock();

#define LOG_FORMAT_BEGIN "%7s | %02d:%02d:%02d | "

		time_t t = time(nullptr);
		tm tms;
		localtime_s(&tms, &t);

		s::String lineBegin = s::strPrintF(LOG_FORMAT_BEGIN, keyword, tms.tm_hour, tms.tm_min, tms.tm_sec);

#if WINDOWS
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, color_windows);
		printf("%s%s\n", (const char*)lineBegin, (const char*)str);
		SetConsoleTextAttribute(hConsole, 7);
#else
		printf("\x1B[%sm%s\x1B[0m%s\n", color_linux, (const char*)lineBegin, (const char*)str);
#endif

		_mutex.Unlock();
	}

	void Trace(const char* format, ...)
	{
		if (Level > LogLevel_Trace) {
			return;
		}

		va_list vl;
		va_start(vl, format);
		s::String str = Format(format, vl);
		va_end(vl);

		LogLine("TRACE", str, 8, "02");
	}

	void Debug(const char* format, ...)
	{
		if (Level > LogLevel_Debug) {
			return;
		}

		va_list vl;
		va_start(vl, format);
		s::String str = Format(format, vl);
		va_end(vl);

		LogLine("DEBUG", str, 7, "38");
	}

	void Info(const char* format, ...)
	{
		if (Level > LogLevel_Info) {
			return;
		}

		va_list vl;
		va_start(vl, format);
		s::String str = Format(format, vl);
		va_end(vl);

		LogLine("INFO", str, 15, "37");
	}

	void Warning(const char* format, ...)
	{
		if (Level > LogLevel_Warning) {
			return;
		}

		va_list vl;
		va_start(vl, format);
		s::String str = Format(format, vl);
		va_end(vl);

		LogLine("WARNING", str, 13, "35");
	}

	void Error(const char* format, ...)
	{
		if (Level > LogLevel_Error) {
			return;
		}

		va_list vl;
		va_start(vl, format);
		s::String str = Format(format, vl);
		va_end(vl);

		LogLine("ERROR", str, 14, "33");
	}

	void Fatal(const char* format, ...)
	{
		if (Level > LogLevel_Fatal) {
			return;
		}

		va_list vl;
		va_start(vl, format);
		s::String str = Format(format, vl);
		va_end(vl);

		LogLine("FATAL", str, 12, "31");
	}
}

VINYL_NS_END;
