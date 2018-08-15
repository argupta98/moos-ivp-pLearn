#--------------------------------------------------------
# The CMakeLists.txt for:              lib_behaviors-test
# Author(s):			       Arjun Gupta                            
#--------------------------------------------------------

# Set System Specific Libraries
if (${WIN32})
  # Windows Libraries
  SET(SYSTEM_LIBS
      )
else (${WIN32})
  # Linux and Apple Libraries
  SET(SYSTEM_LIBS
      m )
endif (${WIN32})


#-------------------------------------------------------
#						BHV_Input
#-------------------------------------------------------
ADD_LIBRARY(BHV_Input SHARED 
   BHV_Input.cpp  NodeReportDefense.cpp)
   
TARGET_LINK_LIBRARIES(BHV_Input
   helmivp
   behaviors 
   ivpbuild 
   logic 
   ivpcore 
   bhvutil 
   mbutil
   ufield
   geometry
   python
   python2.7
   dl
   ${SYSTEM_LIBS}
   ${PYTHON_LIBRARIES})

Find_package(PythonLibs REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})

