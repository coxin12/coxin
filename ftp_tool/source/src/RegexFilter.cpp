/*
 * RegexFilter.cpp
 *
 *  Created on: 2013Äê11ÔÂ19ÈÕ
 *      Author: liangjianqiang
 */

#include "RegexFilter.h"

RegexFilter::RegexFilter() {
    isValidPattern = false;
    pRegex = new regex_match_t();
}

RegexFilter::~RegexFilter() {
    if (pRegex) {
        delete pRegex;
        pRegex = 0;
    }
}

void RegexFilter::setPattern(std::string pattern) {
    Filter::setPattern(pattern);
    if (pRegex->compile_pattern(pattern.c_str())) {
        isValidPattern = true;
    } else {
        isValidPattern = false;
    }
}

bool RegexFilter::match(std::string str) {
    if (isValidPattern == false) {
        return false;
    }
    return pRegex->execute(str.c_str());
}
