cmake_minimum_required(VERSION 3.14)

if (APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.12" CACHE STRING "Minimum OS X deployment version" FORCE)
    find_program(BREW_EXECUTABLE brew HINTS /usr/local/bin)
    if ("${BREW_EXECUTABLE}" STREQUAL "BREW_EXECUTABLE-NOTFOUND")
        message(STATUS "macOS: Homebrew not found.")
    else ()
        message(STATUS "macOS: Homebrew found at ${BREW_EXECUTABLE}.")
        execute_process(COMMAND ${BREW_EXECUTABLE} --prefix qt OUTPUT_VARIABLE QT_PREFIX_PATH)
        string(REGEX REPLACE "\n$" "" QT_PREFIX_PATH_STRIPPED "${QT_PREFIX_PATH}")
        if ("${QT_PREFIX_PATH}" STREQUAL "")
            message(STATUS "macOS: Homebrew: Qt not installed.")
        else ()
            list(APPEND CMAKE_PREFIX_PATH "${QT_PREFIX_PATH_STRIPPED}")
            message(STATUS "macOS: Homebrew Qt at ${QT_PREFIX_PATH_STRIPPED}.")
        endif ()
    endif ()
endif ()

set(QTCREATOR_SRC "" CACHE PATH "Path to Qt Creator sources")
if (NOT WIN32)
    set(QTCREATOR_BIN "" CACHE FILEPATH "Path to Qt Creator binary")
endif ()
set(QTCREATOR_VERSION "4.9.0" CACHE STRING "Target version of Qt Creator")

if (NOT EXISTS "${QTCREATOR_SRC}/src/qtcreatorplugin.pri")
    message(FATAL_ERROR "QTCREATOR_SRC must point to Qt Creator sources.")
endif ()

if (NOT WIN32 AND NOT EXISTS "${QTCREATOR_BIN}")
    message(FATAL_ERROR "QTCREATOR_BIN must point to the Qt Creator executable.")
endif ()

get_filename_component(QTCREATOR_BIN_DIR "${QTCREATOR_BIN}" DIRECTORY)

enable_language(CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)

cmake_policy(SET CMP0020 NEW)
cmake_policy(SET CMP0071 NEW)

find_package(Qt5 COMPONENTS Gui REQUIRED)

project(icontheme LANGUAGES CXX VERSION 0.1.0)

configure_file("${CMAKE_SOURCE_DIR}/icontheme.json.in" "${CMAKE_CURRENT_BINARY_DIR}/icontheme.json")

add_library(icontheme SHARED icontheme.qrc plugin.cpp)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "(Apple)?[Cc]lang" AND NOT MSVC)
    list(APPEND COMPILER_WARNINGS
        "-Weverything"
        "-Wno-c++98-compat"
        "-Wno-c++98-compat-pedantic"
        "-Wno-padded"
    )
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    list(APPEND COMPILER_WARNINGS
        "-Wall"
        "-Wcast-align"
        "-Wcast-qual"
        "-Wconversion"
        "-Wctor-dtor-privacy"
        "-Wdisabled-optimization"
        "-Wdouble-promotion"
        "-Wduplicated-branches"
        "-Wduplicated-cond"
        "-Wextra"
        "-Wformat=2"
        "-Winvalid-pch"
        "-Wlogical-op"
        "-Wmisleading-indentation"
        "-Wmissing-include-dirs"
        "-Wnoexcept"
        "-Wnon-virtual-dtor"
        "-Wnull-dereference"
        "-Wold-style-cast"
        "-Woverloaded-virtual"
        "-Woverloaded-virtual"
        "-Wpedantic"
        "-Wpointer-arith"
        "-Wredundant-decls"
        "-Wshadow"
        "-Wshift-overflow=2"
        "-Wsign-conversion"
        "-Wsized-deallocation"
        "-Wtrampolines"
        "-Wundef"
        "-Wunused"
        "-Wuseless-cast"
        "-Wvector-operation-performance"
        "-Wwrite-strings"
        "-Wzero-as-null-pointer-constant"
        "-pedantic-errors"
        )
else ()
    foreach (flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        string(REPLACE "/W3" "" ${flag_var} "${${flag_var}}")
    endforeach()
    list(APPEND COMPILER_WARNINGS
        "/W4"
    )
endif ()
string(REPLACE ";" " " COMPILER_WARNINGS_STR "${COMPILER_WARNINGS}")

get_target_property(icontheme_SOURCES icontheme SOURCES)

foreach (icontheme_SOURCE ${icontheme_SOURCES})
    set_source_files_properties(${icontheme_SOURCE} PROPERTIES COMPILE_FLAGS "${COMPILER_WARNINGS_STR}")
endforeach ()

set_target_properties(icontheme PROPERTIES AUTOMOC ON AUTORCC ON)

target_include_directories(icontheme SYSTEM PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${Qt5Gui_INCLUDE_DIRS}"
    "${QTCREATOR_SRC}/src/libs"
    "${QTCREATOR_SRC}/src/plugins"
    )

if (APPLE)
    find_library(APPKIT_LIB NAMES AppKit)
    find_library(EXTENSIONSYSTEM_LIB NAMES ExtensionSystem PATHS "${QTCREATOR_BIN_DIR}/../Frameworks")
    set(QTGUI_LIB "-framework QtGui")
    set(QTCORE_LIB "-framework QtCore")
    target_link_options(icontheme PRIVATE
        -Wl,-install_name,@rpath/PlugIns/
        -compatibility_version "${QTCREATOR_VERSION}"
        -iframework "${QTCREATOR_BIN_DIR}/../Frameworks"
    )
    target_link_libraries(icontheme PRIVATE
        "${APPKIT_LIB}"
    )
elseif (UNIX)
    find_library(EXTENSIONSYSTEM_LIB NAMES ExtensionSystem PATHS "${QTCREATOR_BIN_DIR}/../lib/qtcreator")
    set(QTGUI_LIB "${Qt5Gui_LIBRARIES}")
    set(QTCORE_LIB "${Qt5Core_LIBRARIES}")
    set_target_properties(icontheme PROPERTIES INSTALL_RPATH_USE_LINK_PATH ON)
else ()
    find_library(EXTENSIONSYSTEM_LIB NAMES ExtensionSystem4 PATHS "${CMAKE_SOURCE_DIR}/lib/qtcreator")
    set(QTGUI_LIB "${Qt5Gui_LIBRARIES}")
    set(QTCORE_LIB "${Qt5Core_LIBRARIES}")
endif ()

target_link_libraries(icontheme PRIVATE
    "${EXTENSIONSYSTEM_LIB}"
    "${QTGUI_LIB}"
    "${QTCORE_LIB}"
)

if (APPLE)
    install(TARGETS icontheme DESTINATION "${QTCREATOR_BIN_DIR}/../PlugIns")
    install(CODE "execute_process(COMMAND \"sudo xattr -rd com.apple.quarantine ${QTCREATOR_BIN_DIR}/../../\")")
elseif (UNIX)
    install(TARGETS minimap DESTINATION "${QTCREATOR_BIN_DIR}/../lib/qtcreator/plugins")
else ()
    file(TO_CMAKE_PATH $ENV{LOCALAPPDATA} LOCALAPPDATA_PATH)
    if ("${LOCALAPPDATA_PATH}" STREQUAL "")
        message(FATAL_ERROR "%LOCALAPPDATA% not set - plugin cannot be installed")
    endif ()
    set(PLUGINDIR_PATH "${LOCALAPPDATA_PATH}/QtProject/QtCreator/plugins/${QTCREATOR_VERSION}")
    if (NOT IS_DIRECTORY "${PLUGINDIR_PATH}")
        file(MAKE_DIRECTORY "${PLUGINDIR_PATH}")
    endif ()
    install(TARGETS icontheme RUNTIME DESTINATION "${PLUGINDIR_PATH}")
endif ()
