/*
 * finishfile.h
 *
 *  Created on: 2013Äê11ÔÂ7ÈÕ
 *      Author: liangjianqiang
 */

#ifndef FINISHFILE_H_
#define FINISHFILE_H_

class FinishFile {
public:
    static FinishFile *getFinishFile();

    int initialize();
    bool getFinishFlag();
private:
    static FinishFile *finishFile;
    FinishFile();
    virtual ~FinishFile();
};

#endif /* FINISHFILE_H_ */
