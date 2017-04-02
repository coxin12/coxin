#ifndef _CONFIG_H
#define _CONFIG_H

#include <string>
#include <vector>
#include <set>
#include "TransferTask.h"

class Config {
public:
    Config();
    ~Config();

    int ReadFile(std::string configFile);
    std::string GetLogFile(void);
    std::string GetCfgBaseName(void) const {
        return cfgBaseName;
    }
    std::vector<TransferTask> GetTasks(void) const {
        return taskArray;
    }
private:
    void validateTask(TransferTask* pTask);
    void setPassword();
    void validateGlobalSetting();
    void checkConfigFile();

private:
    std::vector<TransferTask> taskArray;
    static std::set<std::string> taskIdSet;
    std::string filepath;   // full path of the configure file
    std::string cfgBaseName;
};

#endif
