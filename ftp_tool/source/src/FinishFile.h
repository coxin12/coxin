/*
 * finishfile.h
 *
 *  Created on: 2013��11��7��
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
