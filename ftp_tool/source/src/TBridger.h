///////////////////////////////////////////////////////////
//  TBridger.h
//  Implementation of the Class TBridger
//  Created on:      05-Ê®¶þÔÂ-2012 11:19:11
//  Original author: chenzhixin
//  VERSION     MODTIME       AUTHOR      MEMO
//  V12.00.001  2012-12-05  chenzhixin  new create
///////////////////////////////////////////////////////////

#ifndef _TBRIDGER_H_

#define _TBRIDGER_H_

#include "Transmitter.h"

class TBridger: public Transmitter {
public:
    TBridger();
    virtual ~TBridger();

    int Connect();
    int ListFile();
    int Transfer();
protected:
    virtual void initSource();
    virtual void initTarget(int targetNum);
    virtual void disconnectSource();
    virtual void disconnectTarget(int targetNum);
private:
    int ConnectSource();
    int ConnectDestination();
    int ReadFileList(const char* ListName, const char* pattern, int MaxFiles);
    int FileBridge(const std::string &source, const std::string &destination,
            int mode);
    int64_t RSize(std::string path, char* filemsg);
    std::string m_srcpath;
};
#endif // end of #ifndef _TBRIDGER_H_
