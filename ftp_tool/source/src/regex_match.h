///////////////////////////////////////////////////////////
//  regex_match.h
//  Implementation of the Class regex_match_t
//  Import author: chenzhixin
//  VERSION     MODTIME       AUTHOR      MEMO
//  V13.00.001  2013-03-28  chenzhixin	针对处理egrep类型正则表达式，引入该模块。
///////////////////////////////////////////////////////////

#ifndef _regex_match_h
#define _regex_match_h

#include <regex.h>
#include <vector>

// regex match wrapper class
class regex_match_t {
protected:
    regex_t _re;
    std::vector<regmatch_t> _rm;
    std::string _errstr;
    std::string _str;
    size_t _submatch_cnt;
    bool _is_open;
public:
    regex_match_t();
    ~regex_match_t();

    void release();

    /**
     * @brief compile a regular expression
     **/
    bool compile_pattern(const char * pattern);

    /**
     * @brief get last error message (regex related)
     */
    const std::string& get_last_errstr() const;

    /**
     * @brief Execute the regex
     */
    bool execute(const char * str);

    /**
     * @brief Get the number of sub pattern matched
     */
    size_t get_submatch_cnt() const;

    /**
     * @breif Get the substr matches the specified sub pattern
     */
    std::string get_submatch_str(size_t i) const;

    /**
     * @breif Get the format string
     */
    std::string get_format_str(const std::string& str,
            const std::string& sformat) const;
    /**
     * @brief Evaluates an expression string
     *  with $n substitutes with matched substring
     */
    std::string eval(const std::string& expr) const;
};

#endif
