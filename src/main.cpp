#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// reports and propagates errors
#include <iostream>
#include <stdexcept>    // error handling library

// provides EXIT_SUCCESS and EXIT_FAILURE macros
#include <cstdlib>      // macros are replaced by definition before compilation

#include <optional>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
    "VK_KHR_portability_subset"
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif


struct QueueFamilyIndices 
{
    std::optional<uint32_t> graphicsFamily;

    bool isComplete()
    {
        return graphicsFamily.has_value();
    }
};

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
    VkDevice device;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkQueue graphicsQueue;

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
        //setupDeugMessenger(); skip for now
        pickPhysicalDevice();
        createLogicalDevice();
    }

    void createLogicalDevice()
    {
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        VkDeviceQueueCreateInfo queueCreateInfo {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
        queueCreateInfo.queueCount = 1;

        float queuePriority = 1.0f;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        VkPhysicalDeviceFeatures deviceFeatures{};

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.pQueueCreateInfos = &queueCreateInfo;
        createInfo.queueCreateInfoCount = 1;
        createInfo.pEnabledFeatures = &deviceFeatures;

        // createInfo.enabledExtensionCount = 0;
        std::vector<const char*> extensions = deviceGetRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t> (extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        if (enableValidationLayers)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else
        {
            createInfo.enabledLayerCount = 0;
        }
        
        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create logical device!");
        }
        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);

    }

    std::vector<const char *> deviceGetRequiredExtensions()
    {
        uint32_t deviceExtensionPropertyCount = 0;
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &deviceExtensionPropertyCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(deviceExtensionPropertyCount);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &deviceExtensionPropertyCount, availableExtensions.data());

        std::vector<const char *> extensions;
        for (const char* extension : deviceExtensions)
        {
            for (const auto& avail : availableExtensions)
            {
                if (strcmp(extension, avail.extensionName) == 0)
                {
                    extensions.push_back(extension);
                    break;
                }
            }
        }
        return extensions;
    }   

    void pickPhysicalDevice()
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        if (deviceCount == 0)
        {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        for (const auto& device : devices) 
        {
            if (isDeviceSuitable(device))
            {
                physicalDevice = device;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE)
        {
            throw std::runtime_error("failed to find a suitable gpu");
        }
    }

    bool isDeviceSuitable(VkPhysicalDevice device)
    {
        // could implement scoring each device for a favorable one (later)
        // VkPhysicalDeviceProperties deviceProperties;
        // vkGetPhysicalDeviceProperties(device, &deviceProperties);
        // VkPhysicalDeviceFeatures deviceFeatures;
        // vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        QueueFamilyIndices indices = findQueueFamilies(device);

        return indices.isComplete();
    }

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies)
        {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = i;
            }
            if (indices.isComplete())
            {
                break;
            }
            i++;
        }
        return indices;
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
                
        // Vulkan needs extensions to interface with the window.
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        
        std::vector<const char*> requiredExtensions;
        for (uint32_t i = 0; i < glfwExtensionCount; i++)
        {
            requiredExtensions.emplace_back(glfwExtensions[i]);
        }
        
        // VK_ERROR_INCOMPATIBLE_DRIVER -> MoltenVK
        #if defined __APPLE__ 
        requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        requiredExtensions.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        #endif

        createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

        createInfo.enabledExtensionCount = (uint32_t) requiredExtensions.size();
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();

        if (enableValidationLayers)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        } 
        else
        {
            createInfo.enabledLayerCount = 0;
        }
        

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
        vkDestroyDevice(device, nullptr);
        vkDestroyInstance(instance, nullptr);
        glfwDestroyWindow(window);
        glfwTerminate();
    }
    
    bool checkValidationLayerSupport()
    {
        uint32_t layerCount;
        VkResult enumerateResult = vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        
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
