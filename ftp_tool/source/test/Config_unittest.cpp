#include "gtest/gtest.h"
#include "Config.h"
#include <iostream>
#include <GlobalSetting.h>

TEST(Config, ReadFile) {
    const char *file = "./etc/FileTrans.ini";
    Config cfg;
    EXPECT_EQ(cfg.ReadFile(file), 4);
}

TEST(Config, GetTasks) {
    const char *file = "./etc/FileTrans.ini";
    Config cfg;
    cfg.ReadFile(file);
    std::vector<TransferTask> taskArray = cfg.GetTasks();
    for (int i = 0; i < taskArray.size(); i++) {
        TransferTask p = taskArray[i];
        std::cout << "TaskID              " << p.taskID << std::endl;

        std::cout << "tmpPath             " << p.tmpPath             << std::endl;
        std::cout << "pattern             " << p.pattern             << std::endl;
        std::cout << "regex_type          " << p.regexType           << std::endl;
        std::cout << "msgName             " << p.taskLogFile         << std::endl;
        std::cout << "bak1Path            " << p.bak1Path            << std::endl;
        std::cout << "bak2Path            " << p.bak2Path            << std::endl;
        std::cout << "bak3Path            " << p.bak3Path            << std::endl;
        std::cout << "bak4Path            " << p.bak4Path            << std::endl;
        std::cout << "isrename            " << p.isRename            << std::endl;
        std::cout << "mode                " << p.transMode           << std::endl;
        std::cout << "maxFiles            " << p.maxFiles            << std::endl;
        std::cout << "interval            " << p.interval            << std::endl;
        std::cout << "translogMaxsize     " << p.transLogMaxSize     << std::endl;
        std::cout << "translogMaxindex    " << p.transLogMaxIndex    << std::endl;
        std::cout << "transBlock          " << p.transBlocks         << std::endl;
        std::cout << "eTime               " << p.lastExecuteTime     << std::endl;

    }
}

TEST(Config, GetLogFile) {
    const char *file = "./etc/FileTrans.ini";
    Config cfg;
    cfg.ReadFile(file);
    EXPECT_STREQ(cfg.GetLogFile().c_str(), "/home/ljq/dssprog/log/FileTrans.log");
}
