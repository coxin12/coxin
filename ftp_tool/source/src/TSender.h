///////////////////////////////////////////////////////////
//  TSender.h
//  Implementation of the Class TSender
//  Created on:      05-Ê®¶þÔÂ-2012 11:19:11
//  Original author: chenzhixin
//  VERSION     MODTIME       AUTHOR      MEMO
//  V12.00.001  2012-12-05  chenzhixin  new create
///////////////////////////////////////////////////////////

#ifndef _TSENDER_H_

#define _TSENDER_H_

#include "Transmitter.h"

class TSender: public Transmitter {

public:
    TSender();
    virtual ~TSender();

    virtual int Connect();
    virtual int Transfer();
    virtual int ListFile();

protected:
    virtual void initTarget(int targetNum);
    virtual void disconnectTarget(int targetNum);
    int64_t RSize(std::string path);

};
#endif // end of #ifndef _TSENDER_H_
