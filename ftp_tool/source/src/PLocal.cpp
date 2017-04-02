/*
 * PLocal.cpp
 *
 *  Created on: 2013年11月15日
 *      Author: liangjianqiang
 */

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include "PLocal.h"

PLocal::PLocal() {
    fd = -1;
}

PLocal::~PLocal() {
}

int PLocal::RemoteChDir(std::string remote_path) {
    workDir = remote_path;
    return 0;
}

int PLocal::RemoteDelete(std::string remoteFile) {
    if (::unlink(remoteFile.c_str()) != 0) {
        throw std::runtime_error(strerror(errno));
    }
    return 0;
}

int PLocal::RemoteOpenFile(std::string remote_path, int type, int mode/* = -1*/) {
    if (type != FILE_READ && type != FILE_WRITE) {
        throw std::runtime_error(
                "Invalid open type. It must be FILE_READ or FILE_WRITE.");
    }
    int flags = (type == FILE_READ) ? (O_RDONLY) : (O_WRONLY|O_CREAT|O_TRUNC);
    mode_t permission = (type == FILE_READ) ? (0) : (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
    fd = ::open(remote_path.c_str(), flags, permission);
    if (fd < 0) {
        throw std::runtime_error(strerror(errno));
    }
    return 0;
}

int PLocal::RemoteCloseFile() {
    if (::close(fd) != 0) {
        throw std::runtime_error(strerror(errno));
    }
    return 0;
}

int PLocal::RemoteFileSize(std::string remote_file, long *size, int mode/* = -1*/) {
    struct stat fileStat;
    if (lstat(remote_file.c_str(), &fileStat) != 0) {
        throw std::runtime_error(strerror(errno));
    }
    *size = fileStat.st_size;
    return 0;
}

int PLocal::RemoteLs(std::string local_file, std::string remote_path) {
    DIR *dir;
    if ((dir = ::opendir(remote_path.c_str())) == 0) {
        throw std::runtime_error(strerror(errno));
    }
    std::ofstream ofs(local_file.c_str());
    if (!ofs) {
        throw std::runtime_error(local_file + ": Open file error");
    }

    struct dirent *entry;
    while ((entry = ::readdir(dir)) != 0) {
//        if (entry->d_type == DT_REG) { // AIX 不支持DT_REG
            ofs << entry->d_name << std::endl;
//        }
    }

    if (::closedir(dir) != 0) {
        throw std::runtime_error(strerror(errno));
    }
    ofs.close();
    return 0;
}

std::vector<FileAttribute> PLocal::RemoteDir(std::string remote_path,
        std::string option) {
    std::vector<FileAttribute> fileList;
    return fileList;
}

int PLocal::RemoteMkDir(std::string remotePath, int mode/* = -1*/) {
    if (::mkdir(remotePath.c_str(), 0775) != 0) {
        throw std::runtime_error(strerror(errno));
    }
    return 0;
}

int PLocal::RemoteRmDir(std::string remotePath) {
    if (::rmdir(remotePath.c_str()) != 0) {
        throw std::runtime_error(strerror(errno));
    }
    return 0;
}

int PLocal::RemotePwd(std::string &outPut) {
    outPut = workDir;
    return 0;
}

int PLocal::RemoteRead(char *sBuf, int maxSize) {
    if (this->fd == 0) {
        throw std::runtime_error("RemoteRead error: fd is null.");
    }

    char tmp[1024];
    char *p_buf = sBuf;
    int n = 0;
    int read_num = 0;
    do {
        n = ::read(fd, tmp, (1024 < maxSize) ? 1024 : maxSize);
        memcpy(p_buf, tmp, n);
        p_buf += n;
        read_num += n;
        maxSize -= n;
    } while ((maxSize > 0) && (n > 0));
    return read_num;
}

int PLocal::RemoteWrite(char *sBuf, int len) {
    if (this->fd == 0) {
        throw std::runtime_error("RemoteWrite error: fd is null.");
    }

    char *p_buf = sBuf;
    int n_write_num = 0;
    int n = 0;
    do {
        n = ::write(fd, p_buf, len);
        if (n < 0) {
            throw std::runtime_error(strerror(errno));
        }
        p_buf += n;
        n_write_num += n;
        len -= n;
    } while ((n > 0) && (len > 0));

    return n_write_num;
}

int PLocal::RemoteRename(std::string remote_old_path, std::string remote_new_path) {
    if (::rename(remote_old_path.c_str(), remote_new_path.c_str()) != 0) {
        throw std::runtime_error(strerror(errno));
    }
    return 0;
}
