# --- Choose CMake version ---
cmake_minimum_required(VERSION 3.0)

# --- Choose project name ---
project(HelloWorld)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set (CMAKE_AUTOMOC ON)

# --- Find and include Qt5 package ---
find_package(Qt5 COMPONENTS Widgets OpenGL Gui REQUIRED)
include_directories(${Qt5Widgets_INCLUDES} ${Qt5OpenGL_INCLUDES})

# --- Find and include WebP package ---
find_package(PkgConfig REQUIRED)
pkg_check_modules(WEBP REQUIRED libwebp)
include_directories(${WEBP_INCLUDE_DIRS})

# --- Find and include tinyGltf package ---
option(TINYGLTF_HEADER_ONLY "Use header only version" ON)
option(TINYGLTF_VALID_JSON "Enable JSON validation" OFF)
option(TINYGLTF_NO_STB_IMAGE "Disable STB Image loading" OFF)
option(TINYGLTF_NO_STB_IMAGE_WRITE "Disable STB Image writing" OFF)

add_subdirectory(lib/tinygltf-release)


# --- Add header files ---
set(HEADER_FILES
    src/Utilitaire/gltfLoader.h
    src/Utilitaire/ShaderManager.h
    src/Utilitaire/BlinnPhongIlluminationModel.h
    src/Utilitaire/DepthPeelingEffect.h
    src/Utilitaire/DynamicShaderBuilder.h
    src/Utilitaire/IShaderModule.h
    src/Widgets/TriangleWidget.h
    src/Widgets/MixWidget.h
    src/Widgets/ShaderWidget.h
    src/Cameras/TrackBall.h
    src/Cameras/Freefly.h

)

# --- Add source files ---
set(SOURCES_FILES
    src/main.cpp
    src/Utilitaire/gltfLoader.cpp
    src/Utilitaire/ShaderManager.cpp
    src/Utilitaire/BlinnPhongIlluminationModel.cpp
    src/Utilitaire/DepthPeelingEffect.cpp
    src/Utilitaire/DynamicShaderBuilder.cpp
    src/Widgets/TriangleWidget.cpp
    src/Widgets/MixWidget.cpp
    src/Widgets/ShaderWidget.cpp
    src/Cameras/TrackBall.cpp
    src/Cameras/Freefly.cpp

)

add_executable(${CMAKE_PROJECT_NAME} ${HEADER_FILES} ${SOURCES_FILES})

# --- Link Qt library ---
target_link_libraries(${CMAKE_PROJECT_NAME} Qt5::Widgets Qt5::OpenGL Qt5::Gui "GL" tinygltf ${WEBP_LIBRARIES})

# --- Include directories ---
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/lib/tinygltf-release ${WEBP_INCLUDE_DIRS})

# --- Set the output folder ---
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

