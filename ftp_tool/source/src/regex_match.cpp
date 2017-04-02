///////////////////////////////////////////////////////////
//  regex_match.cpp
//  Implementation of the Class regex_match_t
//  Import author: chenzhixin
//  VERSION     MODTIME       AUTHOR      MEMO
//  V13.00.001  2013-03-28  chenzhixin	针对处理egrep类型正则表达式，引入该模块。
///////////////////////////////////////////////////////////

#include <stdexcept>
#include <string>
#include <vector>
#include <iostream>
#include <regex.h>
#include <stdlib.h>

#include "regex_match.h"

regex_match_t::regex_match_t() :
        _rm(128), _is_open(false), _submatch_cnt(0) {
}

regex_match_t::~regex_match_t() {
    release();
}

void regex_match_t::release() {
    if (_is_open)
        regfree(&_re);

    _is_open = false;
}

bool regex_match_t::compile_pattern(const char *pattern) {
    if (_is_open)
        release();

    int ret = regcomp(&_re, pattern, REG_EXTENDED);
    if (0 != ret) {
        char errbuf[128];
        regerror(ret, &_re, errbuf, sizeof(errbuf));
        _errstr = errbuf;
        return false;
    }

    _is_open = true;
    return true;
}

const std::string& regex_match_t::get_last_errstr() const {
    return _errstr;
}

bool regex_match_t::execute(const char *str) {
    _str = str;
    int ret = regexec(&_re, str, _rm.size(), &_rm[0], 0);
    if (ret != 0) {
        _submatch_cnt = 0;
        return false;
    }

    size_t i = 1;
    for (i = 1; i < _rm.size(); i++) {
        if (_rm[i].rm_so >= _rm[i].rm_eo)
            break;
    }

    _submatch_cnt = i - 1;
    return true;
}

size_t regex_match_t::get_submatch_cnt() const {
    return _submatch_cnt;
}

std::string regex_match_t::get_submatch_str(size_t i) const {
    if (i > _submatch_cnt)
        return std::string("");

    return _str.substr(_rm[i].rm_so, _rm[i].rm_eo - _rm[i].rm_so);
}

std::string regex_match_t::get_format_str(const std::string& str,
        const std::string& sformat) const {
    std::string ret(str);

    if (!(sformat.empty()) && (ret.size() < sformat.size())) {
        ret = sformat.substr(0, (sformat.size() - ret.size())) + ret;
    }
    return ret;
}

std::string regex_match_t::eval(const std::string& expr) const {
    std::string ret;

    size_t startp = 0;
    while (startp < expr.length()) {
        size_t endp = expr.find('+', startp);
        if (endp == std::string::npos)
            endp = expr.length();
        std::string subexpr(expr, startp, endp - startp);

        startp = endp + 1;
        if (subexpr.empty()) {
            ret += "+";
            continue;
        }
        if (subexpr[0] == '$') {
            std::string::size_type pos = 0, b_pos = 1;
            std::string caps("123456789");
            pos = subexpr.find_first_of(caps); //获取前补字符位置
            std::string sfillpost("");       //前补字符串
            if (pos != std::string::npos && pos > b_pos) {
                sfillpost = subexpr.substr(b_pos, pos - b_pos) + subexpr[1];
            }
            size_t i = atoi(&subexpr[pos]);
            ret += get_format_str(get_submatch_str(i), sfillpost);
        } else
            ret += subexpr;
    }

    return ret;
}

