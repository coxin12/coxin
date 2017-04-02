/*
 * transfertask.h
 *
 *  Created on: 2013��11��5��
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
        std::string sHost;    //��������ַ
        std::string sUser;     //�û���
        std::string sPasswd;   //�û�����
        std::string sPath;    //����������Ŀ¼
        int iProtocol;       //���ݴ���Э��(SFTP��FTP)
        int iLinkMode;          //����ģʽ��SFTP:0������,1����; FTP:PASSIVE ����ģʽ,PORT ����ģʽ)
    };
public:
    TransferTask();
    virtual ~TransferTask();

    void setValue(std::string key, std::string value);
public:
    HostNode srcNode;   //Դ�������ڵ���Ϣ
    std::vector<HostNode> dstNode;   //Ŀ��������ڵ���Ϣ
    std::string taskID;    //�������Ʊ�ʶ
    std::string tmpPath;  //Ŀ���������ʱĿ¼
    std::string pattern;  //�����ļ������ʽ
    std::string taskLogFile;  //�ļ�������־
    std::string bak1Path; //����Ŀ¼1
    std::string bak2Path; //����Ŀ¼2
    std::string bak3Path; //����Ŀ¼3
    std::string bak4Path; //����Ŀ¼4
    std::string isRename;  //�Ƿ�mvĿ���ļ�
    int regexType;     //������ʽ���ͣ�Ĭ��ΪMatch��
    int transMode;           //�������紫��ģʽ(BIN,ASCII)
    int maxFiles;       //һ�δ�������ļ���
    int interval;       //������ʱ��
    int resendInterval; //�ش���ʱ����
    int transLogMaxSize;       //��������־�ֲ��ļ���С
    int transLogMaxIndex;       //��������־�ֲ��ļ�����
    int transBlocks;     // sftp�������ݿ�������һ���СΪ30000�ֽ�

    time_t lastExecuteTime;       //���������ʱ��
};

#endif /* TRANSFERTASK_H_ */
