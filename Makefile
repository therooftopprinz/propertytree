#Compiler and Linker
CC                 := g++
MAKE               := make
CFLAGS             := -std=gnu++14 -Wall -Werror

#Top level
TLD                := ..
GTEST              := $(TLD)/libs/gtest.o

#SERVER
SERVER_TARGET      := $(TLD)/bin/server

#The Directories, Source, Includes, Objects, Binary and Resources
SERVER_SRCDIR         := server/src
SERVER_UTDIR          := server/tests/ut
SERVER_INCDIR         := .
SERVER_TESTINCDIR  	  := $(SERVER_INCDIR) $(TLD)/gtest
SERVER_UT_LD          := pthread m
SERVER_LD          := pthread m

TESTFLAG			  :=

ifneq ($(strip $(TEST)),)
TESTFLAG := --gtest_filter="$(TEST)"
endif

## TARGET DEBUG #########################################################################

BUILDDIR              := $(TLD)/build/normal
BUILDDIR_GCOV         := $(TLD)/build/gcov

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

$(GTEST):
	@echo "Building gtest..."
	$(MAKE) -C $(TLD)/gtest all

server: $(SERVER_OBJECTS)
	@mkdir -p $(SERVER_TARGET)
	@echo Linking $(SERVER_TARGET)/server
	@$(CC) $(CFLAGS) $(SERVER_INCDIR_GCC) -g -c $(SERVER_SRCDIR)/main.cpp -o $(BUILDDIR)/$(SERVER_SRCDIR)/main.cpp.o
	@$(CC) -g $(SERVER_OBJECTS) $(SERVER_LD_GCC) $(BUILDDIR)/$(SERVER_SRCDIR)/main.cpp.o $(GCOV_LD) -o $(SERVER_TARGET)/server

server_ut: $(GTEST) $(SERVER_OBJECTS) $(SERVER_TESTS_OBJECTS)
	@mkdir -p $(SERVER_TARGET)
	@echo Linking $(SERVER_TARGET)/server_ut
	@$(CC) -g $(GTEST) $(SERVER_UT_LD_GCC) $(SERVER_OBJECTS) $(SERVER_TESTS_OBJECTS) $(GCOV_LD) -o $(SERVER_TARGET)/server_ut

server_ut_run: server_ut
	$(SERVER_TARGET)/server_ut $(TESTFLAG)

server_ut_valgrind_run: server_ut
	valgrind --leak-check=full --show-leak-kinds=all -v $(SERVER_TARGET)/server_ut $(TESTFLAG)

$(SERVER_OBJECTS): $(BUILDDIR)/%.cpp.o : %.cpp
	@mkdir -p $(@D)
	@echo "Building SOURCE" $@
	@$(CC) $(CFLAGS) $(SERVER_INCDIR_GCC) -g -c $(patsubst $(BUILDDIR)/%.cpp.o,%.cpp,$@) -o $@

$(SERVER_TESTS_OBJECTS): $(BUILDDIR)/%.cpp.o : %.cpp
	@mkdir -p $(@D)
	@echo "Building TEST" $@
	@$(CC) $(CFLAGS) $(SERVER_TESTINCDIR_GCC) -g -c $(patsubst $(BUILDDIR)/%.cpp.o,%.cpp,$@) -o $@

## TARGET GOV #########################################################################

server_ut_gcov: $(GTEST) $(SERVER_OBJECTS_GCOV) $(SERVER_TESTS_OBJECTS_GCOV)
	@mkdir -p $(SERVER_TARGET)
	@echo Linking $(SERVER_TARGET)/server_ut
	@$(CC) $(GTEST) $(SERVER_UT_LD_GCC) $(SERVER_OBJECTS_GCOV) $(SERVER_TESTS_OBJECTS_GCOV) -lgcov --coverage -o $(SERVER_TARGET)/server_ut_gcov

server_ut_gcov_run: server_ut_gcov
	$(SERVER_TARGET)/server_ut_gcov $(TESTFLAG)

$(SERVER_OBJECTS_GCOV): $(BUILDDIR_GCOV)/%.cpp.o : %.cpp
	@mkdir -p $(@D)
	@echo "Building SOURCE_GCOV" $@
	@$(CC) $(CFLAGS) -fprofile-arcs -ftest-coverage --coverage $(SERVER_INCDIR_GCC) -lgcov -c $(patsubst $(BUILDDIR_GCOV)/%.cpp.o,%.cpp,$@) -o $@

$(SERVER_TESTS_OBJECTS_GCOV): $(BUILDDIR_GCOV)/%.cpp.o : %.cpp
	@mkdir -p $(@D)
	@echo "Building TEST" $@
	@$(CC) $(CFLAGS) $(SERVER_TESTINCDIR_GCC) -fprofile-arcs -ftest-coverage --coverage -lgcov -c $(patsubst $(BUILDDIR_GCOV)/%.cpp.o,%.cpp,$@) -o $@

## MISC #########################################################################

clean_server_ut:
	echo cleaning ut
	find $(BUILDDIR)/$(SERVER_UTDIR) -type f -name *.cpp.o -exec rm {} \;

clean_server:
	echo cleaning server
	find $(BUILDDIR)/$(SERVER_SRCDIR) -type f -name *.cpp.o -exec rm {} \;

clean_gcov:
	echo cleaning GCOV
	find $(BUILDDIR_GCOV) -type f -name *.cpp.o -exec rm {} \;
	find $(BUILDDIR_GCOV) -type f -name *.gcda -exec rm {} \;
	find $(BUILDDIR_GCOV) -type f -name *.gcno -exec rm {} \;

clean: clean_server_ut clean_server clean_gcov
	echo All cleaned
