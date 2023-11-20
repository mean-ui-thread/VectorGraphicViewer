cmake_minimum_required(VERSION 3.20)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(copy_asset)

if(NOT CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    hunter_add_package(SDL2)
    find_package(SDL2 CONFIG REQUIRED)

    hunter_add_package(SDL_image)
    find_package(SDL_image CONFIG REQUIRED)
endif()

hunter_add_package(glm)
find_package(glm REQUIRED)

add_executable(${PROJECT_NAME} WIN32

    AbstractSample.h
    AttributeInfo.h
    CPUUsage.cpp
    CPUUsage.h
    main.cpp
    MemoryUsage.cpp
    MemoryUsage.h
    Sample00_Welcome.cpp
    Sample00_Welcome.h
    Sample01_Texture.cpp
    Sample01_Texture.h
    SampleData.h
    Shader.cpp
    Shader.h
    ShaderProgram.cpp
    ShaderProgram.h
    Texture.cpp
    Texture.h
    VertexBuffer.h
    ViewerApp.cpp
    ViewerApp.h

    # Third party sources
    glad/src/glad.c
    imgui/backends/imgui_impl_opengl3.cpp
    imgui/backends/imgui_impl_opengl3.h
    imgui/backends/imgui_impl_sdl2.cpp
    imgui/backends/imgui_impl_sdl2.h
    imgui/imconfig.h
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_internal.h
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui.cpp
    imgui/imgui.h
    imgui/imstb_rectpack.h
    imgui/imstb_textedit.h
    imgui/imstb_truetype.h
    implot/implot_demo.cpp
    implot/implot_internal.h
    implot/implot_items.cpp
    implot/implot.cpp
    implot/implot.h

)

copy_asset(${PROJECT_NAME} assets/background.jpg)

if(CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    copy_asset(${PROJECT_NAME}
        assets/default_100_es.frag
        assets/default_100_es.vert
    )
else()
    copy_asset(${PROJECT_NAME}
        assets/default_330_core.frag
        assets/default_330_core.vert
    )
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

if(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_FRAMEWORK_IDENTIFIER org.mean-ui-thread.${PROJECT_NAME}
    )
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/implot
    ${CMAKE_CURRENT_SOURCE_DIR}/glad/include
)

if(CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    target_compile_options(${PROJECT_NAME} PRIVATE
        "SHELL:-s USE_SDL=2"
        "SHELL:-s USE_SDL_IMAGE=2"
        "SHELL:-s SDL2_IMAGE_FORMATS='[\"png\",\"jpg\"]'"
        "SHELL:-s WASM=1"
        "SHELL:-s ALLOW_MEMORY_GROWTH=1"
        --no-heap-copy
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        "SHELL:-s USE_SDL=2"
        "SHELL:-s USE_SDL_IMAGE=2"
        "SHELL:-s SDL2_IMAGE_FORMATS='[\"png\",\"jpg\"]'"
        --preload-file ${PROJECT_BINARY_DIR}/assets@./assets
    )
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")

else()
    target_link_libraries(${PROJECT_NAME}
        PRIVATE SDL2::SDL2
        SDL2::SDL2main
        SDL_image::SDL_image
    )
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE glm)
