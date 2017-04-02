#include "gtest/gtest.h"
#include "GlobalSetting.h"
#include <iostream>
#include <stdio.h>
using namespace std;

TEST(GlobalSetting, getInstance) {
    GlobalSetting *global1 = GlobalSetting::getInstance();
    EXPECT_NE(global1, (GlobalSetting *)0);
    GlobalSetting *global2 = GlobalSetting::getInstance();
    EXPECT_TRUE(global1 == global2);
    GlobalSetting *global3 = GlobalSetting::getInstance();
    EXPECT_TRUE(global1 == global3);
    GlobalSetting *global4 = GlobalSetting::getInstance();
    EXPECT_TRUE(global1 == global4);
}

TEST(GlobalSetting, setAttributes) {
    GlobalSetting *tmp = GlobalSetting::getInstance();
    tmp->workPath           = "/abc";
    tmp->logPath            = "/bcd";
    tmp->statFile           = "stat.dat";
    tmp->localAddr          = "127.0.0.1";
    tmp->finishFile          = "finish.ini";
    tmp->passwdFile         = "/etc/passwd";
    tmp->rsaPubFile         = "~/.ssh/id_rsa.pub";
    tmp->rsaFile            = "~/.ssh/id_rsa";
    tmp->threadCount        = 2;
    tmp->interval           = 1000;
    tmp->statInterval       = 100;
    tmp->errLogMaxSize      = 2000;
    tmp->errLogMaxIndex     = 5;
}

TEST(GlobalSetting, getAttributes) {
    GlobalSetting *tmp = GlobalSetting::getInstance();
    EXPECT_STREQ(tmp->workPath       .c_str(),"/abc"                );
    EXPECT_STREQ(tmp->logPath        .c_str(),"/bcd"                );
    EXPECT_STREQ(tmp->statFile       .c_str(),"stat.dat"            );
    EXPECT_STREQ(tmp->localAddr      .c_str(), "127.0.0.1"          );
    EXPECT_STREQ(tmp->finishFile      .c_str(), "finish.ini"         );
    EXPECT_STREQ(tmp->passwdFile     .c_str(), "/etc/passwd"        );
    EXPECT_STREQ(tmp->rsaPubFile     .c_str(), "~/.ssh/id_rsa.pub"  );
    EXPECT_STREQ(tmp->rsaFile        .c_str(), "~/.ssh/id_rsa"      );
    EXPECT_EQ(tmp->threadCount   , 2    );
    EXPECT_EQ(tmp->interval      , 1000 );
    EXPECT_EQ(tmp->statInterval  , 100  );
    EXPECT_EQ(tmp->errLogMaxSize , 2000 );
    EXPECT_EQ(tmp->errLogMaxIndex, 5    );
}

TEST(GlobalSetting, setValue) {
    GlobalSetting *tmp = GlobalSetting::getInstance();
    tmp->setValue("wkpath",           "\"/abcd\""               );
    tmp->setValue("logpath",          "\"/bcde\""               );
    tmp->setValue("statefile",        "\"stat.data\""           );
    tmp->setValue("localaddr",        "\"127.0.0.12\""          );
    tmp->setValue("finshfile",        "\"finish.in\""         );
    tmp->setValue("ftppwdfile",       "\"/etc/password\""        );
    tmp->setValue("rsapubfile",       "\"~/.ssh/id_rsa.pub\""  );
    tmp->setValue("rsafile",          "\"~/.ssh/id_rsa\""      );
    tmp->setValue("threads",          "\"3\""                  );
    tmp->setValue("interval",         "\"2000\""               );
    tmp->setValue("stateinterval",    "\"200\""                );
    tmp->setValue("errlogMaxsize",    "\"4000\""               );
    tmp->setValue("errlogMaxindex",   "\"6\""                  );
    EXPECT_STREQ(tmp->workPath       .c_str(), "/abcd"               );
    EXPECT_STREQ(tmp->logPath        .c_str(), "/bcde"               );
    EXPECT_STREQ(tmp->statFile       .c_str(), "stat.data"           );
    EXPECT_STREQ(tmp->localAddr      .c_str(), "127.0.0.12"          );
    EXPECT_STREQ(tmp->finishFile      .c_str(), "finish.in"         );
    EXPECT_STREQ(tmp->passwdFile     .c_str(), "/etc/password"        );
    EXPECT_STREQ(tmp->rsaPubFile     .c_str(), "~/.ssh/id_rsa.pub"  );
    EXPECT_STREQ(tmp->rsaFile        .c_str(), "~/.ssh/id_rsa"      );
    EXPECT_EQ(tmp->threadCount   , 3    );
    EXPECT_EQ(tmp->interval      , 2000 );
    EXPECT_EQ(tmp->statInterval  , 200  );
    EXPECT_EQ(tmp->errLogMaxSize , 4000 );
    EXPECT_EQ(tmp->errLogMaxIndex, 6    );
}
