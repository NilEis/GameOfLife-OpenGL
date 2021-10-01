CC = gcc
SRC = src
SRCS = $(wildcard $(SRC)/*.c)
OBJ = obj
OBJS = $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SRCS))
FLAGS = -O3 -Wall -fopenacc -fopenacc-dim=1024:1:64 -fopt-info-optimized-omp -fopenmp
#FLAGS += -DFREEGLUT_STATIC
LFLAGS = -lfreeglut -lopengl32 -lglu32 -s#-Wl,--subsystem,windows
#LFLAGS = -lfreeglut_static -lopengl32 -lglu32 -lwinmm -lgdi32 #-Wl,--subsystem,windows -s
BINDIR = bin
BIN = $(BINDIR)/opengl

ifeq ($(OS),Windows_NT)
RM = del /Q /F
CP = copy /Y
ifdef ComSpec
SHELL := $(ComSpec)
endif
ifdef COMSPEC
SHELL := $(COMSPEC)
endif
else
RM = rm -rf
CP = cp -f
endif

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(OBJS) -o $@ $(FLAGS) $(LFLAGS)

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) $(FLAGS) -c $< -o $@

.PHONY : clean acc

ifeq ($(OS),Windows_NT)
OBJS := $(subst /,\, $(OBJS))
BIN := $(subst /,\, $(BIN))
endif
clean:
	$(RM) $(BIN).* $(OBJS)
