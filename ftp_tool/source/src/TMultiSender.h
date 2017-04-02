/*
 * TMultiSender.h
 *
 *  Created on: 2013年11月8日
 *      Author: liangjianqiang
 */

#ifndef TMULTISENDER_H_
#define TMULTISENDER_H_

#include "Transmitter.h"
#include "ResendFile.h"

class TMultiSender: public Transmitter {
public:
    TMultiSender();
    virtual ~TMultiSender();
    virtual int Connect();
    virtual int ListFile();
    virtual int Transfer();
protected:
    virtual void initTarget(int targetNum);
    virtual void disconnectTarget(int targetNum);
private:
    std::vector<int> sendToTemp(std::vector<int> targets,
            const std::string &filename, int64_t sourceSize,
            const std::string &sourceFile, bool isResend);
    std::vector<int> checkFileSize(std::vector<int> targets,
            const std::string& filename, int64_t sourceSize, bool isResend);
    std::vector<int> moveToTargetDir(std::vector<int> targets,
            const std::string &filename, int64_t sourceSize, bool isResend);
    void printSuccessLog(std::vector<int> targets,
            const std::string &filename, int64_t sourceSize,
            const std::string &sourceFile);
    void removeSourceFile(const std::string &filename, const std::string &source);
    bool createResendDir();
    void doResend();
    void removeResendFile(std::string resendFile);
    void cleanDelFailResendFile();
    void moveToResend(const std::string &filename, const std::string &source);
    void addFailOver(int targetNum, const std::string& filename,
            int64_t fileSize);

    std::set<int> failTargets;

    time_t begin_time;  // 传输开始时刻
    time_t trans_time;  // 传输结束时刻
    time_t end_time;    // 移动文件结束时刻
};

#endif /* TMULTISENDER_H_ */
