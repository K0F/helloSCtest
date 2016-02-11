cmake_minimum_required(VERSION 2.8.11)

# This makes sclang/scide work with a Qt installation at a fixed location.
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

include_directories(${CMAKE_SOURCE_DIR}/include/common
                    ${CMAKE_SOURCE_DIR}/include/lang
                    ${CMAKE_SOURCE_DIR}/include/plugin_interface
                    ${CMAKE_SOURCE_DIR}/include/server
                    ${CMAKE_SOURCE_DIR}/common

                    ${YAMLCPP_INCLUDE_DIR}

                    ${CMAKE_SOURCE_DIR}/external_libraries/boost_sync/include

                    ${CMAKE_SOURCE_DIR}/lang/LangSource
                    ${CMAKE_SOURCE_DIR}/lang/LangPrimSource

                    ${CMAKE_SOURCE_DIR}/external_libraries/hidapi/hidapi
                    ${CMAKE_SOURCE_DIR}/external_libraries/hidapi/hidapi_parser

                    ${CMAKE_SOURCE_DIR}/external_libraries/nova-tt
                    ${CMAKE_SOURCE_DIR}/lang/LangSource/Bison)


if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	find_package(ALSA)
endif()

find_package(Readline 5.0)

find_package(Sndfile)

set(hello_sources helloSc.cpp)

if(APPLE)
	set_property(SOURCE ${CMAKE_SOURCE_DIR}/common/SC_DirUtils.cpp PROPERTY COMPILE_FLAGS -xobjective-c++)

	list(APPEND hello_sources ${CMAKE_SOURCE_DIR}/common/SC_Apple.mm)
	set_source_files_properties(${CMAKE_SOURCE_DIR}/common/SC_Apple.mm PROPERTIES COMPILE_FLAGS "-x objective-c++ -fobjc-exceptions")
endif()

if (NOT Boost_FOUND)
	file(GLOB boost_system_sources ../external_libraries/boost/libs/system/src/*cpp)
	file(GLOB boost_fs_sources ../external_libraries/boost/libs/filesystem/src/*cpp)
	file(GLOB boost_regex_sources ../external_libraries/boost/libs/regex/src/*cpp)
	list(APPEND hello_sources ${boost_fs_sources} ${boost_system_sources} ${boost_regex_sources})
endif()

file(GLOB_RECURSE headers ../include/*.h* )
file(GLOB_RECURSE external_headers ../external_libraries/*.h* )
list(APPEND hello_sources ${headers} ${external_headers}) # make qt creator happy

if(NOT WIN32)
	list(APPEND hello_sources LangPrimSource/PyrSerialPrim.cpp)
endif()

set(sclang_parser_source LangSource/Bison/lang11d_tab.cpp)

if(WIN32)
        list(APPEND hello_sources ${CMAKE_SOURCE_DIR}/common/SC_Win32Utils.cpp)
	include_directories (../platform/windows/compat_stuff)
        if(NOT MINGW)
                # mingw-w64 provides getopt
                list(APPEND h
                  llo_sources ../platform/windows/compat_stuff/getopt/getopt.c)
                include_directories(../platform/windows/compat_stuff/getopt)
        endif()

        list(APPEND hello_sources ${CMAKE_SOURCE_DIR}/lang/LangPrimSource/SC_PortMidi.cpp)
endif()

# This sets up the exe icon for windows under mingw.
if(MINGW)
 set(RES_FILES ${CMAKE_SOURCE_DIR}/platform/windows/Resources/sclang.rc)
 set(CMAKE_RC_COMPILER_INIT windres)
 ENABLE_LANGUAGE(RC)
 SET(CMAKE_RC_COMPILE_OBJECT
 "<CMAKE_RC_COMPILER> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
endif(MINGW)

if(SC_QT OR SC_IDE)
	set(QT_COLLIDER_LANG_CLIENT ON)
	include(../QtCollider/CMakeLists.txt)
	list(APPEND hello_sources ${QT_COLLIDER_SRCS})
endif()

if(SC_IDE)
	add_definitions(-DSC_IDE -DQT_NO_KEYWORDS)

	qt5_wrap_cpp( SCLANG_MOC_SRCS ../editors/sc-ide/primitives/sc_ipc_client.hpp )
	list(APPEND hello_sources ../editors/sc-ide/primitives/sc_ipc_client.cpp)
	list(APPEND hello_sources ${SCLANG_MOC_SRCS})
endif()

include(../SCDoc/CMakeLists.txt)
list(APPEND hello_sources ${SCDOC_SRCS})

#add_executable(helloSc ${CMAKE_SOURCE_DIR}/lang/LangSource/cmdLineFuncs.cpp ${RES_FILES})

add_executable(helloSc helloSc.cpp ${RES_FILES})
target_link_libraries(helloSc libsclang)
target_link_libraries(helloSc ${ICU_LIBRARIES})

target_compile_definitions(helloSc PUBLIC USE_SC_TERMINAL_CLIENT)

if(LTO)
	target_compile_definitions(libsclang PUBLIC -flto -flto-report)

        set_property(TARGET helloSc libsclang
					APPEND PROPERTY LINK_FLAGS "-flto -flto-report -fwhole-program")
endif()

if(APPLE)
  add_custom_command(TARGET helloSc POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:SuperCollider>/../MacOS/
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:helloSc> $<TARGET_FILE_DIR:SuperCollider>/../MacOS)
elseif(WIN32)
  install(TARGETS helloSc
			DESTINATION "SuperCollider"
			PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
elseif(NOT APPLE)
  install(TARGETS helloSc
			RUNTIME DESTINATION "bin"
			PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endif()
