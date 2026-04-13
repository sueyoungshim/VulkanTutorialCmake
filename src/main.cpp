#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// reports and propagates errors
#include <iostream>
#include <stdexcept>    // error handling library

// provides EXIT_SUCCESS and EXIT_FAILURE macros
#include <cstdlib>      // macros are replaced by definition before compilation

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif


class HelloTriangleApplication {
public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    GLFWwindow* window;
    VkInstance instance;
    
    /*
     glfw = graphics library framework
        - create and manage windows
        - create OpenGL or Vulkan contexts
        - handle input events (keyboard, mouse, joystick)
        - handle window resizing, closing, etc.
     */
    
    void initWindow()
    {
        glfwInit();     // glfw = graphics library framework
        
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);   // tell glfw to not create the default opengl context
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);     // make it non-resizable by the user
        
        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);    // width, height, title, optionally specify a monitor, relevant to opengl
    }
    
    void initVulkan()
    {
        // initialize vulkan library by creating an instance.
        createInstance();
    }
    
    void createInstance()
    {
        if (enableValidationLayers && !checkValidationLayerSupport())
        {
            throw std::runtime_error("validation layers requested, but not available");
        }
        
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;
        
        VkInstanceCreateInfo createInfo {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
                
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        
        std::vector<const char*> requiredExtensions;
        
        for (uint32_t i = 0; i < glfwExtensionCount; i++)
        {
            requiredExtensions.emplace_back(glfwExtensions[i]);
        }
        
        requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        
        createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

        createInfo.enabledExtensionCount = (uint32_t) requiredExtensions.size();
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();
        
                
        /*
        parameters: pointer to struct with creation info,
                    pointer to custom allocator callbacks, always nullptr in this tutorial,
                    pointer to the variable that stores the handle to the new object
        */
        VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
        if (result != VK_SUCCESS)
        {
            std::cout << result << std::endl;
            throw std::runtime_error("failed to create instance!");
        }
        
    }

    void mainLoop()
    {
        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();   // checks for events like pressing the X button
        }
    }

    void cleanup()
    {
        vkDestroyInstance(instance, nullptr);
        glfwDestroyWindow(window);
        glfwTerminate();
    }
    
    bool checkValidationLayerSupport()
    {
        uint32_t layerCount;
        VkResult enumerateResult = vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::cout << enumerateResult << std::endl;
        std::cout << layerCount << std::endl;
        
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
        
        for (const char* layerName : validationLayers)
        {
            bool layerFound = false;
            
            for (const auto& layerProperties : availableLayers)
            {
                if (strcmp(layerName, layerProperties.layerName) == 0)
                {
                    layerFound = true;
                    break;
                }
            }
            if (!layerFound)
            {
                return false;
            }
        }
        return true;
    }
    
    std::vector<const char*> getRequiredExtensions()
    {
        uint32_t glfwExtensionsCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);
        
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionsCount);
        
        if (enableValidationLayers)
        {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        return extensions;
    }
};

int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
