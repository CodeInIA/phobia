cmake_minimum_required(VERSION 3.22)

set(PROJECT_NAME phobia)
set(CODE_PATH src)
set(CMAKE_CXX_STANDARD 14)
set(EXECUTABLE_OUTPUT_PATH ../binary)

project(${PROJECT_NAME})

# Platform-specific configuration
if(WIN32)
    # Windows (MSYS64/MinGW)
    include_directories(
        ${CODE_PATH}
        C:/msys64/ucrt64/mingw64/include
    )
    
    link_directories(
        C:/msys64/ucrt64/mingw64/lib
    )
    
    set(PLATFORM_LIBS opengl32 glu32 glew32 glfw3 assimp freeimage)
    
elseif(UNIX)
    # Linux
    include_directories(
        ${CODE_PATH}
        /usr/include
    )
    
    set(PLATFORM_LIBS GL GLEW glfw assimp freeimage)
endif()

# Build executable
file(GLOB_RECURSE CODE_FILES ${CODE_PATH}/*.*)
add_executable(${PROJECT_NAME} ${CODE_FILES})
target_link_libraries(${PROJECT_NAME} ${PLATFORM_LIBS})