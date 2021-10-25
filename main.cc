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
    fprintf(stderr, "\n\n");
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

static int init_vulkan([[maybe_unused]] int argc,
                       [[maybe_unused]] char** argv) {
  // Create Instance //////////////////////////////

  if (kEnableValidationLayers &&
      !check_validation_layer_support("VK_LAYER_KHRONOS_validation")) {
    fprintf(stderr,
            "Validation Layer --VK_LAYER_KHRONOS_validation-- not found\n");
    return EXIT_FAILURE;
  }
  /////////////////////////////////////////////////
  return EXIT_SUCCESS;
}

static int app([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
  if (init_vulkan(argc, argv) != EXIT_SUCCESS) {
    return EXIT_FAILURE;
  }
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
