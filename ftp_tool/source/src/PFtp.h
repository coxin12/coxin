/******************************************************************
 *  PFtp.h
 *  Implementation of the Class PFtp
 *  Created on:      05-十二月-2012 11:19:11
 *  Original author: chenzhixin
 *
 * Modified History:
 *     See ChangeLog.
 ******************************************************************/

#if !defined(P_FTP_H)
#define P_FTP_H

#include "ProtocolBase.h"
#include "ftplib_mt.h"

class PFtp: public ProtocolBase {

public:
    PFtp();
    PFtp(std::string username, std::string password) :
            username(username), password(password) {
        ftp_handle = NULL;
        data_handle = NULL;
    }
    virtual ~PFtp();

    void SetUsername(std::string username) {
        this->username = username;
    }
    void SetPassword(std::string password) {
        this->password = password;
    }

    int Connect(std::string host, int mode = -1);
    void DisConnect();
    int GetFile(std::string remote_path, std::string local_path, int mode);
    int Login();
    int PutFile(std::string local_path, std::string remote_path, int mode);
    int RemoteChDir(std::string remote_path);
    int RemoteDelete(std::string remoteFile);
    int RemoteOpenFile(std::string remote_path, int type, int mode);
    int RemoteCloseFile();

    //ftp获取远程文件大小，只能采用binary方式，固mode参数设置无效。
    int RemoteFileSize(std::string remote_file, int64_t *size, int mode);
    int RemoteLs(std::string local_file, std::string remote_path);
    std::vector<FileAttribute> RemoteDir(std::string remote_path,
            std::string option);
    int RemoteMkDir(std::string remotePath, int mode);
    int RemoteRmDir(std::string remotePath);
    int RemotePwd(std::string &outPut);
    int RemoteRead(char *sBuf, int maxSize);
    int RemoteWrite(char *sBuf, int len);
    int RemoteRename(std::string remote_old_path, std::string remote_new_path);

    int SetOptions(int opt, long val);

private:
    std::string username;
    std::string password;
    netbuf *ftp_handle;
    netbuf *data_handle;
};
#endif // !defined(P_FTP_H)
