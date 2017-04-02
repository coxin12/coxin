/*
 * transfertask.cpp
 *
 *  Created on: 2013年11月5日
 *      Author: liangjianqiang
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "TransferTask.h"
#include "GlobalSetting.h"

TransferTask::TransferTask() {
    regexType = Match;
    transMode = BIN;
    maxFiles = 50;
    transLogMaxSize = GlobalSetting::DEFAULT_ERR_LOG_MAX_SIZE;
    transLogMaxIndex = 2;
    transBlocks = 30;
    pattern = "*";
    lastExecuteTime = 0;


    GlobalSetting *global = GlobalSetting::getInstance();
    if (global->interval != 0) {
        interval = global->interval;
    } else {
        interval = 3;
    }
    if (global->resendInterval != 0) {
        resendInterval = global->resendInterval;
    } else {
        resendInterval = GlobalSetting::DEFAULT_RESEND_INTERVAL;
    }
}

TransferTask::~TransferTask() {
}

void TransferTask::setValue(std::string key, std::string value) {
    char buf[255] = { 0 };

    if (strncasecmp(key.c_str(), "source", 6) == 0) {
        int n = 0;
        char user[255] = { 0 };
        char host[255] = { 0 };
        char mode[128] = { 0 };
        char path[1024] = { 0 };
        if (strchr(value.c_str(), '@') == 0) {
            n = sscanf(value.c_str(), "\"%[^\"]\":\"%[^\"]\"", host, path);
            srcNode.sHost = host;
            srcNode.sPath = path;
        } else {
            n = sscanf(value.c_str(),
                    "\"%[^\"]\"@\"%[^\"]\":\"%[^\"]\":\"%[^\"]\"", user, host,
                    mode, path);
            srcNode.sUser = user;
            srcNode.sHost = host;
            srcNode.sPath = path;
        }

        switch (n) {
        case 3:
            srcNode.sPath = mode;
            break;
        case 2:
        case 4:
            break;
        default:
            throw std::runtime_error(value + " : configuration is invalid.");
            break;
        }

        if (strncasecmp(mode, "sftp", 4) == 0) {
            srcNode.iProtocol = SFTP;
            srcNode.iLinkMode = 1;
        } else {
            srcNode.iProtocol = FTP;
            if (strncasecmp(mode, "pasv", 4) == 0) {
                srcNode.iLinkMode = PASSIVE;
            } else {
                srcNode.iLinkMode = PORT;
            }
        }
    } else if (strncasecmp(key.c_str(), "target", 6) == 0) {
        int n = 0;
        char user[255] = { 0 };
        char host[255] = { 0 };
        char mode[128] = { 0 };
        char path[1024] = { 0 };
        HostNode destNodeInfo;

        if (strchr(value.c_str(), '@') == 0) {
            n = sscanf(value.c_str(), "\"%[^\"]\":\"%[^\"]\"", host, path);
            destNodeInfo.sHost = host;
            destNodeInfo.sPath = path;
        } else {
            n = sscanf(value.c_str(),
                    "\"%[^\"]\"@\"%[^\"]\":\"%[^\"]\":\"%[^\"]\"", user, host,
                    mode, path);
            destNodeInfo.sUser = user;
            destNodeInfo.sHost = host;
            destNodeInfo.sPath = path;
        }

        switch (n) {
        case 3:
            destNodeInfo.sPath = mode;
            break;
        case 2:
        case 4:
            break;
        default:
            throw std::runtime_error(value + " : configuration is invalid.");
            break;
        }

        // 修改目的路径参数，如果是根目录则设置为空
        if (destNodeInfo.sPath.compare("/") == 0) {
            destNodeInfo.sPath = "";
        }

        if (strncasecmp(mode, "sftp", 4) == 0) {
            destNodeInfo.iProtocol = SFTP;
            destNodeInfo.iLinkMode = 1;
        } else {
            destNodeInfo.iProtocol = FTP;
            if (strncasecmp(mode, "pasv", 4) == 0) {
                destNodeInfo.iLinkMode = PASSIVE;
            } else {
                destNodeInfo.iLinkMode = PORT;
            }
        }
        dstNode.push_back(destNodeInfo);
    } else if (strncasecmp(key.c_str(), "trans_type", 10) == 0
            || strncasecmp(key.c_str(), "transtype", 9) == 0) {
        sscanf(value.c_str(), "\"%[^\"]\"", buf);
        if (strncasecmp(buf, "ascii", 5) == 0
                || strncasecmp(buf, "asc", 3) == 0) {
            transMode = ASCII;
        } else {
            transMode = BIN;
        }
    } else if (strncasecmp(key.c_str(), "filter", 6) == 0) {
        sscanf(value.c_str(), "\"%[^\"]\"", buf);
        pattern = buf;
    } else if (strncasecmp(key.c_str(), "regex_type", 10) == 0
            || strncasecmp(key.c_str(), "retype", 6) == 0) {
        sscanf(value.c_str(), "\"%[^\"]\"", buf);
        if (strlen(buf) != 1 || buf[0] < '0' || buf[0] > '9') {
            char err[512] = { 0 };
            snprintf(err, strlen(err), "regex_type(%s) not in (0, 1)", buf);
            throw std::runtime_error(err);
        }
        regexType = atoi(buf);
    } else if (strncasecmp(key.c_str(), "bak1path", 8) == 0) {
        sscanf(value.c_str(), "\"%[^\"]\"", buf);
        bak1Path = buf;
    } else if (strncasecmp(key.c_str(), "bak2path", 8) == 0) {
        sscanf(value.c_str(), "\"%[^\"]\"", buf);
        bak2Path = buf;
    } else if (strncasecmp(key.c_str(), "bak3path", 8) == 0) {
        sscanf(value.c_str(), "\"%[^\"]\"", buf);
        bak3Path = buf;
    } else if (strncasecmp(key.c_str(), "bak4path", 8) == 0) {
        sscanf(value.c_str(), "\"%[^\"]\"", buf);
        bak4Path = buf;
    } else if (strncasecmp(key.c_str(), "msgname", 7) == 0) {
        sscanf(value.c_str(), "\"%[^\"]\"", buf);
        taskLogFile = buf;
    } else if (strncasecmp(key.c_str(), "taskid", 6) == 0) {
        sscanf(value.c_str(), "\"%[^\"]\"", buf);
        taskID = buf;
    } else if (strncasecmp(key.c_str(), "temp", 4) == 0) {
        sscanf(value.c_str(), "\"%[^\"]\"", buf);
        tmpPath = buf;
    } else if (strncasecmp(key.c_str(), "maxfiles", 8) == 0) {
        sscanf(value.c_str(), "\"%[^\"]\"", buf);
        maxFiles = atoi(buf);
        if(maxFiles < 50) {
            maxFiles = 50;
        }
    } else if (strncasecmp(key.c_str(), "interval", 8) == 0) {
        sscanf(value.c_str(), "\"%[^\"]\"", buf);
        interval = atoi(buf);
    } else if (strncasecmp(key.c_str(), "resendinterval", 14) == 0) {
        sscanf(value.c_str(), "\"%[^\"]\"", buf);
        resendInterval = atoi(buf);
    } else if (strncasecmp(key.c_str(), "isrename", 8) == 0) {
        sscanf(value.c_str(), "\"%[^\"]\"", buf);
        isRename = buf;
    } else if (strncasecmp(key.c_str(), "translogMaxsize", 15) == 0) {
        sscanf(value.c_str(), "\"%[^\"]\"", buf);
        transLogMaxSize = atoi(buf);
        if (transLogMaxSize == 0) {
            transLogMaxSize = GlobalSetting::DEFAULT_ERR_LOG_MAX_SIZE;
        }
    } else if (strncasecmp(key.c_str(), "translogMaxindex", 16) == 0) {
        sscanf(value.c_str(), "\"%[^\"]\"", buf);
        transLogMaxIndex = atoi(buf);
        if (transLogMaxIndex == 0) {
            transLogMaxIndex = 2;
        }
    } else if (strncasecmp(key.c_str(), "transBlock", 10) == 0) {
        sscanf(value.c_str(), "\"%[^\"]\"", buf);
        transBlocks = atoi(buf);
        if (transBlocks < 30) {
            transBlocks = 30;
        }
    }
}
