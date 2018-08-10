#--------------------------------------------------
# The CMakeLists.txt for:		lib_python
# Author(s): 	     			Arjun Gupta
#--------------------------------------------------

SET(SRC
  python_caller.cpp
)

SET(HEADERS
  python_caller.h
)

ADD_LIBRARY(python ${SRC})

LINK_DIRECTORIES(/System/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/config)

find_package(PythonLibs 2.7 EXACT REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})
target_link_libraries(python ${PYTHON_LIBRARIES})

