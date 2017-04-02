/*
 * MatchFilter.h
 *
 *  Created on: 2013Äê11ÔÂ19ÈÕ
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
