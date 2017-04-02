/*
 * MatchFilter.cpp
 *
 *  Created on: 2013Äê11ÔÂ19ÈÕ
 *      Author: liangjianqiang
 */

#include <fnmatch.h>
#include <stdio.h>
#include <string.h>
#include "MatchFilter.h"

MatchFilter::MatchFilter() {
}

MatchFilter::~MatchFilter() {
}

bool MatchFilter::match(std::string str) {
    const char *p = pattern.c_str();
    char part[128];
    while (1) {
        sscanf(p, "%[^|]", part);

        if (fnmatch(part, str.c_str(), 0) == 0) {
            return true;
            break;
        }

        p = strchr(p, '|');
        if (p == 0) {
            break;
        } else {
            ++p;
        }
    }
    return false;
}
