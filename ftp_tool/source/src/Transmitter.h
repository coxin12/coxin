/*
 * Transmitter.h
 *
 *  Created on: 2013年11月8日
 *      Author: liangjianqiang
 */

#ifndef ITRANSFER_H_
#define ITRANSFER_H_

#include <string.h>
#include <set>
#include "TransferTask.h"
#include <logger/writelog.h>
#include "ProtocolBase.h"
#include "Filter.h"
#include "dstruct.h"
#include "ResendFile.h"

/*
 * 抽象传送类
 */
class Transmitter {
public:
    Transmitter();
    virtual ~Transmitter();
    virtual int Init(const TransferTask &task);
    virtual int Connect() = 0;
    virtual int Disconnect();
    virtual int ListFile() = 0;
    virtual int Transfer() = 0;
protected:
    virtual void initSource() {
    }
    virtual void initTarget(int targetNum) {
    }
    virtual void disconnectSource() {
    }
    virtual void disconnectTarget(int targetNum) {
    }

    int mFileCount;
    std::string self;
    TransferTask mTask;
    Filter *pFilter;
    writelog_t *pLogger;
    ResendFile *resendFile;
    ProtocolBase *srcConn;
    std::vector<ProtocolBase *> destConns;
    std::vector<std::string> mFileNames; //文件列表
    std::vector<int64_t> mFileSizes;     //文件大小列表与文件列表对应

    std::string getTransProtocol(int protocol);
    bool IsDelFailFile(const std::string &name);
    int UnlinkDelFailFile(const std::string &name);
    int CreateDelFailFile(const std::string &name);
    //增加其它信息的传入参数，用于增加其它更详细的信息写入传输日志中
    int WriteTaskLog(const std::string &name, int64_t size,
            const std::string &operate, const std::string &otherMsg = "");
};

#endif /* ITRANSFER_H_ */
