/*
 * RegexFilter.h
 *
 *  Created on: 2013��11��19��
 *      Author: liangjianqiang
 */

#ifndef REGEXFILTER_H_
#define REGEXFILTER_H_

#include "Filter.h"
#include "regex_match.h"

class RegexFilter : public Filter {
public:
    RegexFilter();
    virtual ~RegexFilter();
    virtual void setPattern(std::string pattern);
    virtual bool match(std::string str);
private:
    bool isValidPattern;
    regex_match_t *pRegex;
};

#endif /* REGEXFILTER_H_ */
