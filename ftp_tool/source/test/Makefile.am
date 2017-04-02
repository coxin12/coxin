# noinst_PROGRAMS = test
# test_SOURCES = test.cpp
# test_LDFLAGS = -L$(BIPROG_ROOT)/lib
# INCLUDES = -I../src -I$(BIPROG_ROOT)/include
# DEFS = -D_FTPLIB_NO_COMPAT \
#        -D_AIX \
#        -D__unix__ \
#        -D_THREAD_SAFE \
#        -DFTPLIB_DEFMODE=FTPLIB_PORT \
#        -D_DAEMON
# test_CXXFLAGS = -O2 #-fPIC
#LIBS = -ldesini -llogger -lm -lc -ldl -lpthread

GTEST_FILES = gtest/gtest-all.cc \
			  gtest/gtest.h \
			  gtest/gtest_main.cc

LIBS = -lpthread 
DEFS = -D_DEBUG

noinst_LIBRARIES = libgtest.a libsrc.a
libgtest_a_SOURCES = $(GTEST_FILES)

libsrc_a_SOURCES = \
../src/Config.cpp \
../src/ftplib_mt.cpp \
../src/main.cpp \
../src/PFtp.cpp \
../src/PSftp.cpp \
../src/regex_match.cpp \
../src/TBridger.cpp \
../src/TGeter.cpp \
../src/TLog.cpp \
../src/TMover.cpp \
../src/TSender.cpp \
../src/GlobalSetting.cpp \
../src/TransferTask.cpp \
../src/Queue.cpp \
../src/Factory.cpp \
../src/Transmitter.cpp \
../src/TMultiBridger.cpp \
../src/TMultiSender.cpp \
../src/ResendFile.cpp \
../src/PLocal.cpp \
../src/RegexFilter.cpp \
../src/MatchFilter.cpp \
../src/ThreadManager.cpp \
../src/Mutex.cpp
libsrc_a_CPPFLAGS = -I../src -I$(BIPROG_ROOT)/include

TEST_CPPFLAG = -I../src -I$(BIPROG_ROOT)/include
TEST_LDFLAG = -L$(BIPROG_ROOT)/lib
TEST_LD = libgtest.a libsrc.a -ldesini -ldes -lssh2 -llogger
DEFS = \
-D_FTPLIB_NO_COMPAT \
-D_AIX \
-D__unix__ \
-D_THREAD_SAFE \
-DFTPLIB_DEFMODE=FTPLIB_PORT \
-D_DAEMON

TESTS = Config_unittest GlobalSetting_unittest TransferTask_unittest Queue_unittest \
ResendFile_unittest PLocal_unittest Filter_unittest ThreadManager_unittest
noinst_PROGRAMS = Config_unittest GlobalSetting_unittest TransferTask_unittest Queue_unittest TBridger_unittest TMultiSender_unittest \
ResendFile_unittest PLocal_unittest Filter_unittest ThreadManager_unittest
Config_unittest_SOURCES = Config_unittest.cpp
Config_unittest_CPPFLAGS = $(TEST_CPPFLAG)
Config_unittest_LDFLAGS = $(TEST_LDFLAG)
Config_unittest_LDADD = $(TEST_LD)

GlobalSetting_unittest_SOURCES = GlobalSetting_unittest.cpp
GlobalSetting_unittest_CPPFLAGS = $(TEST_CPPFLAG)
GlobalSetting_unittest_LDFLAGS = $(TEST_LDFLAG)
GlobalSetting_unittest_LDADD = $(TEST_LD)

TransferTask_unittest_SOURCES = TransferTask_unittest.cpp
TransferTask_unittest_CPPFLAGS = $(TEST_CPPFLAG)
TransferTask_unittest_LDFLAGS = $(TEST_LDFLAG)
TransferTask_unittest_LDADD = $(TEST_LD)

Queue_unittest_SOURCES = Queue_unittest.cpp
Queue_unittest_CPPFLAGS = $(TEST_CPPFLAG)
Queue_unittest_LDFLAGS = $(TEST_LDFLAG)
Queue_unittest_LDADD = $(TEST_LD)

TBridger_unittest_SOURCES = TBridger_unittest.cpp
TBridger_unittest_CPPFLAGS = $(TEST_CPPFLAG)
TBridger_unittest_LDFLAGS = $(TEST_LDFLAG)
TBridger_unittest_LDADD = $(TEST_LD)

TMultiSender_unittest_SOURCES = TMultiSender_unittest.cpp
TMultiSender_unittest_CPPFLAGS = $(TEST_CPPFLAG)
TMultiSender_unittest_LDFLAGS = $(TEST_LDFLAG)
TMultiSender_unittest_LDADD = $(TEST_LD)

ResendFile_unittest_SOURCES = ResendFile_unittest.cpp
ResendFile_unittest_CPPFLAGS = $(TEST_CPPFLAG)
ResendFile_unittest_LDFLAGS = $(TEST_LDFLAG)
ResendFile_unittest_LDADD = $(TEST_LD)

PLocal_unittest_SOURCES = PLocal_unittest.cpp
PLocal_unittest_CPPFLAGS = $(TEST_CPPFLAG)
PLocal_unittest_LDFLAGS = $(TEST_LDFLAG)
PLocal_unittest_LDADD = $(TEST_LD)

Filter_unittest_SOURCES = Filter_unittest.cpp
Filter_unittest_CPPFLAGS = $(TEST_CPPFLAG)
Filter_unittest_LDFLAGS = $(TEST_LDFLAG)
Filter_unittest_LDADD = $(TEST_LD)

ThreadManager_unittest_SOURCES = ThreadManager_unittest.cpp
ThreadManager_unittest_CPPFLAGS = $(TEST_CPPFLAG)
ThreadManager_unittest_LDFLAGS = $(TEST_LDFLAG)
ThreadManager_unittest_LDADD = $(TEST_LD)

