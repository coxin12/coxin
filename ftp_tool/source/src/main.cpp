/******************************************************************
 * Copyright by SunRise, 2013-11-07
 * File Name: main.cpp
 *
 * Modified History:
 *     See ChangeLog.
 ******************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <iostream>
#include <fstream>
#include <stdexcept>

#include "pidfile.h"
#include "Config.h"
#include "TLog.h"
#include "GlobalSetting.h"
#include "Queue.h"
#include "Factory.h"
#include "FinishFile.h"
#include "ThreadManager.h"

// --------------------------------------
// VERSION is defined in config.h.
// This header is generated by autotools.
// --------------------------------------
#include "config.h"

bool isShutdown = false;
GlobalSetting *global = 0;
ThreadManager *threadManager = 0;
static Queue waitingQueue;
static Queue runningQueue;
static time_t startTime = time(0);

void printThreadstatistics();
void *threadMain(void *arg);
int enterDaemonMode(void);

extern "C" {
    static void handle_trap(int sig) {
        DP(ERROR, "receive a trap signal\n");
        printf("have trap\n");
        signal(SIGTRAP, handle_trap);
        return;
    }

    static void handle_abort(int sig) {
        DP(ERROR, "receive a abort signal\n");
        printf("have abort\n");
        signal(SIGABRT, handle_abort);
        return;
    }

    static void handle_pipe(int sig) {
        DP(ERROR, "receive a pipe signal\n");
        printf("have pipe\n");
        signal(SIGPIPE, handle_pipe);
        return;
    }
}

void Usage() {
    printf("\n本程序有以下2种使用方式\n");
    printf("1:非监控正常运行模式：%s 配置文件\n", PACKAGE);
    printf("2:非监控调试运行模式：%s -d 配置文件\n", PACKAGE);
}

int main(int argc, char **argv) {
    std::string loglevel("INFO");
    std::string cfgFile;

    if (argc == 3) {
        if (strcmp(argv[1], "-d") == 0) {
            loglevel = "DEBUG";
            cfgFile = argv[2];
        } else {
            Usage();
            fprintf(stderr, "\nVersion: %s\n\n", VERSION);
            return -1;
        }
    } else if (argc == 2) {
        cfgFile = argv[1];
    } else {
        Usage();
        fprintf(stderr, "\nVersion: %s\n\n", VERSION);
        return -1;
    }

#ifdef _DAEMON
    enterDaemonMode();
#endif

    Config cfg;
    int taskCount = 0;
    try {
        taskCount = cfg.ReadFile(cfgFile.c_str());
    } catch (std::exception &e) {
        fprintf(stderr, "ERROR: %s\n", e.what());
        return -1;
    }

    global = GlobalSetting::getInstance();  // 必须在cfg.ReadFile后

    std::string pidfile(global->workPath);
    pidfile += "/" + cfg.GetCfgBaseName() + ".pidfile";
    if(process_running(pidfile.c_str())) {
        fprintf(stderr, "error: cfg(%s) process running!", cfgFile.c_str());
        return -1;
    }
    pidfile_write(pidfile.c_str(),getpid());

    try {
        std::string progLogFile = cfg.GetLogFile();
        TLog::init(progLogFile.c_str(), global->errLogMaxSize,
                global->errLogMaxIndex, loglevel.c_str());
    } catch (std::exception &e) {
        fprintf(stderr, "%s\n", e.what());
        return -1;
    }
    DP(INFO, "==========%s %s startup==========\n", PACKAGE, VERSION);

    FinishFile *finishFile = FinishFile::getFinishFile();
    if (finishFile->initialize() != 0) {
        return -1;
    }

    DP(DEBUG, "tasks count is :%d\n", taskCount);
    DP(DEBUG, "tasks threads count is :%d\n", global->threadCount);
    if (taskCount < global->threadCount) {
        global->threadCount = taskCount;
    }
    DP(DEBUG, "task threads count adjust to : %d\n", global->threadCount);

    signal(SIGTRAP, handle_trap);
    signal(SIGPIPE, handle_pipe);
    signal(SIGABRT, handle_abort);
    signal(SIGTERM, SIG_IGN);

    std::vector<TransferTask> transferTasks = cfg.GetTasks();
    for (size_t i = 0; i < transferTasks.size(); i++) {
        runningQueue.Push(transferTasks[i]);
    }

    try {
        threadManager = ThreadManager::getInstance();
        for (int i = 0; i < global->threadCount; i++) {
            threadManager->createThread((ThreadManager::thread_routine_t)threadMain, 0);
        }
    } catch (std::exception &e) {
        std::cerr << "Create Thread Error: " << e.what() << std::endl;
        exit(1);
    }

    time_t Laster = time(0);
    time_t Current = Laster;
    isShutdown = false;

    while (true) {
        // 主线程
        if (isShutdown) {
            if (threadManager->getThreadCount() == 0) {
                fprintf(stderr, "FileTransfer 程序正常退出 \n");
                DP(INFO, "(main) FileTransfer 程序正常退出\n");
                break;
            } else {
                continue;
            }
        }
        Current = time(0);
        if (Current - Laster > 60) {
            DP(DEBUG, "(main) 进入定时器60秒时间结束....\n");
            Laster = Current;

            isShutdown = finishFile->getFinishFlag();
            if (isShutdown) {
                continue;
            }
        }
        printThreadstatistics();
        try {
            DP(DEBUG, "(main) waitingQueue->TimePop()>>>>Begin<<<<\n");
            TransferTask task = waitingQueue.TimePop();
            DP(DEBUG, "(main) waitingQueue->TimePop()>>>>End<<<<\n");

            DP(DEBUG, "(main) runningQueue->Push()>>>>Begin<<<<\n");
            runningQueue.Push(task);
            DP(DEBUG, "(main) runningQueue->Push()>>>>End<<<<\n");
        } catch (std::exception &e) {
            DP(DEBUG, "(main) waitingQueue->TimePop(): %s\n", e.what());
        }
    }

    pidfile_rm(pidfile.c_str());
    return 0;
}

void printThreadstatistics() {
    static time_t lastPrintTime = time(0);

    time_t currentTime = time(0);
    if (currentTime - lastPrintTime < global->statInterval) {
        return;
    }
    lastPrintTime = currentTime;

    DP(DEBUG, "正在收集打印线程活跃程度统计信息...\n");

    char buff[2048];
    struct tm now;
    time_t clk = time(0);
    if (localtime_r(&clk, &now) == 0) {
        perror("localtime_r");
        return;
    }

    snprintf(buff, sizeof(buff), "%04d/%02d/%02d %02d:%02d:%02d",
            now.tm_year + 1900, now.tm_mon + 1, now.tm_mday, now.tm_hour,
            now.tm_min, now.tm_sec);
    FILE *statFileHandler = fopen(global->statFile.c_str(), "a");
    if (statFileHandler == 0) {
        DP(ERROR, "open file [%s] error [%s]\n", global->statFile.c_str(), strerror(errno));
        return;
    }

    ThreadManager::ThreadIterator i = threadManager->begin();
    int index = 1;
    for (; i != threadManager->end(); ++i, index++) {
        ThreadManager::ThreadInfo info = i->second;
        float scale = float(info.totalRunTime) / float(currentTime - startTime) * 100;
        std::string status;
        if (info.isActive) {
            status = "running";
        } else {
            status = "waiting";
        }
        fprintf(statFileHandler,
                "%-20s thread[%d] is %s now. Total time [%ld]. Scale [%1.2f%%]\n",
                buff, index, status.c_str(), (info.totalRunTime), scale);
    }
    fflush(statFileHandler);
    fclose(statFileHandler);
    DP(DEBUG, "收集打印线程活跃程度统计信息完成\n");
}

void *threadMain(void *arg) {
    signal(SIGTRAP, handle_trap);
    signal(SIGPIPE, handle_pipe);
    signal(SIGABRT, handle_abort);

    TransferTask task;
    time_t currentTime;
    time_t lastRuntime;
    threadManager->setTotalRunTime(pthread_self(), 0);
    while (1) {
        if (isShutdown) {
            fprintf(stderr, "Thread(%ld) Shutdown\n", (long) (pthread_self()));
            DP(DEBUG, "Thread(%ld) Shutdown\n", (long) (pthread_self()));
            break;
        }

        task = runningQueue.Pop();
        currentTime = time(0);
        if (currentTime - task.lastExecuteTime < task.interval) {
            sleep(3);
            DP(DEBUG, "(%s) waitingQueue->Push()>>>>Begin<<<<\n", task.taskID.c_str());
            waitingQueue.Push(task);
            DP(DEBUG, "(%s) waitingQueue->Push()>>>>End<<<<\n", task.taskID.c_str());
            continue;
        }

        lastRuntime = time(0);
        threadManager->setLastRunTime(pthread_self(), lastRuntime);
        threadManager->setIsActive(pthread_self(), true);

        Transmitter *mover = 0;
        try {
            mover = TransmitterKit::createTransmitter(task);
            mover->Init(task);

            if (mover->Connect() < 0) throw -1;

            if (mover->ListFile() < 0) throw -1;

            if (mover->Transfer() < 0) throw -1;

            mover->Disconnect();
            delete mover;
            mover = 0;
        } catch (...) {
            if (mover) {
                delete mover;
                mover = 0;
            }
        }
        task.lastExecuteTime = time(0);

        DP(DEBUG, "(%s) waitingQueue->Push()>>>>Begin<<<<\n", task.taskID.c_str());
        waitingQueue.Push(task);
        DP(DEBUG, "(%s) waitingQueue->Push()>>>>End<<<<\n", task.taskID.c_str());

        time_t totalRunTime = threadManager->getTotalRunTime(pthread_self())
                + (time(0) - lastRuntime);
        threadManager->setTotalRunTime(pthread_self(), totalRunTime);
        threadManager->setIsActive(pthread_self(), false);

        sleep(3);
    }

    threadManager->releaseThread(pthread_self());
    return 0;
}

int enterDaemonMode(void) {
    switch (fork()) {
    case -1:
        fprintf(stderr, "Failed to Enter Daemon Mode\n");
        perror("fork");
        return -1;
        break;
    case 0:
        break;
    default:
        _exit(0);
        break;
    }
    setsid();

    return 0;
}
