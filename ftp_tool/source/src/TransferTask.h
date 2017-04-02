/*
 * transfertask.h
 *
 *  Created on: 2013年11月5日
 *      Author: liangjianqiang
 */

#ifndef TRANSFERTASK_H_
#define TRANSFERTASK_H_

#include <time.h>
#include <string>
#include <vector>
#include <stdexcept>
#include "dstruct.h"

class TransferTask {
public:
    class HostNode {
    public:
        HostNode():
            iProtocol(SFTP), iLinkMode(PORT) {
        }
        std::string sHost;    //服务器地址
        std::string sUser;     //用户名
        std::string sPasswd;   //用户密码
        std::string sPath;    //服务器数据目录
        int iProtocol;       //数据传输协议(SFTP，FTP)
        int iLinkMode;          //链接模式（SFTP:0非阻塞,1阻塞; FTP:PASSIVE 被动模式,PORT 主动模式)
    };
public:
    TransferTask();
    virtual ~TransferTask();

    void setValue(std::string key, std::string value);
public:
    HostNode srcNode;   //源服务器节点信息
    std::vector<HostNode> dstNode;   //目标服务器节点信息
    std::string taskID;    //任务名称标识
    std::string tmpPath;  //目标服务器临时目录
    std::string pattern;  //数据文件名表达式
    std::string taskLogFile;  //文件传输日志
    std::string bak1Path; //备份目录1
    std::string bak2Path; //备份目录2
    std::string bak3Path; //备份目录3
    std::string bak4Path; //备份目录4
    std::string isRename;  //是否mv目标文件
    int regexType;     //正则表达式类型（默认为Match）
    int transMode;           //数据网络传输模式(BIN,ASCII)
    int maxFiles;       //一次传输最大文件数
    int interval;       //任务间隔时间
    int resendInterval; //重传的时间间隔
    int transLogMaxSize;       //任务传输日志分拆文件大小
    int transLogMaxIndex;       //任务传输日志分拆文件个数
    int transBlocks;     // sftp传输数据块数，第一块大小为30000字节

    time_t lastExecuteTime;       //任务最后处理时间
};

#endif /* TRANSFERTASK_H_ */
