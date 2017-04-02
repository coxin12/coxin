/*
 * globalsetting.cpp
 *
 *  Created on: 2013Äê11ÔÂ5ÈÕ
 *      Author: liangjianqiang
 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "GlobalSetting.h"

GlobalSetting *GlobalSetting::globalSetting = 0;

GlobalSetting *GlobalSetting::getInstance() {
    if (globalSetting == 0) {
        globalSetting = new GlobalSetting();
    }
    return globalSetting;
}

const int GlobalSetting::DEFAULT_ERR_LOG_MAX_SIZE = 629145600; // 600 * 1024 * 1024;
const int GlobalSetting::DEFAULT_ERR_LOG_MAX_INDEX = 10;
const char *GlobalSetting::DEFAULT_STATIC_FILE = "./statefile.log";
const int GlobalSetting::DEFAULT_INTERVAL = 10;
const int GlobalSetting::DEFAULT_RESEND_INTERVAL = 60;
const int GlobalSetting::DEFAULT_STAT_INTERVAL = 60;
const char *GlobalSetting::RESEND_DIR_NAME = "resend";

GlobalSetting::GlobalSetting() :
        threadCount(0), interval(DEFAULT_INTERVAL),
        resendInterval(DEFAULT_RESEND_INTERVAL),
        statInterval(DEFAULT_STAT_INTERVAL),
        errLogMaxSize(DEFAULT_ERR_LOG_MAX_SIZE),
        errLogMaxIndex(DEFAULT_ERR_LOG_MAX_INDEX) {
}

GlobalSetting::~GlobalSetting() {
}

void GlobalSetting::setValue(std::string key, std::string value) {
    char buf[256] = { 0 };

    if (strncasecmp(key.c_str(), "wkpath", 6) == 0) {
        sscanf(value.c_str(), "\"%[^\"]\"", buf);
        workPath = buf;
    } else if (strncasecmp(key.c_str(), "threads", 7) == 0) {
        sscanf(value.c_str(), "\"%[^\"]\"", buf);
        threadCount = atoi(buf);
    } else if (strncasecmp(key.c_str(), "logpath", 7) == 0) {
        sscanf(value.c_str(), "\"%[^\"]\"", buf);
        logPath = buf;
    } else if (strncasecmp(key.c_str(), "interval", 8) == 0) {
        sscanf(value.c_str(), "\"%[^\"]\"", buf);
        interval = atoi(buf);
    } else if (strncasecmp(key.c_str(), "resendinterval", 14) == 0) {
        sscanf(value.c_str(), "\"%[^\"]\"", buf);
        resendInterval = atoi(buf);
    } else if (strncasecmp(key.c_str(), "statefile", 9) == 0) {
        sscanf(value.c_str(), "\"%[^\"]\"", buf);
        statFile = buf;
    } else if (strncasecmp(key.c_str(), "stateinterval", 13) == 0) {
        sscanf(value.c_str(), "\"%[^\"]\"", buf);
        statInterval = atoi(buf);
        if (statInterval == 0) {
            statInterval = 60;
        }
    } else if (strncasecmp(key.c_str(), "localaddr", 9) == 0) {
        sscanf(value.c_str(), "\"%[^\"]\"", buf);
        localAddr = buf;
    } else if (strncasecmp(key.c_str(), "finshfile", 9) == 0) {
        sscanf(value.c_str(), "\"%[^\"]\"", buf);
        finishFile = buf;
    } else if (strncasecmp(key.c_str(), "rsapubfile", 10) == 0) {
        sscanf(value.c_str(), "\"%[^\"]\"", buf);
        rsaPubFile = buf;
    } else if (strncasecmp(key.c_str(), "rsafile", 7) == 0) {
        sscanf(value.c_str(), "\"%[^\"]\"", buf);
        rsaFile = buf;
    } else if (strncasecmp(key.c_str(), "ftppwdfile", 10) == 0) {
        sscanf(value.c_str(), "\"%[^\"]\"", buf);
        passwdFile = buf;
    } else if (strncasecmp(key.c_str(), "errlogMaxsize", 13) == 0) {
        sscanf(value.c_str(), "\"%[^\"]\"", buf);
        errLogMaxSize = atoi(buf);
        if (errLogMaxSize == 0) {
            errLogMaxSize = GlobalSetting::DEFAULT_ERR_LOG_MAX_SIZE;
        }
    } else if (strncasecmp(key.c_str(), "errlogMaxindex", 14) == 0) {
        sscanf(value.c_str(), "\"%[^\"]\"", buf);
        errLogMaxIndex = atoi(buf);
        if (errLogMaxIndex == 0) {
            errLogMaxIndex = GlobalSetting::DEFAULT_ERR_LOG_MAX_INDEX;
        }
    }
}
