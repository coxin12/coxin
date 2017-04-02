/*
 * Factory.h
 *
 *  Created on: 2013Äê11ÔÂ8ÈÕ
 *      Author: liangjianqiang
 */

#ifndef TRANSFERFACTORY_H_
#define TRANSFERFACTORY_H_

#include "Transmitter.h"
#include "TransferTask.h"
#include "Filter.h"
#include <stdexcept>

class TransmitterKit {
public:
    static Transmitter *createTransmitter(TransferTask task);
};

class FilterKit {
public:
    static Filter *createFilter(int regexType);
};

#endif /* TRANSFERFACTORY_H_ */
