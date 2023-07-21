#include "dbg.h"
#include "sys_plat.h"
#include <stdarg.h>

void dbg_print(const char* file, const char* func, int len, const char* fmt, ...)
{
	plat_printf("%s(%s) in (%d): ", file, func, len);

	char str_buf[128];
	// 获取可变参数
	va_list args;
	va_start(args, fmt);

	plat_vsprintf(str_buf, fmt, args);

	plat_printf("%s\n",str_buf);
	va_end(args);
}
