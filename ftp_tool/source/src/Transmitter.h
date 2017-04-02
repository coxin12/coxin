/*
 * Transmitter.h
 *
 *  Created on: 2013��11��8��
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
 * ��������
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
    std::vector<std::string> mFileNames; //�ļ��б�
    std::vector<int64_t> mFileSizes;     //�ļ���С�б����ļ��б��Ӧ

    std::string getTransProtocol(int protocol);
    bool IsDelFailFile(const std::string &name);
    int UnlinkDelFailFile(const std::string &name);
    int CreateDelFailFile(const std::string &name);
    //����������Ϣ�Ĵ������������������������ϸ����Ϣд�봫����־��
    int WriteTaskLog(const std::string &name, int64_t size,
            const std::string &operate, const std::string &otherMsg = "");
};

#endif /* ITRANSFER_H_ */
