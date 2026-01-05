cmake_minimum_required(VERSION 3.22)

set(PROJECT_NAME phobia)
set(CODE_PATH src)
set(CMAKE_CXX_STANDARD 14)
set(EXECUTABLE_OUTPUT_PATH ../binary)

project(${PROJECT_NAME})

# Set library paths
set(GLEW_PATH      libraries/glew)
set(GLFW_PATH      libraries/glfw)
set(GLM_PATH       libraries/glm)
set(ASSIMP_PATH    libraries/assimp)
set(FREEIMAGE_PATH libraries/freeimage)

# Include directories
include_directories(
    ${CODE_PATH}
    ${GLEW_PATH}/include
    ${GLFW_PATH}/include
    ${GLM_PATH}/include
    ${ASSIMP_PATH}/include
    ${FREEIMAGE_PATH}/include
)

# Link directories
link_directories(
    ${GLEW_PATH}/lib
    ${GLFW_PATH}/lib
    ${GLM_PATH}/lib
    ${ASSIMP_PATH}/lib
    ${FREEIMAGE_PATH}/lib
)

# Platform-specific libraries
if(WIN32)
    set(PLATFORM_LIBS opengl32 glu32 glew32 glfw3 assimp freeimage)
elseif(UNIX)
    set(PLATFORM_LIBS GL GLU GLEW glfw assimp freeimage)
endif()

# Build executable
file(GLOB_RECURSE CODE_FILES ${CODE_PATH}/*.cpp ${CODE_PATH}/*.h)
add_executable(${PROJECT_NAME} ${CODE_FILES})
target_link_libraries(${PROJECT_NAME} ${PLATFORM_LIBS})

# Copy DLLs to binary directory (Windows)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/libraries/dlls
        ${EXECUTABLE_OUTPUT_PATH}
)