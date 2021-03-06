/*
 * TMultiSender.cpp
 *
 *  Created on: 2013年11月8日
 *      Author: liangjianqiang
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <libgen.h>
#include <fnmatch.h>
#include <fcntl.h>
#include <algorithm>
#include "TMultiSender.h"
#include "TLog.h"
#include "PFtp.h"
#include "PSftp.h"
#include "GlobalSetting.h"
#include "TransferTask.h"
#include "MatchFilter.h"

TMultiSender::TMultiSender() {
    self = "TMultiSender";
    resendFile = 0;
    begin_time = 0;
    trans_time = 0;
    end_time = 0;
}

TMultiSender::~TMultiSender() {
    if (resendFile) {
        delete resendFile;
        resendFile = 0;
    }
}

int TMultiSender::Connect() {
    int successCount = 0;
    for (size_t i = 0; i < mTask.dstNode.size(); i++) {
        const char *taskID = mTask.taskID.c_str();
        const char *hostIP = mTask.dstNode[i].sHost.c_str();
        std::string tmpDir(mTask.dstNode[i].sPath + "/tmp");

        try {
            DP(DEBUG, "(%s) 正在连接远程主机[%s]...\n", taskID, hostIP);
            destConns[i]->Connect(hostIP, mTask.dstNode[i].iLinkMode);
            DP(DEBUG, "(%s) 远程主机[%s]已连接\n", taskID, hostIP);
        } catch(std::exception &e) {
            DP(ERROR, "(%s) 远程主机[%s]连接失败\n", taskID, hostIP);
            DP(ERROR, "(%s) %s[Connect] Connect: %s\n", taskID, self.c_str(), e.what());
            failTargets.insert(i);
            continue;
        }

        try {
            DP(DEBUG, "(%s) 正在登录...\n", taskID);
            destConns[i]->Login();
            DP(DEBUG, "(%s) 登录成功\n", taskID);
        } catch(std::exception &e) {
            DP(ERROR, "(%s) 登录[%s]失败\n", taskID, hostIP);
            DP(ERROR, "(%s) %s[Connect] Login: %s\n", taskID, self.c_str(), e.what());
            failTargets.insert(i);
            continue;
        }

        try {
            DP(DEBUG, "(%s) 切换目标服务器临时目录[%s]...\n", taskID, tmpDir.c_str());
            try {
                destConns[i]->RemoteChDir(tmpDir);
            } catch(std::exception &e) {
                DP(WARNING, "(%s) 目标服务器[%s]临时目录[%s]访问失败", taskID, hostIP, tmpDir.c_str());
                DP(WARNING, "(%s) %s[Connect] RemoteChDir: %s\n", taskID, self.c_str(), e.what());
                try {
                    DP(DEBUG, "(%s) 目标服务器[%s]正在创建临时目录[%s]...", taskID, hostIP, tmpDir.c_str());
                    destConns[i]->RemoteMkDir(tmpDir, mTask.dstNode[i].iLinkMode);
                    DP(DEBUG, "(%s) 目标服务器[%s]创建临时目录[%s]成功", taskID, hostIP, tmpDir.c_str());
                } catch (std::exception &e) {
                    DP(ERROR, "(%s) 目标服务器[%s]创建临时目录[%s]失败", taskID, hostIP, tmpDir.c_str());
                    DP(ERROR, "(%s) %s[Connect] RemoteMkDir: %s\n", taskID, self.c_str(), e.what());
                    failTargets.insert(i);
                    continue;
                }
                destConns[i]->RemoteChDir(tmpDir);
            }
            DP(DEBUG, "(%s) 切换目标服务器临时目录[%s]成功\n", taskID, tmpDir.c_str());
        } catch(std::exception &e) {
            DP(ERROR, "(%s) 目录[%s]切换失败\n", taskID, tmpDir.c_str());
            DP(ERROR, "(%s) %s[Connect] RemoteChDir: %s\n", taskID, self.c_str(), e.what());
            failTargets.insert(i);
            continue;
        }

        if (mTask.dstNode[i].iProtocol == SFTP) {
            destConns[i]->SetOptions(SFTP_MAX_READ_SIZE,
                    mTask.transBlocks * MAX_BLOCK_SIZE);
        }
        successCount++;
    }
    if (successCount == 0) {
        DP(ERROR, "(%s) 所有连接均不可用\n", mTask.taskID.c_str());
        return -1;
    }
    return 0;
}

int TMultiSender::ListFile() {
    FileSet fileSet;
    FileAttribute fileAttr;

    mFileNames.clear();
    mFileSizes.clear();

    DIR *dirp;
    if ((dirp = opendir(mTask.srcNode.sPath.c_str())) == 0) {
        DP(ERROR, "(%s) 目录[%s]打开失败\n", mTask.taskID.c_str(),
        (mTask.srcNode).sPath.c_str());
        DP(ERROR, "(%s) %s[LocalList] opendir: %s\n", mTask.taskID.c_str(), self.c_str(),
        strerror(errno));
        return -1;
    }

    char pathname[255];
    struct dirent *dp;
    struct stat filestat;
    while ((dp = readdir(dirp)) != 0) {
        if (dp->d_name[0] == '.')
            continue;

        if (pFilter->match(dp->d_name) == false) {
            continue;
        }

        snprintf(pathname, sizeof(pathname), "%s/%s",
                (mTask.srcNode).sPath.c_str(), dp->d_name);
        if (stat(pathname, &filestat) < 0) {
            DP(ERROR, "(%s) 文件[%s]信息获取失败\n", pathname);
            DP(ERROR, "(%s) %s[LocalList] stat: %s\n", mTask.taskID.c_str(), self.c_str(),
            strerror(errno));
            closedir(dirp);
            return -1;
        }

        fileAttr.filename = dp->d_name;
        fileAttr.filesize = filestat.st_size;
        fileAttr.mtime = filestat.st_mtime;

        fileSet.insert(fileAttr);
    }

    closedir(dirp);

    int n = 0;
    FileSet::iterator it;
    for (it = fileSet.begin(); it != fileSet.end() && n < mTask.maxFiles;
            ++it) {
        mFileNames.push_back(it->filename);
        mFileSizes.push_back(it->filesize);
        ++n;
    }

    mFileCount = n;

    return mFileCount;
}

int TMultiSender::Transfer() {
    begin_time = 0;
    trans_time = 0;
    end_time = 0;

    if (createResendDir() == false) {
        DP(ERROR, "(%s) 创建重传目录[%s/%s]失败，传输未能开始\n",mTask.taskID.c_str(),
        mTask.srcNode.sPath.c_str(), GlobalSetting::RESEND_DIR_NAME);
        return -1;
    }

    resendFile = new ResendFile(mTask);
    if (resendFile->isExist()) {
        try {
            resendFile->loadFile();
            resendFile->validate();
            DP(INFO, "(%s) 当前时间%ld, 上次重传时间%ld, 时间间隔%d\n",mTask.taskID.c_str(), time(0), resendFile->getLastExecuteTime(), mTask.resendInterval);
            if (time(0) - resendFile->getLastExecuteTime()
                    >= mTask.resendInterval) {
                DP(DEBUG, "(%s) 进入重传流程...\n",mTask.taskID.c_str());
                doResend();
                DP(DEBUG, "(%s) 退出重传流程...\n",mTask.taskID.c_str());
            }
        } catch (std::exception &e) {
            DP(ERROR, "(%s) [%s]重传失败\n",mTask.taskID.c_str(), e.what());
            DP(ERROR, "(%s) 退出重传流程...\n",mTask.taskID.c_str());
        }
    } else {
        resendFile->settLastExecuteTime(time(0));    // 若有新的重传文件添加进来，这是它的最后执行时间
    }

    for (int xFile = 0; xFile < mFileCount; ++xFile) {
        bool isResendFile = false;
        std::string filename = mFileNames[xFile];
        int64_t filesize = mFileSizes[xFile];
        std::string source = std::string(mTask.srcNode.sPath) + "/" + filename;

        if (IsDelFailFile(filename)) {
            if (unlink(source.c_str()) == 0) {
                UnlinkDelFailFile(filename);
            }
            continue;
        }

        if (filesize == 0) {
            DP(WARNING, "(%s) 空文件[%s](%d)\n", mTask.taskID.c_str(), filename.c_str(), filesize);
        }

        std::vector<int> targets;
        for(size_t i = 0; i < mTask.dstNode.size(); i++) {
            if (failTargets.find(i) != failTargets.end()) {
                std::string resendPath(mTask.srcNode.sPath + "/resend/" + filename);
                resendFile->add(i, resendPath, filesize);
                isResendFile = true;
                continue;
            }
            targets.push_back(i);
        }

        time(&begin_time);
        targets = sendToTemp(targets, filename, filesize, source, false);  // 传到临时目录
        targets = checkFileSize(targets, filename, filesize, false);  // 检查文件大小
        time(&trans_time);
        if (strncasecmp(mTask.isRename.c_str(), "no", 2) != 0) {
            targets = moveToTargetDir(targets, filename, filesize, false);  // 移动至目标目录
        }
        time(&end_time);

        if (targets.empty()) {
            DP(DEBUG, "(%s) 所有目标主机传输失败\n", mTask.taskID.c_str());
        } else {
            printSuccessLog(targets, filename, filesize, source);  // 打印成功信息
        }

        // 删除源文件
        try {
            if (isResendFile || targets.size() < mTask.dstNode.size()) {
                moveToResend(filename, source);
            } else {
                removeSourceFile(filename, source);
            }
        } catch (...) {
            continue;
        }
    }
    DP(DEBUG, "(%s) MultiSend 文件个数:%d 已完成\n",mTask.taskID.c_str(), mFileCount);
    resendFile->close();
    return 0;
}

bool TMultiSender::createResendDir() {
    std::string resendDir(mTask.srcNode.sPath + "/" + GlobalSetting::RESEND_DIR_NAME);
    struct stat dirStat;
    if (lstat(resendDir.c_str(), &dirStat) == 0) {
        if ((dirStat.st_mode & S_IFMT) == S_IFDIR) {
            return true;
        }
    } else {
        if (mkdir(resendDir.c_str(), 0775) == 0) {
            return true;
        } else {
            DP(ERROR, "(%s) mkdir: %s\n",mTask.taskID.c_str(), strerror(errno));
            return false;
        }
    }
    return false;
}

void TMultiSender::addFailOver(int targetNum, const std::string& filename,
        int64_t fileSize) {
    failTargets.insert(targetNum);
    std::string resendPath(mTask.srcNode.sPath + "/resend/" + filename);
    resendFile->add(targetNum, resendPath, fileSize);
}

std::vector<int> TMultiSender::sendToTemp(std::vector<int> targets,
        const std::string &filename, int64_t fileSize,
        const std::string &sourceFilePath, bool isResend) {
    long BUF_SIZE = mTask.transBlocks * MAX_BLOCK_SIZE;
    const char *sTaskID = mTask.taskID.c_str();
    const char *sSrcFile = sourceFilePath.c_str();
    const char *sFilename = filename.c_str();

    WriteTaskLog(sFilename, fileSize, "transfer ...");

    int srcFD = ::open(sSrcFile, O_RDONLY);
    if (srcFD <= 0) {
        DP(ERROR, "(%s) 打开源文件[%s]失败:[%s]", sTaskID, sFilename, strerror(errno));
        throw std::runtime_error(filename + ":" + strerror(errno));
    }
    for (size_t i = 0; i < targets.size(); i++) {
        std::string tempFile(mTask.dstNode[targets[i]].sPath + "/tmp/" + filename);
        const char *sHostIP = mTask.dstNode[targets[i]].sHost.c_str();
        const char *sTempFile = tempFile.c_str();
        DP(DEBUG, "(%s) 正在传输文件从[%s]到[%s:%s] ...\n", sTaskID, sSrcFile, sHostIP, sTempFile);

        destConns[targets[i]]->RemoteOpenFile(tempFile, FILE_WRITE, mTask.transMode);
    }

    char *buffer = new char[BUF_SIZE];
    memset(buffer, 0, BUF_SIZE);
    int n = 0;
    while ((n = ::read(srcFD, buffer, BUF_SIZE)) != 0) {
        for (size_t i = 0; i < targets.size(); i++) {
            if (failTargets.find(targets[i]) != failTargets.end()) {
                continue;
            }
            try {
                int writeCount = 0;
                if ((writeCount = destConns[targets[i]]->RemoteWrite(buffer, n)) < n) {
                    char err[1024] = { 0 };
                    snprintf(err, sizeof(err), "(%s) <%s> lines:%d short write: passed [%d], wrote [%d]\n",
                            mTask.taskID.c_str(), __FILE__, __LINE__, n, writeCount);
                    throw std::runtime_error(err);
                }
            } catch (std::exception &e) {
                DP(ERROR, "(%s) 文件[%s]传输至[%s]失败\n", sTaskID, filename.c_str(), mTask.dstNode[targets[i]].sHost.c_str());
                DP(ERROR, "(%s) %s[Transfer] RemoteWrite(%s): %s\n",
                        sTaskID, self.c_str(), sSrcFile, e.what());
                if (!isResend) {
                    addFailOver(targets[i], filename, fileSize);
                } else {
                    failTargets.insert(targets[i]);
                }
            }
        }
        if (targets.size() == 1 && failTargets.find(targets[0]) != failTargets.end()) {
            break;
        }
    }
    delete []buffer;
    for (size_t i = 0; i < targets.size(); i++) {
        destConns[targets[i]]->RemoteCloseFile();
    }
    ::close(srcFD);

    std::vector<int> successTargets;
    for (std::vector<int>::iterator i = targets.begin(); i != targets.end(); ++i) {
        if (failTargets.find(*i) == failTargets.end()) {
            successTargets.push_back(*i);
        }
    }
    if (!successTargets.empty()) {
        DP(DEBUG, "(%s) 文件[%s]传输成功[%d台主机]\n", sTaskID, sFilename, successTargets.size());
        WriteTaskLog(sFilename, fileSize, "transfer ok");
    }
    return successTargets;
}

std::vector<int> TMultiSender::checkFileSize(std::vector<int> targets,
        const std::string& filename, int64_t sourceSize, bool isResend) {
    int64_t fsize = 0;
    const char *taskid = mTask.taskID.c_str();
    const char *file = filename.c_str();
    for (size_t i = 0; i < targets.size(); i++) {
        std::string tempFile(mTask.dstNode[targets[i]].sPath + "/tmp/" + filename);
        const char *tmpFile = tempFile.c_str();
        const char *hostIP = mTask.dstNode[targets[i]].sHost.c_str();
        try {
            DP(DEBUG, "(%s) 获取远程文件[%s:%s]大小......\n", taskid, hostIP, tmpFile);
            destConns[targets[i]]->RemoteFileSize(tempFile, &fsize, mTask.transMode);
            DP(DEBUG, "(%s) 获取远程文件[%s:%s]大小[%ld]成功\n", taskid, hostIP, tmpFile, fsize);
        } catch (std::exception &e) {
            DP(ERROR, "(%s) 获取远程文件[%s:%s]大小失败\n", taskid, hostIP, tmpFile);
            DP(ERROR, "(%s) %s[Transfer] RemoteFileSize: %s\n", taskid, self.c_str(), e.what());
            if (!isResend) {
                addFailOver(targets[i], filename, sourceSize);
            } else {
                failTargets.insert(targets[i]);
            }
        }
        if (fsize != sourceSize) {
            DP(ERROR, "(%s) 文件[%s]大小不匹配\n", taskid, file);
            DP(ERROR, "(%s) Source[%d], Destination[%d]\n", taskid, sourceSize, fsize);
            if (!isResend) {
                addFailOver(targets[i], filename, sourceSize);
            } else {
                failTargets.insert(targets[i]);
            }
        }
    }

    std::vector<int> successTargets;
    for (std::vector<int>::iterator i = targets.begin(); i != targets.end(); ++i) {
        if (failTargets.find(*i) == failTargets.end()) {
            successTargets.push_back(*i);
        }
    }
    return successTargets;
}

std::vector<int> TMultiSender::moveToTargetDir(std::vector<int> targets,
        const std::string &filename, int64_t sourceSize, bool isResend) {
    const char *taskid = mTask.taskID.c_str();
    const char *file = filename.c_str();
    for (size_t i = 0; i < targets.size(); i++) {
        std::string tempFile(mTask.dstNode[targets[i]].sPath + "/tmp/" + filename);
        const char *tmpFile = tempFile.c_str();
        std::string destination(mTask.dstNode[targets[i]].sPath + "/" + filename);
        const char *destFile = destination.c_str();
        try {
            DP(DEBUG, "(%s) 正在将文件[%s]移到目标目录...\n", taskid, file);
            destConns[targets[i]]->RemoteRename(tempFile, destination);
            DP(DEBUG, "(%s) 文件[%s]移动成功\n", taskid, file);
        } catch (std::exception &e) {
            DP(ERROR, "(%s) 文件[%s]移动失败\n", taskid, file);
            DP(ERROR, "(%s) %s[Transfer] RemoteRename from (%s) to (%s): %s\n",
                    taskid, self.c_str(), tmpFile, destFile, e.what());
            if (!isResend) {
                addFailOver(targets[i], filename, sourceSize);
            } else {
                failTargets.insert(targets[i]);
            }
        }
    }

    std::vector<int> successTargets;
    for (std::vector<int>::iterator i = targets.begin(); i != targets.end(); ++i) {
        if (failTargets.find(*i) == failTargets.end()) {
            successTargets.push_back(*i);
        }
    }
    if (!successTargets.empty()) {
        WriteTaskLog(filename, sourceSize, "send ok");
    }
    return successTargets;
}

void TMultiSender::printSuccessLog(std::vector<int> targets,
        const std::string &filename, int64_t sourceSize,
        const std::string &sourceFile) {
    const char *taskid = mTask.taskID.c_str();
    const char *file = filename.c_str();
    std::string transProto = getTransProtocol(mTask.srcNode.iProtocol);

    int trans_dur = trans_time - begin_time;
    trans_dur = trans_dur > 0 ? trans_dur : 1;
    long total_duration = end_time - begin_time;
    double trans_rate = sourceSize / KByte / trans_dur;

    DP(DEBUG, "(%s) 记录文件[%s]传输信息(%d,%d,%d)\n", taskid, file, begin_time,
            trans_time, end_time);
    for (size_t i = 0; i < targets.size(); i++) {
        std::string destination(mTask.dstNode[targets[i]].sPath + "/" + filename);
        std::string targetProto = getTransProtocol(mTask.dstNode[targets[i]].iProtocol);
        char lastMsg[1024] = {0};
        sprintf(lastMsg,
            "Succeed file from [%s] %s:%s to [%s] %s:%s %ld OK ( %.2f Kb/s)",
            transProto.c_str(), mTask.srcNode.sHost.c_str(), sourceFile.c_str(),
            targetProto.c_str(), mTask.dstNode[targets[i]].sHost.c_str(), destination.c_str(),
            total_duration, trans_rate);
        WriteTaskLog(filename, sourceSize, lastMsg);
    }
}

void TMultiSender::removeSourceFile(const std::string &filename, const std::string &source) {
    DP(DEBUG, "(%s) 正在删除源文件[%s]...\n", mTask.taskID.c_str(), filename.c_str());
    if (unlink(source.c_str()) < 0) {
        CreateDelFailFile(filename.c_str());
        DP(ERROR, "(%s) 源文件[%s]删除失败\n", mTask.taskID.c_str(), filename.c_str());
        DP(ERROR, "(%s) %s[Transfer] unlink(%s): %s\n", mTask.taskID.c_str(), self.c_str(), source.c_str(), strerror(errno));
        throw std::runtime_error(source + ":" + strerror(errno));
    }
    DP(DEBUG, "(%s) 源文件[%s]删除成功\n", mTask.taskID.c_str(), filename.c_str());
}

void TMultiSender::doResend() {
    resendFile->settLastExecuteTime(time(0));
    std::set<std::string> successFileSet;
    int targetNum = -1;
    begin_time = 0;
    trans_time = 0;
    end_time = 0;

    ResendFile::Line line;
    while ((targetNum = resendFile->fetch(line)) >= 0) {
        if (failTargets.find(targetNum) != failTargets.end()) {
            DP(ERROR, "(%s) 主机[%s]的连接不可用, 文件[%s]重传失败\n", mTask.taskID.c_str(),
            mTask.dstNode[targetNum].sHost.c_str(), line.sourceFile.c_str());
            continue;
        }
        std::string filename(basename(const_cast<char *>(line.sourceFile.c_str())));

        int target[1] = { targetNum };
        std::vector<int> targets(target, target + 1);

        time(&begin_time);
        targets = sendToTemp(targets, filename, line.sourceFileSize, line.sourceFile, true);  // 传到临时目录
        targets = checkFileSize(targets, filename, line.sourceFileSize, true);  // 检查文件大小
        time(&trans_time);
        if (strncasecmp(mTask.isRename.c_str(), "no", 2) != 0) {
            targets = moveToTargetDir(targets, filename, line.sourceFileSize, true);  // 移动至目标目录
        }
        time(&end_time);

        if (targets.empty()) {
            DP(DEBUG, "(%s) 所有目标主机传输失败\n", mTask.taskID.c_str());
            continue;
        } else {
            printSuccessLog(targets, filename, line.sourceFileSize, line.sourceFile);  // 打印成功信息
            resendFile->setCurrentSuccess();
            successFileSet.insert(line.sourceFile);
        }
    }
    cleanDelFailResendFile();
    // 删除重传成功的文件
    for (std::set<std::string>::iterator i = successFileSet.begin();
            i != successFileSet.end(); ++i) {
        if (resendFile->count(*i) == 0) {
            try {
                removeResendFile(*i);
            } catch (...) {
                continue;
            }
        }
    }
}

void TMultiSender::cleanDelFailResendFile() {
    std::string resendDir(mTask.srcNode.sPath + "/" + GlobalSetting::RESEND_DIR_NAME);
    DIR *dirp;
    if ((dirp = opendir(resendDir.c_str())) == 0) {
        DP(ERROR, "(%s) 目录[%s]打开失败\n", mTask.taskID.c_str(),
                resendDir.c_str());
        DP(ERROR, "(%s) %s[LocalList] opendir: %s\n", mTask.taskID.c_str(), self.c_str(),
        strerror(errno));
        return ;
    }

    char pathname[255];
    struct dirent *dp;
    std::string prefix("/DELFAIL_");
    prefix.append(mTask.taskID);
    prefix.append("_");
    MatchFilter mFilter;
    mFilter.setPattern(prefix + "*");
    while ((dp = readdir(dirp)) != 0) {
        if (dp->d_name[0] == '.') {
            continue;
        }

        if (mFilter.match(dp->d_name) == false) {
            continue;
        }

        snprintf(pathname, sizeof(pathname), "%s/%s",
                resendDir.c_str(), dp->d_name);
        char resendFile[512];
        if (sscanf(pathname, (prefix + "%[^\"]").c_str(), resendFile) == 1) {
            DP(DEBUG, "(%s) 清理重传文件[%s]\n", mTask.taskID.c_str(), resendFile);
            if (unlink((resendDir + "/" + resendFile).c_str()) != 0) {
                DP(DEBUG, "(%s) 清理重传文件[%s]失败\n", mTask.taskID.c_str(), resendFile);
            } else {
                DP(DEBUG, "(%s) 清理重传文件[%s]成功\n", mTask.taskID.c_str(), resendFile);
            }
        }
    }

    closedir(dirp);
}

void TMultiSender::removeResendFile(std::string resendFile) {
    std::string filename(basename(const_cast<char *>(resendFile.c_str())));
    DP(DEBUG, "(%s) 正在删除重传文件[%s]...\n", mTask.taskID.c_str(), filename.c_str());
    if (unlink(resendFile.c_str()) < 0) {
        int fd;

        std::string delFailFile;
        delFailFile = mTask.srcNode.sPath + "/" + GlobalSetting::RESEND_DIR_NAME + "/DELFAIL_"
                + mTask.taskID + "_" + resendFile;

        if ((fd = open(delFailFile.c_str(), O_WRONLY | O_CREAT, 0755)) < 0) {
            DP(ERROR, "(%s) 文件[%s]打开失败\n", mTask.taskID.c_str(), delFailFile.c_str());
            DP(ERROR, "(%s) FileTrans[DelFail] open(%s): %s\n", mTask.taskID.c_str(),
            delFailFile.c_str(), strerror(errno));
        }

        close(fd);
        DP(ERROR, "(%s) 重传文件[%s]删除失败\n", mTask.taskID.c_str(), filename.c_str());
        DP(ERROR, "(%s) %s[Transfer] unlink(%s): %s\n", mTask.taskID.c_str(), self.c_str(), resendFile.c_str(), strerror(errno));
    }
}

void TMultiSender::moveToResend(const std::string &filename, const std::string &source) {
    std::string resendDir(mTask.srcNode.sPath + "/" + GlobalSetting::RESEND_DIR_NAME);
    std::string resendFile(resendDir + "/" + filename);
    DP(DEBUG, "(%s) 文件[%s]移至重传目录[%s]\n", mTask.taskID.c_str(), filename.c_str(), resendDir.c_str());
    if (rename(source.c_str(), resendFile.c_str()) != 0) {
        DP(ERROR, "(%s) 文件[%s]移至重传目录[%s]失败\n", mTask.taskID.c_str(), filename.c_str(), resendDir.c_str());
    }
    DP(DEBUG, "(%s) 文件[%s]移至重传目录[%s]成功\n", mTask.taskID.c_str(), filename.c_str(), resendDir.c_str());
}

void TMultiSender::initTarget(int targetNum) {
    if (mTask.dstNode[targetNum].iProtocol == FTP) {
        destConns.push_back(
                new PFtp(mTask.dstNode[targetNum].sUser, mTask.dstNode[targetNum].sPasswd));
    } else {
        GlobalSetting *global = GlobalSetting::getInstance();
        destConns.push_back(
                new PSftp(mTask.dstNode[targetNum].sUser, mTask.dstNode[targetNum].sPasswd,
                        global->rsaPubFile, global->rsaFile));
    }
}

void TMultiSender::disconnectTarget(int targetNum) {
    if (destConns[targetNum]) {
        destConns[targetNum]->DisConnect();
        delete destConns[targetNum];
        destConns[targetNum] = 0;
    }
}
