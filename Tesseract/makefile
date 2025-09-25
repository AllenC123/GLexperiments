SHELL=/bin/bash

LINKFLAGS := -lGL -lGLU -lglut -lm
WARNFLAGS := -Wall -Wextra -Wpedantic
CODEFILES := $(wildcard *.cpp)
OBJ_FILENAMES := $(patsubst %.cpp,%.o,$(CODEFILES))
# for older GPUs, enable accumulation buffer flag: '-DACCUMULATION_BUFFER'

# using -O3 might actually be detrimental?
tesseractski: $(OBJ_FILENAMES)
	g++ -std=c++23 -O2 ${WARNFLAGS} $^ -o $@ $(LINKFLAGS)

%.o: %.cpp
	g++ -std=c++23 -O2 ${WARNFLAGS} -c $< $(LINKFLAGS)

.PHONY: run
run: tesseractski
#	GLUT_FPS=2500 ./tesseractski&
	./tesseractski&

.PHONY: test
test:
	@echo $(LINKFLAGS)
	@echo $(CODEFILES)
	@echo $(OBJ_FILENAMES)

.PHONY: clean
clean:
	rm $(OBJ_FILENAMES)
	rm tesseractski
