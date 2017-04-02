///////////////////////////////////////////////////////////
//  TGeter.h
//  Implementation of the Class TGeter
//  Created on:      05-Ê®¶þÔÂ-2012 11:19:11
//  Original author: chenzhixin
//  VERSION     MODTIME       AUTHOR      MEMO
//  V12.00.001  2012-12-05  chenzhixin  new create
///////////////////////////////////////////////////////////

#ifndef _TGETER_H_

#define _TGETER_H_

#include "Transmitter.h"

class TGeter: public Transmitter {

public:
    TGeter();
    virtual ~TGeter();

    int Connect();
    int Init(const TransferTask &task);
    int ListFile();
    int Transfer();
protected:
    virtual void initSource();
    virtual void disconnectSource();
private:
    std::string m_srcpath;
    int ChkBakPathSpace();
    int ReadFileList(const char* ListName, const char* pattern, int MaxFiles);
    int FileLink(const std::string &srcFile, const std::string &fileName);
};
#endif // end of #ifndef _TGETER_H_
