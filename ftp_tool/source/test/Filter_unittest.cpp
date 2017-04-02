#include "gtest/gtest.h"
#include <iostream>
#include "Filter.h"
#include "MatchFilter.h"
#include "RegexFilter.h"
using namespace std;

TEST(MatchFilter, match) {
    Filter *filter = new MatchFilter;
    filter->setPattern("*");
    EXPECT_TRUE(filter->match("abc"));

    filter->setPattern("*.d");
    EXPECT_FALSE(filter->match("abc"));

    filter->setPattern("*.h|*.cpp");
    EXPECT_TRUE(filter->match("main.cpp"));
    EXPECT_TRUE(filter->match("stdio.h"));
    EXPECT_FALSE(filter->match("main.cc"));

    filter->setPattern("*.h|*.cpp|");
    EXPECT_TRUE(filter->match("main.cpp"));
    EXPECT_TRUE(filter->match("stdio.h"));
    EXPECT_FALSE(filter->match("main.cc"));
}

TEST(RegexFilter, match) {
    Filter *filter = new RegexFilter;
    filter->setPattern("*");
    EXPECT_FALSE(filter->match("a"));

    filter->setPattern("a*");
    EXPECT_TRUE(filter->match("a"));
    EXPECT_TRUE(filter->match("ab"));
    EXPECT_TRUE(filter->match("a+"));

    filter->setPattern("go+g[lo]?e");
    EXPECT_TRUE(filter->match("google"));
    EXPECT_TRUE(filter->match("gooogle"));
    EXPECT_TRUE(filter->match("gooogoe"));
    EXPECT_FALSE(filter->match("ggoe"));
}
