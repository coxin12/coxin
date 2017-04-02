/*
 * globalsetting.h
 *
 *  Created on: 2013��11��5��
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
    std::string workPath;   //�������ڻ����Ĺ���Ŀ¼
    std::string logPath; //������־Ŀ¼����־���������ļ���Ϊ���������磺filetrans.ini ��logfileΪfiletrans.log)
    std::string statFile;   //����ִ��ͳ����־
    int threadCount;        //���߳���
    int interval;           //����Ĭ�ϴ�����ʱ��
    int resendInterval;     //�ش���ʱ����
    int statInterval;       //����ִ��ͳ����־���Ӽ��ʱ��
    std::string localAddr;  //����IP��ַ
    std::string finishFile;  //����ֹͣ��־�ļ�
    std::string passwdFile; //�����������ļ�
    std::string rsaPubFile; //��Կ�ļ�
    std::string rsaFile;    //˽Կ�ļ�
    int errLogMaxSize;      //������־�ֲ��ļ���С
    int errLogMaxIndex;     //������־�ֲ��ļ�����
};

#endif /* GLOBALSETTING_H_ */
