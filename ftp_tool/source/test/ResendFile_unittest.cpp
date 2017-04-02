#include "gtest/gtest.h"
#include <iostream>
#include <fstream>
#include "GlobalSetting.h"
#include "TransferTask.h"
#include "ResendFile.h"
using namespace std;

TEST(ResendFile, construct) {
    GlobalSetting *global = GlobalSetting::getInstance();
    global->workPath = "./data";
    TransferTask task;
    task.taskID = "mytask";
    TransferTask::HostNode node1;
    node1.sHost = "10.200.1.12";
    node1.sPath = "/app/etl/test";
    TransferTask::HostNode node2;
    node2.sHost = "10.200.1.13";
    node2.sPath = "/app/etl/ljq2/test";
    TransferTask::HostNode node3;
    node3.sHost = "172.20.36.93";
    node3.sPath = "/home/ljq/ft";
    task.dstNode.push_back(node1);
    task.dstNode.push_back(node2);
    task.dstNode.push_back(node3);

    EXPECT_NO_THROW(ResendFile resendFile(task));
}

TEST(ResendFile, isExist) {
    GlobalSetting *global = GlobalSetting::getInstance();
    global->workPath = "./data";
    TransferTask task;
    task.taskID = "mytask";
    TransferTask::HostNode node1;
    node1.sHost = "10.200.1.12";
    node1.sPath = "/app/etl/test";
    TransferTask::HostNode node2;
    node2.sHost = "10.200.1.13";
    node2.sPath = "/app/etl/ljq2/test";
    TransferTask::HostNode node3;
    node3.sHost = "172.20.36.93";
    node3.sPath = "/home/ljq/ft";
    task.dstNode.push_back(node1);
    task.dstNode.push_back(node2);
    task.dstNode.push_back(node3);

    unlink("./data/mytask.resend");
    ResendFile resendFile(task);
    EXPECT_FALSE(resendFile.isExist());
    std::ofstream ofs("./data/mytask.resend");
    ofs << "LastExecuteTime=138000000" << std::endl;
    ofs << "10.200.1.12&&/app/etl/test&&/BIDATA/etl/TW_USR_MO_201311&&887" << std::endl;
    ofs.close();
    EXPECT_TRUE(resendFile.isExist());
}

TEST(ResendFile, loadFile) {
    GlobalSetting *global = GlobalSetting::getInstance();
    global->workPath = "./data";
    TransferTask task;
    task.taskID = "mytask";
    TransferTask::HostNode node1;
    node1.sHost = "10.200.1.12";
    node1.sPath = "/app/etl/test";
    TransferTask::HostNode node2;
    node2.sHost = "10.200.1.13";
    node2.sPath = "/app/etl/ljq2/test";
    TransferTask::HostNode node3;
    node3.sHost = "172.20.36.93";
    node3.sPath = "/home/ljq/ft";
    task.dstNode.push_back(node1);
    task.dstNode.push_back(node2);
    task.dstNode.push_back(node3);

    ResendFile resendFile(task);
    EXPECT_TRUE(resendFile.isExist());
    EXPECT_NO_THROW(resendFile.loadFile());
}

TEST(ResendFile, validate) {
    GlobalSetting *global = GlobalSetting::getInstance();
    global->workPath = "./data";
    TransferTask task;
    task.taskID = "mytask";
    TransferTask::HostNode node1;
    node1.sHost = "10.200.1.12";
    node1.sPath = "/app/etl/test";
    TransferTask::HostNode node2;
    node2.sHost = "10.200.1.13";
    node2.sPath = "/app/etl/ljq2/test";
    TransferTask::HostNode node3;
    node3.sHost = "172.20.36.93";
    node3.sPath = "/home/ljq/ft";
    task.dstNode.push_back(node1);
    task.dstNode.push_back(node2);
    task.dstNode.push_back(node3);

    ResendFile resendFile(task);
    EXPECT_TRUE(resendFile.isExist());
    EXPECT_NO_THROW(resendFile.loadFile());
    EXPECT_NO_THROW(resendFile.validate());
}

TEST(ResendFile, add) {
    GlobalSetting *global = GlobalSetting::getInstance();
    global->workPath = "./data";
    TransferTask task;
    task.taskID = "mytask";
    TransferTask::HostNode node1;
    node1.sHost = "10.200.1.12";
    node1.sPath = "/app/etl/test";
    TransferTask::HostNode node2;
    node2.sHost = "10.200.1.13";
    node2.sPath = "/app/etl/ljq2/test";
    TransferTask::HostNode node3;
    node3.sHost = "172.20.36.93";
    node3.sPath = "/home/ljq/ft";
    task.dstNode.push_back(node1);
    task.dstNode.push_back(node2);
    task.dstNode.push_back(node3);

    ResendFile resendFile(task);
    EXPECT_TRUE(resendFile.isExist());
    EXPECT_NO_THROW(resendFile.loadFile());
    EXPECT_NO_THROW(resendFile.validate());
    EXPECT_NO_THROW(resendFile.add(1, "/BIDATA/etl/TW_USR_MO_201311", 887));
}

TEST(ResendFile, close) {
    GlobalSetting *global = GlobalSetting::getInstance();
    global->workPath = "./data";
    TransferTask task;
    task.taskID = "mytask";
    TransferTask::HostNode node1;
    node1.sHost = "10.200.1.12";
    node1.sPath = "/app/etl/test";
    TransferTask::HostNode node2;
    node2.sHost = "10.200.1.13";
    node2.sPath = "/app/etl/ljq2/test";
    TransferTask::HostNode node3;
    node3.sHost = "172.20.36.93";
    node3.sPath = "/home/ljq/ft";
    task.dstNode.push_back(node1);
    task.dstNode.push_back(node2);
    task.dstNode.push_back(node3);

    std::ofstream ofs("./data/mytask.resend", std::ios_base::trunc);
    ofs << "LastExecuteTime=138000000" << std::endl;
    ofs << "10.200.1.12&&/app/etl/test&&/BIDATA/etl/TW_USR_MO_201311&&887" << std::endl;
    ofs.close();
    ResendFile resendFile(task);
    EXPECT_TRUE(resendFile.isExist());
    EXPECT_NO_THROW(resendFile.loadFile());
    EXPECT_NO_THROW(resendFile.validate());
    EXPECT_NO_THROW(resendFile.close());
}

TEST(ResendFile, close_when_empty) {
    GlobalSetting *global = GlobalSetting::getInstance();
    global->workPath = "./data";
    TransferTask task;
    task.taskID = "mytask";
    TransferTask::HostNode node1;
    node1.sHost = "10.200.1.12";
    node1.sPath = "/app/etl/test";
    TransferTask::HostNode node2;
    node2.sHost = "10.200.1.13";
    node2.sPath = "/app/etl/ljq2/test";
    TransferTask::HostNode node3;
    node3.sHost = "172.20.36.93";
    node3.sPath = "/home/ljq/ft";
    task.dstNode.push_back(node1);
    task.dstNode.push_back(node2);
    task.dstNode.push_back(node3);

    std::ofstream ofs("./data/mytask.resend", std::ios_base::trunc);
    ofs << "LastExecuteTime=138000000" << std::endl;
    ofs << "10.200.1.12&&/app/etl/test&&/BIDATA/etl/TW_USR_MO_201311&&887" << std::endl;
    ofs.close();
    ResendFile resendFile(task);
    EXPECT_TRUE(resendFile.isExist());
    EXPECT_NO_THROW(resendFile.loadFile());
    EXPECT_NO_THROW(resendFile.validate());
    ResendFile::Line line;
    EXPECT_EQ(0, resendFile.fetch(line));
    EXPECT_STREQ("/BIDATA/etl/TW_USR_MO_201311", line.sourceFile.c_str());
    EXPECT_EQ(887, line.sourceFileSize);
    EXPECT_NO_THROW(resendFile.setCurrentSuccess());
    EXPECT_NO_THROW(resendFile.close());
    EXPECT_FALSE(resendFile.isExist());
}

TEST(ResendFile, fetch) {
    GlobalSetting *global = GlobalSetting::getInstance();
    global->workPath = "./data";
    TransferTask task;
    task.taskID = "mytask";
    TransferTask::HostNode node1;
    node1.sHost = "10.200.1.12";
    node1.sPath = "/app/etl/test";
    TransferTask::HostNode node2;
    node2.sHost = "10.200.1.13";
    node2.sPath = "/app/etl/ljq2/test";
    TransferTask::HostNode node3;
    node3.sHost = "172.20.36.93";
    node3.sPath = "/home/ljq/ft";
    task.dstNode.push_back(node1);
    task.dstNode.push_back(node2);
    task.dstNode.push_back(node3);

    std::ofstream ofs("./data/mytask.resend");
    ofs << "LastExecuteTime=138000000" << std::endl;
    ofs << "10.200.1.12&&/app/etl/test&&/BIDATA/etl/TW_USR_MO_201311&&887" << std::endl;
    ofs.close();
    ResendFile resendFile(task);
    EXPECT_NO_THROW(resendFile.loadFile());
    EXPECT_NO_THROW(resendFile.validate());
    ResendFile::Line line;
    EXPECT_EQ(resendFile.fetch(line), 0);
    EXPECT_NO_THROW(resendFile.close());
}

//TEST(ResendFile, fetch_none) {
//    GlobalSetting *global = GlobalSetting::getInstance();
//    global->workPath = "./data";
//    TransferTask task;
//    task.taskID = "mytask";
//    TransferTask::HostNode node1;
//    node1.sHost = "10.200.1.12";
//    node1.sPath = "/app/etl/test";
//    TransferTask::HostNode node2;
//    node2.sHost = "10.200.1.13";
//    node2.sPath = "/app/etl/ljq2/test";
//    TransferTask::HostNode node3;
//    node3.sHost = "172.20.36.93";
//    node3.sPath = "/home/ljq/ft";
//    task.dstNode.push_back(node1);
//    task.dstNode.push_back(node2);
//    task.dstNode.push_back(node3);
//
//    std::ofstream ofs("./data/mytask.resend", std::ios_base::trunc);
//    ofs << "LastExecuteTime=138000000" << std::endl;
//    ofs << "10.200.1.18&&/app/etl/test&&/BIDATA/etl/TW_USR_MO_201311&&887" << std::endl;
//    ofs.close();
//    ResendFile resendFile(task);
//    EXPECT_NO_THROW(resendFile.loadFile());
//    EXPECT_THROW(resendFile.validate(), std::runtime_error);
//    ResendFile::Line line;
//    EXPECT_THROW(resendFile.fetch(line), std::runtime_error);
//    resendFile.close();
//}

TEST(ResendFile, count) {
    GlobalSetting *global = GlobalSetting::getInstance();
    global->workPath = "./data";
    TransferTask task;
    task.taskID = "mytask";
    TransferTask::HostNode node1;
    node1.sHost = "10.200.1.12";
    node1.sPath = "/app/etl/test";
    TransferTask::HostNode node2;
    node2.sHost = "10.200.1.13";
    node2.sPath = "/app/etl/ljq2/test";
    TransferTask::HostNode node3;
    node3.sHost = "172.20.36.93";
    node3.sPath = "/home/ljq/ft";
    task.dstNode.push_back(node1);
    task.dstNode.push_back(node2);
    task.dstNode.push_back(node3);

    std::ofstream ofs("./data/mytask.resend", std::ios_base::trunc);
    ofs << "LastExecuteTime=138000000" << std::endl;
    ofs << "10.200.1.18&&/app/etl/test&&/BIDATA/etl/TW_USR_MO_201311&&887" << std::endl;
    ofs.close();
    ResendFile resendFile(task);
    EXPECT_NO_THROW(resendFile.loadFile());
    EXPECT_THROW(resendFile.validate(), std::runtime_error);
    EXPECT_EQ(resendFile.count("/BIDATA/etl/TW_USR_MO_201311"), 1);
    EXPECT_NO_THROW(resendFile.setCurrentSuccess());
    EXPECT_EQ(resendFile.count("/BIDATA/etl/TW_USR_MO_201311"), 0);
    resendFile.close();
}

TEST(ResendFile, resend_flow) {
    GlobalSetting *global = GlobalSetting::getInstance();
    global->workPath = "./data";
    TransferTask task;
    task.taskID = "mytask";
    TransferTask::HostNode node1;
    node1.sHost = "10.200.1.12";
    node1.sPath = "/app/etl/test";
    TransferTask::HostNode node2;
    node2.sHost = "10.200.1.13";
    node2.sPath = "/app/etl/ljq2/test";
    TransferTask::HostNode node3;
    node3.sHost = "172.20.36.93";
    node3.sPath = "/home/ljq/ft";
    task.dstNode.push_back(node1);
    task.dstNode.push_back(node2);
    task.dstNode.push_back(node3);

    std::ofstream ofs("./data/mytask.resend", std::ios_base::trunc);
    ofs << "LastExecuteTime=138000000" << std::endl;
    ofs << "10.200.1.12&&/app/etl/test&&/BIDATA/etl/TW_USR_MO_201311&&887" << std::endl;
    ofs << "10.200.1.13&&/app/etl/ljq2/test&&/BIDATA/etl/TW_USR_MO_201311&&887" << std::endl;
    ofs << "172.20.36.93&&/home/ljq/ft&&/BIDATA/etl/HO_NO&&1024000" << std::endl;
    ofs.close();
    ResendFile resendFile(task);
    EXPECT_TRUE(resendFile.isExist());
    EXPECT_NO_THROW(resendFile.loadFile());
    EXPECT_NO_THROW(resendFile.validate());
    EXPECT_EQ(138000000, resendFile.getLastExecuteTime());
    ResendFile::Line line;
    EXPECT_EQ(0, resendFile.fetch(line));
    EXPECT_STREQ("/BIDATA/etl/TW_USR_MO_201311", line.sourceFile.c_str());
    EXPECT_EQ(887, line.sourceFileSize);
    EXPECT_NO_THROW(resendFile.setCurrentSuccess());

    EXPECT_EQ(1, resendFile.fetch(line));
    EXPECT_STREQ("/BIDATA/etl/TW_USR_MO_201311", line.sourceFile.c_str());
    EXPECT_EQ(887, line.sourceFileSize);
    EXPECT_NO_THROW(resendFile.setCurrentSuccess());

    EXPECT_EQ(2, resendFile.fetch(line));
    EXPECT_STREQ("/BIDATA/etl/HO_NO", line.sourceFile.c_str());
    EXPECT_EQ(1024000, line.sourceFileSize);
    EXPECT_NO_THROW(resendFile.setCurrentSuccess());

    EXPECT_NO_THROW(resendFile.close());
    EXPECT_FALSE(resendFile.isExist());
}

TEST(ResendFile, resend_test) {
    GlobalSetting *global = GlobalSetting::getInstance();
    global->workPath = "./data";
    TransferTask task;
    task.taskID = "mytask";
    TransferTask::HostNode node1;
    node1.sHost = "10.200.1.12";
    node1.sPath = "/app/etl/test";
    TransferTask::HostNode node2;
    node2.sHost = "10.200.1.13";
    node2.sPath = "/app/etl/_qht/";
    TransferTask::HostNode node3;
    node3.sHost = "172.20.36.93";
    node3.sPath = "/home/ljq/ft";
    task.dstNode.push_back(node1);
    task.dstNode.push_back(node2);
    task.dstNode.push_back(node3);

    std::ofstream ofs("./data/mytask.resend", std::ios_base::trunc);
    ofs << "LastExcuteTime=1391829906" << std::endl;
    ofs << "10.200.1.13&&/app/etl/_qht/&&/app/etl/_qht/data/resend/AÎªÄãÖÓÇéDVDÔÁÓïÖÐ×Ö.rmvb1&&464084495" << std::endl;
    ofs.close();
    ResendFile resendFile(task);
    EXPECT_TRUE(resendFile.isExist());
    EXPECT_NO_THROW(resendFile.loadFile());
    EXPECT_NO_THROW(resendFile.validate());
    EXPECT_EQ(1391829906, resendFile.getLastExecuteTime());
    ResendFile::Line line;
    while (resendFile.fetch(line) >= 0) {
    }

    EXPECT_NO_THROW(resendFile.close());
}

