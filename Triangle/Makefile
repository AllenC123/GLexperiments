MAKEFLAGS += -j

CXX = g++
CXXFLAGS := -std=c++23 -O3 -pipe -Wall -Wextra -Wpedantic -Wfatal-errors
CXXFLAGS += -Wno-unused-parameter -Wno-unused-variable
LDFLAGS := -lGL -lglfw

target_executable := triangle
SOURCES = $(wildcard *.cpp)
OBJECTFILE_DIR = build/objects
OBJFILES := $(patsubst %.cpp, $(OBJECTFILE_DIR)/%.o, $(SOURCES))
DEPFILES := $(OBJFILES:.o=.d)


.PHONY: default
default: ${target_executable}

# build directories
SUBDIRS := build/objects
.PHONY: subdirs
subdirs: $(SUBDIRS)
$(SUBDIRS):
	@mkdir --verbose --parents $@
# '--parents' also prevents errors if it already exists


${target_executable}: ${OBJFILES} | ${SUBDIRS}
	${CXX} ${CXXFLAGS} ${OBJFILES} -o $@ ${LDFLAGS}

$(OBJECTFILE_DIR)/%.o: %.cpp Makefile | ${SUBDIRS}
	$(CXX) $(CXXFLAGS) -MMD -c $< -o $@


.PHONY: clean
clean:
	@-rm --verbose ${target_executable} 2> /dev/null || true
	@-rm --verbose ${OBJFILES} 2> /dev/null || true
	@-rm --verbose ${DEPFILES} 2> /dev/null || true


-include $(DEPFILES)
