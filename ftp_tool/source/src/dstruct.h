/******************************************************************
 *  dstruct.h
 *  data struct of program
 *  Created on:      05-十二月-2012 11:19:11
 *  Original author: chenzhixin
 *
 * Modified History:
 *     See ChangeLog.
 ******************************************************************/

#ifndef _DSTRUCT_H_
#define _DSTRUCT_H_
#include <string>
#include <set>

enum {
    PORT, PASSIVE
}; //FTP 数据链路建立模式(主动、被动)

enum {
    ASCII, BIN
}; //数据传输模式

enum {
    FTP, SFTP
}; //数据传输协议

enum {
    MOVE, GET, SEND, BRIDGE, MULTI_SEND, MULTI_BRIDGE
}; //文件传输操作类型

enum {
    Match = 0, EREs
}; //匹配表达式类型
//Match:模式匹配，最简单的形式 '?' 和 '*' 通配符,shell中ls命令行的模式匹配表达式
//EREs:扩展的正则表达式Extended Regular Expression ，egrep命令使用的正则表达式

struct FileAttribute {
    std::string filename;
    unsigned long permissions;
    unsigned long filesize;
    unsigned long uid;
    unsigned long gid;
    unsigned long atime;
    unsigned long mtime;

    bool operator <(const FileAttribute &rhs) const {
        return this->mtime < rhs.mtime;
    }
};

typedef std::set<FileAttribute, std::less<FileAttribute> > FileSet;

#define KByte         1024.00
#define MByte         1048576.00                /* 1024*1024 = 1048576 */

#endif
