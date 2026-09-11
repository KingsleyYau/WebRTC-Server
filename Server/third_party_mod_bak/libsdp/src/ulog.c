#include "ulog.h"

static SDP_LOG_FUNC_IMP gLogImp;

void ulog_set_func(SDP_LOG_FUNC_IMP logImp) {
	gLogImp = logImp;
}

void ulog_simple_log(const char *file, int line, int level, const char *fmt, ...) {
	char logBuffer[4096] = {'\0'};

    if( gLogImp ) {
        va_list	ap;
        va_start(ap, fmt);
        vsnprintf(logBuffer, 4096 - 1, fmt, ap);
        va_end(ap);

    	gLogImp(file, line, level, logBuffer);

    } else {
        va_list	ap;
        va_start(ap, fmt);
        vsnprintf(logBuffer, 4096 - 1, fmt, ap);
        va_end(ap);

        printf("[%d] %s:%d %s \n", level, file, line, logBuffer);
    }

}
