#include "gtest/gtest.h"
#include <iostream>
#include <pthread.h>
#include "ThreadManager.h"
using namespace std;

TEST(ThreadManager, getInstance) {
    ThreadManager *manager1 = ThreadManager::getInstance();
    ThreadManager *manager2 = ThreadManager::getInstance();
    EXPECT_EQ(manager1, manager2);
}

