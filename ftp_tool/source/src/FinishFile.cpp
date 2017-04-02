/*
 * finishfile.cpp
 *
 *  Created on: 2013Äê11ÔÂ7ÈÕ
 *      Author: liangjianqiang
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdexcept>
#include "FinishFile.h"
#include "TLog.h"
#include "GlobalSetting.h"

FinishFile::FinishFile() {
}

FinishFile::~FinishFile() {
}

FinishFile *FinishFile::finishFile = 0;
FinishFile *FinishFile::getFinishFile() {
    if (finishFile == 0) {
        finishFile = new FinishFile();
    }
    return finishFile;
}

int FinishFile::initialize() {
    GlobalSetting *global = GlobalSetting::getInstance();
    const char *finishfile = global->finishFile.c_str();
    try {
        if (0 == strlen(finishfile)) {
            throw std::runtime_error("finshfile is empty");
        }
        DP(DEBUG, "Open finshfile:%s\n", finishfile);
        FILE *pFinishFile = fopen(finishfile, "w");
        if (!pFinishFile) {
            throw std::runtime_error(
                    std::string("Open Error ") + strerror(errno));
        }
        char flagmsg[] = "finshflag=0";
        DP(DEBUG, "write finshfile:%s content: %s\n", finishfile, flagmsg);
        if (EOF == fputs(flagmsg, pFinishFile)) {
            fclose(pFinishFile);
            throw std::runtime_error(
                    std::string("Write Error ") + strerror(errno));
        }
        DP(DEBUG, "close finshfile:%s\n", finishfile);
        if (0 != fclose(pFinishFile)) {
            throw std::runtime_error(
                    std::string("Close Error ") + strerror(errno));
        }
    } catch (std::exception &e) {
        fprintf(stderr, "%s", (global->finishFile + ": " + e.what()).c_str());
        DP(ERROR, (global->finishFile + ": " + e.what()).c_str());
        return -1;
    }
    return 0;
}

bool FinishFile::getFinishFlag() {
    GlobalSetting *global = GlobalSetting::getInstance();
    const char *finishFile = global->finishFile.c_str();
    try {
        FILE *pFinishFile = fopen(finishFile, "r");
        if (!pFinishFile) {
            throw std::runtime_error(
                    std::string("Open Error ") + strerror(errno));
        }
        char lineBuffer[256] = { 0 };
        char key[256] = { 0 };
        char value[256] = { 0 };
        if (fgets(lineBuffer, sizeof(lineBuffer), pFinishFile)) {
            if (sscanf(lineBuffer, "%[^=]=%s", key, value) == 2) {
                if (1 == atoi(value)) {
                    DP(INFO, (global->finishFile + ": set Shutdown flag\n").c_str());
                    return true;
                }
            } else {
                fclose(pFinishFile);
                throw std::runtime_error(
                        std::string("key must be \"finshflag\" but there is ")
                                + key);
            }
        } else {
            fclose(pFinishFile);
            throw std::runtime_error("File is empty");
        }
        fclose(pFinishFile);
    } catch (std::exception &e) {
        DP(ERROR, (global->finishFile + ": " + e.what() + "\n").c_str());
        return true;
    }

    return false;
}
