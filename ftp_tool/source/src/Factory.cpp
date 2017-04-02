/*
 * Factory.cpp
 *
 *  Created on: 2013Äê11ÔÂ8ÈÕ
 *      Author: liangjianqiang
 */

#include "dstruct.h"
#include "Factory.h"
#include "TMover.h"
#include "TGeter.h"
#include "TSender.h"
#include "TBridger.h"
#include "TMultiSender.h"
#include "TMultiBridger.h"
#include "MatchFilter.h"
#include "RegexFilter.h"

Transmitter *TransmitterKit::createTransmitter(TransferTask task) {
    if (strcasecmp(task.srcNode.sHost.c_str(), "local") == 0) {
        if (task.dstNode.size() == 1) {
            if (strcasecmp(task.dstNode[0].sHost.c_str(), "local") == 0) {
                return new TMover;
            } else {
                return new TSender;
            }
        } else if (task.dstNode.size() > 1) {
            return new TMultiSender;
        } else {
            throw std::runtime_error("Unkonwn transfer type");
        }
    } else {
        if (task.dstNode.size() == 1) {
            if (strcasecmp(task.dstNode[0].sHost.c_str(), "local") == 0) {
                return new TGeter;
            } else {
                return new TBridger;
            }
        } else if (task.dstNode.size() > 1) {
            return new TMultiBridger;
        } else {
            throw std::runtime_error("Unkonwn transfer type");
        }
    }
}

Filter *FilterKit::createFilter(int filterType) {
    if (filterType == Match) {
        return new MatchFilter;
    } else if (filterType == EREs) {
        return new RegexFilter;
    } else {
        throw std::runtime_error("Unkonwn filter type");
    }
}
