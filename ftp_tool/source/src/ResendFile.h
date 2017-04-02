/*
 * ResendFile.h
 *
 *  Created on: 2013年11月12日
 *      Author: liangjianqiang
 */

#ifndef RESENDFILE_H_
#define RESENDFILE_H_

#include <time.h>
#include <string>
#include <list>
#include "TransferTask.h"

class ResendFile {
public:
    class Line {
    public:
        Line() {
            sourceFileSize = -1;
            success = false;
        }

        std::string targetIP;
        std::string targetDir;
        std::string sourceFile;
        int64_t sourceFileSize;
        bool success;

        bool operator <(const Line &rhs) const;
        bool operator ==(const Line &rhs) const;
    };
    typedef std::vector<Line> Lines;
    typedef std::vector<Line>::iterator LinesPtr;
public:
    ResendFile(const TransferTask &task);
    virtual ~ResendFile();

    std::string getFilePath() const {
        return filePath;
    }

    void settLastExecuteTime(time_t t) {
        lastExecuteTime = t;
    }

    time_t getLastExecuteTime() const {
        return lastExecuteTime;
    }

    size_t getSize() {
        return fileLines.size();
    }

    bool isExist();
    void loadFile();
    void validate();

    /*
     * 取出需要重传的源文件，并返回与配置里匹配的target序号（序号从0开始）
     *
     * @param   line      输出参数。输出取到的重传文件里的行
     * @return  与配置里匹配的target序号（序号从0开始）。若没有取到重传文件则返回-1
     */
    int fetch(Line &line);

    void setCurrentSuccess();
    int count(const std::string &sourceFile);
    /*
     * 添加一条重传记录
     * @param   targetNum       任务配置中的target序号（序号从0开始）
     * @param   sourceFile      重传的源文件路径
     * @param   sourceFileSize  重传文件大小
     */
    void add(int targetNum, const std::string &sourceFile, int64_t sourceFileSize);
    void close();
private:
    TransferTask task;
    std::vector<Line> fileLines;
    std::string filePath;
    std::vector<Line>::iterator pCurrentLine;
    std::vector<Line>::iterator pLastLine;
    time_t lastExecuteTime;     // 最后运行时间

    static const size_t FIELD_COUNT;
    static const char* SEPARATOR;
    static const char* EQUAL;
};

#endif /* RESENDFILE_H_ */
