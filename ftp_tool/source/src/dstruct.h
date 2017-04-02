/******************************************************************
 *  dstruct.h
 *  data struct of program
 *  Created on:      05-ʮ����-2012 11:19:11
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
}; //FTP ������·����ģʽ(����������)

enum {
    ASCII, BIN
}; //���ݴ���ģʽ

enum {
    FTP, SFTP
}; //���ݴ���Э��

enum {
    MOVE, GET, SEND, BRIDGE, MULTI_SEND, MULTI_BRIDGE
}; //�ļ������������

enum {
    Match = 0, EREs
}; //ƥ����ʽ����
//Match:ģʽƥ�䣬��򵥵���ʽ '?' �� '*' ͨ���,shell��ls�����е�ģʽƥ����ʽ
//EREs:��չ��������ʽExtended Regular Expression ��egrep����ʹ�õ�������ʽ

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
