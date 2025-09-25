MAKEFLAGS += -j8
CXX = g++-13
# make --recon -d --no-builtin-rules

target_executable := TORUS
IMGUI_DIR := ./imgui
IMGUI_SOURCES := $(wildcard ${IMGUI_DIR}/*.cpp)
IMGUI_SOURCES += $(wildcard ${IMGUI_DIR}/backends/*.cpp)

#SOURCES = main.cpp Torus.cpp Torus_Rendering.cpp
SOURCES = $(wildcard *.cpp)
OBJECTFILE_DIR = build/objects
OBJECTFILE_DIR_IMGUI = build/objects_imgui
#OBJFILES = $(addsuffix .o, $(basename $(notdir $(SOURCES))))
OBJFILES := $(patsubst %.cpp,$(OBJECTFILE_DIR)/%.o, $(SOURCES))
DEPFILES := $(OBJFILES:.o=.d)
OBJFILES_IMGUI := $(patsubst ${IMGUI_DIR}/%.cpp,$(OBJECTFILE_DIR_IMGUI)/%.o, $(IMGUI_SOURCES))
DEPFILES_IMGUI := $(OBJFILES_IMGUI:.o=.d)

#CXXFLAGS = -std=c++20 -O3 -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
CXXFLAGS := -std=c++20 -O3 -pipe -Wall -Wextra -Wpedantic -Wfatal-errors
#CXXFLAGS += -Wall -Wformat
LDFLAGS := -lglfw -lGL -limgui -lglut -lGLU -lpthread

PROJECT_DIR := $(shell pwd)

MAKEOPTS += -j8

# TODO:
# OBJFILES should point to or output-to the imgui/objectfiles sub-folder
# clean unnecessary header/code files out of imgui folder
# move libimgui.so location to imgui folder instead

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------


.PHONY: default
default: ${target_executable}


# build directories
SUBDIRS := build/objects build/objects_imgui build/objects_imgui/backends
.PHONY: subdirs
subdirs: $(SUBDIRS)
$(SUBDIRS):
	@mkdir --verbose --parents $@
# '--parents' also prevents errors if it already exists


${target_executable}: ${OBJFILES} libimgui.so | ${SUBDIRS}
	${CXX} -L${PROJECT_DIR} ${CXXFLAGS} ${OBJFILES} -o $@ ${LDFLAGS} -Wl,-rpath,${PROJECT_DIR}
# the linking MUST be done in a seperate step, because the compiler doesn't recognize "rpath" option
# 'rpath' hardcodes the dynamic-library location into the binary; otherwise you have to set your 'DYNAMIC_LINK_PATH'(whatever it is) env-var when launching the program
# https://www.cprogramming.com/tutorial/shared-libraries-linux-gcc.html


$(OBJECTFILE_DIR)/%.o: %.cpp Makefile | ${SUBDIRS}
	$(CXX) $(CXXFLAGS) -MMD -c $< -o $@ -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends


# compile all imgui object files with -fpic   # maybe -lglfw -lGL
$(OBJECTFILE_DIR_IMGUI)/%.o: $(IMGUI_DIR)/%.cpp | ${SUBDIRS}
	$(CXX) -fpic $(CXXFLAGS) -MMD -c $< -o $@ -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends


# create the shared library for imgui
# the name MUST be in the form "lib_.so"; otherwise the compiler won't find it with the '-l' flag
libimgui.so: ${IMGUI_SOURCES} ${OBJFILES_IMGUI} Makefile | ${SUBDIRS}
	$(CXX) -shared ${CXXFLAGS} ${OBJFILES_IMGUI} -lglfw -lGL -o $@


.PHONY: clean
clean:
	@-rm --verbose ${target_executable} 2> /dev/null || true
	@-rm --verbose ${OBJFILES} 2> /dev/null || true
	@-rm --verbose ${DEPFILES} 2> /dev/null || true
# prefixed '@' prevents make from echoing the command
# prefixed '-' causes make to ignore nonzero exit-codes (instead of aborting), but it still reports these errors:
# 		'make: [Makefile:24: clean] Error 1 (ignored)'
# which is why we've appended '|| true'; it ensures the exit-code is always 0, suppressing those messages
#
# 'rm' (even without verbose) will also print it's own additional error-messages: 
# 		'rm: cannot remove 'fluidsym_dbg': No such file or directory'
# so we pipe to '/dev/null' to suppress that as well

.PHONY: reallyclean
reallyclean: clean
	@-rm --verbose libimgui.so 2> /dev/null || true
	@-rm --verbose ${OBJFILES_IMGUI} 2> /dev/null || true
	@-rm --verbose ${DEPFILES_IMGUI} 2> /dev/null || true


# this has to be at the end of the file?
-include $(DEPFILES)
-include $(DEPFILES_IMGUI)
# Include the .d makefiles. The '-' at the front suppresses the errors of missing depfiles.
# Initially, all the '.d' files will be missing, and we don't want those errors to show up.
