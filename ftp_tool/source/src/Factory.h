/*
 * Factory.h
 *
 *  Created on: 2013��11��8��
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
