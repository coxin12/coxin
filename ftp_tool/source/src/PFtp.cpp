/******************************************************************
 *  PFtp.cpp
 *  Implementation of the Class PFtp
 *  Created on:      05-十二月-2012 11:19:11
 *  Original author: chenzhixin
 *
 * Modified History:
 *     See ChangeLog.
 ******************************************************************/

#include "PFtp.h"
#include "dstruct.h"
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <stdexcept>

int TimeOutIdle(netbuf *ctl, int xfered, void *arg)
{
	printf("select timeout(%d,%d)", ctl->idletime.tv_sec, ctl->idletime.tv_usec);
	return 0;
}

PFtp::PFtp() {
    ftp_handle = NULL;
    data_handle = NULL;
}

PFtp::~PFtp() {
    if (ftp_handle != NULL) {
        FtpQuit(ftp_handle);
    }
    ftp_handle = NULL;
    if (data_handle != NULL) {
        FtpClose(data_handle);
    }
    data_handle = NULL;
}

int PFtp::Connect(std::string host, int mode) {
    if (!FtpConnect(host.c_str(), &ftp_handle, mode)) {
        std::ostringstream oss;
        oss << "FTP Connect error: " << FtpLastResponse(ftp_handle);
        //FtpQuit(ftp_handle);
        //ftp_handle = NULL;
        throw std::runtime_error(oss.str());
    }
    SetOptions(FTPLIB_IDLETIME, 3*1000); // 默认超时时间为3s
    SetOptions(FTPLIB_CALLBACK, (long)TimeOutIdle); // 默认超时回调函数，不做任何处理，只返回错误值0
    return 0;
}

void PFtp::DisConnect() {
    if (ftp_handle != NULL) {
        FtpQuit(ftp_handle);
        ftp_handle = NULL;
    }
}

int PFtp::Login() {
    if (ftp_handle == NULL) {
        throw std::runtime_error("FTP not Connect!");
    }
    if (!FtpLogin(username.c_str(), password.c_str(), ftp_handle)) {
        std::ostringstream oss;
        oss << "FTP Login error: " << ftp_handle->response;
        //FtpQuit(ftp_handle);
        //ftp_handle = NULL;
        throw std::runtime_error(oss.str());
    }
    return 0;
}

int PFtp::GetFile(std::string remote_path, std::string local_path, int mode) {
    if (ftp_handle == NULL) {
        throw std::runtime_error("FTP not Connect!");
    }
    if (mode != ASCII && mode != BIN) {
        throw std::runtime_error(
                "Invalid transfer type. It must be ASCII or BIN .");
    }
    int mod = (mode == ASCII) ? FTPLIB_ASCII : FTPLIB_IMAGE;

    if (!FtpGet(local_path.c_str(), remote_path.c_str(), mod, ftp_handle)) {
        std::ostringstream oss;
        oss << "FTP GetFile(" << remote_path << " >> " << local_path
                << ") error: " << ftp_handle->response;
        //FtpQuit(ftp_handle);
        //ftp_handle = NULL;
        throw std::runtime_error(oss.str());
    }

    return 0;
}

int PFtp::PutFile(std::string local_path, std::string remote_path, int mode) {
    if (ftp_handle == NULL) {
        throw std::runtime_error("FTP not Connect!");
    }

    if (mode != ASCII && mode != BIN) {
        throw std::runtime_error(
                "Invalid transfer type. It must be ASCII or BIN .");
    }
    int mod = (mode == ASCII) ? FTPLIB_ASCII : FTPLIB_IMAGE;

    if (!FtpPut(local_path.c_str(), remote_path.c_str(), mod, ftp_handle)) {
        std::ostringstream oss;
        oss << "FTP PutFile(" << local_path << " >> " << remote_path
                << ") error: " << ftp_handle->response;
        //FtpQuit(ftp_handle);
        //ftp_handle = NULL;
        throw std::runtime_error(oss.str());
    }
    return 0;
}

int PFtp::RemoteChDir(std::string remote_path) {
    if (ftp_handle == NULL) {
        throw std::runtime_error("FTP not Connect!");
    }
    if (!FtpChdir(remote_path.c_str(), ftp_handle)) {
        std::ostringstream oss;
        oss << "FTP ChDir(" << remote_path << ") error: "
                << ftp_handle->response;
        //FtpQuit(ftp_handle);
        //ftp_handle = NULL;
        throw std::runtime_error(oss.str());
    }
    return 0;
}

int PFtp::RemoteDelete(std::string remoteFile) {
    if (ftp_handle == NULL) {
        throw std::runtime_error("FTP not Connect!");
    }
    if (!FtpDelete(remoteFile.c_str(), ftp_handle)) {
        std::ostringstream oss;
        oss << "FTP Delete file(" << remoteFile << ") error: "
                << ftp_handle->response;
        //FtpQuit(ftp_handle);
        //ftp_handle = NULL;
        throw std::runtime_error(oss.str());
    }
    return 0;
}

int PFtp::RemoteOpenFile(std::string remote_path, int type, int mode) {
    if (ftp_handle == NULL) {
        throw std::runtime_error("FTP not Connect!");
    }

    if (type != FILE_READ && type != FILE_WRITE) {
        throw std::runtime_error(
                "Invalid open type. It must be FILE_READ or FILE_WRITE.");
    }
    int typ = (type == FILE_READ) ? FTPLIB_FILE_READ : FTPLIB_FILE_WRITE;

    if (mode != ASCII && mode != BIN) {
        throw std::runtime_error(
                "Invalid transfer type. It must be ASCII or BIN .");
    }
    int mod = (mode == ASCII) ? FTPLIB_ASCII : FTPLIB_IMAGE;

    if (!FtpAccess(remote_path.c_str(), typ, mod, ftp_handle, &data_handle)) {
        std::ostringstream oss;
        oss << "FTP Open file(" << remote_path << ") error: "
                << ftp_handle->response;
        FtpClose(data_handle);
        data_handle = NULL;
        //FtpQuit(ftp_handle);
        //ftp_handle = NULL;
        throw std::runtime_error(oss.str());
    }
    return 0;
}

int PFtp::RemoteCloseFile() {
    if (data_handle != NULL) {
        FtpClose(data_handle);
    }
    data_handle = NULL;
    return 0;
}

int PFtp::RemoteFileSize(std::string remote_file, int64_t *size, int mode) {
    if (ftp_handle == NULL) {
        throw std::runtime_error("FTP not Connect!");
    }

    if (mode != ASCII && mode != BIN) {
        throw std::runtime_error(
                "Invalid transfer type. It must be ASCII or BIN .");
    }

    int mod = FTPLIB_IMAGE; //只有采用bin方式获取才正确

    int64_t isize;

    if (!(FtpSize(remote_file.c_str(), &isize, mod, ftp_handle))) {
        std::ostringstream oss;
        oss << "FTP FileSize(" << remote_file << ") error: "
                << ftp_handle->response;
        //FtpQuit(ftp_handle);
        //ftp_handle = NULL;
        throw std::runtime_error(oss.str());
    }
    *size = isize;
    return 0;
}

int PFtp::RemoteLs(std::string local_file, std::string remote_path) {
    if (ftp_handle == NULL) {
        throw std::runtime_error("FTP not Connect!");
    }

    if (!FtpLsRT(local_file.c_str(), remote_path.c_str(), ftp_handle)) {
        std::ostringstream oss;
        oss << "FTP LsRT(" << remote_path << ") error: "
                << ftp_handle->response;
        //FtpQuit(ftp_handle);
        //ftp_handle = NULL;
        throw std::runtime_error(oss.str());
    }
    return 0;
}

std::vector<FileAttribute> PFtp::RemoteDir(std::string remote_path,
        std::string option) {
    std::vector<FileAttribute> v_files;
    return v_files;
}

int PFtp::RemoteMkDir(std::string remotePath, int mode) {
    if (ftp_handle == NULL) {
        throw std::runtime_error("FTP not Connect!");
    }
    if (!FtpMkdir(remotePath.c_str(), ftp_handle)) {
        std::ostringstream oss;
        oss << "FTP MkDir(" << remotePath << ") error: "
                << ftp_handle->response;
        throw std::runtime_error(oss.str());
    }
    return 0;
}

int PFtp::RemoteRmDir(std::string remotePath) {
    if (ftp_handle == NULL) {
        throw std::runtime_error("FTP not Connect!");
    }
    if (!FtpRmdir(remotePath.c_str(), ftp_handle)) {
        std::ostringstream oss;
        oss << "FTP RmDir(" << remotePath << ") error: "
                << ftp_handle->response;
        throw std::runtime_error(oss.str());
    }
    return 0;
}

int PFtp::RemotePwd(std::string &outPut) {
    if (ftp_handle == NULL) {
        throw std::runtime_error("FTP not Connect!");
    }
    char sPath[1024] = { 0 };
    if (!FtpPwd(sPath, 1024, ftp_handle) || strlen(sPath) == 0) {
        std::ostringstream oss;
        oss << "FTP Pwd error: " << ftp_handle->response;
        //FtpQuit(ftp_handle);
        //ftp_handle = NULL;
        throw std::runtime_error(oss.str());
    }
    outPut = sPath;
    return 0;
}

int PFtp::RemoteRead(char *sBuf, int maxSize) {
    if (data_handle == NULL) {
        throw std::runtime_error("file not open, cann't read!");
    }
    int64_t readSize;
    if ((readSize = FtpRead(sBuf, maxSize, data_handle)) < 0) {
        std::ostringstream oss;
        oss << "FTP ReadDate error: " << data_handle->response;
        //FtpClose(data_handle);
        //data_handle = NULL;
        throw std::runtime_error(oss.str());
    }
    return readSize;
}

int PFtp::RemoteWrite(char *sBuf, int len) {
    if (data_handle == NULL) {
        throw std::runtime_error("file not open, cann't read!");
    }

    int64_t readSize;
    if ((readSize = FtpWrite(sBuf, len, data_handle)) < len) {
        std::ostringstream oss;
        oss << "FTP WriteDate error: " << data_handle->response;
        //FtpClose(data_handle);
        //data_handle = NULL;
        throw std::runtime_error(oss.str());
    }
    return readSize;
}

int PFtp::RemoteRename(std::string remote_old_path,
        std::string remote_new_path) {

    if (ftp_handle == NULL) {
        throw std::runtime_error("FTP not Connect!");
    }

    if (!FtpRename(remote_old_path.c_str(), remote_new_path.c_str(),
            ftp_handle)) {
        std::ostringstream oss;
        oss << "FTP Rename (" << remote_old_path << " >> " << remote_new_path
                << ")error: " << ftp_handle->response;
        //FtpQuit(ftp_handle);
        //ftp_handle = NULL;
        throw std::runtime_error(oss.str());
    }
    return 0;
}

int PFtp::SetOptions(int opt, long val) {
    if (ftp_handle == NULL) {
        throw std::runtime_error("FTP not Connect!");
    }

    if (!FtpOptions(opt, val, ftp_handle)) {
        std::ostringstream oss;
        oss << "FTP Set Options (" << opt << "," << val << ")error: "
                << ftp_handle->response;
        throw std::runtime_error(oss.str());
    }
    return 0;
}

