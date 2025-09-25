SHELL=/bin/bash

LINKFLAGS := -lGL -lGLU -lglut -lm
WARNFLAGS := -Wall -Wextra -Wpedantic
CODEFILES := $(wildcard *.cpp)
OBJ_FILENAMES := $(patsubst %.cpp,%.o,$(CODEFILES))


cylinder: $(OBJ_FILENAMES)
	g++ -std=c++23 -O2 ${WARNFLAGS} $^ -o $@ $(LINKFLAGS)

%.o: %.cpp
	g++ -std=c++23 -O2 ${WARNFLAGS} -c $< $(LINKFLAGS)

.PHONY: test
test:
	@echo $(LINKFLAGS)
	@echo $(CODEFILES)
	@echo $(OBJ_FILENAMES)

.PHONY: clean
clean:
	rm $(OBJ_FILENAMES)
	rm cylinder
