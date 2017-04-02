/******************************************************************
 *  PFtp.h
 *  SFTP–≠“È¿‡
 *  Created on:      2012-12-05
 *  Original author: liangjianqiang
 *
 * Modified History:
 *     See ChangeLog.
 ******************************************************************/

#if !defined(P_SFTP_H)
#define P_SFTP_H

#include "ProtocolBase.h"
#include <pthread.h>
#include <libssh2.h>
#include <libssh2_sftp.h>

const int MAX_BLOCK_SIZE = 30000;

class PSftp: public ProtocolBase {
public:
    PSftp() :
            sock(0), max_read_size(MAX_BLOCK_SIZE), isRenameOverwrite(true), ssh_session(
                    0), sftp_session(0), sftp_handle(0) {
    }
    PSftp(std::string username, std::string password,
            std::string id_rsa_pub_path, std::string id_rsa_path) :
            sock(0), max_read_size(MAX_BLOCK_SIZE), isRenameOverwrite(true), ssh_session(
                    0), sftp_session(0), sftp_handle(0), username(username), password(
                    password), id_rsa_pub_path(id_rsa_pub_path), id_rsa_path(
                    id_rsa_path) {
    }
    virtual ~PSftp();

    void SetMaxReadSize(int max_read_size) {
        this->max_read_size = max_read_size;
    }
    void SetUsername(std::string username) {
        this->username = username;
    }
    void SetPassword(std::string password) {
        this->password = password;
    }
    void SetIdRsaPubPath(std::string id_rsa_pub_path) {
        this->id_rsa_pub_path = id_rsa_pub_path;
    }
    void SetIdRsaPath(std::string id_rsa_path) {
        this->id_rsa_path = id_rsa_path;
    }

    int Connect(std::string host, int mode = -1);
    void DisConnect();

    /*
     * @brief Login the remote system.
     *
     * @remark  It will examine both password authorization's and public-key
     *          authorization's conditions, then determine which authorization
     *          will use. If both are avaliable, it will try password authorization
     *          first, and public-key authorization when password way failed.
     */
    int Login();
    int GetFile(std::string remote_path, std::string local_path, int mode = -1);
    int PutFile(std::string local_path, std::string remote_path, int mode = -1);
    int RemoteChDir(std::string remote_path);
    int RemoteDelete(std::string remoteFile);
    int RemoteOpenFile(std::string remote_path, int type, int mode = -1);
    int RemoteCloseFile();
    int RemoteFileSize(std::string remote_file, int64_t *size, int mode = -1);
    int RemoteLs(std::string local_file, std::string remote_path);
    std::vector<FileAttribute> RemoteDir(std::string remote_path,
            std::string option);
    int RemoteMkDir(std::string remotePath, int mode = -1);
    int RemoteRmDir(std::string remotePath);
    int RemotePwd(std::string &outPut);
    int RemoteRead(char *sBuf, int maxSize);
    int RemoteWrite(char *sBuf, int len);

    /*
     * @brief Rename the remote path to a new one.
     *
     * @remark  There is a known bug in it. If the 'remote_new_path' is
     *          already exist, this function will fail because the 'flags'
     *          in function 'sftp_rename'(in sftp.c) is not set if sftp->version
     *          is smaller than 5, but comment that condition it still fail.
     */
    int RemoteRename(std::string remote_old_path, std::string remote_new_path);
    int SetOptions(int opt, long val);

private:
    int remoteOpenDir(std::string remote_path);
    int remoteCloseDir();
    std::string getLastSSHErrorMsg();

private:
    int sock;
    int max_read_size;
    bool isRenameOverwrite;
    LIBSSH2_SESSION *ssh_session;
    LIBSSH2_SFTP *sftp_session;
    LIBSSH2_SFTP_HANDLE *sftp_handle;
    std::string username;
    std::string password;
    std::string id_rsa_pub_path;
    std::string id_rsa_path;
    std::string current_remote_path;

public:
    static pthread_rwlock_t rwlock_t;
};
#endif // !defined(P_SFTP_H)
