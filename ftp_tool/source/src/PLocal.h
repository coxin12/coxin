/*
 * PLocal.h
 *
 *  Created on: 2013年11月15日
 *      Author: liangjianqiang
 */

#ifndef PLOCAL_H_
#define PLOCAL_H_

#include <string>
#include "ProtocolBase.h"

// 本地命令协议
class PLocal: public ProtocolBase {
public:
    PLocal();
    virtual ~PLocal();

    virtual int Connect(std::string host, int mode = -1) {
        return 0;
    }

    virtual void DisConnect() {
    }

    virtual int Login() {
        return 0;
    }

    virtual int GetFile(std::string remote_path, std::string local_path,
            int mode = -1) {
        return 0;
    }

    virtual int PutFile(std::string local_path, std::string remote_path,
            int mode = -1) {
        return 0;
    }

    virtual int RemoteChDir(std::string remote_path);
    virtual int RemoteDelete(std::string remoteFile);
    virtual int RemoteOpenFile(std::string remote_path, int type,
            int mode = -1);
    virtual int RemoteCloseFile();
    virtual int RemoteFileSize(std::string remote_file, long *size, int mode =
            -1);
    virtual int RemoteLs(std::string local_file, std::string remote_path);
    virtual std::vector<FileAttribute> RemoteDir(std::string remote_path,
            std::string option);
    virtual int RemoteMkDir(std::string remotePath, int mode = -1);
    virtual int RemoteRmDir(std::string remotePath);
    virtual int RemotePwd(std::string &outPut);
    virtual int RemoteRead(char *sBuf, int maxSize);
    virtual int RemoteWrite(char *sBuf, int len);
    virtual int RemoteRename(std::string remote_old_path,
            std::string remote_new_path);

    virtual int SetOptions(int opt, long val) {
        return 0;
    }
private:
    std::string workDir;

    int fd;
};

#endif /* PLOCAL_H_ */
