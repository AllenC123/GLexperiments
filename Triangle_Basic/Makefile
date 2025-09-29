MAKEFLAGS += -j --no-builtin-rules --no-builtin-variables --warn-undefined-variables
#make --recon --debug=v --trace --no-builtin-rules --no-builtin-variables --warn-undefined-variables

target_executable := triangle

CXX := g++
CXXFLAGS := -std=c++23 -O2 -pipe
CXXFLAGS += -Wno-unused-parameter
LINKFLAGS := -lGL -lglfw
WARNFLAGS := -Wall -Wextra -Wpedantic -Wfatal-errors

# completely disables shaders; shader-related functions and variables will not be compiled
#CXXFLAGS += -DDISABLE_SHADERS

# build directories
OBJECTFILE_DIR := build/objects
DEPFILE_SUBDIR := build/depfile
SUBDIRS := ${OBJECTFILE_DIR} ${DEPFILE_SUBDIR}
SOURCES := $(wildcard *.cpp)
OBJFILES := $(patsubst %.cpp, $(OBJECTFILE_DIR)/%.o, $(SOURCES))
DEPFILES := $(patsubst $(OBJECTFILE_DIR)/%.o, $(DEPFILE_SUBDIR)/%.d, $(OBJFILES))


.PHONY: default
default: ${target_executable}


.PHONY: subdirs
subdirs: $(SUBDIRS)
$(SUBDIRS):
	@mkdir --verbose --parents $@
# '--parents' also prevents errors if it already exists


${target_executable}: ${OBJFILES} | ${SUBDIRS}
	${CXX} ${CXXFLAGS} ${WARNFLAGS} ${OBJFILES} -o $@ ${LINKFLAGS}

${OBJECTFILE_DIR}/%.o: %.cpp Makefile | ${SUBDIRS}
	${CXX} ${CXXFLAGS} ${WARNFLAGS} -MMD -MF "${DEPFILE_SUBDIR}/$(*F).d" -c $< -o $@


.PHONY: clean
clean:
	@-rm --verbose ${target_executable} 2> /dev/null || true
	@-rm --verbose ${OBJFILES} 2> /dev/null || true
	@-rm --verbose ${DEPFILES} 2> /dev/null || true


-include $(DEPFILES)
