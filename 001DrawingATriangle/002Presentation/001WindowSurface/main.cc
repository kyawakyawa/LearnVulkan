#include <vulkan/vulkan.h>

#include <iostream>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#ifdef NDEBUG
constexpr bool kEnableOutput           = false;
constexpr bool kEnableValidationLayers = false;
#else
constexpr bool kEnableOutput           = true;
constexpr bool kEnableValidationLayers = true;
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

struct QueueuFamilyIndices {
  std::optional<uint32_t> graphics_family;
  std::optional<uint32_t> present_family;

  bool IsComplete() {
    return graphics_family.has_value() && present_family.has_value();
  }
};

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
    // ValidationLayerが有効な時にValidation Layerがサポートされているか確認
    if (kEnableValidationLayers && !CheckValidationLayerSupport()) {
      throw std::runtime_error(
          "Validation Layerが有効ですが、見つかりませんでした");
    }

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

    // Validation Layers の情報を入れる
    VkDebugUtilsMessengerCreateInfoEXT debug_create_info;
    if constexpr (kEnableValidationLayers) {
      create_info.enabledLayerCount =
          static_cast<uint32_t>(validation_layers_.size());
      create_info.ppEnabledLayerNames = validation_layers_.data();

      // デバックメッセンジャーの情報を入力する
      PopulateDebugMessengerCreateInfo(debug_create_info);
      create_info.pNext =
          (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
    } else {
      create_info.enabledLayerCount = 0;
    }

    // 拡張機能
    auto extensions = GetRequiredExtensions();
    create_info.enabledExtensionCount =
        static_cast<uint32_t>(extensions.size());
    create_info.ppEnabledExtensionNames = extensions.data();

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
  bool CheckValidationLayerSupport() {
    uint32_t layer_count = 0;
    // nullptrを渡すとレイヤーの数を取得できる
    vkEnumerateInstanceLayerProperties(
        /*uint32_t *pPropertyCount      =*/&layer_count,
        /*VkLayerProperties *pProperties=*/nullptr);

    std::vector<VkLayerProperties> available_layers(layer_count);
    vkEnumerateInstanceLayerProperties(
        /*uint32_t *pPropertyCount      =*/&layer_count,
        /*VkLayerProperties *pProperties=*/available_layers.data());

    if (kEnableOutput) {
      fprintf(stderr, "---Layer リスト---\n");
      for (const VkLayerProperties& prop : available_layers) {
        fprintf(stderr, "\t* %s\n", prop.layerName);
      }
      fprintf(stderr, "\n\n");
    }

    for (const char* layer_name : validation_layers_) {
      bool layer_found = false;

      for (const auto& layer_properties : available_layers) {
        if (std::string(layer_name) ==
            std::string(layer_properties.layerName)) {
          layer_found = true;
          break;
        }
      }

      if (!layer_found) {
        return false;
      }
    }

    return true;
  }

  // Validation Layer用のコールバック関数
  static VKAPI_ATTR VkBool32 VKAPI_CALL
  DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                VkDebugUtilsMessageTypeFlagsEXT message_type,
                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                void* pUserData) {
    if (kEnableOutput) {
      fprintf(stderr, "validation layer: %s\n", pCallbackData->pMessage);
    }

    if (message_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
      // Message is important enough to show
    }

    return VK_FALSE;
  }

  //デバックメッセンジャーのセット
  void SetupDebugMessenger() {
    if (!kEnableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT create_info = {};
    PopulateDebugMessengerCreateInfo(create_info);

    if (CreateDebugUtilsMessengerEXT(instance_, &create_info, nullptr,
                                     &debug_messenger_) != VK_SUCCESS) {
      throw std::runtime_error("failed to set up debug messenger!");
    }
  }

  // デバックメッセンジャーの情報を入力する
  void PopulateDebugMessengerCreateInfo(
      VkDebugUtilsMessengerCreateInfoEXT& create_info) {
    create_info       = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info.pfnUserCallback = DebugCallback;
  }

  // 利用する拡張機能を指定する
  std::vector<const char*> GetRequiredExtensions() {
    uint32_t glfw_extension_count = 0;
    const char** glfw_extensions;
    glfw_extensions = glfwGetRequiredInstanceExtensions(
        /*uint32_t *count = */ &glfw_extension_count);

    std::vector<const char*> extensions(glfw_extensions,
                                        glfw_extensions + glfw_extension_count);

    if (kEnableValidationLayers) {
      extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
  }

  // デバックメッセンジャーの作成
  VkResult CreateDebugUtilsMessengerEXT(
      VkInstance instance,
      const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
      const VkAllocationCallbacks* pAllocator,
      VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        /*VkInstance instance =*/instance,
        /*const char *pName   =*/"vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
      return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
      return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
  }

  // デバックメッセンジャーの終了
  void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                     VkDebugUtilsMessengerEXT debug_messenger,
                                     const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        /*VkInstance instance =*/instance,
        /*const char *pName   =*/"vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
      func(instance, debug_messenger, pAllocator);
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

    window_ = glfwCreateWindow(WIDTH_, HEIGHT_, "Vulkan", nullptr, nullptr);
  }

  // 使用可能な物理デバイスを調べ、一つ取得する
  void PickPhysicalDevice() {
    uint32_t device_count = 0;

    vkEnumeratePhysicalDevices(
        /*VkInstance instance =*/instance_,
        /*uint32_t *pPhysicalDeviceCount =*/&device_count,
        /*VkPhysicalDevice *pPhysicalDevices =*/
        nullptr  // nullptrを渡すと数を取得できる
    );

    if (device_count == 0) {
      throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(
        /*VkInstance instance =*/instance_,
        /*uint32_t *pPhysicalDeviceCount =*/&device_count,
        /*VkPhysicalDevice *pPhysicalDevices =*/
        devices.data()  // ここで指定したポインタに物理デバイスの情報を入れる
    );

    for (const auto& device : devices) {
      if (IsDeviceSuitable(device)) {
        physical_device_ = device;
        break;
      }
    }

    if (physical_device_ == VK_NULL_HANDLE) {
      throw std::runtime_error("failed to find a suitable GPU!");
    }
  }

  bool IsDeviceSuitable(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties device_properties;
    VkPhysicalDeviceFeatures device_features;

    vkGetPhysicalDeviceProperties(device, &device_properties);
    vkGetPhysicalDeviceFeatures(device, &device_features);

    const bool condition0 =
        /* グラフィックカード */
        device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ||
        /* 統合GPU */
        device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
    if (kEnableOutput &&
        device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      fprintf(stderr, "グラフィックカードが検出されました\n");
    } else if (kEnableOutput && device_properties.deviceType ==
                                    VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
      fprintf(stderr, "統合GPUが検出されました\n");
    }
    const bool condition1 = device_features.geometryShader;

    QueueuFamilyIndices indices = FindQueueFamilies(device);

    return condition0 && condition1 && indices.IsComplete();
  }

  void CreateLogicalDevice() {
    // 作成するキューの指定
    QueueuFamilyIndices indices = FindQueueFamilies(physical_device_);

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    std::set<uint32_t> unique_queue_families = {indices.graphics_family.value(),
                                                indices.present_family.value()};

    const float queue_priority = 1.0f;  // [0.0f,1.0f]

    for (uint32_t queue_family : unique_queue_families) {
      VkDeviceQueueCreateInfo queue_create_info = {};
      queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queue_create_info.queueFamilyIndex = queue_family;
      queue_create_info.queueCount       = 1;
      queue_create_info.pQueuePriorities = &queue_priority;

      queue_create_infos.emplace_back(queue_create_info);
    }

    // 使うDevice Feature を指定する(今回は何も指定しない)
    VkPhysicalDeviceFeatures device_features = {};

    // Logical Device の作成
    VkDeviceCreateInfo create_info = {};
    create_info.sType              = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    create_info.queueCreateInfoCount =
        static_cast<uint32_t>(queue_create_infos.size());
    create_info.pQueueCreateInfos = queue_create_infos.data();

    create_info.pEnabledFeatures = &device_features;

    create_info.enabledExtensionCount = 0;

    if (kEnableValidationLayers) {
      create_info.enabledLayerCount =
          static_cast<uint32_t>(validation_layers_.size());
      create_info.ppEnabledLayerNames = validation_layers_.data();
    } else {
      create_info.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physical_device_, &create_info, nullptr, &device_) !=
        VK_SUCCESS) {
      throw std::runtime_error("failed to create logical device!");
    }

    // Graphics Queueのハンドルを取得 (graphics_queue_)
    vkGetDeviceQueue(device_, indices.graphics_family.value(), 0,
                     &graphics_queue_);
    // Presentation Queueのハンドルを取得 (present_queue_)
    vkGetDeviceQueue(device_, indices.present_family.value(), 0,
                     &present_queue_);
  }

  QueueuFamilyIndices FindQueueFamilies(VkPhysicalDevice device) {
    QueueuFamilyIndices indices;

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count,
                                             nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count,
                                             queue_families.data());

    int i = 0;
    for (const auto& queue_family : queue_families) {
      // Graphics Queue Family
      if (queue_family.queueCount > 0 &&
          queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        indices.graphics_family = i;
        break;
      }
    }

    for (const auto& queue_family : queue_families) {
      // Presentation Queue Family
      {
        VkBool32 presentation_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_,
                                             &presentation_support);
        if (queue_family.queueCount > 0 && presentation_support) {
          indices.present_family = i;
        }
        break;
      }
      i++;
    }

    return indices;
  }

  void InitVulkan() {
    CreateInstance();
    SetupDebugMessenger();
    CreateSurface();
    PickPhysicalDevice();
    CreateLogicalDevice();
  }

  void CreateSurface() {
    if (glfwCreateWindowSurface(instance_, window_, nullptr, &surface_) !=
        VK_SUCCESS) {
      throw std::runtime_error("failed to create window suface!");
    }
  }

  void MainLoop() {
    while (!glfwWindowShouldClose(window_)) {
      //マウス操作などのイベントを取り出し記録する
      glfwPollEvents();
    }
  }

  void CleanUp() {
    vkDestroyDevice(device_, nullptr);
    if (kEnableValidationLayers) {
      DestroyDebugUtilsMessengerEXT(instance_, debug_messenger_, nullptr);
    }
    // Window Surfaceはinstanceより前にDestroyする
    vkDestroySurfaceKHR(instance_, surface_, nullptr);
    vkDestroyInstance(instance_, nullptr);
    if (window_ != nullptr) glfwDestroyWindow(window_);
    glfwTerminate();
  }

  const int WIDTH_  = 800;
  const int HEIGHT_ = 600;

  const std::vector<const char*> validation_layers_ = {
      "VK_LAYER_KHRONOS_validation"};

  GLFWwindow* window_ = nullptr;

  VkSurfaceKHR surface_;

  VkInstance instance_;

  VkDebugUtilsMessengerEXT debug_messenger_;

  // 物理デバイス 今回は一つのみ扱う
  VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;

  // 論理デバイス こちらも一つのみ扱う
  VkDevice device_;

  VkQueue graphics_queue_;
  VkQueue present_queue_;
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
