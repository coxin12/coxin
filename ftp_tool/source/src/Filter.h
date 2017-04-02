/*
 * Filter.h
 *
 *  Created on: 2013Äê11ÔÂ19ÈÕ
 *      Author: liangjianqiang
 */

#ifndef FILTER_H_
#define FILTER_H_

#include <string>

class Filter {
public:
    virtual ~Filter() {
    }
    virtual void setPattern(std::string pattern) {
        this->pattern = pattern;
    }
    virtual bool match(std::string str) = 0;
protected:
    std::string pattern;
};

#endif /* FILTER_H_ */
