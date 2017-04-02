/*
 * ResendFile.h
 *
 *  Created on: 2013��11��12��
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
     * ȡ����Ҫ�ش���Դ�ļ�����������������ƥ���target��ţ���Ŵ�0��ʼ��
     *
     * @param   line      ������������ȡ�����ش��ļ������
     * @return  ��������ƥ���target��ţ���Ŵ�0��ʼ������û��ȡ���ش��ļ��򷵻�-1
     */
    int fetch(Line &line);

    void setCurrentSuccess();
    int count(const std::string &sourceFile);
    /*
     * ���һ���ش���¼
     * @param   targetNum       ���������е�target��ţ���Ŵ�0��ʼ��
     * @param   sourceFile      �ش���Դ�ļ�·��
     * @param   sourceFileSize  �ش��ļ���С
     */
    void add(int targetNum, const std::string &sourceFile, int64_t sourceFileSize);
    void close();
private:
    TransferTask task;
    std::vector<Line> fileLines;
    std::string filePath;
    std::vector<Line>::iterator pCurrentLine;
    std::vector<Line>::iterator pLastLine;
    time_t lastExecuteTime;     // �������ʱ��

    static const size_t FIELD_COUNT;
    static const char* SEPARATOR;
    static const char* EQUAL;
};

#endif /* RESENDFILE_H_ */
