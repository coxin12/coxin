///////////////////////////////////////////////////////////
//  TBridger.cpp
//  Implementation of the Class TBridger
//  Created on:      05-十二月-2012 11:19:11
//  Original author: chenzhixin
//  VERSION     MODTIME       AUTHOR      MEMO
//  V12.00.001  2012-12-05  chenzhixin  new create
//  V13.00.001	2013-03-29  chenzhixin	增加对ftp与sftp混合使用于同一作业任务时，
//                                      读取数据量不同步,影响传输速度的问题。
///////////////////////////////////////////////////////////

#include "TLog.h"
#include "PFtp.h"
#include "PSftp.h"
#include "TBridger.h"
#include <errno.h>
#include <stdlib.h>
#include "GlobalSetting.h"

TBridger::TBridger() {
    self = "TBridger";
    srcConn = NULL;
}

TBridger::~TBridger() {
}

int TBridger::ConnectSource() {
    DP( DEBUG, "(%s) 正在连接源远程主机[%s]...\n", mTask.taskID.c_str(), mTask.srcNode.sHost.c_str() );
    try
    {
        srcConn->Connect(mTask.srcNode.sHost, mTask.srcNode.iLinkMode);
    }
    catch(std::exception &e)
    {
        DP(ERROR, "(%s) 源远程主机[%s]连接失败\n", mTask.taskID.c_str(), mTask.srcNode.sHost.c_str());
        DP(ERROR, "(%s) %s[ConnectSource] Connect: %s\n", mTask.taskID.c_str(), self.c_str(), e.what());
        return -1;
    }
    DP( DEBUG, "(%s) 源远程主机[%s]已连接\n", mTask.taskID.c_str(), mTask.srcNode.sHost.c_str() );
    DP( DEBUG, "(%s) 正在登录源远程主机[%s]...\n", mTask.taskID.c_str(), mTask.srcNode.sHost.c_str() );
    try
    {
        srcConn->Login();
    }
    catch(std::exception &e)
    {
        DP(ERROR, "(%s) 登录源远程主机[%s]失败\n", mTask.taskID.c_str(), mTask.srcNode.sHost.c_str());
        DP(ERROR, "(%s) %s[ConnectSource] Login: %s\n", mTask.taskID.c_str(), self.c_str(), e.what());
        return -1;
    }
    DP( DEBUG, "(%s) 登录源远程主机[%s]成功\n", mTask.taskID.c_str(), mTask.srcNode.sHost.c_str() );
    return 0;
}

int TBridger::ConnectDestination() {
    DP( DEBUG, "(%s) 正在连接目标远程主机[%s]...\n", mTask.taskID.c_str(), mTask.dstNode[0].sHost.c_str() );
    try
    {
        destConns[0]->Connect(mTask.dstNode[0].sHost, mTask.dstNode[0].iLinkMode);
    }
    catch(std::exception &e)
    {
        DP(ERROR, "(%s) 目标远程主机[%s]连接失败\n", mTask.taskID.c_str(), mTask.dstNode[0].sHost.c_str());
        DP(ERROR, "(%s) %s[ConnectDestination] Connect: %s\n", mTask.taskID.c_str(), self.c_str(), e.what());
        return -1;
    }
    DP( DEBUG, "(%s) 目标远程主机[%s]已连接\n", mTask.taskID.c_str(), mTask.srcNode.sHost.c_str() );
    DP( DEBUG, "(%s) 正在登录目标远程主机[%s]...\n", mTask.taskID.c_str(), mTask.dstNode[0].sHost.c_str() );
    try
    {
        destConns[0]->Login();
    }
    catch(std::exception &e)
    {
        DP(ERROR, "(%s) 登录目标远程主机[%s]失败\n", mTask.taskID.c_str(), mTask.dstNode[0].sHost.c_str());
        DP(ERROR, "(%s) %s[ConnectDestination] Login: %s\n", mTask.taskID.c_str(), self.c_str(), e.what());
        return -1;
    }
    DP( DEBUG, "(%s) 登录目标远程主机[%s]成功\n", mTask.taskID.c_str(), mTask.dstNode[0].sHost.c_str() );
    return 0;
}

int TBridger::Connect() {
    if (ConnectSource() < 0 || ConnectDestination() < 0)
        return -1;
    DP( DEBUG, "(%s) 切换源目录[%s]...\n", mTask.taskID.c_str(), mTask.srcNode.sPath.c_str() );
    try {
        srcConn->RemoteChDir(mTask.srcNode.sPath);
    } catch (std::exception &e) {
        DP(ERROR, "(%s) 源目录[%s]切换失败\n", mTask.taskID.c_str(), mTask.srcNode.sPath.c_str());
        DP(ERROR, "(%s) %s[Connect] RemoteChDir: %s\n", mTask.taskID.c_str(), self.c_str(), e.what());
        return -1;
    }
    DP( DEBUG, "(%s) 切换源目录[%s]成功\n", mTask.taskID.c_str(), mTask.srcNode.sPath.c_str() );
    DP( DEBUG, "(%s) 切换目标服务器临时目录[%s]...\n", mTask.taskID.c_str(), mTask.tmpPath.c_str() );
    try {
        destConns[0]->RemoteChDir(mTask.tmpPath);
    } catch (std::exception &e) {
        DP(ERROR, "(%s) 目录[%s]切换失败\n", mTask.taskID.c_str(), mTask.tmpPath.c_str());
        DP(ERROR, "(%s) %s[Connect] RemoteChDir: %s\n", mTask.taskID.c_str(), self.c_str(), e.what());
        return -1;
    }

    if (mTask.srcNode.iProtocol == SFTP) {
        srcConn->SetOptions(SFTP_MAX_READ_SIZE,
                mTask.transBlocks * MAX_BLOCK_SIZE);
    }
    if (mTask.dstNode[0].iProtocol == SFTP) {
        destConns[0]->SetOptions(SFTP_MAX_READ_SIZE,
                mTask.transBlocks * MAX_BLOCK_SIZE);
    }
    DP( DEBUG, "(%s) 切换目标服务器临时目录[%s]成功\n", mTask.taskID.c_str(), mTask.tmpPath.c_str() );
    return 0;
}

int TBridger::ReadFileList(const char* ListName, const char* pattern,
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

            if (pFilter->match(value2)) {
                mFileNames.push_back(value2);
                mFileSizes.push_back(strtoll(value1, NULL, 10));
            } else {
                continue;
            }

            break;

        case 8:

            if (*buff == 'd')
                continue;

            if (pFilter->match(value6)) {
                mFileNames.push_back(value6);
                mFileSizes.push_back(strtoll(value3, NULL, 10));
            } else {
                continue;
            }

            break;

        case 9:

            if (*buff == 'd')
                continue;

            if (*buff != 'l') {
                if (pFilter->match(value4)) {
                    mFileNames.push_back(value4);
                    mFileSizes.push_back(strtoll(value3, NULL, 10));
                } else {
                    continue;
                }

                break;
            }

        case 11:

            if (*buff != 'l')
                continue;

            if (pFilter->match(value4)) {
                char path[1024];
                sprintf(path, "%s/%s", mTask.srcNode.sPath.c_str(), value4);
                try {
                    srcConn->RemoteFileSize(path, &fsize, mTask.transMode);
                    if (fsize < 0)
                        continue;
                } catch (std::exception& e) {
                    DP(ERROR, "(%s) 源远程主机[%s]获取link文件(%s)大小失败\n", mTask.taskID.c_str(), mTask.srcNode.sHost.c_str(), value4);
                    DP(ERROR, "(%s) %s[ListFile](%s) RemoteFileSize: %s\n", mTask.taskID.c_str(), self.c_str(), value4, e.what());
                    continue;
                }

                mFileNames.push_back(value4);
                mFileSizes.push_back(fsize);
            } else {
                continue;
            }

            break;

        default:

            continue;

            break;
        }

        ++i;
    }

    fclose(fp);
    DP(DEBUG, "(%s) 读取ReadFileList文件以完成,本次需bridge文件个数为:%d\n", mTask.taskID.c_str(),i);
    return i;
}

int TBridger::ListFile() {
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
        DP(ERROR, "(%s) %s[ReadFileList] fopen: %s\n", mTask.taskID.c_str(), self.c_str(), strerror(errno));
    }
    DP( DEBUG, "(%s) 删除ReadFileList文件[%s]成功\n", mTask.taskID.c_str(), FileList );
    return mFileCount;
}

int TBridger::FileBridge(const std::string &source,
        const std::string &destination, int mode) {
    int l, c;
    char *sBuf;
    size_t buf_size;

    srcConn->RemoteOpenFile(source, FILE_READ, mode);
    destConns[0]->RemoteOpenFile(destination, FILE_WRITE, mode);

    int64_t total = 0;
    buf_size =
            (mTask.srcNode.iProtocol == SFTP
                    || mTask.dstNode[0].iProtocol == SFTP) ?
                    (MAX_BLOCK_SIZE * mTask.transBlocks) : BUFSIZ;

    sBuf = new char[buf_size];
    memset(sBuf, 0, buf_size);
    try {
        while ((l = srcConn->RemoteRead(sBuf, buf_size)) > 0) {
            if ((c = destConns[0]->RemoteWrite(sBuf, l)) < l) {
                char err[1024] = { 0 };
                snprintf(err, sizeof(err), "(%s) <%s> lines:%d short write: passed [%d], wrote [%d]\n",
                        mTask.taskID.c_str(), __FILE__, __LINE__, l, c);
                DP(ERROR, err);
                throw std::runtime_error(err);
            }
            total += c;
        }
    }
    catch (std::exception& e)
    {
        delete []sBuf;
        srcConn->RemoteCloseFile();
        destConns[0]->RemoteCloseFile();
        throw e;
    }

    DP(DEBUG, "(%s) [%d]Bytes Transfered\n", mTask.taskID.c_str(), total);

    delete[] sBuf;
    srcConn->RemoteCloseFile();
    destConns[0]->RemoteCloseFile();
    return 0;
}

int TBridger::Transfer() {
    std::string source;
    std::string destination;
    std::string tempfile;
    char lastMsg[1024] = { 0 };

    time_t b_time;
    time_t t_time;
    time_t e_time;

    DP( DEBUG, "(%s) 开始Bridge 文件个数:%d....\n",mTask.taskID.c_str(), mFileCount );
    for (int xFile = 0; xFile < mFileCount; ++xFile) {
        source = mTask.srcNode.sPath + "/" + mFileNames[xFile];
        if (IsDelFailFile(mFileNames[xFile])) {
            DP( DEBUG, "(%s) 正在尝试删除文件[%s] ...\n", mTask.taskID.c_str(), mFileNames[xFile].c_str() );
            try
            {
                srcConn->RemoteDelete(source);
            }
            catch(std::exception& e)
            {
                DP(ERROR, "(%s) 文件[%s]删除失败\n", mTask.taskID.c_str(), source.c_str());
                DP(ERROR, "(%s) %s[Transfer] RemoteDelete(%s): %s\n", mTask.taskID.c_str(), self.c_str(), source.c_str(), e.what());
                continue;
            }
            DP( DEBUG, "(%s) 文件[%s]删除成功\n", mTask.taskID.c_str(), mFileNames[xFile].c_str() );
            DP( DEBUG, "(%s) 清除查重临时标志文件\n", mTask.taskID.c_str());
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
            FileBridge(source, tempfile, mTask.transMode);
        } catch (std::exception& e) {
            DP(ERROR, "(%s) 文件[%s]到[%s]输失败\n", mTask.taskID.c_str(), source.c_str(), tempfile.c_str());
            DP(ERROR, "(%s) %s[Transfer] FileBridge from (%s) to (%s): %s\n",
            mTask.taskID.c_str(), self.c_str(), source.c_str(), tempfile.c_str(), e.what());

            continue;
        }
        DP( DEBUG, "(%s) 文件[%s]传输成功\n", mTask.taskID.c_str(), mFileNames[xFile].c_str() );

        int64_t sz;
        char filemsg[1024] = { 0 };
        DP(DEBUG, "(%s) 文件[%s].......匹配\n",mTask.taskID.c_str(), mFileNames[xFile].c_str());
        if ((sz = RSize(tempfile, filemsg)) != mFileSizes[xFile]) {
            DP(ERROR, "(%s) 文件[%s]大小不匹配\n", mTask.taskID.c_str(), mFileNames[xFile].c_str());
            DP(ERROR, "(%s) Source[%lld], Destination[%lld]\n", mTask.taskID.c_str(), mFileSizes[xFile], sz);
            continue;
        }

        time(&t_time);
        DP(DEBUG, "(%s) 文件[%s]大小匹配\n",mTask.taskID.c_str(), mFileNames[xFile].c_str());
        impl::RTrim(filemsg, '\n');
        WriteTaskLog(mFileNames[xFile], mFileSizes[xFile], "transfer ok",
                filemsg);

        /******************
         *      *if rename = no then the follow code not be execute
         *           *
         *                *****************/

        if (strncasecmp(mTask.isRename.c_str(), "no", 2) != 0) {
            destination = std::string(mTask.dstNode[0].sPath) + "/"
                    + mFileNames[xFile];
            DP( DEBUG, "(%s) 正在将文件[%s]移到目标目录[%s]...\n", mTask.taskID.c_str(), tempfile.c_str(), destination.c_str());
            try {
                destConns[0]->RemoteRename(tempfile, destination);
            } catch (std::exception& e) {
                DP(ERROR, "(%s) 文件[%s]移动失败\n", mTask.taskID.c_str(), mFileNames[xFile].c_str());
                DP(ERROR, "(%s) %s[Transfer] RemoteRename form[%s] to [%s]: %s\n", mTask.taskID.c_str(),
                        self.c_str(), tempfile.c_str(), destination.c_str(), e.what());
                continue;
            }
            DP( DEBUG, "(%s) 文件[%s]到[%s]移动成功\n", mTask.taskID.c_str(), tempfile.c_str(), destination.c_str());
            WriteTaskLog(mFileNames[xFile], mFileSizes[xFile], "bridge ok");
        }

        time(&e_time);
        DP( DEBUG, "(%s) 记录文件[%s]传输信息(%d,%d,%d)\n", mTask.taskID.c_str(), mFileNames[xFile].c_str(),
        b_time, t_time, e_time );
        int itime = t_time - b_time;
        itime = itime > 0 ? itime : 1;

        memset(lastMsg, 0, sizeof(lastMsg));
        sprintf(lastMsg,
                "Succeed file from [%s] %s:%s to [%s] %s:%s %ld OK ( %.2f Kb/s)",
                getTransProtocol(mTask.srcNode.iProtocol).c_str(),
                mTask.srcNode.sHost.c_str(), source.c_str(),
                getTransProtocol(mTask.dstNode[0].iProtocol).c_str(),
                mTask.dstNode[0].sHost.c_str(), destination.c_str(),
                e_time - b_time, mFileSizes[xFile] / KByte / itime);
        WriteTaskLog(mFileNames[xFile], mFileSizes[xFile], lastMsg);

        DP( DEBUG, "(%s) 正在删除源文件[%s]...\n", mTask.taskID.c_str(), mFileNames[xFile].c_str() );
        try {
            srcConn->RemoteDelete(source);
        } catch (std::exception& e) {
            DP(ERROR, "(%s) 源文件[%s]删除失败\n", mTask.taskID.c_str(), mFileNames[xFile].c_str());
            DP(ERROR, "(%s) %s[Transfer] RemoteDelete: %s\n", mTask.taskID.c_str(), self.c_str(), e.what());
            CreateDelFailFile(mFileNames[xFile]);
            continue;
        }
        DP( DEBUG, "(%s) 源文件[%s]删除成功\n", mTask.taskID.c_str(), mFileNames[xFile].c_str() );
    }
    DP( DEBUG, "(%s) Bridge 文件个数:%d 已完成\n",mTask.taskID.c_str(), mFileCount );

    return 0;
}

int64_t TBridger::RSize(std::string path, char *filemsg) {
    int64_t fsize = -1;

    DP(DEBUG, "(%s) 获取远程文件[%s]大小......\n", mTask.taskID.c_str(), path.c_str());
    try {
        destConns[0]->RemoteFileSize(path, &fsize, mTask.transMode);
    } catch (std::exception &e) {
        DP(ERROR, "(%s) 获取远程文件[%s]大小失败\n", mTask.taskID.c_str(), path.c_str());
        DP(ERROR, "(%s) %s[Transfer] RemoteRSize: %s\n", mTask.taskID.c_str(), self.c_str(), e.what());
        return -1;
    }
    DP(DEBUG, "(%s) 获取远程文件[%s]大小[%ld]成功\n", mTask.taskID.c_str(), path.c_str(), fsize);
    return fsize;
}

void TBridger::initSource() {
    if (mTask.srcNode.iProtocol == FTP) {
        srcConn = new PFtp(mTask.srcNode.sUser, mTask.srcNode.sPasswd);
    } else {
        GlobalSetting *global = GlobalSetting::getInstance();
        srcConn = new PSftp(mTask.srcNode.sUser, mTask.srcNode.sPasswd,
                global->rsaPubFile, global->rsaFile);
    }
}

void TBridger::initTarget(int targetNum) {
    if (mTask.dstNode[targetNum].iProtocol == FTP) {
        destConns.push_back(new PFtp(mTask.dstNode[targetNum].sUser,
                mTask.dstNode[targetNum].sPasswd));
    } else {
        GlobalSetting *global = GlobalSetting::getInstance();
        destConns.push_back(new PSftp(mTask.dstNode[targetNum].sUser,
                mTask.dstNode[targetNum].sPasswd, global->rsaPubFile, global->rsaFile));
    }
}

void TBridger::disconnectSource() {
    if (srcConn) {
        srcConn->DisConnect();
        delete srcConn;
        srcConn = 0;
    }
}
void TBridger::disconnectTarget(int targetNum) {
    if (destConns[targetNum]) {
        destConns[targetNum]->DisConnect();
        delete destConns[targetNum];
        destConns[targetNum] = 0;
    }
}
