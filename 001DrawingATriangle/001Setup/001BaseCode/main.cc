#include <vulkan/vulkan.h>

#include <iostream>
#include <stdexcept>
#include <functional>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class HelloTriangleApplication {
public:
  void Run() {
    InitWindow();
    InitVulkan();
    MainLoop();
    CleanUp();
  }

private:
  void InitWindow() {
    // GLFWの初期化
    if (glfwInit() == GL_FALSE) {
      throw "Err Init GLFW";
    }

    // OpenGLのコンテクストを作らない
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // ひとまずウィンドウサイズの変更を無効化
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH_, HEIGHT_, "Vulkan", nullptr, nullptr);
  }
  void InitVulkan() {
  }

  void MainLoop() {
    while (!glfwWindowShouldClose(window)) {
      //マウス操作などのイベントを取り出し記録する
      glfwPollEvents();
    }
  }

  void CleanUp() {
    if (window != nullptr) glfwDestroyWindow(window);
    glfwTerminate();
  }

  const int WIDTH_  = 800;
  const int HEIGHT_ = 600;

  GLFWwindow* window = nullptr;
};

int main() {
  HelloTriangleApplication app;

  try {
    app.Run();
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  return 0;
}