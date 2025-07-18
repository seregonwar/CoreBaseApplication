find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(cba_gui main.cpp GuiManager.cpp)

target_link_libraries(cba_gui PRIVATE core imgui::imgui glfw OpenGL::GL)
add_definitions(-DUSE_PYTHON_BINDINGS)