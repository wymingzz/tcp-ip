#include "dbg.h"
#include "sys_plat.h"
#include <stdarg.h>

void dbg_print(int m_level, int s_level, const char* file, const char* func, int len, const char* fmt, ...)
{
	if (m_level < s_level)return;

	static const char* title[] = {
		 "none: ",
		 DBG_STYLE_ERROR"error: ",
		 DBG_STYLE_WARNING"warning: ",
		 DBG_STYLE_RESET"info: "
	};

	plat_printf("%s%s(%s) in (%d): ", title[s_level], file, func, len);

	char str_buf[128];
	// 获取可变参数
	va_list args;
	va_start(args, fmt);

	plat_vsprintf(str_buf, fmt, args);

	plat_printf("%s\n", str_buf);
	plat_printf(DBG_STYLE_RESET);
	va_end(args);
}
