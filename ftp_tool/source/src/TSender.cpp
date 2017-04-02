///////////////////////////////////////////////////////////
//  TSender.cpp
//  Implementation of the Class TSender
//  Created on:      05-ʮ����-2012 11:19:11
//  Original author: chenzhixin
//  VERSION     MODTIME       AUTHOR      MEMO
//  V12.00.001  2012-12-05  chenzhixin  new create
///////////////////////////////////////////////////////////

#include "TLog.h"
#include "TSender.h"
#include "PFtp.h"
#include "PSftp.h"
#include <errno.h>
#include "GlobalSetting.h"
#include <dirent.h>

TSender::TSender() {
    self = "TSender";
}

TSender::~TSender() {
}

int TSender::Connect() {
    DP( DEBUG, "(%s) ��������Զ������[%s]...\n", mTask.taskID.c_str(), mTask.dstNode[0].sHost.c_str() );
    try
    {
        destConns[0]->Connect(mTask.dstNode[0].sHost, mTask.dstNode[0].iLinkMode);
    }
    catch(std::exception &e)
    {
        DP(ERROR, "(%s) Զ������[%s]����ʧ��\n", mTask.taskID.c_str(), mTask.dstNode[0].sHost.c_str());
        DP(ERROR, "(%s) %s[Connect] Connect: %s\n", mTask.taskID.c_str(), self.c_str(), e.what());
        return -1;
    }
    DP( DEBUG, "(%s) Զ������[%s]������\n", mTask.taskID.c_str(), mTask.dstNode[0].sHost.c_str() );
    DP( DEBUG, "(%s) ���ڵ�¼...\n", mTask.taskID.c_str() );
    try
    {
        destConns[0]->Login();
    }
    catch(std::exception &e)
    {
        DP(ERROR, "(%s) ��¼[%s]ʧ��\n", mTask.taskID.c_str(), mTask.dstNode[0].sHost.c_str());
        DP(ERROR, "(%s) %s[Connect] Login: %s\n", mTask.taskID.c_str(), self.c_str(), e.what());
        return -1;
    }
    DP( DEBUG, "(%s) ��¼�ɹ�\n", mTask.taskID.c_str() );

    DP( DEBUG, "(%s) �л�Ŀ���������ʱĿ¼[%s]...\n", mTask.taskID.c_str(), mTask.tmpPath.c_str() );
    try
    {
        destConns[0]->RemoteChDir(mTask.tmpPath);
    }
    catch(std::exception &e)
    {
        DP(ERROR, "(%s) Ŀ¼[%s]�л�ʧ��\n", mTask.taskID.c_str(), mTask.tmpPath.c_str());
        DP(ERROR, "(%s) %s[Connect] RemoteChDir: %s\n", mTask.taskID.c_str(), self.c_str(), e.what());
        return -1;
    }
    if ( mTask.dstNode[0].iProtocol == SFTP )
    {
        destConns[0]->SetOptions(SFTP_MAX_READ_SIZE, mTask.transBlocks * MAX_BLOCK_SIZE);
    }
    DP( DEBUG, "(%s) �л�Ŀ���������ʱĿ¼[%s]�ɹ�\n", mTask.taskID.c_str(), mTask.tmpPath.c_str() );
    return 0;
}

int TSender::Transfer() {
    std::string source;
    std::string destination;
    std::string tempfile;
    char lastMsg[1024] = { 0 };
    time_t b_time;
    time_t t_time;
    time_t e_time;

    for (int xFile = 0; xFile < mFileCount; ++xFile) {
        source = std::string(mTask.srcNode.sPath) + "/"
                + mFileNames[xFile];

        if (IsDelFailFile(mFileNames[xFile])) {
            if (unlink(source.c_str()) == 0)
                UnlinkDelFailFile(mFileNames[xFile]);
            continue;
        }

        if (mFileSizes[xFile] == 0)
            DP(WARNING, "(%s) ���ļ�[%s](%d)\n", mTask.taskID.c_str(), mFileNames[xFile].c_str(), mFileSizes[xFile]);

        tempfile = std::string(mTask.tmpPath) + "/" + mFileNames[xFile];

        DP( DEBUG, "(%s) ���ڴ����ļ���[%s]��[%s] ...\n", mTask.taskID.c_str(), source.c_str(),tempfile.c_str());
        WriteTaskLog(mFileNames[xFile].c_str(), mFileSizes[xFile],
                "transfer ...");

        time(&b_time);

        try {
            destConns[0]->PutFile(source, tempfile, mTask.transMode);
        } catch (std::exception &e) {
            DP(ERROR, "(%s) �ļ�[%s]����ʧ��\n", mTask.taskID.c_str(), mFileNames[xFile].c_str());
            DP(ERROR, "(%s) %s[Transfer] PutFile(%s): %s\n", mTask.taskID.c_str(), self.c_str(), source.c_str(), e.what());
            continue;
        }
        DP( DEBUG, "(%s) �ļ�[%s]����ɹ�\n", mTask.taskID.c_str(), mFileNames[xFile].c_str() );

        int64_t sz = 0;
        if ((sz = RSize(tempfile)) != mFileSizes[xFile]) {
            DP(ERROR, "(%s) �ļ�[%s]��С��ƥ��\n", mTask.taskID.c_str(), mFileNames[xFile].c_str());
            DP(ERROR, "(%s) Source[%d], Destination[%d]\n", mTask.taskID.c_str(), mFileSizes[xFile], sz);
            continue;
        }
        time(&t_time);
        WriteTaskLog(mFileNames[xFile].c_str(), mFileSizes[xFile],
                "transfer ok");

        if (strncasecmp(mTask.isRename.c_str(), "no", 2) != 0) {
            destination = std::string(mTask.dstNode[0].sPath) + "/"
                    + mFileNames[xFile];
            DP( DEBUG, "(%s) ���ڽ��ļ�[%s]�Ƶ�Ŀ��Ŀ¼...\n", mTask.taskID.c_str(), mFileNames[xFile].c_str() );
            try {
                destConns[0]->RemoteRename(tempfile, destination);
            } catch (std::exception &e) {
                DP(ERROR, "(%s) �ļ�[%s]�ƶ�ʧ��\n", mTask.taskID.c_str(), mFileNames[xFile].c_str());
                DP(ERROR, "(%s) %s[Transfer] RemoteRename from (%s) to (%s): %s\n",
                        mTask.taskID.c_str(), self.c_str(), tempfile.c_str(), destination.c_str(), e.what());
                continue;
            }
            DP( DEBUG, "(%s) �ļ�[%s]�ƶ��ɹ�\n", mTask.taskID.c_str(), mFileNames[xFile].c_str() );
            WriteTaskLog(mFileNames[xFile], mFileSizes[xFile], "send ok");
        }

        time(&e_time);

        DP( DEBUG, "(%s) ��¼�ļ�[%s]������Ϣ(%d,%d,%d)\n", mTask.taskID.c_str(), mFileNames[xFile].c_str(),
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

        DP( DEBUG, "(%s) ����ɾ��Դ�ļ�[%s]...\n", mTask.taskID.c_str(), mFileNames[xFile].c_str() );
        if (unlink(source.c_str()) < 0) {
            CreateDelFailFile(mFileNames[xFile].c_str());
            DP(ERROR, "(%s) Դ�ļ�[%s]ɾ��ʧ��\n", mTask.taskID.c_str(), mFileNames[xFile].c_str());
            DP(ERROR, "(%s) %s[Transfer] unlink(%s): %s\n", mTask.taskID.c_str(), self.c_str(), source.c_str(), strerror(errno));
            continue;
        }
        DP( DEBUG, "(%s) Դ�ļ�[%s]ɾ���ɹ�\n", mTask.taskID.c_str(), mFileNames[xFile].c_str() );
    }
    DP( DEBUG, "(%s) Send �ļ�����:%d �����\n",mTask.taskID.c_str(), mFileCount );
    return 0;
}

int TSender::ListFile() {
    FileSet llist;
    FileAttribute fileAttr;

    mFileNames.clear();
    mFileSizes.clear();

    DIR *dirp;
    if ((dirp = opendir(mTask.srcNode.sPath.c_str())) == NULL) {
        DP(ERROR, "(%s) Ŀ¼[%s]��ʧ��\n", mTask.taskID.c_str(),
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
            DP(ERROR, "(%s) �ļ�[%s]��Ϣ��ȡʧ��\n", pathname);
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

void TSender::disconnectTarget(int targetNum) {
    if (destConns[0]) {
        destConns[0]->DisConnect();
        delete destConns[0];
        destConns[0] = 0;
    }
}

void TSender::initTarget(int targetNum) {
    if (mTask.dstNode[targetNum].iProtocol == FTP) {
        destConns.push_back(new PFtp(mTask.dstNode[targetNum].sUser, mTask.dstNode[targetNum].sPasswd));
    } else {
        GlobalSetting *global = GlobalSetting::getInstance();
        destConns.push_back(new PSftp(mTask.dstNode[targetNum].sUser, mTask.dstNode[targetNum].sPasswd,
                global->rsaPubFile, global->rsaFile));
    }
}

int64_t TSender::RSize(std::string path) {
    int64_t fsize = -1;

    DP(DEBUG, "(%s) ��ȡԶ���ļ�[%s]��С......\n", mTask.taskID.c_str(), path.c_str());
    try {
        destConns[0]->RemoteFileSize(path, &fsize, mTask.transMode);
    } catch (std::exception &e) {
        DP(ERROR, "(%s) ��ȡԶ���ļ�[%s]��Сʧ��\n", mTask.taskID.c_str(), path.c_str());
        DP(ERROR, "(%s) %s[Transfer] RemoteRSize: %s\n", mTask.taskID.c_str(),
                self.c_str(), e.what());
        return -1;
    }
    DP(DEBUG, "(%s) ��ȡԶ���ļ�[%s]��С[%ld]�ɹ�\n", mTask.taskID.c_str(), path.c_str(), fsize);
    return fsize;
}
