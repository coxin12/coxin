///////////////////////////////////////////////////////////
//  TMover.h
//  Implementation of the Class TMover
//  Created on:      05-ʮ����-2012 11:19:14
//  Original author: chenzhixin
//  VERSION     MODTIME       AUTHOR      MEMO
//  V12.00.001  2012-12-05  chenzhixin  new create
//  V13.00.003  2013-10-30  chenzhixin  �޸�Fnmatch��������ԭ��֧�ֶ���ƥ�䷽ʽ���޸�Ϊ�û�ָ����ƥ�䷽ʽ��
//                                       ���޸�һ�����ʽ����ƥ�䷽ʽ�������岻ͬ�Ķ������ʹ�������⡣
///////////////////////////////////////////////////////////

#ifndef _TMOVER_H_
#define _TMOVER_H_

#include "Transmitter.h"

class TMover : public Transmitter {
public:
    TMover();
    virtual ~TMover();

    virtual int Connect() {
        return 0;
    }
    virtual int Disconnect() {
        return 0;
    }
    virtual int ListFile();
    virtual int Transfer();
private:
    int Copy(const std::string &source, const std::string &destination);

};
#endif // end of #ifndef _TMOVER_H_
