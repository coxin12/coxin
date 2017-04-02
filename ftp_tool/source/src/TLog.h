#ifndef _TLOG_H
#define _TLOG_H

#include <sys/types.h>
#include <pthread.h>
#include <logger/writelog.h>

#define ERROR 0
#define WARNING 1
#define INFO 2
#define DEBUG 3

#define DP TLog::print

namespace impl {
    void RTrim(char *ps, char cContent);
}

class TLog {
public:
    static void init(const char *PathName, int max_size, int max_index,
            const char *level = "INFO");
    static void destroy();

    static void print(int type, const char *fmt, ...);
private:
    static writelog_t *logger;
};

#endif
