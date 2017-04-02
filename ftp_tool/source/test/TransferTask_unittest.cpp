#include "gtest/gtest.h"
#include "TransferTask.h"
#include <iostream>
#include <stdio.h>
using namespace std;

TEST(TransferTask, setValue) {
    TransferTask task;
    task.setValue("source", "\"etl\"@\"10.200.1.12\":\"sftp\":\"/app/etl/ljq2/test\"");
    task.setValue("target", "\"etl\"@\"10.200.1.13\":\"pasv\":\"/app/etl/ljq2\"");
    task.setValue("target", "\"ljq\"@\"10.200.1.13\":\"sftp\":\"/app/etl/ljq2\"");
    task.setValue("trans_type",         "\"bin\"");
    task.setValue("filter",             "\"*.cpp\"");
    task.setValue("regex_type",         "\"1\"");
    task.setValue("bak1path",           "\"bak1\"");
    task.setValue("bak2path",           "\"bak2\"");
    task.setValue("bak3path",           "\"bak3\"");
    task.setValue("bak4path",           "\"bak4\"");
    task.setValue("msgname",            "\"mylog\"");
    task.setValue("taskid",             "\"mytask\"");
    task.setValue("temp",               "\"/tmp\"");
    task.setValue("maxfiles",           "\"1024\"");
    task.setValue("interval",           "\"1000\"");
    task.setValue("isrename",           "\"true\"");
    task.setValue("translogMaxsize",    "\"10\"");
    task.setValue("translogMaxindex",   "\"2\"");
    task.setValue("transBlock",         "\"10\"");

    EXPECT_STREQ(task.srcNode.sHost.c_str(), "10.200.1.12");
    EXPECT_STREQ(task.srcNode.sUser.c_str(), "etl");
    EXPECT_STREQ(task.srcNode.sPath.c_str(), "/app/etl/ljq2/test");
    EXPECT_EQ(task.srcNode.iProtocol, 1);
    EXPECT_EQ(task.srcNode.iLinkMode, 1);

    EXPECT_EQ(task.dstNode.size(), 2);
    // EXPECT_STREQ(task.dstNode.sHost.c_str(), "10.200.1.13");
    // EXPECT_STREQ(task.dstNode.sUser.c_str(), "etl");
    // EXPECT_STREQ(task.dstNode.sPath.c_str(), "/app/etl/ljq2");
    // EXPECT_EQ(task.dstNode.iProtocol, 0);
    // EXPECT_EQ(task.dstNode.iMode, 1);

    EXPECT_STREQ(task.taskID.c_str(),       "mytask");
    EXPECT_STREQ(task.tmpPath.c_str(),      "/tmp");
    EXPECT_STREQ(task.pattern.c_str(),      "*.cpp");
    EXPECT_STREQ(task.taskLogFile.c_str(),  "mylog");
    EXPECT_STREQ(task.bak1Path.c_str(),     "bak1");
    EXPECT_STREQ(task.bak2Path.c_str(),     "bak2");
    EXPECT_STREQ(task.bak3Path.c_str(),     "bak3");
    EXPECT_STREQ(task.bak4Path.c_str(),     "bak4");
    EXPECT_STREQ(task.isRename.c_str(),     "true");
    EXPECT_EQ(task.regexType, 1);
    EXPECT_EQ(task.transMode, 1);
    EXPECT_EQ(task.maxFiles, 1024);
    EXPECT_EQ(task.interval, 1000);
    EXPECT_EQ(task.transLogMaxSize, 10);
    EXPECT_EQ(task.transLogMaxIndex, 2);
    EXPECT_EQ(task.transBlocks, 30);
}
