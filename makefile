OS = $(shell uname -o)

# Directories and such
BLDDIR = build
SRCDIR = src
OBJDIR = $(BLDDIR)/obj

GAME = slidingtiles

# Compiler/flags
CC = gcc
CFLAGS = -Wall -std=c99 -g
LDFLAGS = 

ifeq ($(OS),GNU/Linux)
  CFLAGS += -DUNIX
  LDFLAGS += lglfw -lGL -lGLU -lm -Wl,-rpath,.
else
	CFLAGS += -DGLFW_DLL -DWIN32
  LDFLAGS += -lglfwdll -lopengl32 -lglu32 -lmingw32 -mwindows
endif

# Files
SRCS = $(wildcard $(SRCDIR)/soil/*.c) $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))

# Suffixes
.SUFFIXES:
.SUFFXIES: .o .c .h

# Targets
all: $(BLDDIR)/$(GAME)

$(BLDDIR)/$(GAME): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) -o $@ $^

run:
	./$(BLDDIR)/$(GAME)

clean:
	rm -rf $(BLDDIR)
