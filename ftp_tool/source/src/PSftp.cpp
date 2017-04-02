/******************************************************************
 *  PFtp.cpp
 *  SFTP协议类
 *  Created on:      2012-12-05
 *  Original author: liangjianqiang
 *
 * Modified History:
 *     See ChangeLog.
 ******************************************************************/

#include "PSftp.h"

#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include "TLog.h"

pthread_rwlock_t PSftp::rwlock_t = PTHREAD_RWLOCK_INITIALIZER;

PSftp::~PSftp() {
    if (this->sftp_handle) {
        this->sftp_handle = NULL;
    }

    if (this->sftp_session) {
        this->sftp_session = NULL;
    }

    if (this->ssh_session) {
        this->ssh_session = NULL;
    }

    this->sock = 0;
}

/*
 * mode 参数没有使用
 */
int PSftp::Connect(std::string host, int mode/* = -1*/) {
    pthread_rwlock_wrlock(&PSftp::rwlock_t);
    int ret = libssh2_init(0);
    pthread_rwlock_unlock(&PSftp::rwlock_t);

    if (ret) {
        throw std::runtime_error(
                "libssh2_init error: " + this->getLastSSHErrorMsg());
    }

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(22);   // ssh端口
    sin.sin_addr.s_addr = inet_addr(host.c_str());

    this->sock = socket(AF_INET, SOCK_STREAM, 0);
    ret = ::connect(this->sock, (struct sockaddr*) (&sin),
            sizeof(struct sockaddr_in));
    if (ret) {
        throw std::runtime_error("Failed to connect host " + host);
    }

    // 初始化SSH session
    this->ssh_session = libssh2_session_init();
    if (!this->ssh_session) {
        throw std::runtime_error(
                "libssh2_session_init() error: " + this->getLastSSHErrorMsg());
    }

    libssh2_session_set_blocking(this->ssh_session, 1);

    // 建立SSH连接
    ret = libssh2_session_handshake(this->ssh_session, this->sock);
    if (ret) {
        throw std::runtime_error(
                "Fail to establishing SSH session: "
                        + this->getLastSSHErrorMsg());
    }

    return 0;
}

void PSftp::DisConnect() {
    if (this->sftp_session) {
        libssh2_sftp_shutdown(this->sftp_session);
    }

    if (this->ssh_session) {
        libssh2_session_disconnect(this->ssh_session, "");
        libssh2_session_free(this->ssh_session);
        ::close(this->sock);
        libssh2_exit();
    }
    // 重置成员变量
    this->username = "";
    this->password = "";
    this->id_rsa_pub_path = "";
    this->id_rsa_path = "";
    this->current_remote_path = "";
    this->isRenameOverwrite = true;
    this->max_read_size = MAX_BLOCK_SIZE;
}

int PSftp::Login() {
    if (this->username.empty()) {
        throw std::runtime_error("Login error: username is empty.");
    }

    // 检查认证方式
    int auth_pw = 0;
    char *userauthlist = libssh2_userauth_list(this->ssh_session,
            this->username.c_str(), this->username.length());
    if (strstr(userauthlist, "password")) {
        if (!this->password.empty()) {
            auth_pw |= 1;
        }
    }
    if (strstr(userauthlist, "publickey")) {
        if (!this->id_rsa_pub_path.empty() && !this->id_rsa_path.empty()) {
            auth_pw |= 2;
        }
    }
    // free(userauthlist);

    // 开始认证
    switch (auth_pw) {
    case 3:
        // 两种认证方式都支持的情况下，先使用密码认证，失败后再尝试公钥认证
        if (libssh2_userauth_password(this->ssh_session, this->username.c_str(),
                this->password.c_str()) != 0) {
            if (libssh2_userauth_publickey_fromfile(this->ssh_session, this->username.c_str(),
                    this->id_rsa_pub_path.c_str(), this->id_rsa_path.c_str(), "")
                    != 0) {
                throw std::runtime_error(
                        "Login error: Both authorization method are failed.");
            }
        }
        break;
    case 2:
        if (libssh2_userauth_publickey_fromfile(this->ssh_session, this->username.c_str(),
                this->id_rsa_pub_path.c_str(), this->id_rsa_path.c_str(), "")
                != 0) {
            throw std::runtime_error(
                    "Login error:(libssh2_userauth_publickey_fromfile) "
                            + this->getLastSSHErrorMsg());
        }
        break;
    case 1:
        if (libssh2_userauth_password(this->ssh_session, this->username.c_str(),
                this->password.c_str()) != 0) {
            throw std::runtime_error(
                    "Login error:(libssh2_userauth_password) "
                            + this->getLastSSHErrorMsg());
        }
        break;
    case 0:
        // 可能的情况：
        // 1.服务器没有适合的认证方式
        // 2.支持的认证方式提供的信息不全。
        throw std::runtime_error(
                "Login error: No authorization method was avaliable.");
        break;
    }

    // 建立SFTP session
    this->sftp_session = libssh2_sftp_init(this->ssh_session);
    if (!this->sftp_session) {
        std::ostringstream oss;
        oss << "Unable to init SFTP session: "
                << libssh2_sftp_last_error(this->sftp_session);
        throw std::runtime_error(oss.str());
    }
    return 0;
}

/*
 * mode 参数没有使用
 */
int PSftp::GetFile(std::string remote_path, std::string local_path,
        int mode/* = -1*/) {
    int get_size = 0;
    // 打开sftp读句柄
    this->RemoteOpenFile(remote_path, FILE_READ);

    int outFile = ::open(local_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC,
            S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (outFile <= 0) {
        this->RemoteCloseFile();
        throw std::runtime_error(
                local_path + ": Local file cannot open to write.");
    }

    char *buf = new char[this->max_read_size];
    if (!buf) {
        ::close(outFile);
        this->RemoteCloseFile();
        throw std::runtime_error("GetFile: Allocation failed.");
    }
    int n = 0;
    while ((n = this->RemoteRead(buf, this->max_read_size)) != 0) {
        get_size += ::write(outFile, buf, n);
    }

    delete[] buf;
    ::close(outFile);
    this->RemoteCloseFile();
    return get_size;
}

/*
 * mode 参数没有使用
 */
int PSftp::PutFile(std::string local_path, std::string remote_path,
        int mode/* = -1*/) {
    int put_size = 0;
    int inFile = ::open(local_path.c_str(), O_RDONLY);
    if (inFile <= 0) {
        throw std::runtime_error(local_path + ": Local file does not exist.");
    }

    // 打开sftp写句柄
    this->RemoteOpenFile(remote_path, FILE_WRITE);

    char *buf = new char[this->max_read_size];
    if (!buf) {
        this->RemoteCloseFile();
        ::close(inFile);
        throw std::runtime_error("PutFile: Allocation failed.");
    }

    int n = 0;
    while ((n = ::read(inFile, buf, this->max_read_size)) != 0) {
        put_size += this->RemoteWrite(buf, n);
    }

    delete[] buf;
    this->RemoteCloseFile();
    ::close(inFile);
    return put_size;
}

int PSftp::RemoteChDir(std::string remote_path) {
    this->current_remote_path = remote_path;
    LIBSSH2_SFTP_HANDLE *sftp_handle =
            libssh2_sftp_opendir(this->sftp_session, remote_path.c_str());
    if (!sftp_handle) {
        std::ostringstream oss;
        oss << remote_path << ": Unable to open dir with SFTP. "
                << strerror(libssh2_sftp_last_error(this->sftp_session));
        throw std::runtime_error(oss.str());
    }
    char buffer[512];
    char longentry[512];
    LIBSSH2_SFTP_ATTRIBUTES attr;
    int ret = libssh2_sftp_readdir_ex(sftp_handle, buffer, sizeof(buffer),
            longentry, sizeof(longentry), &attr);  // 如果ret=0则表示没权限读该目录
    if (ret == 0) {
        libssh2_sftp_closedir(this->sftp_handle);
        std::ostringstream oss;
        oss << remote_path
                << ": Unable to open dir with SFTP. Permission denied";
        throw std::runtime_error(oss.str());
    }
    libssh2_sftp_closedir(this->sftp_handle);

    return 0;
}

/*
 * mode 参数没有使用
 */
int PSftp::RemoteOpenFile(std::string remote_path, int type,
        int mode/* = -1*/) {
    if (type != FILE_READ && type != FILE_WRITE) {
        throw std::runtime_error(
                "Invalid open type. It must be SFTP_READ or SFTP_WRITE.");
    }
    unsigned long flags =
            (type == FILE_READ) ?
                    (LIBSSH2_FXF_READ) :
                    (LIBSSH2_FXF_WRITE | LIBSSH2_FXF_CREAT | LIBSSH2_FXF_TRUNC);
    long permission =
            (type == FILE_READ) ?
                    (0) :
                    (LIBSSH2_SFTP_S_IRUSR | LIBSSH2_SFTP_S_IWUSR
                            | LIBSSH2_SFTP_S_IRGRP | LIBSSH2_SFTP_S_IWGRP
                            | LIBSSH2_SFTP_S_IROTH);
    std::string msg = (type == FILE_READ) ? ("read") : ("write");

    this->sftp_handle =
            libssh2_sftp_open(this->sftp_session, remote_path.c_str(), flags, permission);
    if (!this->sftp_handle) {
        std::ostringstream oss;
        oss << remote_path
                << ": Unable to open file for " + msg + " with SFTP. ";
        oss << strerror(libssh2_sftp_last_error(this->sftp_session));
        throw std::runtime_error(oss.str());
    }
    return 0;
}

int PSftp::RemoteCloseFile() {
    int ret = libssh2_sftp_close(this->sftp_handle);
    if (ret == 0) {
        this->sftp_handle = 0;
    }
    return ret;
}

/*
 * mode 参数没有使用
 */
int PSftp::RemoteFileSize(std::string remote_file, int64_t *size,
        int mode/* = -1*/) {
    LIBSSH2_SFTP_ATTRIBUTES attr;
    int ret = libssh2_sftp_stat(this->sftp_session, remote_file.c_str(), &attr);
    if (ret < 0) {
        throw std::runtime_error(
                "RemoteFileSize error: File \"" + remote_file
                        + "\" does not exist or cannot be read.");
    }
    *size = (int64_t) attr.filesize;
    return ret;
}

int PSftp::RemoteLs(std::string local_file, std::string remote_path) {
    std::ofstream ofs(local_file.c_str());
    int ret = 0;
    if ((ret = this->remoteOpenDir(remote_path))) {
        std::ostringstream oss;
        oss << remote_path << ": Unable to open dir with SFTP. "
                << strerror(ret);
        throw std::runtime_error(oss.str());
    }

    char mem[512];
    char longentry[512];
    LIBSSH2_SFTP_ATTRIBUTES attrs;

    while ((ret = libssh2_sftp_readdir_ex(this->sftp_handle, mem, sizeof(mem),
            longentry, sizeof(longentry), &attrs)) > 0) {
        ofs << longentry << std::endl;
    }

    if ((ret = this->remoteCloseDir())) {
        std::ostringstream oss;
        oss << remote_path << ": Unable to close dir with SFTP. "
                << strerror(ret);
        throw std::runtime_error(oss.str());
    }
    ofs.close();
    return 0;
}

std::vector<FileAttribute> PSftp::RemoteDir(std::string remote_path,
        std::string option) {
    bool b_show_all = false;
    bool b_sort_by_time = false;
    bool b_reverse_sort = false;
    for (size_t i = 0; i < option.length(); i++) {
        switch (option[i]) {
        case 'a':
            b_show_all = true;
            break;
        case 't':
            b_sort_by_time = true;
            break;
        case 'r':
            b_reverse_sort = true;
            break;
        default:
            throw std::runtime_error(
                    std::string("Invalid Option '") + option[i] + "'.");
            break;
        }
    }

    std::vector<FileAttribute> v_files;
    int ret = 0;
    if ((ret = this->remoteOpenDir(remote_path))) {
        std::ostringstream oss;
        oss << remote_path << ": Unable to open dir with SFTP. "
                << strerror(ret);
        throw std::runtime_error(oss.str());
    }

    char mem[512];
    char longentry[512];
    LIBSSH2_SFTP_ATTRIBUTES attrs;
    FileAttribute tmp;

    while ((ret = libssh2_sftp_readdir_ex(this->sftp_handle, mem, sizeof(mem),
            longentry, sizeof(longentry), &attrs)) > 0) {
        if ((!b_show_all) && (mem[0] == '.')) {
            continue;
        }
        tmp.filename = mem;
        tmp.permissions = attrs.permissions;
        tmp.filesize = attrs.filesize;
        tmp.uid = attrs.uid;
        tmp.gid = attrs.gid;
        tmp.atime = attrs.atime;
        tmp.mtime = attrs.mtime;
        v_files.push_back(tmp);
    }

    if ((ret = this->remoteCloseDir())) {
        std::ostringstream oss;
        oss << remote_path << ": Unable to close dir with SFTP. "
                << strerror(ret);
        throw std::runtime_error(oss.str());
    }

    if (b_sort_by_time) {
        sort(v_files.rbegin(), v_files.rend());
    }

    if (b_reverse_sort) {
        reverse(v_files.begin(), v_files.end());
    }

    return v_files;
}

int PSftp::RemotePwd(std::string &outPut) {
    outPut = this->current_remote_path;
    return 0;
}

int PSftp::RemoteRead(char *sBuf, int maxSize) {
    if (this->sftp_handle == 0) {
        throw std::runtime_error("RemoteRead error: sftp_handle is null.");
    }

    char *tmp = new char[this->max_read_size];
    if (tmp == 0) {
        throw std::runtime_error("RemoteRead: Allocation failed.");
    }
    memset(tmp, 0, this->max_read_size);
    char *p_buf = sBuf;

    int n = 0;
    int read_num = 0;
    do {
        n = libssh2_sftp_read(this->sftp_handle, tmp,
                (this->max_read_size < maxSize) ?
                        this->max_read_size : maxSize);
        memcpy(p_buf, tmp, n);
        p_buf += n;
        read_num += n;
        maxSize -= n;
    } while ((maxSize > 0) && (n > 0));
    delete[] tmp;
    return read_num;
}

int PSftp::RemoteWrite(char *sBuf, int len) {
    if (this->sftp_handle == 0) {
        throw std::runtime_error("RemoteWrite error: sftp_handle is null.");
    }

    char *p_buf = sBuf;
    int n_write_num = 0;
    int n = 0;
    do {
        n = libssh2_sftp_write(this->sftp_handle, p_buf, len);
        if (n == -31) {
            std::ostringstream oss;
            oss << "RemoteWrite error: remote filesystem may be full. " << n
                    << ".";
            throw std::runtime_error(oss.str());
        } else if (n < 0) {
            std::ostringstream oss;
            oss << "RemoteWrite error: unknow error " << n << ".";
            throw std::runtime_error(oss.str());
        }
        p_buf += n;
        n_write_num += n;
        len -= n;
    } while ((n > 0) && (len > 0));

    return n_write_num;
}

int PSftp::RemoteDelete(std::string remoteFile) {
    int ret = 0;
    if ((ret = libssh2_sftp_unlink(this->sftp_session, remoteFile.c_str()))) {
        std::ostringstream oss;
        oss << "RemoteDelete(" << remoteFile << ") error: "
                << libssh2_sftp_last_error(this->sftp_session);
        throw std::runtime_error(oss.str());
    }
    return ret;
}

int PSftp::RemoteRename(std::string remote_old_path,
        std::string remote_new_path) {
    int ret = 0;
    if (this->isRenameOverwrite) {
        try {
            this->RemoteDelete(remote_new_path);
        } catch (...) {
        }
    }
    // libssh2_sftp_rename不能做覆盖操作，需观察以后的libssh2版本是否修正该问题
    if ((ret =
            libssh2_sftp_rename(this->sftp_session, remote_old_path.c_str(), remote_new_path.c_str()))) {
        std::ostringstream oss;
        oss << "RemoteRename(" << remote_old_path << ", " << remote_new_path
                << ") error: ";
        oss << libssh2_sftp_last_error(this->sftp_session);
        throw std::runtime_error(oss.str());
    }
    return ret;
}

/*
 * mode 参数没有使用
 */
int PSftp::RemoteMkDir(std::string remotePath, int mode/* = -1*/) {
    int ret = 0;
    if ((ret = libssh2_sftp_mkdir(this->sftp_session, remotePath.c_str(), 0775))) {
        std::ostringstream oss;
        oss << "RemoteMkDir(" << remotePath << ") error: "
                << libssh2_sftp_last_error(this->sftp_session);
        throw std::runtime_error(oss.str());
    }
    return ret;
}

int PSftp::RemoteRmDir(std::string remotePath) {
    int ret = 0;
    if ((ret = libssh2_sftp_rmdir(this->sftp_session, remotePath.c_str()))) {
        std::ostringstream oss;
        oss << "RemoteRmDir(" << remotePath << ") error: "
                << libssh2_sftp_last_error(this->sftp_session);
        throw std::runtime_error(oss.str());
    }
    return ret;
}

int PSftp::SetOptions(int opt, long val) {
    switch (opt) {
    case SFTP_COMPRESS:
        // 设置压缩选项
        if (!this->ssh_session) {
            throw std::runtime_error("SetOptions error: ssh session is null.");
        }

        if (libssh2_session_flag(this->ssh_session, LIBSSH2_FLAG_COMPRESS,
                val)) {
            throw std::runtime_error(
                    "SetOptions error:(libssh2_session_flag) "
                            + this->getLastSSHErrorMsg());
        }
        break;
    case SFTP_MAX_READ_SIZE:
        // 设置最大读取字节数
        this->max_read_size = int(val);
        break;
    case SFTP_RENAME_OVERWRITE:
        if (val == 1) {
            this->isRenameOverwrite = true;
        } else if (val == 0) {
            this->isRenameOverwrite = false;
        } else {
            std::ostringstream oss;
            oss << "Invalid value of option SFTP_RENAME_OVERWRITE: " << val;
            throw std::runtime_error(oss.str());
        }
        break;
    default:
        // 其它选项不处理
        break;
    }
    return 0;
}

int PSftp::remoteOpenDir(std::string remote_path) {
    this->sftp_handle =
            libssh2_sftp_opendir(this->sftp_session, remote_path.c_str());
    if (!this->sftp_handle) {
        return libssh2_sftp_last_error(this->sftp_session);
    }
    return 0;
}

int PSftp::remoteCloseDir() {
    int ret = 0;
    if (this->sftp_handle) {
        if (libssh2_sftp_closedir(this->sftp_handle)) {
            ret = libssh2_sftp_last_error(this->sftp_session);
        }
        this->sftp_handle = 0;
    }
    return ret;
}

std::string PSftp::getLastSSHErrorMsg() {
    std::string errMsg;
    if (this->ssh_session) {
        int errno;
        char msg[1024] = { 0 };
        char *pmsg = msg;
        int len = 0;
        errno = libssh2_session_last_error(this->ssh_session, &pmsg, &len, 0);
        std::ostringstream oss;
        oss << errno << " " << msg;
        errMsg = oss.str();
    }
    return errMsg;
}
