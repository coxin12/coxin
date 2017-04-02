///////////////////////////////////////////////////////////
//  TMover.cpp
//  Implementation of the Class TMover
//  Created on:      05-十二月-2012 11:19:15
//  Original author: chenzhixin
//  VERSION     MODTIME       AUTHOR      MEMO
//  V12.00.001  2012-12-05  chenzhixin  new create
//  V13.00.001  2013-03-28  chenzhixin  增加egrep类型的正则表达式，进行文件名匹配。
//  V13.00.003  2013-10-30  chenzhixin  修改Fnmatch方法，将原来支持多种匹配方式，修改为用户指定的匹配方式，
//                                       以修复一个表达式两个匹配方式表达的意义不同的而引起抄送错误的问题。
///////////////////////////////////////////////////////////
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sstream>

#include "Config.h"
#include "TLog.h"
#include "TMover.h"
#include "GlobalSetting.h"

TMover::TMover() {
    self = "TMover";
}

TMover::~TMover() {
}

int TMover::ListFile() {
    FileSet llist;
    FileAttribute fileAttr;

    mFileNames.clear();
    mFileSizes.clear();

    DIR *dirp;
    if ((dirp = opendir(mTask.srcNode.sPath.c_str())) == NULL) {
        DP(ERROR, "(%s) 目录[%s]打开失败\n", mTask.taskID.c_str(),
                (mTask.srcNode).sPath.c_str());
        DP(ERROR, "(%s) %s[LocalList] opendir: %s\n", mTask.taskID.c_str(),
                self.c_str(), strerror(errno));
        return -1;
    }

    char pathname[255];
    struct dirent *dp;
    struct stat filestat;
    while ((dp = readdir(dirp)) != NULL) {
        if (dp->d_name[0] == '.')
            continue;

        if (pFilter->match(dp->d_name) == false) {
            continue;
        }

        snprintf(pathname, sizeof(pathname), "%s/%s",
                (mTask.srcNode).sPath.c_str(), dp->d_name);
        if (stat(pathname, &filestat) < 0) {
            DP(ERROR, "(%s) 文件[%s]信息获取失败\n", pathname);
            DP(ERROR, "(%s) %s[LocalList] stat: %s\n", mTask.taskID.c_str(),
                    self.c_str(), strerror(errno));
            closedir(dirp);
            return -1;
        }

        fileAttr.filename = dp->d_name;
        fileAttr.filesize = filestat.st_size;
        fileAttr.mtime = filestat.st_mtime;

        llist.insert(fileAttr);
    }

    closedir(dirp);

    int n = 0;
    FileSet::iterator it;
    for (it = llist.begin(); it != llist.end() && n < mTask.maxFiles; ++it) {
        mFileNames.push_back(it->filename);
        mFileSizes.push_back(it->filesize);
        ++n;
    }

    mFileCount = n;

    return mFileCount;
}

int TMover::Transfer() {
    std::string source;
    std::string temp;
    std::string destination;

    DP(DEBUG, "(%s) 开始Move 文件个数:%d....\n", mTask.taskID.c_str(), mFileCount);
    for (int xFile = 0; xFile < mFileCount; ++xFile) {
        source = std::string((mTask.srcNode).sPath) + "/"
                + mFileNames[xFile];

        if (IsDelFailFile(mFileNames[xFile])) {
            if (unlink(source.c_str()) == 0)
                UnlinkDelFailFile(mFileNames[xFile]);
            continue;
        }

        if (mFileSizes[xFile] == 0)
            DP(WARNING, "(%s) 空文件[%s](%d)\n", mTask.taskID.c_str(),
            mFileNames[xFile].c_str(), mFileSizes[xFile]);

        temp = std::string(mTask.tmpPath) + "/" + mFileNames[xFile];
        destination = std::string(mTask.dstNode[0].sPath) + "/"
                + mFileNames[xFile];

        DP(DEBUG, "(%s) Link 文件[%s]到[%s]...\n", mTask.taskID.c_str(), source.c_str(), destination.c_str());
        if (link(source.c_str(), destination.c_str()) == 0) {
            WriteTaskLog(mFileNames[xFile], mFileSizes[xFile], "link ok");

            if (unlink(source.c_str()) < 0) {
                CreateDelFailFile(mFileNames[xFile]);
                DP(ERROR, "(%s) 文件[%s]删除失败\n", mTask.taskID.c_str(),
                mFileNames[xFile].c_str());
                DP(ERROR, "%s[Transfer] file[%s] unlink: %s\n",
                        self.c_str(), source.c_str(), strerror(errno));
                continue;
            }

            WriteTaskLog(mFileNames[xFile], mFileSizes[xFile], "move ok");
            continue;
        }

        if (EXDEV != errno) {
            DP(ERROR, "(%s) 文件[%s]移动失败\n", mTask.taskID.c_str(),
                    mFileNames[xFile].c_str());
            DP(ERROR, "%s[Transfer] file[%s] link to [%s]: %s\n",
                    self.c_str(), source.c_str(), destination.c_str(), strerror(errno));
            continue;
        }
        WriteTaskLog(mFileNames[xFile].c_str(), mFileSizes[xFile],
                "copy ...");

        DP(DEBUG, "(%s) Copy 文件[%s]到[%s]...\n", mTask.taskID.c_str(), source.c_str(), temp.c_str());
        if (Copy(source, temp) < 0) {
            DP(ERROR, "(%s) 文件[%s]移动失败\n", mTask.taskID.c_str(),
            mFileNames[xFile].c_str());
            continue;
        }
        WriteTaskLog(mFileNames[xFile].c_str(), mFileSizes[xFile],
                "copy ok");

        DP(DEBUG, "(%s) 删除 文件[%s]...\n", mTask.taskID.c_str(), source.c_str());
        if (unlink(source.c_str()) < 0) {
            CreateDelFailFile(mFileNames[xFile].c_str());
            DP(ERROR, "(%s) 文件[%s]删除失败\n", mTask.taskID.c_str(),
                    mFileNames[xFile].c_str());
            DP(ERROR, "%s[Transfer] file[%s] unlink: %s\n", self.c_str(),
                    source.c_str(), strerror(errno));
            continue;
        }

        DP(DEBUG, "(%s) Link 文件[%s]到[%s]...\n", mTask.taskID.c_str(), temp.c_str(), destination.c_str());
        if (link(temp.c_str(), destination.c_str()) < 0 && EEXIST != errno) {
            DP(ERROR, "(%s) 文件[%s]连接失败\n", mTask.taskID.c_str(),
                    mFileNames[xFile].c_str());
            DP(ERROR, "%s[Transfer] file[%s] link to [%s]: %s\n",
                    self.c_str(), temp.c_str(), destination.c_str(), strerror(errno));
            continue;
        }
        DP(DEBUG, "(%s) 删除临时文件[%s]...\n", mTask.taskID.c_str(), temp.c_str());
        unlink(temp.c_str());
        WriteTaskLog(mFileNames[xFile].c_str(), mFileSizes[xFile],
                "move ok");
    }
    DP(DEBUG, "(%s) Move 文件个数:%d 已完成\n",mTask.taskID.c_str(), mFileCount);
    return 0;
}

int TMover::Copy(const std::string &source, const std::string &destination) {
    int srcfd, destfd;
    char buf[8192];

    if ((srcfd = open(source.c_str(), O_RDONLY)) < 0) {
        DP(ERROR, "(%s) 文件[%s]打开失败\n", mTask.taskID.c_str(), source.c_str());
        DP(ERROR, "(%s) %s[Copy] open(%s): %s\n", mTask.taskID.c_str(),
                self.c_str(), source.c_str(), strerror(errno));
        return -1;
    }

    if ((destfd = open(destination.c_str(), O_WRONLY | O_CREAT | O_EXCL,
    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) < 0) {
        DP(ERROR, "(%s) 文件[%s]打开失败\n", mTask.taskID.c_str(), destination.c_str());
        DP(ERROR, "(%s) %s[Copy] open(%s): %s\n", mTask.taskID.c_str(),
                self.c_str(), destination.c_str(), strerror(errno));
        close(srcfd);
        return -1;
    }

    int n, l;
    int retval = 0;
    while ((n = read(srcfd, buf, sizeof(buf))) > 0) {
        if ((l = write(destfd, buf, n)) != n) {
            DP(ERROR, "short write! read[%d], wrote[%d]\n", n, l);
            retval = -1;
            break;
        }
    }

    close(srcfd);
    close(destfd);

    return retval;
}
