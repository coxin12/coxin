/*
 * TMultiBridger.h
 *
 *  Created on: 2013��11��8��
 *      Author: liangjianqiang
 */

#ifndef TMULTIBRIDGER_H_
#define TMULTIBRIDGER_H_

#include "Transmitter.h"
#include "ResendFile.h"

class TMultiBridger: public Transmitter {
public:
    TMultiBridger();
    virtual ~TMultiBridger();
    virtual int Connect();
    virtual int ListFile();
    virtual int Transfer();
protected:
    virtual void initSource();
    virtual void initTarget(int targetNum);
    virtual void disconnectSource();
    virtual void disconnectTarget(int targetNum);
private:
    bool createResendDir();
    void removeSourceFile(const std::string &filename, const std::string &source);
    void doResend();
    void removeResendFile(std::string resendFile);
    void cleanDelFailResendFile();
    void moveToResend(const std::string &filename, const std::string &source);
    int ReadFileList(const char* ListName, const char* pattern, int MaxFiles);
    std::vector<int> sendToTemp(std::vector<int> targets,
            const std::string &filename, int64_t sourceSize,
            const std::string &sourceFile, bool isResend);
    std::vector<int> checkFileSize(std::vector<int> targets,
            const std::string& filename, int64_t sourceSize, bool isResend);
    std::vector<int> moveToTargetDir(std::vector<int> targets,
            const std::string &filename, int64_t sourceSize, bool isResend);
    void printSuccessLog(std::vector<int> targets, const std::string &filename,
            int64_t sourceSize, const std::string &sourceFile);
    void addFailOver(int targetNum, const std::string& filename,
                int64_t fileSize);

    std::set<int> failTargets;
    time_t begin_time;  // ���俪ʼʱ��
    time_t trans_time;  // �������ʱ��
    time_t end_time;    // �ƶ��ļ�����ʱ��
};

#endif /* TMULTIBRIDGER_H_ */
