/*
 * Transmitter.cpp
 *
 *  Created on: 2013年11月8日
 *      Author: liangjianqiang
 */

#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sstream>
#include "Transmitter.h"
#include "TLog.h"
#include "GlobalSetting.h"
#include "Factory.h"

Transmitter::Transmitter() {
    pLogger = 0;
    srcConn = 0;
    mFileCount = 0;
    pFilter = 0;
    resendFile = 0;
}

Transmitter::~Transmitter() {
    if (pLogger != 0) {
        delete pLogger;
        pLogger = 0;
    }
    if (pFilter != 0) {
        delete pFilter;
        pFilter = 0;
    }
    mFileNames.clear();
    mFileSizes.clear();
    destConns.clear();
}

int Transmitter::Init(const TransferTask &task) {
    mTask = task;

    pLogger = new writelog_t(mTask.taskID, mTask.taskLogFile,
            mTask.transLogMaxSize, "INFO", mTask.transLogMaxIndex);
    pFilter = FilterKit::createFilter(mTask.regexType);
    pFilter->setPattern(mTask.pattern);

    initSource();
    if (mTask.dstNode.size() <= 0) {
        throw std::runtime_error("There is no destination node in task. Initialize failed");
    }
    destConns.reserve(mTask.dstNode.size());
    for (size_t i = 0; i < mTask.dstNode.size(); i++) {
        initTarget(i);
    }

    return 0;
}

int Transmitter::Disconnect() {
    disconnectSource();
    if (mTask.dstNode.size() <= 0) {
        return -1;
    }
    for (size_t i = 0; i < mTask.dstNode.size(); i++) {
        disconnectTarget(i);
    }
    return 0;
}

//采用公共日志系统实现该函数.
int Transmitter::WriteTaskLog(const std::string &name, int64_t size,
        const std::string &operate, const std::string &listMsg/*=""*/) {
    std::stringstream ostr;
    ostr << "[" << self << "] [" << mTask.taskID << "] " << name << "\tsize:" << size << "\t" << operate;
    if (!listMsg.empty()) {
        ostr << std::endl;
        ostr << listMsg;
    }

    pLogger->info("%s", ostr.str().c_str());

    return 0;
}

int Transmitter::CreateDelFailFile(const std::string &name) {
    int fd;

    std::string counter;
    GlobalSetting *global = GlobalSetting::getInstance();
    counter = global->workPath + "/DELFAIL_" + mTask.taskID + "_" + name;

    if ((fd = open(counter.c_str(), O_WRONLY | O_CREAT, 0755)) < 0) {
        DP(ERROR, "(%s) 文件[%s]打开失败\n", mTask.taskID.c_str(), counter.c_str());
        DP(ERROR, "(%s) FileTrans[DelFail] open(%s): %s\n", mTask.taskID.c_str(),
        counter.c_str(), strerror(errno));
        return -1;
    }

    close(fd);

    return 0;
}

bool Transmitter::IsDelFailFile(const std::string &name) {
    std::string counter;
    GlobalSetting *global = GlobalSetting::getInstance();
    counter = global->workPath + "/DELFAIL_" + mTask.taskID + "_" + name;

    if (access(counter.c_str(), F_OK) == 0) {
        return true;
    } else {
        return false;
    }
}

int Transmitter::UnlinkDelFailFile(const std::string &name) {
    std::string counter;
    GlobalSetting *global = GlobalSetting::getInstance();
    counter = global->workPath + "/DELFAIL_" + mTask.taskID + "_" + name;

    unlink(counter.c_str());

    return 0;
}

std::string Transmitter::getTransProtocol(int protocol) {
    switch (protocol) {
    case FTP:
        return "FTP";
        break;
    case SFTP:
        return "SFTP";
        break;
    }
    return "UNKNOW_PROTOCOL";
}
