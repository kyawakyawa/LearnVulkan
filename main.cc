// C headers
#include <cstdio>
#include <cstdlib>

// C++ headers
#include <string>
#include <vector>

#include "vulkan/vulkan.h"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#ifdef NDEBUG
constexpr bool kEnableOutput           = false;
constexpr bool kEnableValidationLayers = false;
#else
constexpr bool kEnableOutput           = true;
constexpr bool kEnableValidationLayers = true;
#endif

constexpr uint32_t gWidth  = 512;
constexpr uint32_t gHeight = 512;

// Vulkan のAPIが返す引数をもらってエラーチェックを行う
#define VK_CHECK_RESULT(f, m)                                           \
  {                                                                     \
    VkResult res = (f);                                                 \
    if (res != VK_SUCCESS) {                                            \
      fprintf(stderr, "Fatal : VkResult is %d in %s at line %d\n", res, \
              __FILE__, __LINE__);                                      \
      fprintf(stderr, "Message: %s\n", m);                              \
    }                                                                   \
  }

static bool check_validation_layer_support(const char* validation_layer_name) {
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
    fprintf(stderr, "---Validation Layer List---\n");
    for (const VkLayerProperties& prop : available_layers) {
      fprintf(stderr, "\t* %s\n", prop.layerName);
    }
    fprintf(stderr, "---------------------------\n\n");
  }

  bool layer_found = false;

  for (const auto& layer_properties : available_layers) {
    if (std::string(validation_layer_name) ==
        std::string(layer_properties.layerName)) {
      layer_found = true;
      break;
    }
  }

  if (!layer_found) {
    return false;
  }

  return true;
}

// callback for Validaton Layer
static VKAPI_ATTR VkBool32 VKAPI_CALL
debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
               [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT message_type,
               const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
               [[maybe_unused]] void* pUserData) {
  fprintf(stderr, "validation layer: %s\n", pCallbackData->pMessage);

  if (message_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    // Message is important enough to show
  }

  return VK_FALSE;
}

static void set_debug_messenger_create_info(
    VkDebugUtilsMessengerCreateInfoEXT* create_info) {
  *create_info       = {};
  create_info->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  create_info->messageSeverity =
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  create_info->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  create_info->pfnUserCallback = debug_callback;
}

static std::vector<const char*> get_required_extensions() {
  uint32_t glfw_extension_count = 0;
  const char** glfw_extensions;
  glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

  std::vector<const char*> extensions(glfw_extensions,
                                      glfw_extensions + glfw_extension_count);

  if (kEnableValidationLayers) {
    extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  return extensions;
}

static int init_vulkan([[maybe_unused]] int argc, [[maybe_unused]] char** argv,
                       VkInstance* pInstance) {
  // Create Instance //////////////////////////////

  if (kEnableValidationLayers &&
      !check_validation_layer_support("VK_LAYER_KHRONOS_validation")) {
    fprintf(stderr,
            "Validation Layer --VK_LAYER_KHRONOS_validation-- not found\n");
    return EXIT_FAILURE;
  }

  //// Application's info
  VkApplicationInfo app_info  = {};
  app_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName   = "Vulkan App";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName        = "No Engine";
  app_info.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion         = VK_API_VERSION_1_0;

  //// Info for vkCreateInstance
  VkInstanceCreateInfo create_info = {};
  create_info.sType                = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo     = &app_info;

  //////  Set Validatoin Layer's information
  std::vector<const char*> validation_layers = {"VK_LAYER_KHRONOS_validation"};

  VkDebugUtilsMessengerCreateInfoEXT debug_create_info;
  if constexpr (kEnableValidationLayers) {
    create_info.enabledLayerCount =
        static_cast<uint32_t>(validation_layers.size());
    create_info.ppEnabledLayerNames = validation_layers.data();

    // Set debug messenger's information
    set_debug_messenger_create_info(&debug_create_info);
    create_info.pNext = reinterpret_cast<VkDebugUtilsMessengerCreateInfoEXT*>(
        &debug_create_info);
  } else {
    create_info.enabledLayerCount = 0;
  }
  //////  Set Extensions
  auto extensions                   = get_required_extensions();
  create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  create_info.ppEnabledExtensionNames = extensions.data();

  VK_CHECK_RESULT(vkCreateInstance(
                      /*const VkInstanceCreateInfo *pCreateInfo=*/&create_info,
                      /*const VkAllocationCallbacks *pAllocator=*/nullptr,
                      /*VkInstance *pInstance                  =*/pInstance),
                  "Faild to create Instance")

  if constexpr (kEnableOutput) {
    fprintf(stderr, "\n---Instance was created.---\n\n");
  }

  /////////////////////////////////////////////////
  return EXIT_SUCCESS;
}

static void clean_up(const VkInstance instance) {
  vkDestroyInstance(instance, nullptr);
}

static int app([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
  if (glfwInit() == GLFW_FALSE) {
    return EXIT_FAILURE;
  }
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  GLFWwindow* window =
      glfwCreateWindow(int(gWidth), int(gHeight), "Vulkan", nullptr, nullptr);

  VkInstance instance;
  if (init_vulkan(argc, argv, &instance) != EXIT_SUCCESS) {
    return EXIT_FAILURE;
  }

  clean_up(instance);

  glfwDestroyWindow(window);
  glfwTerminate();

  return EXIT_SUCCESS;
}

#ifdef USE_GLOG
#include <glog/logging.h>
#endif

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
#ifdef USE_GLOG
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();
#endif
  return app(argc, argv);
}
