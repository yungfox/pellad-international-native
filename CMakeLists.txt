# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required(VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project(pellad-international)

if(WIN32)
    execute_process(COMMAND cmd "/c get_windows_dependencies.bat" WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endif()

set(SOURCES
    main.c
    include/glad/glad.c
)

add_executable(pellad-international ${SOURCES})
target_include_directories(pellad-international PRIVATE include)
target_include_directories(pellad-international PRIVATE include/glad)

if(WIN32)
    add_compile_options(/W3)
    set(LIBS opengl32 gdi32 ${PROJECT_SOURCE_DIR}/lib/freetype2/win64/freetype.lib)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    target_include_directories(pellad-international PRIVATE include/freetype2)
    set(CMAKE_C_FLAGS "/utf-8")
else()
    add_compile_options(-Wall -Wextra)
    find_package(Freetype REQUIRED)
    target_include_directories(pellad-international PRIVATE ${FREETYPE_INCLUDE_DIRS})
    target_link_libraries(pellad-international PRIVATE ${FREETYPE_LIBRARIES})

    if(APPLE)
        set(LIBS "-framework OpenGL -framework Cocoa")
    else()
        set(LIBS X11 GL Xrandr)
    endif()
endif()

target_link_libraries(pellad-international PRIVATE ${LIBS})
