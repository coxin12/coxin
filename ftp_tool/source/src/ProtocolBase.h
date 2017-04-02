/******************************************************************
 *  ProtocolBase.h
 *  �ļ�����Э��ӿ���
 *  Created on:      2012-12-05
 *  Original author: liangjianqiang
 *
 * Modified History:
 *     See ChangeLog.
 ******************************************************************/

#if !defined(BPFILE_TRANSFER_H)
#define BPFILE_TRANSFER_H

#include <vector>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include "dstruct.h"

enum OPEN_FILE_TYPE {
    FILE_READ, /*�Զ���ʽ���ļ�*/
    FILE_WRITE, /*��д��ʽ���ļ�*/
};

// SetOptions������opt������ö��
enum OPTION_NAME {
    SFTP_COMPRESS, /*ѹ��ѡ�0Ϊ��ʹ��ѹ����1Ϊʹ��ѹ������Ĭ�Ͻ���������Ϊ��ѹ����*/
    SFTP_MAX_READ_SIZE, /*����ȡ�ֽ�����Ĭ��Ϊ30000��*/
    SFTP_RENAME_OVERWRITE, /*rename�����Ƿ񸲸�ѡ�0Ϊ�����ǣ�1Ϊ���ǡ���Ĭ��Ϊ1��*/
};

class ProtocolBase {
public:
    virtual ~ProtocolBase() {
    }

    virtual int Connect(std::string host, int mode = -1) = 0;
    virtual void DisConnect() = 0;
    virtual int Login() = 0;
    virtual int GetFile(std::string remote_path, std::string local_path,
            int mode = -1) = 0;
    virtual int PutFile(std::string local_path, std::string remote_path,
            int mode = -1) = 0;
    virtual int RemoteChDir(std::string remote_path) = 0;
    virtual int RemoteDelete(std::string remoteFile) = 0;

    /*
     * @brief Open the remote file handle.
     * @param type  it refer to the enum OPEN_FILE_TYPE.
     */
    virtual int RemoteOpenFile(std::string remote_path, int type,
            int mode = -1) = 0;
    virtual int RemoteCloseFile() = 0;

    /*
     * @brief Get the remote file's size.
     * @param remote_file   the remote file you want to know
     * @param size          the file size will return to this pointer.
     * @param mode          just use in FTP to determined transfer mode.
     */
    virtual int RemoteFileSize(std::string remote_file, int64_t *size, int mode =
            -1) = 0;

    /*
     * @brief Get a list of files from remote system and save the result to local system.
     * @param local_file    the local_file where the result save in
     * @param remote_path   the remote path you want to list
     */
    virtual int RemoteLs(std::string local_file, std::string remote_path) = 0;

    /*
     * @brief Get a list of files from remote system
     * @param remote_path   the remote path you want to list
     * @param option        supported options include 'a', 't', 'r' which meaning is like 'ls'
     *
     * @return It return a 'file_attribute_t' vector which contain the files infomation.
     */
    virtual std::vector<FileAttribute> RemoteDir(std::string remote_path,
            std::string option) = 0;
    virtual int RemoteMkDir(std::string remotePath, int mode = -1) = 0;
    virtual int RemoteRmDir(std::string remotePath) = 0;
    virtual int RemotePwd(std::string &outPut) = 0;
    virtual int RemoteRead(char *sBuf, int maxSize) = 0;
    virtual int RemoteWrite(char *sBuf, int len) = 0;
    virtual int RemoteRename(std::string remote_old_path,
            std::string remote_new_path) = 0;

    /*
     * @brief Set options of specified protocol.
     * @param opt   refer to the enum OPTION_NAME
     * @param val   value of the certain option. it can be a long integer or enum too.
     */
    virtual int SetOptions(int opt, long val) = 0;
};

#endif // !defined(BPFILE_TRANSFER_H)
