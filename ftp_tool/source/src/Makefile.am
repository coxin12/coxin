dist_noinst_HEADERS = \
ProtocolBase.h \
Config.h \
dstruct.h \
FinishFile.h \
ftplib_mt.h \
GlobalSetting.h \
PFtp.h \
PSftp.h \
Queue.h \
regex_match.h \
TBridger.h \
TGeter.h \
TLog.h \
TMover.h \
TMultiBridger.h \
TMultiSender.h \
TransferTask.h \
TSender.h \
Transmitter.h \
Factory.h \
ResendFile.h \
pidfile.h \
PLocal.h \
Filter.h \
MatchFilter.h \
RegexFilter.h \
ThreadManager.h \
Mutex.h

bin_PROGRAMS = FileTransfer
FileTransfer_SOURCES = \
Config.cpp \
FinishFile.cpp \
ftplib_mt.cpp \
GlobalSetting.cpp \
main.cpp \
PFtp.cpp \
PSftp.cpp \
Queue.cpp \
regex_match.cpp \
TBridger.cpp \
TGeter.cpp \
TLog.cpp \
TMover.cpp \
TMultiBridger.cpp \
TMultiSender.cpp \
TransferTask.cpp \
TSender.cpp \
Transmitter.cpp \
Factory.cpp \
ResendFile.cpp \
pidfile.cpp \
PLocal.cpp \
MatchFilter.cpp \
RegexFilter.cpp \
ThreadManager.cpp \
Mutex.cpp

FileTransfer_CPPFLAGS = -I../include -I$(BIPROG_ROOT)/include
FileTransfer_LDFLAGS = -L$(BIPROG_ROOT)/lib
FileTransfer_LDADD = -lpthread -ldesini -llogger -lssh2
DEFS = \
    -D_FTPLIB_NO_COMPAT \
    -D_AIX \
    -D__unix__ \
    -D_THREAD_SAFE \
    -DFTPLIB_DEFMODE=FTPLIB_PORT \
    -D_DAEMON
#CXXFLAGS = -O2
