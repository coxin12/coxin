/*
 * ResendFile.cpp
 *
 *  Created on: 2013年11月12日
 *      Author: liangjianqiang
 */

#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <TLog.h>
#include "ResendFile.h"
#include "GlobalSetting.h"

namespace util {
    std::vector<std::string> split(const char *sep, std::string str) {
        std::vector<std::string> v;
        size_t sep_len = strlen(sep);
        size_t p1 = 0;
        size_t p2 = 0;
        do {
            p2 = str.find(sep, p1);
            v.push_back(str.substr(p1, p2 - p1));
            p1 = p2 + sep_len;
        } while (p2 != std::string::npos);

        return v;
    }

    bool isLineSuccess(ResendFile::Line line) {
        return line.success;
    }

    bool isLineNotSuccess(ResendFile::Line line) {
        return line.success == false;
    }
}

bool ResendFile::Line::operator <(const Line &rhs) const {
    if (targetIP == rhs.targetIP && targetDir == rhs.targetDir) {
        return sourceFile < rhs.sourceFile;
    }
    if (targetIP == rhs.targetIP) {
        return targetDir < rhs.targetDir;
    }
    return targetIP < rhs.targetIP;
}

bool ResendFile::Line::operator ==(const Line &rhs) const {
    if (targetIP == rhs.targetIP && targetDir == rhs.targetDir && sourceFile == rhs.sourceFile) {
    	return true;
    } else {
    	return false;
    }
}

const size_t ResendFile::FIELD_COUNT = 4;
const char* ResendFile::SEPARATOR = "&&";
const char* ResendFile::EQUAL = "=";
ResendFile::ResendFile(const TransferTask &task) :
        task(task) {
    GlobalSetting *global = GlobalSetting::getInstance();
    filePath = (global->workPath + "/" + task.taskID + ".resend");
    pCurrentLine = fileLines.end();
    lastExecuteTime = -1;
}

ResendFile::~ResendFile() {
    fileLines.clear();
}

bool ResendFile::isExist() {
    struct stat sb;
    if (lstat(filePath.c_str(), &sb) == 0) {
        if (S_ISREG(sb.st_mode)) {
            return true;
        }
    }
    return false;
}

void ResendFile::loadFile() {
    fileLines.clear();
    std::ifstream file(filePath.c_str());
    if (!file) {
        throw std::runtime_error(filePath + ": Open file error");
    }

    std::string tmpLine;
    if (std::getline(file, tmpLine)) {
        std::vector<std::string> fields = util::split(EQUAL, tmpLine);
        if (fields.size() != 2) {
            throw std::runtime_error(tmpLine + ": Format error");
        }
        lastExecuteTime = atol(fields[1].c_str());
        if (lastExecuteTime <= 0) {
            throw std::runtime_error(filePath + ": lastExecuteTime is not set.");
        }
    }

    while (std::getline(file, tmpLine)) {
        if (tmpLine.empty()) {
            continue;
        }
        std::vector<std::string> fields = util::split(SEPARATOR, tmpLine);
        if (fields.size() != FIELD_COUNT) {
            throw std::runtime_error(tmpLine + ": Format error");
        }
        Line resendLine;
        resendLine.targetIP = fields[0];
        resendLine.targetDir = fields[1];
        resendLine.sourceFile = fields[2];
        resendLine.sourceFileSize = atol(fields[3].c_str());
        fileLines.push_back(resendLine);
    }
    file.close();
    pCurrentLine = fileLines.begin();
    pLastLine = fileLines.begin();
}

void ResendFile::validate() {
    for (LinesPtr line = fileLines.begin();
            line != fileLines.end(); ++line) {
        int ret = false;
        for (size_t i = 0; i < task.dstNode.size(); i++) {
            bool isSameIP = line->targetIP.compare(task.dstNode[i].sHost) == 0;
            bool isSamePath = line->targetDir.compare(task.dstNode[i].sPath) == 0;
            if (isSameIP && isSamePath) {
                ret = true;
                break;
            }
        }
        if (ret != true) {
            std::string err("");
            err += line->targetIP + "&&" + line->targetDir;
            err += " does not match any target in task[" + task.taskID + "]";
            throw std::runtime_error(err);
        }
    }
}

int ResendFile::fetch(Line &line) {
    if (pCurrentLine == fileLines.end()) {
        return -1;
    }
    size_t i = 0;
    for (; i < task.dstNode.size(); i++) {
        bool isSameIP = pCurrentLine->targetIP.compare(task.dstNode[i].sHost) == 0;
        bool isSamePath = pCurrentLine->targetDir.compare(task.dstNode[i].sPath) == 0;
        if (isSameIP && isSamePath) {
            line = *pCurrentLine;
            break;
        }
    }
    if (i == task.dstNode.size()) {
        std::string err("");
        err += pCurrentLine->targetIP + "&&" + pCurrentLine->targetDir;
        err += " does not match any target in task[" + task.taskID + "]";
        throw std::runtime_error(err);
    }
    if (pLastLine != pCurrentLine) {
        pLastLine = pCurrentLine;
    }
    ++pCurrentLine;
    return i;
}

void ResendFile::setCurrentSuccess() {
    pLastLine->success = true;
}

int ResendFile::count(const std::string &sourceFile) {
    int count = 0;
    for (LinesPtr i = fileLines.begin();
            i != fileLines.end(); ++i) {
        if (i->sourceFile == sourceFile && i->success == false) {
            count++;
        }
    }
    return count;
}

void ResendFile::add(int targetNum, const std::string &sourceFile, int64_t sourceFileSize) {
    pCurrentLine = fileLines.end();     // 避免在add后再fetch
    Line line;
    line.targetIP = task.dstNode[targetNum].sHost;
    line.targetDir = task.dstNode[targetNum].sPath;
    line.sourceFile = sourceFile;
    line.sourceFileSize = sourceFileSize;
    fileLines.push_back(line);
}

void ResendFile::close() {
    size_t successCount = std::count_if(fileLines.begin(), fileLines.end(), util::isLineSuccess);
    if (successCount == fileLines.size()) {
        unlink(filePath.c_str());
        return;
    }

    Lines tmp;
    tmp.resize(fileLines.size() - successCount);
    LinesPtr pEnd = std::remove_if(fileLines.begin(), fileLines.end(), util::isLineSuccess);
    std::copy(fileLines.begin(), pEnd, tmp.begin());
    sort(tmp.begin(), tmp.end());

    fileLines.clear();
    fileLines.resize(tmp.size());
    std::copy(tmp.begin(), tmp.end(), fileLines.begin());
    pCurrentLine = fileLines.end();
    tmp.clear();

    std::ofstream file(filePath.c_str(), std::ios_base::trunc);
    if (!file) {
        throw std::runtime_error(filePath + ": Open file error");
    }
    file << "LastExcuteTime" << EQUAL << lastExecuteTime << std::endl;
    Line *tmpLine = 0;
    for (LinesPtr line = fileLines.begin();
                line != fileLines.end(); ++line) {
        if (tmpLine == 0) {
            tmpLine = &(*line);
	} else if (*tmpLine == *line) {
	    continue;
	} else {
        tmpLine = &(*line);
	}
        file << line->targetIP << SEPARATOR;
        file << line->targetDir << SEPARATOR;
        file << line->sourceFile << SEPARATOR;
        file << line->sourceFileSize << std::endl;
    }
    file.close();
}
