#include "gtest/gtest.h"
#include <iostream>
#include "PLocal.h"
using namespace std;

TEST(PLocal, RemoteOpenFile) {
    PLocal local;
    EXPECT_EQ(local.RemoteOpenFile("./data/abc", FILE_WRITE), 0);
}

TEST(PLocal, RemoteDelete) {
    PLocal local;
    EXPECT_EQ(local.RemoteDelete("./data/abc"), 0);
}

TEST(PLocal, RemoteCloseFile) {
    PLocal local;
    EXPECT_EQ(local.RemoteOpenFile("./data/abc", FILE_WRITE), 0);
    EXPECT_EQ(local.RemoteCloseFile(), 0);
}

TEST(PLocal, RemoteFileSize) {
    PLocal local;
    long fileSize = -1;
    EXPECT_EQ(local.RemoteFileSize("./data/abc", &fileSize), 0);
    EXPECT_EQ(fileSize, 0);
}

TEST(PLocal, RemoteLs) {
    PLocal local;
    EXPECT_EQ(local.RemoteLs("./data/ls.tmp", "./data"), 0);
    EXPECT_EQ(local.RemoteDelete("./data/ls.tmp"), 0);
}

TEST(PLocal, RemoteMkDir) {
    PLocal local;
    EXPECT_EQ(local.RemoteMkDir("./data/dir"), 0);
}

TEST(PLocal, RemoteRmDir) {
    PLocal local;
    EXPECT_EQ(local.RemoteRmDir("./data/dir"), 0);
}

TEST(PLocal, RemoteRename) {
    PLocal local;
    EXPECT_EQ(local.RemoteRename("./data/abc", "./data/bcd"), 0);
}

TEST(PLocal, RemoteWrite_and_RemoteRead) {
    const char *content = "hello world";
    PLocal local;
    EXPECT_EQ(local.RemoteOpenFile("./data/bcd", FILE_WRITE), 0);
    EXPECT_EQ(local.RemoteWrite(const_cast<char *>(content), strlen(content)), strlen(content));
    EXPECT_EQ(local.RemoteCloseFile(), 0);
    EXPECT_EQ(local.RemoteOpenFile("./data/bcd", FILE_READ), 0);
    char fileContent[1024] = {0};
    EXPECT_EQ(local.RemoteRead(fileContent, sizeof(fileContent)), strlen(content));
    EXPECT_STREQ(fileContent, content);
    EXPECT_EQ(local.RemoteCloseFile(), 0);
    EXPECT_EQ(local.RemoteDelete("./data/bcd"), 0);
}
