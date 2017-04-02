#include "gtest/gtest.h"
#include <iostream>
#include "Config.h"
#include "Queue.h"
#include "TBridger.h"
#include "TransferTask.h"
#include "Factory.h"
#include "TLog.h"
using namespace std;

TEST(TBridger, ReadFile) {
    TLog::init("/tmp/test.log", 102400,
                    2, "DEBUG");
    const char *file = "./etc/FileTrans.ini";
    Config cfg;
    EXPECT_EQ(cfg.ReadFile(file), 4);

    Queue taskQueue(cfg.GetTasks());

    while (!taskQueue.empty()) {
        TransferTask task = taskQueue.Pop();
        Transmitter *transmitter = TransmitterKit::createTransmitter(task);
        transmitter->Init(task);

        if (transmitter->Connect() < 0) {
            transmitter->Disconnect();
            fprintf(stderr, "(%s) Connect ERROR\n", task.taskID.c_str());
            continue;
        }

        if (transmitter->ListFile() < 0) {
            transmitter->Disconnect();
            fprintf(stderr, "(%s) ListFile ERROR\n", task.taskID.c_str());
            continue;
        }

        transmitter->Transfer();
        transmitter->Disconnect();
        delete transmitter;
    }
}

