/**
 *	\file
 *	\brief
 *	\author		$Author: ljy $
 *	\version	$Revision: 1.5 $
 *	\date
 *	$Id: TLog.cpp,v 1.5 2007/03/13 01:42:01 ljy Exp $
 *	\history
 {*
 * $Log: TLog.cpp,v $
 * Revision 1.5  2007/03/13 01:42:01  ljy
 * 增加警告级别的日志
 *
 * Revision 1.4  2007/01/30 08:28:13  ljy
 * 增加对符号连接的支持
 *
 * Revision 1.3  2007/01/05 08:17:46  ljy
 * 程序移植到linux
 *
 * Revision 1.2  2007/01/04 08:58:34  ljy
 * HP系统移植
 *
 * Revision 1.1  2007/01/04 01:32:58  ljy
 * 代码首次入库
 *
 * Revision 12.00.002  2012/08/07 chenzhixin
 * 更改LOG类内部实现
 *
 }*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdexcept>
#include <logger/writelog.h>

#include "TLog.h"

namespace impl {
    void RTrim(char *ps, char cContent) {
        int nLength = strlen(ps) - 1;
        for (int i = nLength; i > 0; i--) {
            if (ps[i] == cContent) {
                ps[i] = '\0';
            } else {
                break;
            }
        }
    }
}

writelog_t* TLog::logger = 0;

void TLog::init(const char *PathName, int max_size, int max_index,
        const char *level) {
    if (logger == 0) {
        logger = new writelog_t("mainlog", PathName, max_size, level, max_index);
    }
}

void TLog::destroy() {
    if (logger) {
        delete logger;
        logger = 0;
    }
}

void TLog::print(int type, const char *fmt, ...) {
    if (logger == 0) {
        throw std::runtime_error(std::string("TLog is not Init!"));
    }
    char msg[2048];
    va_list args;

    va_start(args, fmt);
    vsprintf(msg, fmt, args);
    va_end(args);
    impl::RTrim(msg, '\n');

    switch (type) {
    case DEBUG:
        logger->debug("%s", msg);
        break;

    case INFO:
        logger->info("%s", msg);
        break;
    case WARNING:
        logger->warn("%s", msg);
        break;

    case ERROR:
        logger->error("%s", msg);
        break;

    default:
        return;
        break;
    }
}
