#Compiler and Linker
CC                 := g++
AR                 := ar
MAKE               := make
CFLAGS             := -std=c++17 -ggdb3 -O0 -Wall -Werror

#Top level
TLD                := .
GTEST              := libs/gtest.o

#TARGET
SERVER_TARGET      := bin/server
CLIENT_TARGET      := bin/client
COMMON_TARGET      := libs

#The Directories, Source, Includes, Objects, Binary and Resources
SERVER_SRCDIR         := server/src
SERVER_UTDIR          := server/tests/ut
SERVER_INCDIR         := .
SERVER_TESTINCDIR  	  := $(SERVER_INCDIR) gtest
SERVER_UT_LD          := pthread m
SERVER_LD          := pthread m

CLIENT_SRCDIR         := client/src
CLIENT_UTDIR          := client/tests/ut
CLIENT_INCDIR         := .
CLIENT_TESTINCDIR  	  := $(CLIENT_INCDIR) gtest
CLIENT_UT_LD          := pthread m
CLIENT_LD          := pthread m

COMMON_SRCDIR         := common/src
COMMON_INCDIR         := .
COMMON_LD          := pthread m
COMMON_TESTING_SRCDIR         := common/TestingFramework
COMMON_TESTING_INCDIR         := $(COMMON_INCDIR) gtest
COMMON_TESTING_TESTINCDIR     := $(COMMON_TESTING_INCDIR) gtest
COMMON_TESTING_UT_LD          := pthread m
COMMON_TESTING_LD          := pthread m

TESTFLAG			  :=

ifneq ($(strip $(TEST)),)
TESTFLAG := --gtest_filter="$(TEST)"
endif

## TARGET DEBUG #########################################################################

BUILDDIR              := build/normal
BUILDDIR_GCOV         := build/gcov

SERVER_INCDIR_GCC     := $(addprefix -I, $(SERVER_INCDIR))
SERVER_TESTINCDIR_GCC := $(addprefix -I, $(SERVER_TESTINCDIR))
SERVER_UT_LD_GCC 	  := $(addprefix -l, $(SERVER_UT_LD))
SERVER_LD_GCC 	  := $(addprefix -l, $(SERVER_UT_LD))
SERVER_SOURCES        := $(shell find $(SERVER_SRCDIR) -type f -name *.cpp -not -name main.cpp)
SERVER_TEST_SOURCES   := $(shell find $(SERVER_UTDIR) -type f -name *.cpp)
SERVER_OBJECTS        := $(addprefix $(BUILDDIR)/, $(SERVER_SOURCES:.cpp=.cpp.o))
SERVER_OBJECTS_GCOV   := $(addprefix $(BUILDDIR_GCOV)/, $(SERVER_SOURCES:.cpp=.cpp.o))
SERVER_TESTS_OBJECTS  	   := $(addprefix $(BUILDDIR)/, $(SERVER_TEST_SOURCES:.cpp=.cpp.o))
SERVER_TESTS_OBJECTS_GCOV  := $(addprefix $(BUILDDIR_GCOV)/, $(SERVER_TEST_SOURCES:.cpp=.cpp.o))

SERVER_DEPS = $(SERVER_OBJECTS:%.o=%.d)
SERVER_TEST_DEPS = $(SERVER_TESTS_OBJECTS:%.o=%.d)

-include $(SERVER_DEPS)
-include $(SERVER_TEST_DEPS)

CLIENT_INCDIR_GCC     := $(addprefix -I, $(CLIENT_INCDIR))
CLIENT_TESTINCDIR_GCC := $(addprefix -I, $(CLIENT_TESTINCDIR))
CLIENT_UT_LD_GCC 	  := $(addprefix -l, $(CLIENT_UT_LD))
CLIENT_LD_GCC 	  := $(addprefix -l, $(CLIENT_UT_LD))
CLIENT_SOURCES        := $(shell find $(CLIENT_SRCDIR) -type f -name *.cpp)
CLIENT_TEST_SOURCES   := $(shell find $(CLIENT_UTDIR) -type f -name *.cpp)
CLIENT_OBJECTS        := $(addprefix $(BUILDDIR)/, $(CLIENT_SOURCES:.cpp=.cpp.o))
CLIENT_OBJECTS_GCOV   := $(addprefix $(BUILDDIR_GCOV)/, $(CLIENT_SOURCES:.cpp=.cpp.o))
CLIENT_TESTS_OBJECTS  	   := $(addprefix $(BUILDDIR)/, $(CLIENT_TEST_SOURCES:.cpp=.cpp.o))
CLIENT_TESTS_OBJECTS_GCOV  := $(addprefix $(BUILDDIR_GCOV)/, $(CLIENT_TEST_SOURCES:.cpp=.cpp.o))

CLIENT_DEPS = $(CLIENT_OBJECTS:%.o=%.d)
CLIENT_TEST_DEPS = $(CLIENT_TESTS_OBJECTS:%.o=%.d)

-include $(CLIENT_DEPS)
-include $(CLIENT_TEST_DEPS)

COMMON_INCDIR_GCC     := $(addprefix -I, $(COMMON_INCDIR))
COMMON_SOURCES        := $(shell find $(COMMON_SRCDIR) -type f -name *.cpp)
COMMON_OBJECTS        := $(addprefix $(BUILDDIR)/, $(COMMON_SOURCES:.cpp=.cpp.o))
COMMON_OBJECTS_GCOV   := $(addprefix $(BUILDDIR_GCOV)/, $(COMMON_SOURCES:.cpp=.cpp.o))
COMMON_LD_GCC 	  := $(addprefix -l, $(COMMON_LD))
COMMON_TESTING_INCDIR_GCC     := $(addprefix -I, $(COMMON_TESTING_INCDIR))
COMMON_TESTING_SOURCES        := $(shell find $(COMMON_TESTING_SRCDIR) -type f -name *.cpp)
COMMON_TESTING_OBJECTS        := $(addprefix $(BUILDDIR)/, $(COMMON_TESTING_SOURCES:.cpp=.cpp.o))
COMMON_TESTING_OBJECTS_GCOV   := $(addprefix $(BUILDDIR_GCOV)/, $(COMMON_TESTING_SOURCES:.cpp=.cpp.o))
COMMON_TESTING_LD_GCC     := $(addprefix -l, $(COMMON_TESTING_LD))

COMMON_DEPS = $(COMMON_OBJECTS:%.o=%.d)
COMMON_TEST_DEPS = $(COMMON_TESTING_OBJECTS:%.o=%.d)

-include $(COMMON_DEPS)
-include $(COMMON_TEST_DEPS)


## TARGET GTEST #########################################################################

$(GTEST):
	@echo "Building gtest..."
	$(MAKE) -C gtest all

## TARGET COMMON #########################################################################

$(COMMON_TARGET)/common.a: $(COMMON_OBJECTS)
	@mkdir -p $(COMMON_TARGET)
	@echo Archiving $(COMMON_TARGET)/common.a
	@$(AR) rcs $(COMMON_TARGET)/common.a $(COMMON_OBJECTS)

$(COMMON_OBJECTS): $(BUILDDIR)/%.cpp.o : %.cpp
	@mkdir -p $(@D)
	@echo "Building COMMON SOURCE" $@
	@$(CC) $(CFLAGS) $(COMMON_INCDIR_GCC) -MMD -g -c $(patsubst $(BUILDDIR)/%.cpp.o,%.cpp,$@) -o $@

## TARGET COMMON TESTING #########################################################################

$(COMMON_TESTING_OBJECTS): $(BUILDDIR)/%.cpp.o : %.cpp
	@mkdir -p $(@D)
	@echo "Building COMMON TESTING" $@
	@$(CC) $(CFLAGS) $(COMMON_TESTING_INCDIR_GCC) -MMD -g -c $(patsubst $(BUILDDIR)/%.cpp.o,%.cpp,$@) -o $@

$(COMMON_TARGET)/common_testing.a: $(COMMON_TESTING_OBJECTS)
	@mkdir -p $(COMMON_TARGET)
	@echo Archiving $(COMMON_TARGET)/common_testing.a
	@$(AR) rcs $(COMMON_TARGET)/common_testing.a $(COMMON_TESTING_OBJECTS)

## TARGET SERVER #########################################################################

server: $(COMMON_TARGET)/common.a $(SERVER_OBJECTS)
	@mkdir -p $(SERVER_TARGET)
	@echo Linking $(SERVER_TARGET)/server
	@$(CC) $(CFLAGS) $(SERVER_INCDIR_GCC) -g -c $(SERVER_SRCDIR)/main.cpp -o $(BUILDDIR)/$(SERVER_SRCDIR)/main.cpp.o
	@$(CC) -g $(SERVER_OBJECTS) $(SERVER_LD_GCC) $(COMMON_TARGET)/common.a $(BUILDDIR)/$(SERVER_SRCDIR)/main.cpp.o -o $(SERVER_TARGET)/server

$(SERVER_OBJECTS): $(BUILDDIR)/%.cpp.o : %.cpp
	@mkdir -p $(@D)
	@echo "Building SOURCE" $@
	@$(CC) $(CFLAGS) $(SERVER_INCDIR_GCC) -MMD -g -c $(patsubst $(BUILDDIR)/%.cpp.o,%.cpp,$@) -o $@

## TARGET SERVER UT #########################################################################

server_ut: $(GTEST) $(COMMON_TARGET)/common.a $(SERVER_OBJECTS) $(SERVER_TESTS_OBJECTS) $(COMMON_TARGET)/common_testing.a
	@mkdir -p $(SERVER_TARGET)
	@echo Linking $(SERVER_TARGET)/server_ut
	@$(CC) -g $(GTEST) $(SERVER_UT_LD_GCC) $(SERVER_OBJECTS) $(SERVER_TESTS_OBJECTS) $(COMMON_TARGET)/common.a $(COMMON_TARGET)/common_testing.a -o $(SERVER_TARGET)/server_ut

server_ut_run: server_ut
	$(SERVER_TARGET)/server_ut $(TESTFLAG)

server_ut_valgrind_run: server_ut
	valgrind --leak-check=full --show-leak-kinds=all -v $(SERVER_TARGET)/server_ut $(TESTFLAG)

server_ut_valgrind_run_detailed: server_ut
	valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all -v $(SERVER_TARGET)/server_ut $(TESTFLAG)

$(SERVER_TESTS_OBJECTS): $(BUILDDIR)/%.cpp.o : %.cpp
	@mkdir -p $(@D)
	@echo "Building TEST" $@
	@$(CC) $(CFLAGS) $(SERVER_TESTINCDIR_GCC) -MMD -g -c $(patsubst $(BUILDDIR)/%.cpp.o,%.cpp,$@) -o $@

## TARGET CLIENT #########################################################################

$(CLIENT_OBJECTS): $(BUILDDIR)/%.cpp.o : %.cpp
	@mkdir -p $(@D)
	@echo "Building SOURCE" $@
	@$(CC) $(CFLAGS) $(CLIENT_INCDIR_GCC) -MMD -g -c $(patsubst $(BUILDDIR)/%.cpp.o,%.cpp,$@) -o $@

## TARGET CLIENT UT #########################################################################

client_ut: $(GTEST) $(CLIENT_OBJECTS) $(CLIENT_TESTS_OBJECTS) $(COMMON_TARGET)/common.a $(COMMON_TARGET)/common_testing.a
	@mkdir -p $(CLIENT_TARGET)
	@echo Linking $(CLIENT_TARGET)/client_ut
	$(CC) -g $(GTEST) $(CLIENT_UT_LD_GCC) $(CLIENT_OBJECTS) $(CLIENT_TESTS_OBJECTS) $(COMMON_TARGET)/common_testing.a $(COMMON_TARGET)/common.a -o $(CLIENT_TARGET)/client_ut

client_ut_run: client_ut
	$(CLIENT_TARGET)/client_ut $(TESTFLAG)

client_ut_valgrind_run: client_ut
	valgrind --leak-check=full --show-leak-kinds=all -v $(CLIENT_TARGET)/client_ut $(TESTFLAG)

client_ut_valgrind_run_detailed: client_ut
	valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all -v $(CLIENT_TARGET)/client_ut $(TESTFLAG)

client: $(CLIENT_OBJECTS) $(COMMON_TARGET)/common.a
	@mkdir -p $(CLIENT_TARGET)
	@echo Archiving $(CLIENT_TARGET)/client.a
	@$(AR) rcs $(CLIENT_TARGET)/client.a $(CLIENT_OBJECTS) $(COMMON_TARGET)/common.a

$(CLIENT_TESTS_OBJECTS): $(BUILDDIR)/%.cpp.o : %.cpp
	@mkdir -p $(@D)
	@echo "Building TEST" $@
	@$(CC) $(CFLAGS) $(CLIENT_TESTINCDIR_GCC) -MMD -g -c $(patsubst $(BUILDDIR)/%.cpp.o,%.cpp,$@) -o $@

## TARGET GCOV COMMON #########################################################################

$(COMMON_TARGET)/gcov/common_testing.a: $(COMMON_TESTING_OBJECTS_GCOV)
	@mkdir -p $(COMMON_TARGET)
	@echo Archiving $(COMMON_TARGET)/gcov/common_testing.a
	@$(AR) rcs $(COMMON_TARGET)/gcov/common_testing.a $(COMMON_TESTING_OBJECTS_GCOV)

$(COMMON_TESTING_OBJECTS_GCOV): $(BUILDDIR_GCOV)/%.cpp.o : %.cpp
	@mkdir -p $(@D)
	@echo "Building_COMMON_TESTING_GCOV" $@
	@$(CC) $(CFLAGS) $(COMMON_TESTING_INCDIR_GCC) -lgcov --coverage -g -c $(patsubst $(BUILDDIR_GCOV)/%.cpp.o,%.cpp,$@) -o $@

$(COMMON_TARGET)/gcov/common.a: $(COMMON_OBJECTS_GCOV)
	@mkdir -p $(COMMON_TARGET)/gcov/
	@echo Archiving	 $(COMMON_TARGET)/gcov/common.a
	@$(AR) rcs $(COMMON_TARGET)/gcov/common.a $(COMMON_OBJECTS_GCOV)

$(COMMON_OBJECTS_GCOV): $(BUILDDIR_GCOV)/%.cpp.o : %.cpp
	@mkdir -p $(@D)
	@echo "Building_COMMON_SOURCE_GCOV" $@
	@$(CC) $(CFLAGS) $(COMMON_INCDIR_GCC) -lgcov --coverage -g -c $(patsubst $(BUILDDIR_GCOV)/%.cpp.o,%.cpp,$@) -o $@

## TARGET GCOV SERVER #########################################################################

server_ut_gcov: $(GTEST) $(SERVER_OBJECTS_GCOV) $(SERVER_TESTS_OBJECTS_GCOV) $(COMMON_TARGET)/gcov/common.a $(COMMON_TARGET)/gcov/common_testing.a
	@mkdir -p $(SERVER_TARGET)
	@echo Linking $(SERVER_TARGET)/server_ut
	@$(CC) $(GTEST) $(SERVER_UT_LD_GCC) $(SERVER_OBJECTS_GCOV) $(SERVER_TESTS_OBJECTS_GCOV) $(COMMON_TARGET)/gcov/common.a $(COMMON_TARGET)/gcov/common_testing.a -lgcov --coverage -o $(SERVER_TARGET)/server_ut_gcov

server_ut_gcov_run: server_ut_gcov
	$(SERVER_TARGET)/server_ut_gcov $(TESTFLAG)

$(SERVER_OBJECTS_GCOV): $(BUILDDIR_GCOV)/%.cpp.o : %.cpp
	@mkdir -p $(@D)
	@echo "Building_SOURCE_GCOV" $@
	@$(CC) $(CFLAGS) -fprofile-arcs -ftest-coverage --coverage $(SERVER_INCDIR_GCC) -lgcov -c $(patsubst $(BUILDDIR_GCOV)/%.cpp.o,%.cpp,$@) -o $@

$(SERVER_TESTS_OBJECTS_GCOV): $(BUILDDIR_GCOV)/%.cpp.o : %.cpp
	@mkdir -p $(@D)
	@echo "Building_TEST_GCOV" $@
	@$(CC) $(CFLAGS) $(SERVER_TESTINCDIR_GCC) -fprofile-arcs -ftest-coverage --coverage -lgcov -c $(patsubst $(BUILDDIR_GCOV)/%.cpp.o,%.cpp,$@) -o $@

## TARGET GCOV CLIENT #########################################################################

client_ut_gcov: $(GTEST) $(CLIENT_OBJECTS_GCOV) $(CLIENT_TESTS_OBJECTS_GCOV) $(COMMON_TARGET)/gcov/common.a $(COMMON_TARGET)/gcov/common_testing.a
	@mkdir -p $(CLIENT_TARGET)
	@echo Linking $(CLIENT_TARGET)/client_ut
	$(CC) $(GTEST) $(CLIENT_UT_LD_GCC) $(CLIENT_OBJECTS_GCOV) $(CLIENT_TESTS_OBJECTS_GCOV) $(COMMON_TARGET)/gcov/common.a $(COMMON_TARGET)/gcov/common_testing.a -lgcov --coverage -o $(CLIENT_TARGET)/client_ut_gcov

client_ut_gcov_run: client_ut_gcov
	$(CLIENT_TARGET)/client_ut_gcov $(TESTFLAG)

$(CLIENT_OBJECTS_GCOV): $(BUILDDIR_GCOV)/%.cpp.o : %.cpp
	@mkdir -p $(@D)
	@echo "Building SOURCE_GCOV" $@
	@$(CC) $(CFLAGS) -fprofile-arcs -ftest-coverage --coverage $(CLIENT_INCDIR_GCC) -lgcov -c $(patsubst $(BUILDDIR_GCOV)/%.cpp.o,%.cpp,$@) -o $@

$(CLIENT_TESTS_OBJECTS_GCOV): $(BUILDDIR_GCOV)/%.cpp.o : %.cpp
	@mkdir -p $(@D)
	@echo "Building TEST" $@
	@$(CC) $(CFLAGS) $(CLIENT_TESTINCDIR_GCC) -fprofile-arcs -ftest-coverage --coverage -lgcov -c $(patsubst $(BUILDDIR_GCOV)/%.cpp.o,%.cpp,$@) -o $@

## MISC #########################################################################

clean_common:
	echo cleaning common
	find $(BUILDDIR)/$(COMMON_SRCDIR) -type f -name *.cpp.o -exec rm {} \;
	rm $(COMMON_TARGET)/common.a

clean_common_testing:
	echo cleaning common
	find $(BUILDDIR)/$(COMMON_TESTING_SRCDIR) -type f -name *.cpp.o -exec rm {} \;
	rm $(COMMON_TARGET)/common_testing.a

clean_server_ut:
	echo cleaning ut
	find $(BUILDDIR)/$(SERVER_UTDIR) -type f -name *.cpp.o -exec rm {} \;

clean_server: clean_server_ut
	echo cleaning server
	find $(BUILDDIR)/$(SERVER_SRCDIR) -type f -name *.cpp.o -exec rm {} \;

clean_client_ut:
	echo cleaning ut
	find $(BUILDDIR)/$(CLIENT_UTDIR) -type f -name *.cpp.o -exec rm {} \;

clean_client: clean_client_ut
	echo cleaning server
	find $(BUILDDIR)/$(CLIENT_SRCDIR) -type f -name *.cpp.o -exec rm {} \;

clean_gcov:
	echo cleaning GCOV
	find $(BUILDDIR_GCOV) -type f -name *.cpp.o -exec rm {} \;
	find $(BUILDDIR_GCOV) -type f -name *.gcda -exec rm {} \;
	find $(BUILDDIR_GCOV) -type f -name *.gcno -exec rm {} \;

clean: clean_client_ut clean_client clean_server_ut clean_server clean_gcov clean_common clean_common_testing
	echo All cleaned
