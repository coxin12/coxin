///////////////////////////////////////////////////////////
//  TGeter.cpp
//  Implementation of the Class TGeter
//  Created on:      05-十二月-2012 11:19:11
//  Original author: chenzhixin
//  VERSION     MODTIME       AUTHOR      MEMO
//  V12.00.001  2012-12-05  chenzhixin  new create
///////////////////////////////////////////////////////////

#if _ARCH_IBM
#include <sys/statfs.h>
#elif _ARCH_SUN
#include <sys/statvfs.h>
#define statfs statvfs
#else
#include <sys/vfs.h>
#endif

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "dstruct.h"
#include "TLog.h"
#include "TGeter.h"
#include "PSftp.h"
#include "PFtp.h"
#include "GlobalSetting.h"

namespace impl {
    const size_t BLOCK_SIZE = 512;
    const unsigned long REQSPACE = 200;
    const int MAX_CHK = 3;

    bool getDiskSpace(const char *path, double *total, double *available) {
        size_t bsize;
#if defined(sun) || defined (__sun__)
        struct statvfs buf;
#else
        struct statfs buf;
#endif


#if defined(sun) || defined(__sun__)
        if( statvfs( path, &buf ) < 0 ) {
            return false;
        }
#else
        if (statfs((char *) path, &buf) < 0) {
            return false;
        }
#endif

#if defined(sun)||defined(__sun__)
        bsize = buf.f_frsize;
#else
        bsize = buf.f_bsize;
#endif

        if (buf.f_bavail == -1) {
            return false;
        }

        if (bsize == -1) {
            bsize = BLOCK_SIZE;
        }

        *total = 1.0 * buf.f_blocks * bsize;
        *available = 1.0 * buf.f_bavail * bsize;

        return true;
    }
}

TGeter::TGeter() {
    self = "TGeter";
    srcConn = NULL;
}

TGeter::~TGeter() {
}

int TGeter::ChkBakPathSpace() {
    double total;
    double avail;
    int iChkNum = 0;
    if (mTask.bak1Path.empty())
        return 0;
    do {
        if (!impl::getDiskSpace(mTask.bak1Path.c_str(), &total, &avail)) {
            total = 0;
            avail = 0;
            sleep(5);
            continue;
        }

        if ((unsigned long) (avail / MByte) <= impl::REQSPACE) {
            DP(ERROR,"(%s) not enough space in %s, less than %dMBytes\n", mTask.taskID.c_str(), mTask.bak1Path.c_str(), impl::REQSPACE);
            ++iChkNum;
            sleep(180);
        }
    }while ((unsigned long)(avail/MByte) <= impl::REQSPACE && iChkNum < impl::MAX_CHK );

    if (iChkNum > impl::MAX_CHK) {
        DP(ERROR, "(%s) disk(%s) not enough space timeout 180 * %d second.\n", mTask.taskID.c_str(), mTask.bak1Path.c_str(), impl::MAX_CHK);
        return -1;
    }
    DP(DEBUG, "(%s) check disk space, directory: %s, free: %ld\n", mTask.taskID.c_str(),
    mTask.bak1Path.c_str(), avail/MByte);
    return 0;
}

int TGeter::Init(const TransferTask &task) {
    Transmitter::Init(task);
    if (ChkBakPathSpace()) {
        return -1;
    }
    return 0;
}

int TGeter::Connect() {
    DP( DEBUG, "(%s) 正在连接远程主机[%s]...\n", mTask.taskID.c_str(), (mTask.srcNode).sHost.c_str() );
    try
    {
        srcConn->Connect((mTask.srcNode).sHost, (mTask.srcNode).iLinkMode);
    }
    catch(std::exception &e)
    {
        DP(ERROR, "(%s) 远程主机[%s]连接失败\n", mTask.taskID.c_str(), (mTask.srcNode).sHost.c_str());
        DP(ERROR, "(%s) %s[Connect] Connect: %s\n", mTask.taskID.c_str(), self.c_str(), e.what());
        return -1;
    }
    DP( DEBUG, "(%s) 远程主机[%s]已连接\n", mTask.taskID.c_str(), (mTask.srcNode).sHost.c_str() );
    DP( DEBUG, "(%s) 正在登录...\n", mTask.taskID.c_str() );
    try
    {
        srcConn->Login();
    }
    catch(std::exception &e)
    {
        DP(ERROR, "(%s) 登录[%s]失败\n", mTask.taskID.c_str(), (mTask.srcNode).sHost.c_str());
        DP(ERROR, "(%s) %s[Connect] Login: %s\n", mTask.taskID.c_str(), self.c_str(), e.what());
        return -1;
    }
    DP( DEBUG, "(%s) 登录成功\n", mTask.taskID.c_str() );
    DP( DEBUG, "(%s) 切换源目录[%s]...\n", mTask.taskID.c_str(), (mTask.srcNode).sPath.c_str() );
    try
    {
        srcConn->RemoteChDir((mTask.srcNode).sPath);
    }
    catch(std::exception &e)
    {
        DP(ERROR, "(%s) 目录[%s]切换失败\n", mTask.taskID.c_str(), (mTask.srcNode).sPath.c_str());
        DP(ERROR, "(%s) %s[Connect] RemoteChDir: %s\n", mTask.taskID.c_str(), self.c_str(), e.what());
        return -1;
    }
    if ( (mTask.srcNode).iProtocol == SFTP )
    {
        srcConn->SetOptions(SFTP_MAX_READ_SIZE, mTask.transBlocks * MAX_BLOCK_SIZE);
    }

    DP( DEBUG, "(%s) 切换源目录[%s]成功\n", mTask.taskID.c_str(), (mTask.srcNode).sPath.c_str() );

    return 0;
}

int TGeter::ReadFileList(const char* ListName, const char* pattern,
        int MaxFiles) {
    FILE* fp;
    char buff[255];
    char idle[255];
    char value1[100], value2[100], value3[100], value4[100], value5[100],
            value6[100];

    int64_t fsize;
    DP(DEBUG, "(%s) ReadFileList文件[%s]正在打开...\n", mTask.taskID.c_str(), ListName);
    if ((fp = fopen(ListName, "r")) == NULL) {
        DP(ERROR, "(%s) 文件[%s]打开失败\n", mTask.taskID.c_str(), ListName);
        DP(ERROR, "(%s) %s[ReadFileList] fopen: %s\n", mTask.taskID.c_str(), self.c_str(), strerror(errno));
        return -1;
    }
    DP(DEBUG, "(%s) ReadFileList文件[%s]打开成功\n", mTask.taskID.c_str(), ListName);

    int n;
    int i = 0;
    DP(DEBUG, "(%s) 正在读取ReadFileList文件[%s]....\n", mTask.taskID.c_str(), ListName);
    while (fgets(buff, sizeof(buff), fp) != NULL) {
        if (i >= MaxFiles)
            break;

        n = sscanf(buff, "%s %s %s %s %s %s %s %s %s %s %s", idle, idle, value1,
                value2, value3, idle, idle, value6, value4, idle, value5);

        switch (n) {
        case 4:

            if (strncasecmp(value1, "<DIR>", 5) == 0)
                continue;

            if (pFilter->match(value2) == false) {
                continue;
            }

            mFileNames.push_back(value2);
            mFileSizes.push_back(strtoll(value1, NULL, 10));

            break;

        case 8:

            if (*buff == 'd')
                continue;

            if (pFilter->match(value6) == false) {
                continue;
            }

            mFileNames.push_back(value6);
            mFileSizes.push_back(strtoll(value3, NULL, 10));

            break;

        case 9:

            if (*buff == 'd')
                continue;

            if (*buff != 'l') {
                if (pFilter->match(value4) == false) {
                    continue;
                }

                mFileNames.push_back(value4);
                mFileSizes.push_back(strtoll(value3, NULL, 10));

                break;
            }

        case 11:

            if (*buff != 'l')
                continue;

            if (pFilter->match(value4) == false) {
                continue;
            }

            try {
                char path[1024];
                sprintf(path, "%s/%s", (mTask.srcNode).sPath.c_str(), value4);
                srcConn->RemoteFileSize(path, &fsize, mTask.transMode);
                if (fsize < 0)
                    continue;
            } catch (std::exception& e) {
                DP(ERROR, "(%s) 远程主机[%s]获取link文件(%s)大小失败\n", mTask.taskID.c_str(), (mTask.srcNode).sHost.c_str(), value4);
                DP(ERROR, "(%s) %s[ListFile](%s) RemoteFileSize: %s\n", mTask.taskID.c_str(), self.c_str(), value4, e.what());
                continue;
            }

            mFileNames.push_back(value4);
            mFileSizes.push_back(fsize);

            break;

            default:

            continue;

            break;
        }

        ++i;
    }

    fclose(fp);
    DP(DEBUG, "(%s) 读取ReadFileList文件以完成,本次需get文件个数为:%d\n", mTask.taskID.c_str(),i);
    return i;
}

int TGeter::ListFile() {

    DP( DEBUG, "(%s) 正在获取当前路径...\n", mTask.taskID.c_str() );
    try
    {
        srcConn->RemotePwd( m_srcpath );
    }
    catch(std::exception &e)
    {
        DP(ERROR, "(%s) 获取当前路径失败\n", mTask.taskID.c_str());
        DP(ERROR, "(%s) %s[ListFile] Pwd: %s\n", mTask.taskID.c_str(), self.c_str(), e.what());
        return -1;
    }
    DP( DEBUG, "(%s) 当前路径为[%s]\n", mTask.taskID.c_str(), m_srcpath.c_str() );

    char FileList[512]= {0};
    char value[5][100];
    memset(value, 0, 5*100);

    char tempid[128]= {0};
    int n = sscanf(mTask.taskID.c_str(), "%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/", value[0], value[1], value[2],value[3], value[4]);
    if( n == EOF)
    {
        sprintf(tempid, "%d", (int)(pthread_self()));
    }
    else if( n == 0 )
    {
        sprintf(tempid, "%d", (int)(pthread_self()));
    }
    else if( n > 0 )
    {
        strcpy(tempid, value[0]);
    }
    GlobalSetting *global = GlobalSetting::getInstance();
    snprintf( FileList, sizeof(FileList), "%s/%s_%u.lst", global->workPath.c_str(), tempid, (int)(pthread_self()) );

    DP( DEBUG, "(%s) 正在获取文件列表(%s)...\n", mTask.taskID.c_str(), FileList );
    try
    {
        srcConn->RemoteLs(FileList, m_srcpath);
    }
    catch(std::exception &e)
    {
        DP(ERROR, "(%s) 获取文件列表失败\n", mTask.taskID.c_str());
        DP(ERROR, "(%s) %s[ListFile] RemoteLs(%s): %s\n", mTask.taskID.c_str(), self.c_str(), m_srcpath.c_str(), e.what());
        return -1;
    }
    DP( DEBUG, "(%s) 文件列表获取成功\n", mTask.taskID.c_str() );

    mFileCount = ReadFileList(FileList, mTask.pattern.c_str(), mTask.maxFiles);

    DP( DEBUG, "(%s) 正在删除ReadFileList文件[%s]...\n", mTask.taskID.c_str(), FileList );
    if( unlink(FileList) )
    {
        DP(ERROR, "(%s) 删除文件[%s]失败\n", mTask.taskID.c_str(), FileList );
        DP(ERROR, "(%s) %s[ReadFileList] unlink: %s\n", mTask.taskID.c_str(), self.c_str(), strerror(errno));
    }
    DP( DEBUG, "(%s) 删除ReadFileList文件[%s]成功\n", mTask.taskID.c_str(), FileList );
    return mFileCount;
}

int TGeter::FileLink(const std::string &srcFile, const std::string &fileName) {
    std::string bakName;
    if (!mTask.bak1Path.empty()) {
        bakName = std::string(mTask.bak1Path) + "/" + fileName;
        unlink(bakName.c_str());
        if (link(srcFile.c_str(), bakName.c_str()) < 0) {
            return -1;
        }
        DP( DEBUG, "(%s) link文件[%s]到[%s]成功\n", mTask.taskID.c_str(), fileName.c_str(), mTask.bak1Path.c_str());
    }
    if (!mTask.bak2Path.empty()) {
        bakName = std::string(mTask.bak2Path) + "/" + fileName;
        unlink(bakName.c_str());
        if (link(srcFile.c_str(), bakName.c_str()) < 0) {
            return -1;
        }
        DP( DEBUG, "(%s) link文件[%s]到[%s]成功\n", mTask.taskID.c_str(), fileName.c_str(), mTask.bak2Path.c_str());
    }
    if (!mTask.bak3Path.empty()) {
        bakName = std::string(mTask.bak3Path) + "/" + fileName;
        unlink(bakName.c_str());
        if (link(srcFile.c_str(), bakName.c_str()) < 0) {
            return -1;
        }
        DP( DEBUG, "(%s) link文件[%s]到[%s]成功\n", mTask.taskID.c_str(), fileName.c_str(), mTask.bak3Path.c_str());
    }
    if (!mTask.bak4Path.empty()) {
        bakName = std::string(mTask.bak4Path) + "/" + fileName;
        unlink(bakName.c_str());
        if (link(srcFile.c_str(), bakName.c_str()) < 0) {
            return -1;
        }
        DP( DEBUG, "(%s) link文件[%s]到[%s]成功\n", mTask.taskID.c_str(), fileName.c_str(), mTask.bak4Path.c_str());
    }

    return 0;
}

int TGeter::Transfer() {
    std::string source;
    std::string destination;
    std::string tempfile;
    char lastMsg[1024] = { 0 };

    time_t b_time;
    time_t t_time;
    time_t e_time;

    DP( DEBUG, "(%s) 开始Get 文件个数:%d....\n",mTask.taskID.c_str(), mFileCount );
    for (int xFile = 0; xFile < mFileCount; ++xFile) {
        source = std::string((mTask.srcNode).sPath) + "/"
                + mFileNames[xFile];
        if (IsDelFailFile(mFileNames[xFile])) {
            DP( DEBUG, "(%s) 正在尝试删除文件[%s] ...\n", mTask.taskID.c_str(), mFileNames[xFile].c_str() );
            try
            {
                srcConn->RemoteDelete(source.c_str());
            }
            catch(std::exception& e)
            {
                DP(ERROR, "(%s) 文件[%s]删除失败\n", mTask.taskID.c_str(), source.c_str());
                DP(ERROR, "(%s) %s[Transfer] FtpDelete: %s\n", mTask.taskID.c_str(), self.c_str(), e.what());
                continue;
            }
            DP( DEBUG, "(%s) 文件[%s]删除成功\n", mTask.taskID.c_str(), source.c_str() );
            DP( DEBUG, "(%s) 清除查重临时标志文件\n", mTask.taskID.c_str() );
            UnlinkDelFailFile(mFileNames[xFile]);
            continue;
        }

        if (mFileSizes[xFile] == 0)
            DP(WARNING, "(%s) 空文件[%s](%d)\n", mTask.taskID.c_str(), mFileNames[xFile].c_str(), mFileSizes[xFile]);

        tempfile = std::string(mTask.tmpPath) + "/" + mFileNames[xFile];

        DP( DEBUG, "(%s) 正在传输文件从[%s]到[%s] ...\n", mTask.taskID.c_str(), source.c_str(),tempfile.c_str());
        WriteTaskLog(mFileNames[xFile].c_str(), mFileSizes[xFile],
                "transfer ...");
        time(&b_time);
        try {
            srcConn->GetFile(source, tempfile, mTask.transMode);
        } catch (std::exception& e) {
            DP(ERROR, "(%s) 文件[%s]到[%s]传输失败\n", mTask.taskID.c_str(), source.c_str(), tempfile.c_str());
            DP(ERROR, "(%s) %s[Transfer] GetFile(%s): %s\n", mTask.taskID.c_str(), self.c_str(), source.c_str(), e.what());
            continue;
        }
        DP( DEBUG, "(%s) 文件[%s]传输成功\n", mTask.taskID.c_str(), mFileNames[xFile].c_str() );

        char filemsg[1024] = { 0 };
        struct stat filestat;
        if (stat(tempfile.c_str(), &filestat) != 0) {
            DP(ERROR, "(%s) File(%s) stat error: %s\n", mTask.taskID.c_str(), tempfile.c_str(), strerror(errno));
            unlink(tempfile.c_str());
            break;
        }

        if (mFileSizes[xFile] != filestat.st_size) {
            DP(ERROR, "(%s) 文件[%s]大小不匹配\n", mTask.taskID.c_str(), mFileNames[xFile].c_str());
            DP(ERROR, "(%s) Source[%lld], Destination[%lld]\n", mTask.taskID.c_str(), mFileSizes[xFile], filestat.st_size);
            //unlink(tempfile);
            continue;
        }
        time(&t_time);

        impl::RTrim(filemsg, '\n');
        WriteTaskLog(mFileNames[xFile].c_str(), mFileSizes[xFile],
                "transfer ok", filemsg);

        if (FileLink(tempfile, mFileNames[xFile]) != 0) {
            DP(ERROR, "(%s) 文件[%s]备份失败\n", mTask.taskID.c_str(), mFileNames[xFile].c_str());
            DP(ERROR, "(%s) %s[Transfer] [%s] FileLink: %s!\n", mTask.taskID.c_str(),
                    self.c_str(), tempfile.c_str(), strerror(errno));
            unlink( tempfile.c_str() );
            continue;
        }

        if (strncasecmp(mTask.isRename.c_str(), "no", 2) != 0) {
            destination = std::string(mTask.dstNode[0].sPath) + "/"
                    + mFileNames[xFile];
            DP( DEBUG, "(%s) 正在将文件[%s]移到目标目录[%s]...\n", mTask.taskID.c_str(), tempfile.c_str(), destination.c_str());
            if (rename(tempfile.c_str(), destination.c_str())) {
                DP(ERROR, "(%s) 文件[%s]移动失败\n", mTask.taskID.c_str(), mFileNames[xFile].c_str());
                DP(ERROR, "(%s) %s[Transfer] Rename form[%s] to [%s]: %s\n", mTask.taskID.c_str(),
                        self.c_str(), tempfile.c_str(), destination.c_str(), strerror(errno));
                continue;
            }
            DP( DEBUG, "(%s) 文件[%s]到[%s]移动成功\n", mTask.taskID.c_str(), tempfile.c_str(), destination.c_str());
            WriteTaskLog(mFileNames[xFile], mFileSizes[xFile], "get ok");
        }
        time(&e_time);
        DP( DEBUG, "(%s) 记录文件[%s]传输信息(%d,%d,%d)\n", mTask.taskID.c_str(), mFileNames[xFile].c_str(),
        b_time, t_time, e_time );
        int itime = t_time - b_time;
        itime = itime > 0 ? itime : 1;

        memset(lastMsg, 0, sizeof(lastMsg));
        sprintf(lastMsg,
                "Succeed file from (%s) %s:%s to (%s) %s:%s %ld OK ( %.2f Kb/s)",
                getTransProtocol((mTask.srcNode).iProtocol).c_str(),
                (mTask.srcNode).sHost.c_str(), source.c_str(),
                getTransProtocol(mTask.dstNode[0].iProtocol).c_str(),
                mTask.dstNode[0].sHost.c_str(), destination.c_str(),
                e_time - b_time, mFileSizes[xFile] / KByte / itime);
        WriteTaskLog(mFileNames[xFile], mFileSizes[xFile], lastMsg);

        DP( DEBUG, "(%s) 正在删除源文件[%s]...\n", mTask.taskID.c_str(), mFileNames[xFile].c_str() );
        try {
            srcConn->RemoteDelete(source);
        } catch (std::exception& e) {
            DP(ERROR, "(%s) 源文件[%s]删除失败\n", mTask.taskID.c_str(), mFileNames[xFile].c_str());
            DP(ERROR, "(%s) %s[Transfer] RemoteDelete(%s): %s\n", source.c_str(), mTask.taskID.c_str(), self.c_str(), e.what());
            CreateDelFailFile(mFileNames[xFile]);
            continue;
        }
        DP( DEBUG, "(%s) 源文件[%s]删除成功\n", mTask.taskID.c_str(), mFileNames[xFile].c_str() );
    }
    DP( DEBUG, "(%s) Get 文件个数:%d 已完成\n",mTask.taskID.c_str(), mFileCount );
    return 0;
}

void TGeter::initSource() {
    if ((mTask.srcNode).iProtocol == FTP) {
        srcConn = new PFtp((mTask.srcNode).sUser,
                (mTask.srcNode).sPasswd);
    } else {
        GlobalSetting *global = GlobalSetting::getInstance();
        srcConn = new PSftp((mTask.srcNode).sUser,
                (mTask.srcNode).sPasswd, global->rsaPubFile, global->rsaFile);
    }
}

void TGeter::disconnectSource() {
    if (srcConn) {
        srcConn->DisConnect();
        delete srcConn;
        srcConn = 0;
    }
}
