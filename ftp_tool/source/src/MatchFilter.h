/*
 * MatchFilter.h
 *
 *  Created on: 2013��11��19��
 *      Author: liangjianqiang
 */

#ifndef MATCHFILTER_H_
#define MATCHFILTER_H_

#include "Filter.h"

class MatchFilter : public Filter {
public:
    MatchFilter();
    virtual ~MatchFilter();
    virtual bool match(std::string str);
};

#endif /* MATCHFILTER_H_ */
