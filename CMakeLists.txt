if(WIN32)
    cmake_minimum_required(VERSION 3.1.0)
    set(CMAKE_CXX_STANDARD 11)
else()
    cmake_minimum_required(VERSION 2.8.12)
    add_compile_options(-std=c++11)
endif()

# --- BUILD CONFIGURATION ---

project(pylet)

add_definitions(-DQT_NO_KEYWORDS)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(Python_ADDITIONAL_VERSIONS 3.5)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(CORE_SOURCE
    src/pylet.cpp
)

set(GUI_SOURCE
    src/gui/pylet_window.cpp
    src/gui/pylet_window.h
    src/gui/info_box.cpp
    src/gui/info_box.h
)

set(GUI_EDITOR_SOURCE
    src/gui/editor/code_editor_interface.cpp
    src/gui/editor/code_editor_interface.h
    src/gui/editor/code_editor_numbers.h 
    src/gui/editor/code_editor_highlighter.cpp
    src/gui/editor/code_editor_highlighter.h
    src/gui/editor/editor_stack.cpp
    src/gui/editor/editor_stack.h
)

set(PYTHON_SOURCE
    src/python/qconsole.cpp
    src/python/qconsole.h
    src/python/qpyconsole.cpp
    src/python/qpyconsole.h
)

set(SOURCE
    ${CORE_SOURCE}
    ${GUI_SOURCE}
    ${GUI_EDITOR_SOURCE}
    ${PYTHON_SOURCE}
)

set(RESOURCES
    res/icons.qrc
    res/fonts.qrc
)

set(GUI_TYPE "")

if(WIN32)
    set(GUI_TYPE WIN32)
elseif(UNIX)
    if(APPLE)
        set(GUI_TYPE MACOSX_BUNDLE)
        set(MACOSX_BUNDLE_BUNDLE_NAME Pylet)
        set(MACOSX_BUNDLE_ICON_FILE pylet_temp_logo.icns)
        set(APP_ICON ${CMAKE_CURRENT_SOURCE_DIR}/res/icons/pylet_temp_logo.icns)
        set(SOURCE ${SOURCE} ${APP_ICON})
        set_source_files_properties(${APP_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    endif(APPLE)
else()
    # Suspend the configuration process if the target OS isn't recognized. 
    MESSAGE(FATAL_ERROR "Target OS not recognized or supported, aborting CMake process.")
endif()

#set(Boost_USE_STATIC_LIBS       OFF)
#set(Boost_USE_MULTITHREADED      ON)
#set(Boost_USE_STATIC_RUNTIME    OFF)

find_package(PythonLibs 3 REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
#find_package(Boost COMPONENTS python3 REQUIRED)

qt5_add_resources(UI_RESOURCES ${RESOURCES})

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${Qt5Gui_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5Core_INCLUDE_DIRS}
    ${PYTHON_INCLUDE_DIRS}
    #${Boost_INCLUDE_DIRS}
)
link_directories(
    #${Boost_LIBRARY_DIRS}
)
add_executable(pylet
    ${GUI_TYPE}
    ${SOURCE}
    ${UI_RESOURCES}
)
target_link_libraries(pylet
    Qt5::Widgets
    Qt5::Core
    Qt5::Gui
    ${PYTHON_LIBRARIES}
    #${Boost_LIBRARIES}
)
if(MSVC)
    source_group("src\\" FILES ${CORE_SOURCE})
    source_group("src\\gui" FILES ${GUI_SOURCE})
    source_group("src\\gui\\editor" FILES ${GUI_EDITOR_SOURCE})
    source_group("src\\python" FILES ${PYTHON_SOURCE})

    set(WIN_LINKFLAGS \"${CMAKE_CURRENT_SOURCE_DIR}/res/app_icon.res\")
    set_target_properties(pylet PROPERTIES LINK_FLAGS ${WIN_LINKFLAGS})
    set_target_properties(pylet PROPERTIES COMPILE_FLAGS "/EHsc")
endif()

# --- RUNTIME DEPENDENCIES ---

if(WIN32)
    cmake_policy(SET CMP0026 OLD)

    get_target_property(BuildLocation pylet LOCATION)
    get_filename_component(TargetDirectory ${BuildLocation} PATH)

    if (EXISTS ${Qt5Core_DIR}/../../../bin/icuin53.dll)
        set(ICU_VER 53)
    else()
        set(ICU_VER 54)
    endif()

    foreach(DEPENDENCY
        Qt5Widgetsd.dll
        Qt5Guid.dll
        Qt5Cored.dll
    )
        add_custom_command(TARGET pylet POST_BUILD
            # Copy the given dependency into the executable directory.
            COMMAND ${CMAKE_COMMAND} -E copy ${Qt5Core_DIR}/../../../bin/${DEPENDENCY} ${TargetDirectory}
            COMMENT "Copying ${DEPENDENCY} into '${TargetDirectory}/'" VERBATIM
        )
    endforeach()
endif()

if(APPLE)
    add_custom_command(TARGET pylet POST_BUILD
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin/pylet.app/Contents/MacOS
        COMMAND install_name_tool -change /Library/Frameworks/Python.framework/Versions/3.5/Python @executable_path/../Frameworks/Python.framework/Versions/3.5/Python pylet
    )
    add_custom_command(TARGET pylet POST_BUILD
        WORKING_DIRECTORY ${Qt5Core_DIR}/../../../bin
        COMMAND ./macdeployqt ${CMAKE_BINARY_DIR}/bin/pylet.app
    )
endif()
