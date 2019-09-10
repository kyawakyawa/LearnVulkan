#include <vulkan/vulkan.h>

#include <functional>
#include <iostream>
#include <stdexcept>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#ifdef NDEBUG
constexpr bool kEnableOutput = false;
#else
constexpr bool kEnableOutput = true;
#endif

// Vulkan のAPIが返す引数をもらってエラーチェックを行う
#define VK_CHECK_RESULT(f, m)                                           \
  {                                                                     \
    VkResult res = (f);                                                 \
    if (res != VK_SUCCESS) {                                            \
      fprintf(stderr, "Fatal : VkResult is %d in %s at line %d\n", res, \
              __FILE__, __LINE__);                                      \
      throw std::runtime_error(m);                                      \
    }                                                                   \
  }

class HelloTriangleApplication {
public:
  void Run() {
    InitWindow();
    InitVulkan();
    MainLoop();
    CleanUp();
  }

private:
  void CreateInstance() {
    // アプリケーションの情報を入力
    VkApplicationInfo app_info  = {};
    app_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName   = "Hello Triangle";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName        = "No Engine";
    app_info.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion         = VK_API_VERSION_1_0;

    // インスタンス作成時の情報を入力
    VkInstanceCreateInfo create_info = {};
    create_info.sType                = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo     = &app_info;

    // GLFWの拡張を出力する
    uint32_t glfw_extension_count = 0;
    const char** glfw_extensions;
    glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
    create_info.enabledExtensionCount   = glfw_extension_count;
    create_info.ppEnabledExtensionNames = glfw_extensions;

    if constexpr (kEnableOutput) {
      fprintf(stderr, "---GLFW 拡張---\n");
      for (uint32_t i = 0; i < glfw_extension_count; i++) {
        fprintf(stderr, "%s\n", glfw_extensions[i]);
      }
    }

    // レイヤーの数 (次でやるの今は0)
    create_info.enabledLayerCount = 0;

    VK_CHECK_RESULT(
        vkCreateInstance(
            /*const VkInstanceCreateInfo *pCreateInfo=*/&create_info,
            /*const VkAllocationCallbacks *pAllocator=*/nullptr,
            /*VkInstance *pInstance                  =*/&instance_),
        "Instanceの作成に失敗しました");

    if constexpr (kEnableOutput) {
      fprintf(stderr, "\n---インスタンスを作成しました---\n\n");
    }

    uint32_t extension_count = 0;
    // 拡張機能のプロパティを取得する
    vkEnumerateInstanceExtensionProperties(
        /*const char *pLayerName            =*/nullptr,
        /*uint32_t *pPropertyCount          =*/&extension_count,
        /*VkExtensionProperties *pProperties=*/nullptr);
    std::vector<VkExtensionProperties> extension_properties(extension_count);
    vkEnumerateInstanceExtensionProperties(
        /*const char *pLayerName            =*/nullptr,
        /*uint32_t *pPropertyCount          =*/&extension_count,
        /*VkExtensionProperties *pProperties=*/extension_properties.data());
    if constexpr (kEnableOutput) {
      fprintf(stderr, "---拡張機能 リスト---\n");
      for (const VkExtensionProperties& prop : extension_properties) {
        fprintf(stderr, "\t* %s\n", prop.extensionName);
      }
      fprintf(stderr, "\n\n");
    }
  }
  void InitWindow() {
    // GLFWの初期化
    if (glfwInit() == GL_FALSE) {
      throw std::runtime_error("GLFWの初期化に失敗しました");
    }

    // OpenGLのコンテクストを作らない
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // ひとまずウィンドウサイズの変更を無効化
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH_, HEIGHT_, "Vulkan", nullptr, nullptr);
  }
  void InitVulkan() { CreateInstance(); }

  void MainLoop() {
    while (!glfwWindowShouldClose(window)) {
      //マウス操作などのイベントを取り出し記録する
      glfwPollEvents();
    }
  }

  void CleanUp() {
    vkDestroyInstance(instance_, nullptr);
    if (window != nullptr) glfwDestroyWindow(window);
    glfwTerminate();
  }

  const int WIDTH_  = 800;
  const int HEIGHT_ = 600;

  GLFWwindow* window = nullptr;

  VkInstance instance_;
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
