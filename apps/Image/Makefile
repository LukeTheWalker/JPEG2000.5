SHELL := /bin/bash

IDIR = include
SDIR = src
BINDIR = bin
ODIR = obj
OUTDIR = output

TRIDIR = ../../include
TRLIBPATH  = ../../lib
TRLIB = tr

SUBDIRS = $(IDIR) $(SDIR) $(TDIR) $(BINDIR) $(OUTDIR) $(ODIR)

CXX := g++

O_LEVEL = 3

CVWARNINGS_SUPPRESS = -Wno-deprecated-anon-enum-enum-conversion -Wno-deprecated-enum-enum-conversion

CV_FLAGS = `pkg-config --cflags opencv4` $(CVWARNINGS_SUPPRESS)
CV_LIBS = `pkg-config --libs opencv4`

# if macos add these flags
ifeq ($(shell uname), Darwin)
	CXXOMPFLAGS := -I/opt/homebrew/opt/libomp/include -Xclang -fopenmp
	LOMPFLAGS := -L/opt/homebrew/opt/libomp/lib -lomp
else
	CXXOMPFLAGS := -fopenmp
	LOMPFLAGS := -fopenmp
endif

NVCC := $(shell command -v nvcc 2> /dev/null)
NVCC_FLAGS :=  -I$(IDIR) -std=c++20 -arch=sm_80 -O$(O_LEVEL) -Xcompiler -Wall,-Wextra
NVCC_LIBS := -lcuda -lcudart

# if nvcc is installed, use it
ifneq ($(NVCC),)
	CUDA_ROOT_DIR = /opt/cuda
	CUDA_LIB_DIR= -L$(CUDA_ROOT_DIR)/lib64
	# CUDA include directory:
	CUDA_INC_DIR= -I$(CUDA_ROOT_DIR)/include
	# CUDA linking libraries:
	CUDA_LINK_LIBS= -lcudart
endif

CXXFLAGS= -I$(IDIR) -std=c++20 -g -O$(O_LEVEL) -Wall -Wextra $(CXXOMPFLAGS) $(CV_FLAGS) -I$(TRIDIR) 
LDFLAGS = $(LOMPFLAGS) $(CV_LIBS) -L$(TRLIBPATH) -l$(TRLIB) $(CUDA_LIB_DIR) $(CUDA_LINK_LIBS)

DEPS = $(IDIR)/$(wildcard *.hpp *.cuh)

_CXXFILES = $(wildcard $(SDIR)/*.cpp)
CXXFILES = $(notdir $(_CXXFILES))

_OBJ = $(_CXXFILES:.cpp=.o)
OBJ = $(patsubst $(SDIR)/%,$(ODIR)/%,$(_OBJ))

TARGET := $(BINDIR)/app

build: subdirs $(TARGET)

run: build $(TARGET)
	time ./$(TARGET)

$(TARGET): $(OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(ODIR)/%.o: $(SDIR)/%.cpp $(DEPS)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

$(ODIR)/test/%.o: $(TDIR)/%.cpp $(DEPS)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

clean:
	rm -f $(ODIR)/*.o $(TARGET)

.PHONY: clean run subdirs

subdirs: | $(SUBDIRS)

$(SUBDIRS):
	mkdir -p $@