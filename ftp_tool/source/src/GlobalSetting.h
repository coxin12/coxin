/*
 * globalsetting.h
 *
 *  Created on: 2013年11月5日
 *      Author: liangjianqiang
 */

#ifndef GLOBALSETTING_H_
#define GLOBALSETTING_H_

#include <stdio.h>
#include <string>

class GlobalSetting {
public:
    static const int DEFAULT_ERR_LOG_MAX_SIZE;
    static const int DEFAULT_ERR_LOG_MAX_INDEX;
    static const char *DEFAULT_STATIC_FILE;
    static const int DEFAULT_INTERVAL;
    static const int DEFAULT_RESEND_INTERVAL;
    static const int DEFAULT_STAT_INTERVAL;
    static const char *RESEND_DIR_NAME;

    static GlobalSetting *getInstance();

    void setValue(std::string key, std::string value);
private:
    static GlobalSetting *globalSetting;
    GlobalSetting();
    virtual ~GlobalSetting();

public:
    std::string workPath;   //程序所在机器的工作目录
    std::string logPath; //跟踪日志目录，日志名以配置文件名为基础，（如：filetrans.ini 则logfile为filetrans.log)
    std::string statFile;   //任务执行统计日志
    int threadCount;        //总线程数
    int interval;           //任务默认处理间隔时间
    int resendInterval;     //重传的时间间隔
    int statInterval;       //任务执行统计日志增加间隔时间
    std::string localAddr;  //本地IP地址
    std::string finishFile;  //程序停止标志文件
    std::string passwdFile; //加密密码存放文件
    std::string rsaPubFile; //公钥文件
    std::string rsaFile;    //私钥文件
    int errLogMaxSize;      //错误日志分拆文件大小
    int errLogMaxIndex;     //错误日志分拆文件个数
};

#endif /* GLOBALSETTING_H_ */
