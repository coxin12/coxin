/******************************************************************
 * Copyright by SunRise, 2013-11-07
 * File Name: Config.cpp
 *
 * Modified History:
 *     See ChangeLog.
 ******************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <libgen.h>
#include <desini/desini.h>
#include "dstruct.h"
#include "Config.h"
#include "GlobalSetting.h"
#include "TLog.h"

namespace util {
    void delSpace(char *buf) {
        int i = 0;
        char *p = buf;
        char tmpbuf[255] = { 0 };

        if (p == 0) {
            return;
        }

        while (*p != '\0') {
            if (*p != 32 && *p != 9) {
                tmpbuf[i++] = *p;
            }
            ++p;
        }
        tmpbuf[i] = '\0';

        strcpy(buf, tmpbuf);
    }

    bool IsBigEndian() {
        int a = 0x1234;
        char b = *(char*) &a;
        if (b == 0x12) {
            return true;
        } else {
            return false;
        }
    }
}

std::set<std::string> Config::taskIdSet;

Config::Config() {
}

Config::~Config() {
}

int Config::ReadFile(std::string configFile) {
    GlobalSetting *global = GlobalSetting::getInstance();   // ��ʼ��ȫ������
    filepath = configFile;
    const char *cfgfile = configFile.c_str();
    taskIdSet.clear();
    taskArray.clear();

    cfgBaseName = basename(const_cast<char *>(filepath.c_str()));
    std::string::size_type pos = cfgBaseName.find_last_of(".");
    if (pos != std::string::npos) {
        cfgBaseName = cfgBaseName.substr(0, pos);
    }

    TransferTask *pTask = 0;

    checkConfigFile();

    FILE *fp = 0;
    if ((fp = fopen(cfgfile, "r")) == 0) {
        throw std::runtime_error(configFile + ":" + strerror(errno));
    }

    bool isGlobalPara = false;
    bool isTaskPara = false;
    char lineBuf[1024] = { 0 };
    while (fgets(lineBuf, sizeof(lineBuf), fp) != 0) {
        if (lineBuf[0] == '#') {
            bool isTaskEnd = strncmp(lineBuf + 1, "[TASK]", 6) == 0;
            if (isTaskEnd) {
                isTaskPara = false;
            } else {
                continue;
            }
        }

        if (strncmp(lineBuf, "[ALL]", 5) == 0) {
            isGlobalPara = true;
            isTaskPara = false;
        } else if (strncmp(lineBuf, "[TASK]", 6) == 0) {
            isGlobalPara = false;
            isTaskPara = true;

            if (pTask != 0) {
                validateTask(pTask);
                taskArray.push_back(*pTask);
                taskIdSet.insert(pTask->taskID);
            }
            pTask = new TransferTask();
        } else {
            if (!isTaskPara && !isGlobalPara) {
                continue;
            }

            char keyword[128] = { 0 };
            char value[1024] = { 0 };
            if (sscanf(lineBuf, "%[^=]=%s", keyword, value) != 2) {
                continue;   // �����Ƿ�������
            }

            util::delSpace(keyword);
            util::delSpace(value);

            if (isTaskPara) {
                pTask->setValue(keyword, value);
            } else {
                global->setValue(keyword, value);
            }
        }
    }

    if (pTask != 0) {
        validateTask(pTask);
        taskArray.push_back(*pTask);
        taskIdSet.insert(pTask->taskID);
    }
    delete pTask;
    pTask = 0;
    fclose(fp);

    validateGlobalSetting();
    setPassword();

    if (taskIdSet.empty()) {
        throw std::runtime_error("Task count is 0. Check .ini file");
    }
    return taskIdSet.size();
}

std::string Config::GetLogFile() {
    GlobalSetting *global = GlobalSetting::getInstance();
    std::string logFile;
    if (global->logPath.empty()) {
        logFile.append("./log/");
    } else {
        logFile.append(global->logPath);
        logFile.append("/");
    }
    logFile.append(cfgBaseName + ".log");

    // ����Ƿ��д
    FILE* fp = fopen(logFile.c_str(), "a");
    if (fp == 0) {
        throw std::runtime_error(logFile + ": Can't open error log file: " + strerror(errno));
    }
    fclose(fp);
    return logFile;
}

void Config::validateTask(TransferTask* pTask) {
    bool isBigEndian = util::IsBigEndian();
    if (pTask->taskID.empty()) {
        throw std::runtime_error("��ҵ���������д���taskidδ���õ���ҵ�����������taskid!");
    }
    if (taskIdSet.find(pTask->taskID) != taskIdSet.end()) {
        throw std::runtime_error(pTask->taskID + ": taskid�ظ�!");
    }
    for (size_t i = 0; i < pTask->dstNode.size(); i++) {
        if (isBigEndian
                && ((pTask->srcNode).iProtocol == SFTP
                        || pTask->dstNode[i].iProtocol == SFTP)) {
            throw std::runtime_error("����֧�ִ���ֽ���ķ�����");
        }
    }
}

void Config::setPassword() {
    GlobalSetting *global = GlobalSetting::getInstance();
    DesIniFile desinifile(global->passwdFile);
    if (!desinifile.ReadFile()) {
        throw std::runtime_error(std::string("��ȡ���������ļ�(\"") + global->passwdFile + std::string("\") ����"));
    }
    std::string session;
    std::string user;
    std::string password;
    std::string head("ftp:"); //����ԭftp�������ø�ʽ.�����޸�.
    for (size_t i = 0; i < taskArray.size(); i++) {
        session = head + taskArray[i].srcNode.sHost;
        user = taskArray[i].srcNode.sUser;
        password = desinifile.GetValue(session, user);
        taskArray[i].srcNode.sPasswd = password;
        for (size_t j = 0; j < taskArray[i].dstNode.size(); j++) {
            std::string host(taskArray[i].dstNode[j].sHost);
            session = head + host;
            user = taskArray[i].dstNode[j].sUser;
            password = desinifile.GetValue(session, user);
            taskArray[i].dstNode[j].sPasswd = password;
        }
    }
}

void Config::validateGlobalSetting() {
    struct stat buf;
    GlobalSetting *global = GlobalSetting::getInstance();

    // ��鱾�ع���Ŀ¼����
    if (global->workPath.empty()) {
        throw std::runtime_error("����Ŀ¼������wkpath��δ���ã�");
    }
    if (lstat(global->workPath.c_str(), &buf) < 0) {
        throw std::runtime_error(global->workPath + ":" + strerror(errno));
    }
    if (!S_ISDIR( buf.st_mode )) {
        throw std::runtime_error(global->workPath + ": ����Ŀ¼");
    }
    // ����½��������ļ�
    if (global->passwdFile.empty()) {
        throw std::runtime_error("�����ļ�������ftppwdfile��δ���ã�");
    }
    //��ȡ�����ftp��¼����
    if (lstat(global->passwdFile.c_str(), &buf) < 0) {
        throw std::runtime_error(global->passwdFile + ":" + strerror(errno));
    }
    if (!S_ISREG( buf.st_mode )) {
        if ( S_ISDIR(buf.st_mode)) {
            throw std::runtime_error(global->passwdFile + ": �����ļ�");
        }
        throw std::runtime_error(global->passwdFile + ": ������ͨ�ļ�");
    }
}

void Config::checkConfigFile() {
    const char* cfgfile = filepath.c_str();
    struct stat buf;
    if (lstat(cfgfile, &buf) < 0) {
        throw std::runtime_error(filepath + ":" + strerror(errno));
    }
    if (!S_ISREG(buf.st_mode)) {
        if (S_ISDIR(buf.st_mode)) {
            throw std::runtime_error(filepath + ":�����ļ�");
        }
        throw std::runtime_error(filepath + ":������ͨ�ļ�");
    }
}
