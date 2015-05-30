cmake_minimum_required (VERSION 2.8.9)
 
project(scpspt)

#-------------------------------------------------------------------------------
#set up Qt5

# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)
# Tell CMake to run uic when necessary:
#set(CMAKE_AUTOUIC ON)
# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Widgets finds its own dependencies (Core and Gui).
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5SerialPort REQUIRED)

#-------------------------------------------------------------------------------
#set up compiler flags and executable names

#set(CMAKE_CXX_FLAGS "-fPIC") #or -fPIE

if(MSVC)
    set(CMAKE_DEBUG_POSTFIX "d")
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP") #multi-processor compilation
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP") #multi-processor compilation
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    #check for G++ 4.7+
    execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE G++_VERSION)
    if (G++_VERSION VERSION_LESS 4.7)
            message(SEND_ERROR "You need at least G++ 4.7 to compile scpspt!")
    endif()
    #set up compiler flags for GCC
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O2") #support C++11 for std::, optimize
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s") #strip binary
endif()

#-------------------------------------------------------------------------------
#define basic sources and headers

set(TARGET_HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/src/TerminalWidget.h
)

set(TARGET_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/src/TerminalWidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/scpspt.cpp
)

set(TARGET_FORMS
	${CMAKE_CURRENT_SOURCE_DIR}/src/scpspt.ui
)
qt5_wrap_ui(FORMS_ADDED ${TARGET_FORMS})

set(TARGET_RESOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/src/resources.qrc
)
qt5_add_resources(RESOURCE_ADDED ${TARGET_RESOURCES})

#Add source directory to include directories for building .ui files etc.
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

#-------------------------------------------------------------------------------
#set up build directories

set(dir ${CMAKE_CURRENT_SOURCE_DIR})
set(EXECUTABLE_OUTPUT_PATH ${dir} CACHE PATH "Build directory" FORCE)
set(LIBRARY_OUTPUT_PATH ${dir} CACHE PATH "Build directory" FORCE)

#-------------------------------------------------------------------------------
#define target

add_executable(scpspt ${TARGET_SOURCES} ${TARGET_HEADERS} ${RESOURCE_ADDED} ${FORMS_ADDED})
qt5_use_modules(scpspt Core Gui Widgets SerialPort)

#add Qt main libraries
if(MSVC)
    target_link_libraries (scpspt ${QT_QTMAIN_LIBRARY})
	set(CMAKE_CREATE_WIN32_EXE "/SUBSYSTEM:WINDOWS /ENTRY:\"mainCRTStartup\"")
endif()
