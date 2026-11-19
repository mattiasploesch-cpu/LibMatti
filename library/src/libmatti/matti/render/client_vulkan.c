












#include "libmatti/matti/render/render_internal.h"

#include "libmatti/org/lwjgl/glfw/GLFW.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(MATTI_HAVE_VULKAN)

#include <vulkan/vulkan.h>

#define VK_FRAMES_IN_FLIGHT 2

typedef struct LIBMATTI_VulkanFrame
{
    VkCommandBuffer command;
    VkSemaphore image_available;
    VkFence in_flight;

    VkBuffer staging;
    VkDeviceMemory staging_memory;
    void *staging_mapped;
    VkDeviceSize staging_capacity;




    VkImage texture;
    VkDeviceMemory texture_memory;
    VkImageLayout texture_layout;
    VkFormat texture_format;
    int texture_width;
    int texture_height;
} LIBMATTI_VulkanFrame;

typedef struct LIBMATTI_VulkanBackend
{
    long window;
    int glfw_ready;

    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physical_device;
    VkPhysicalDeviceProperties properties;
    VkDevice device;
    VkQueue queue;
    uint32_t queue_family;
    int queue_family_valid;

    VkSwapchainKHR swapchain;
    VkFormat format;
    VkColorSpaceKHR color_space;
    int format_is_bgr;
    VkExtent2D extent;
    uint32_t image_count;
    VkImage *images;
    VkImageLayout *layouts;
    VkFence *image_in_flight;
    VkSemaphore *render_finished;

    VkCommandPool command_pool;
    LIBMATTI_VulkanFrame frames[VK_FRAMES_IN_FLIGHT];
    uint32_t frame_index;
    int vsync;

    int need_recreate;
    int failed;
    int initialized;
} LIBMATTI_VulkanBackend;

static int vulkan_is_available(void)
{
    return 1;
}

static int format_is_32bit_color(VkFormat format)
{
    return format == VK_FORMAT_B8G8R8A8_UNORM || format == VK_FORMAT_B8G8R8A8_SRGB
           || format == VK_FORMAT_R8G8B8A8_UNORM || format == VK_FORMAT_R8G8B8A8_SRGB;
}

static int format_supports_blit(VkPhysicalDevice device, VkFormat format)
{
    VkFormatProperties properties;
    vkGetPhysicalDeviceFormatProperties(device, format, &properties);
    const VkFormatFeatureFlags required = VK_FORMAT_FEATURE_BLIT_SRC_BIT
                                          | VK_FORMAT_FEATURE_BLIT_DST_BIT;
    return (properties.optimalTilingFeatures & required) == required;
}

static uint32_t find_memory_type(const LIBMATTI_VulkanBackend *backend, uint32_t type_bits,
                                 VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memory;
    vkGetPhysicalDeviceMemoryProperties(backend->physical_device, &memory);
    for (uint32_t i = 0; i < memory.memoryTypeCount; i++)
    {
        if ((type_bits & (1u << i)) != 0
            && (memory.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }
    return UINT32_MAX;
}

static int device_has_extension(VkPhysicalDevice device, const char *name)
{
    uint32_t count = 0;
    if (vkEnumerateDeviceExtensionProperties(device, NULL, &count, NULL) != VK_SUCCESS
        || count == 0)
        return 0;

    VkExtensionProperties *extensions = calloc(count, sizeof(VkExtensionProperties));
    if (extensions == NULL)
        return 0;
    int found = 0;
    if (vkEnumerateDeviceExtensionProperties(device, NULL, &count, extensions) == VK_SUCCESS)
    {
        for (uint32_t i = 0; i < count && !found; i++)
            found = strcmp(extensions[i].extensionName, name) == 0;
    }
    free(extensions);
    return found;
}

static int find_queue_family(VkPhysicalDevice device, VkSurfaceKHR surface, uint32_t *out_family)
{
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, NULL);
    if (count == 0)
        return 0;

    VkQueueFamilyProperties *families = calloc(count, sizeof(VkQueueFamilyProperties));
    if (families == NULL)
        return 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, families);

    int found = 0;
    for (uint32_t i = 0; i < count; i++)
    {
        if ((families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)
            continue;
        VkBool32 present = VK_FALSE;
        if (vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present) == VK_SUCCESS
            && present == VK_TRUE)
        {
            *out_family = i;
            found = 1;
            break;
        }
    }
    free(families);
    return found;
}





static int choose_surface_format(VkPhysicalDevice device, VkSurfaceKHR surface,
                                 VkFormat *out_format, VkColorSpaceKHR *out_color_space,
                                 int *out_is_bgr)
{
    uint32_t count = 0;
    if (vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, NULL) != VK_SUCCESS
        || count == 0)
        return 0;

    VkSurfaceFormatKHR *formats = calloc(count, sizeof(VkSurfaceFormatKHR));
    if (formats == NULL)
        return 0;
    if (vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, formats) != VK_SUCCESS)
    {
        free(formats);
        return 0;
    }

    static const VkFormat preferred[] = {
        VK_FORMAT_B8G8R8A8_UNORM,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_FORMAT_B8G8R8A8_SRGB,
        VK_FORMAT_R8G8B8A8_SRGB,
    };

    int found = 0;
    for (size_t preference = 0; preference < sizeof(preferred) / sizeof(preferred[0])
                               && !found;
         preference++)
    {



        for (int pass = 0; pass < 2 && !found; pass++)
        {
            for (uint32_t i = 0; i < count && !found; i++)
            {
                if (formats[i].format != preferred[preference])
                    continue;
                if (pass == 0 && formats[i].colorSpace != VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                    continue;
                if (!format_supports_blit(device, formats[i].format))
                    continue;
                *out_format = formats[i].format;
                *out_color_space = formats[i].colorSpace;
                *out_is_bgr = formats[i].format == VK_FORMAT_B8G8R8A8_UNORM
                              || formats[i].format == VK_FORMAT_B8G8R8A8_SRGB;
                found = 1;
            }
        }
    }
    free(formats);
    return found;
}

static VkPresentModeKHR choose_present_mode(VkPhysicalDevice device, VkSurfaceKHR surface,
                                            int vsync)
{
    if (vsync)
        return VK_PRESENT_MODE_FIFO_KHR;

    uint32_t count = 0;
    if (vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, NULL) == VK_SUCCESS
        && count > 0)
    {
        VkPresentModeKHR *modes = calloc(count, sizeof(VkPresentModeKHR));
        if (modes != NULL)
        {
            if (vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, modes)
                == VK_SUCCESS)
            {
                int mailbox = 0;
                int immediate = 0;
                for (uint32_t i = 0; i < count; i++)
                {
                    if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
                        mailbox = 1;
                    if (modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
                        immediate = 1;
                }
                free(modes);
                if (mailbox)
                    return VK_PRESENT_MODE_MAILBOX_KHR;
                if (immediate)
                    return VK_PRESENT_MODE_IMMEDIATE_KHR;
            }
            else
            {
                free(modes);
            }
        }
    }


    return VK_PRESENT_MODE_FIFO_KHR;
}

static int surface_capabilities(LIBMATTI_VulkanBackend *backend,
                                VkSurfaceCapabilitiesKHR *out_capabilities)
{
    return vkGetPhysicalDeviceSurfaceCapabilitiesKHR(backend->physical_device, backend->surface,
                                                     out_capabilities)
           == VK_SUCCESS;
}

static void destroy_frame_resources(LIBMATTI_VulkanBackend *backend, LIBMATTI_VulkanFrame *frame)
{
    if (backend->device == VK_NULL_HANDLE)
        return;
    if (frame->texture != VK_NULL_HANDLE)
        vkDestroyImage(backend->device, frame->texture, NULL);
    if (frame->texture_memory != VK_NULL_HANDLE)
        vkFreeMemory(backend->device, frame->texture_memory, NULL);
    if (frame->staging_mapped != NULL)
        vkUnmapMemory(backend->device, frame->staging_memory);
    if (frame->staging != VK_NULL_HANDLE)
        vkDestroyBuffer(backend->device, frame->staging, NULL);
    if (frame->staging_memory != VK_NULL_HANDLE)
        vkFreeMemory(backend->device, frame->staging_memory, NULL);
    if (frame->image_available != VK_NULL_HANDLE)
        vkDestroySemaphore(backend->device, frame->image_available, NULL);
    if (frame->in_flight != VK_NULL_HANDLE)
        vkDestroyFence(backend->device, frame->in_flight, NULL);
    memset(frame, 0, sizeof(*frame));
}

static void destroy_swapchain_resources(LIBMATTI_VulkanBackend *backend)
{
    if (backend->device != VK_NULL_HANDLE && backend->render_finished != NULL)
    {
        for (uint32_t i = 0; i < backend->image_count; i++)
        {
            if (backend->render_finished[i] != VK_NULL_HANDLE)
                vkDestroySemaphore(backend->device, backend->render_finished[i], NULL);
        }
    }
    free(backend->images);
    free(backend->layouts);
    free(backend->image_in_flight);
    free(backend->render_finished);
    backend->images = NULL;
    backend->layouts = NULL;
    backend->image_in_flight = NULL;
    backend->render_finished = NULL;
    backend->image_count = 0;
    if (backend->device != VK_NULL_HANDLE && backend->swapchain != VK_NULL_HANDLE)
        vkDestroySwapchainKHR(backend->device, backend->swapchain, NULL);
    backend->swapchain = VK_NULL_HANDLE;
}

static int ensure_frame_texture(LIBMATTI_VulkanBackend *backend, LIBMATTI_VulkanFrame *frame,
                                int width, int height)
{
    if (width <= 0 || height <= 0)
        return 0;
    if (frame->texture != VK_NULL_HANDLE && frame->texture_format == backend->format
        && frame->texture_width == width && frame->texture_height == height)
        return 1;




    if (frame->texture != VK_NULL_HANDLE)
        vkDestroyImage(backend->device, frame->texture, NULL);
    if (frame->texture_memory != VK_NULL_HANDLE)
        vkFreeMemory(backend->device, frame->texture_memory, NULL);
    frame->texture = VK_NULL_HANDLE;
    frame->texture_memory = VK_NULL_HANDLE;
    frame->texture_layout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImageCreateInfo image_info = {0};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.format = backend->format;
    image_info.extent.width = (uint32_t) width;
    image_info.extent.height = (uint32_t) height;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    if (vkCreateImage(backend->device, &image_info, NULL, &frame->texture) != VK_SUCCESS)
    {
        LIBMATTI_Render_SetLastError("Vulkan texture image creation failed (%dx%d)", width, height);
        return 0;
    }

    VkMemoryRequirements requirements;
    vkGetImageMemoryRequirements(backend->device, frame->texture, &requirements);
    uint32_t memory_type = find_memory_type(backend, requirements.memoryTypeBits,
                                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (memory_type == UINT32_MAX)
        memory_type = find_memory_type(backend, requirements.memoryTypeBits, 0);
    if (memory_type == UINT32_MAX)
    {
        vkDestroyImage(backend->device, frame->texture, NULL);
        frame->texture = VK_NULL_HANDLE;
        LIBMATTI_Render_SetLastError("Vulkan has no memory type for the frame texture");
        return 0;
    }

    VkMemoryAllocateInfo allocation = {0};
    allocation.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocation.allocationSize = requirements.size;
    allocation.memoryTypeIndex = memory_type;
    if (vkAllocateMemory(backend->device, &allocation, NULL, &frame->texture_memory) != VK_SUCCESS)
    {
        vkDestroyImage(backend->device, frame->texture, NULL);
        frame->texture = VK_NULL_HANDLE;
        LIBMATTI_Render_SetLastError("Vulkan frame texture memory allocation failed");
        return 0;
    }
    if (vkBindImageMemory(backend->device, frame->texture, frame->texture_memory, 0) != VK_SUCCESS)
    {
        vkDestroyImage(backend->device, frame->texture, NULL);
        vkFreeMemory(backend->device, frame->texture_memory, NULL);
        frame->texture = VK_NULL_HANDLE;
        frame->texture_memory = VK_NULL_HANDLE;
        LIBMATTI_Render_SetLastError("Vulkan frame texture memory bind failed");
        return 0;
    }

    frame->texture_format = backend->format;
    frame->texture_width = width;
    frame->texture_height = height;
    frame->texture_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    return 1;
}

static int ensure_staging(LIBMATTI_VulkanBackend *backend, LIBMATTI_VulkanFrame *frame,
                          VkDeviceSize size)
{
    if (frame->staging != VK_NULL_HANDLE && frame->staging_capacity >= size)
        return 1;

    if (frame->staging_mapped != NULL)
        vkUnmapMemory(backend->device, frame->staging_memory);
    if (frame->staging != VK_NULL_HANDLE)
        vkDestroyBuffer(backend->device, frame->staging, NULL);
    if (frame->staging_memory != VK_NULL_HANDLE)
        vkFreeMemory(backend->device, frame->staging_memory, NULL);
    frame->staging = VK_NULL_HANDLE;
    frame->staging_memory = VK_NULL_HANDLE;
    frame->staging_mapped = NULL;
    frame->staging_capacity = 0;

    VkBufferCreateInfo buffer_info = {0};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(backend->device, &buffer_info, NULL, &frame->staging) != VK_SUCCESS)
    {
        LIBMATTI_Render_SetLastError("Vulkan staging buffer creation failed (%llu bytes)",
                                     (unsigned long long) size);
        return 0;
    }

    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(backend->device, frame->staging, &requirements);
    uint32_t memory_type = find_memory_type(backend, requirements.memoryTypeBits,
                                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                                                | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (memory_type == UINT32_MAX)
    {
        vkDestroyBuffer(backend->device, frame->staging, NULL);
        frame->staging = VK_NULL_HANDLE;
        LIBMATTI_Render_SetLastError("Vulkan has no host-visible coherent staging memory");
        return 0;
    }

    VkMemoryAllocateInfo allocation = {0};
    allocation.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocation.allocationSize = requirements.size;
    allocation.memoryTypeIndex = memory_type;
    if (vkAllocateMemory(backend->device, &allocation, NULL, &frame->staging_memory)
        != VK_SUCCESS)
    {
        vkDestroyBuffer(backend->device, frame->staging, NULL);
        frame->staging = VK_NULL_HANDLE;
        LIBMATTI_Render_SetLastError("Vulkan staging memory allocation failed");
        return 0;
    }
    if (vkBindBufferMemory(backend->device, frame->staging, frame->staging_memory, 0)
            != VK_SUCCESS
        || vkMapMemory(backend->device, frame->staging_memory, 0, VK_WHOLE_SIZE, 0,
                       &frame->staging_mapped)
               != VK_SUCCESS)
    {
        if (frame->staging_memory != VK_NULL_HANDLE)
            vkFreeMemory(backend->device, frame->staging_memory, NULL);
        vkDestroyBuffer(backend->device, frame->staging, NULL);
        frame->staging = VK_NULL_HANDLE;
        frame->staging_memory = VK_NULL_HANDLE;
        LIBMATTI_Render_SetLastError("Vulkan staging memory bind/map failed");
        return 0;
    }

    frame->staging_capacity = size;
    return 1;
}

static int create_swapchain(LIBMATTI_VulkanBackend *backend, int width, int height)
{
    VkSurfaceCapabilitiesKHR capabilities;
    if (!surface_capabilities(backend, &capabilities))
    {
        LIBMATTI_Render_SetLastError("Vulkan surface capabilities query failed");
        return 0;
    }
    if ((capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) == 0)
    {
        LIBMATTI_Render_SetLastError(
            "Vulkan swapchain does not support TRANSFER_DST (the presentation path)");
        return 0;
    }

    VkFormat format;
    VkColorSpaceKHR color_space;
    int is_bgr;
    if (!choose_surface_format(backend->physical_device, backend->surface, &format, &color_space,
                               &is_bgr))
    {
        LIBMATTI_Render_SetLastError(
            "Vulkan surface has no 32-bit blit-capable format (B8G8R8A8/R8G8B8A8)");
        return 0;
    }

    VkExtent2D extent = capabilities.currentExtent;
    if (extent.width == UINT32_MAX)
    {
        extent.width = (uint32_t) width;
        extent.height = (uint32_t) height;
        if (extent.width < capabilities.minImageExtent.width)
            extent.width = capabilities.minImageExtent.width;
        if (extent.width > capabilities.maxImageExtent.width)
            extent.width = capabilities.maxImageExtent.width;
        if (extent.height < capabilities.minImageExtent.height)
            extent.height = capabilities.minImageExtent.height;
        if (extent.height > capabilities.maxImageExtent.height)
            extent.height = capabilities.maxImageExtent.height;
    }
    if (extent.width == 0 || extent.height == 0)
        return 0;

    uint32_t image_count = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount)
        image_count = capabilities.maxImageCount;

    VkCompositeAlphaFlagBitsKHR composite = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    if ((capabilities.supportedCompositeAlpha & composite) == 0)
    {
        if (capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR)
            composite = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
        else if (capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR)
            composite = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
        else
            composite = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
    }

    VkSwapchainCreateInfoKHR create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = backend->surface;
    create_info.minImageCount = image_count;
    create_info.imageFormat = format;
    create_info.imageColorSpace = color_space;
    create_info.imageExtent = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.preTransform = capabilities.currentTransform;
    create_info.compositeAlpha = composite;
    create_info.presentMode = choose_present_mode(backend->physical_device, backend->surface,
                                                  backend->vsync);
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    if (vkCreateSwapchainKHR(backend->device, &create_info, NULL, &swapchain) != VK_SUCCESS)
    {
        LIBMATTI_Render_SetLastError("Vulkan swapchain creation failed (%ux%u)", extent.width,
                                     extent.height);
        return 0;
    }

    uint32_t count = 0;
    if (vkGetSwapchainImagesKHR(backend->device, swapchain, &count, NULL) != VK_SUCCESS
        || count == 0)
    {
        vkDestroySwapchainKHR(backend->device, swapchain, NULL);
        LIBMATTI_Render_SetLastError("Vulkan swapchain returned no images");
        return 0;
    }

    VkImage *images = calloc(count, sizeof(VkImage));
    VkImageLayout *layouts = calloc(count, sizeof(VkImageLayout));
    VkFence *image_in_flight = calloc(count, sizeof(VkFence));
    VkSemaphore *render_finished = calloc(count, sizeof(VkSemaphore));
    if (images == NULL || layouts == NULL || image_in_flight == NULL || render_finished == NULL)
    {
        free(images);
        free(layouts);
        free(image_in_flight);
        free(render_finished);
        vkDestroySwapchainKHR(backend->device, swapchain, NULL);
        LIBMATTI_Render_SetLastError("out of memory for the Vulkan swapchain images");
        return 0;
    }
    vkGetSwapchainImagesKHR(backend->device, swapchain, &count, images);
    for (uint32_t i = 0; i < count; i++)
        layouts[i] = VK_IMAGE_LAYOUT_UNDEFINED;

    VkSemaphoreCreateInfo semaphore_info = {0};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    for (uint32_t i = 0; i < count; i++)
    {
        if (vkCreateSemaphore(backend->device, &semaphore_info, NULL, &render_finished[i])
            != VK_SUCCESS)
        {
            for (uint32_t j = 0; j < i; j++)
                vkDestroySemaphore(backend->device, render_finished[j], NULL);
            free(images);
            free(layouts);
            free(image_in_flight);
            free(render_finished);
            vkDestroySwapchainKHR(backend->device, swapchain, NULL);
            LIBMATTI_Render_SetLastError("Vulkan per-image semaphore creation failed");
            return 0;
        }
    }

    backend->swapchain = swapchain;
    backend->format = format;
    backend->color_space = color_space;
    backend->format_is_bgr = is_bgr;
    backend->extent = extent;
    backend->image_count = count;
    backend->images = images;
    backend->layouts = layouts;
    backend->image_in_flight = image_in_flight;
    backend->render_finished = render_finished;
    backend->need_recreate = 0;
    return 1;
}

static int recreate_swapchain(LIBMATTI_VulkanBackend *backend, int width, int height)
{
    if (backend->device == VK_NULL_HANDLE)
        return 0;





    vkDeviceWaitIdle(backend->device);
    destroy_swapchain_resources(backend);
    if (width <= 0 || height <= 0)
        return 1;

    return create_swapchain(backend, width, height);
}

static int create_instance(LIBMATTI_VulkanBackend *backend)
{
    if (!LIBMATTI_GLFW_glfwVulkanSupported())
    {
        LIBMATTI_Render_SetLastError("GLFW reports that Vulkan is not supported on this system");
        return 0;
    }

    uint32_t extension_count = 0;
    const char **extensions = LIBMATTI_GLFW_glfwGetRequiredInstanceExtensions(&extension_count);
    if (extensions == NULL || extension_count == 0)
    {
        LIBMATTI_Render_SetLastError("GLFW did not return the required Vulkan instance extensions");
        return 0;
    }

    VkApplicationInfo application = {0};
    application.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application.pApplicationName = "Matticraft";
    application.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    application.pEngineName = "libmatti";
    application.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    application.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &application;
    create_info.enabledExtensionCount = extension_count;
    create_info.ppEnabledExtensionNames = extensions;

    if (vkCreateInstance(&create_info, NULL, &backend->instance) != VK_SUCCESS)
    {
        LIBMATTI_Render_SetLastError("Vulkan instance creation failed");
        return 0;
    }
    return 1;
}

static int pick_physical_device(LIBMATTI_VulkanBackend *backend)
{
    uint32_t count = 0;
    if (vkEnumeratePhysicalDevices(backend->instance, &count, NULL) != VK_SUCCESS || count == 0)
    {
        LIBMATTI_Render_SetLastError("no Vulkan physical device found");
        return 0;
    }

    VkPhysicalDevice *devices = calloc(count, sizeof(VkPhysicalDevice));
    if (devices == NULL)
        return 0;
    if (vkEnumeratePhysicalDevices(backend->instance, &count, devices) != VK_SUCCESS)
    {
        free(devices);
        LIBMATTI_Render_SetLastError("Vulkan physical device enumeration failed");
        return 0;
    }

    VkPhysicalDevice best = VK_NULL_HANDLE;
    uint32_t best_family = 0;
    int best_score = -1;

    for (uint32_t i = 0; i < count; i++)
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(devices[i], &properties);

        uint32_t family = 0;
        if (!find_queue_family(devices[i], backend->surface, &family))
            continue;
        if (!device_has_extension(devices[i], VK_KHR_SWAPCHAIN_EXTENSION_NAME))
            continue;

        VkFormat format;
        VkColorSpaceKHR color_space;
        int is_bgr;
        if (!choose_surface_format(devices[i], backend->surface, &format, &color_space, &is_bgr))
            continue;

        uint32_t mode_count = 0;
        if (vkGetPhysicalDeviceSurfacePresentModesKHR(devices[i], backend->surface, &mode_count,
                                                      NULL)
                != VK_SUCCESS
            || mode_count == 0)
            continue;

        VkSurfaceCapabilitiesKHR capabilities;
        if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(devices[i], backend->surface, &capabilities)
                != VK_SUCCESS
            || (capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) == 0)
            continue;

        int score = 100;
        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            score = 1000;
        else if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
            score = 500;
        else if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU)
            score = 300;

        if (score > best_score)
        {
            best = devices[i];
            best_family = family;
            best_score = score;
        }
    }
    free(devices);

    if (best == VK_NULL_HANDLE)
    {
        LIBMATTI_Render_SetLastError(
            "no Vulkan device with a graphics+present queue and a usable swapchain was found");
        return 0;
    }

    backend->physical_device = best;
    backend->queue_family = best_family;
    backend->queue_family_valid = 1;
    vkGetPhysicalDeviceProperties(best, &backend->properties);
    return 1;
}

static int create_device(LIBMATTI_VulkanBackend *backend)
{
    float priority = 1.0f;
    VkDeviceQueueCreateInfo queue_info = {0};
    queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info.queueFamilyIndex = backend->queue_family;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = &priority;

    const char *device_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    VkDeviceCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount = 1;
    create_info.pQueueCreateInfos = &queue_info;
    create_info.enabledExtensionCount = 1;
    create_info.ppEnabledExtensionNames = device_extensions;

    if (vkCreateDevice(backend->physical_device, &create_info, NULL, &backend->device)
        != VK_SUCCESS)
    {
        LIBMATTI_Render_SetLastError("Vulkan logical device creation failed");
        backend->device = VK_NULL_HANDLE;
        return 0;
    }

    vkGetDeviceQueue(backend->device, backend->queue_family, 0, &backend->queue);
    return 1;
}

static int create_command_pool(LIBMATTI_VulkanBackend *backend)
{
    VkCommandPoolCreateInfo pool_info = {0};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_info.queueFamilyIndex = backend->queue_family;
    if (vkCreateCommandPool(backend->device, &pool_info, NULL, &backend->command_pool)
        != VK_SUCCESS)
    {
        LIBMATTI_Render_SetLastError("Vulkan command pool creation failed");
        return 0;
    }

    VkCommandBufferAllocateInfo allocate_info = {0};
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.commandPool = backend->command_pool;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandBufferCount = VK_FRAMES_IN_FLIGHT;
    VkCommandBuffer buffers[VK_FRAMES_IN_FLIGHT] = {0};
    if (vkAllocateCommandBuffers(backend->device, &allocate_info, buffers) != VK_SUCCESS)
    {
        LIBMATTI_Render_SetLastError("Vulkan command buffer allocation failed");
        return 0;
    }
    for (uint32_t i = 0; i < VK_FRAMES_IN_FLIGHT; i++)
        backend->frames[i].command = buffers[i];
    return 1;
}

static int create_frame_sync(LIBMATTI_VulkanBackend *backend)
{
    VkSemaphoreCreateInfo semaphore_info = {0};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fence_info = {0};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (uint32_t i = 0; i < VK_FRAMES_IN_FLIGHT; i++)
    {
        LIBMATTI_VulkanFrame *frame = &backend->frames[i];
        if (vkCreateSemaphore(backend->device, &semaphore_info, NULL, &frame->image_available)
                != VK_SUCCESS
            || vkCreateFence(backend->device, &fence_info, NULL, &frame->in_flight)
                   != VK_SUCCESS)
        {
            LIBMATTI_Render_SetLastError("Vulkan synchronization object creation failed");
            return 0;
        }
    }
    return 1;
}

static int create_window(LIBMATTI_Renderer *renderer, LIBMATTI_VulkanBackend *backend)
{
    if (!LIBMATTI_GLFW_glfwInit())
    {
        const char *description = NULL;
        LIBMATTI_GLFW_glfwGetError(&description);
        LIBMATTI_Render_SetLastError("GLFW init failed: %s",
                                     description != NULL ? description : "unknown error");
        return 0;
    }
    backend->glfw_ready = 1;

    LIBMATTI_GLFW_glfwDefaultWindowHints();
    LIBMATTI_GLFW_glfwWindowHint(LIBMATTI_GLFW_CLIENT_API, LIBMATTI_GLFW_NO_API);
    LIBMATTI_GLFW_glfwWindowHint(LIBMATTI_GLFW_RESIZABLE,
                                renderer->config.fullscreen ? LIBMATTI_GLFW_FALSE
                                                            : LIBMATTI_GLFW_TRUE);





    int window_width = renderer->width;
    int window_height = renderer->height;
    int monitor_x = 0, monitor_y = 0;
    long monitor = 0;
    if (renderer->config.fullscreen)
    {
        LIBMATTI_GLFW_glfwWindowHint(LIBMATTI_GLFW_DECORATED, LIBMATTI_GLFW_FALSE);
        LIBMATTI_GLFW_glfwWindowHint(LIBMATTI_GLFW_VISIBLE, LIBMATTI_GLFW_FALSE);
        monitor = LIBMATTI_GLFW_glfwGetPrimaryMonitor();
        if (monitor != 0)
        {
            int mode_width = 0, mode_height = 0;
            LIBMATTI_GLFW_glfwGetMonitorPos(monitor, &monitor_x, &monitor_y);
            if (LIBMATTI_GLFW_glfwGetVideoMode(monitor, &mode_width, &mode_height, NULL, NULL,
                                               NULL, NULL)
                && mode_width > 0 && mode_height > 0)
            {
                window_width = mode_width;
                window_height = mode_height;
            }
        }
    }

    backend->window = LIBMATTI_GLFW_glfwCreateWindow(window_width, window_height,
                                                     renderer->title, 0, 0);
    if (backend->window == 0)
    {
        const char *description = NULL;
        LIBMATTI_GLFW_glfwGetError(&description);
        LIBMATTI_Render_SetLastError("Vulkan window creation failed: %s",
                                     description != NULL ? description : "unknown error");
        return 0;
    }

    if (renderer->config.fullscreen)
    {
        LIBMATTI_GLFW_glfwSetWindowPos(backend->window, monitor_x, monitor_y);
        LIBMATTI_GLFW_glfwShowWindow(backend->window);
        LIBMATTI_GLFW_glfwFocusWindow(backend->window);
        if (!LIBMATTI_Render_Resize(renderer, window_width, window_height))
        {
            LIBMATTI_Render_SetLastError("could not allocate the fullscreen framebuffer");
            return 0;
        }
    }
    return 1;
}

static int vulkan_init(LIBMATTI_Renderer *renderer)
{
    LIBMATTI_VulkanBackend *backend = calloc(1, sizeof(LIBMATTI_VulkanBackend));
    if (backend == NULL)
    {
        LIBMATTI_Render_SetLastError("out of memory while creating the Vulkan backend");
        return 0;
    }
    renderer->backend = backend;
    backend->vsync = renderer->config.vsync ? 1 : 0;

    if (!create_window(renderer, backend))
        goto fail;
    if (!create_instance(backend))
        goto fail;
    if (LIBMATTI_GLFW_glfwCreateWindowSurface(backend->instance, backend->window, NULL,
                                              (void **) &backend->surface)
        != VK_SUCCESS)
    {
        LIBMATTI_Render_SetLastError("Vulkan window surface creation failed");
        goto fail;
    }
    if (!pick_physical_device(backend))
        goto fail;
    if (!create_device(backend))
        goto fail;
    if (!create_command_pool(backend))
        goto fail;
    if (!create_frame_sync(backend))
        goto fail;

    int width = 0, height = 0;
    LIBMATTI_GLFW_glfwGetFramebufferSize(backend->window, &width, &height);
    if (width <= 0) width = renderer->width;
    if (height <= 0) height = renderer->height;
    if (!create_swapchain(backend, width, height))
        goto fail;

    backend->initialized = 1;
    return 1;

fail:
    return 0;
}

static void vulkan_backend_free(LIBMATTI_VulkanBackend *backend)
{
    if (backend == NULL)
        return;

    if (backend->device != VK_NULL_HANDLE)
        vkDeviceWaitIdle(backend->device);

    if (backend->device != VK_NULL_HANDLE)
    {
        destroy_swapchain_resources(backend);
        for (uint32_t i = 0; i < VK_FRAMES_IN_FLIGHT; i++)
            destroy_frame_resources(backend, &backend->frames[i]);
        if (backend->command_pool != VK_NULL_HANDLE)
            vkDestroyCommandPool(backend->device, backend->command_pool, NULL);
        vkDestroyDevice(backend->device, NULL);
    }
    if (backend->instance != VK_NULL_HANDLE && backend->surface != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(backend->instance, backend->surface, NULL);
    if (backend->instance != VK_NULL_HANDLE)
        vkDestroyInstance(backend->instance, NULL);

    if (backend->window != 0)
        LIBMATTI_GLFW_glfwDestroyWindow(backend->window);
    if (backend->glfw_ready)
        LIBMATTI_GLFW_glfwTerminate();
    free(backend);
}

static void vulkan_shutdown(LIBMATTI_Renderer *renderer)
{
    if (renderer == NULL)
        return;
    vulkan_backend_free((LIBMATTI_VulkanBackend *) renderer->backend);
    renderer->backend = NULL;
}

static void vulkan_poll_events(LIBMATTI_Renderer *renderer)
{
    (void) renderer;
    LIBMATTI_GLFW_glfwPollEvents();
}

static int vulkan_should_close(const LIBMATTI_Renderer *renderer)
{
    const LIBMATTI_VulkanBackend *backend = renderer->backend;
    return backend == NULL || backend->failed || backend->window == 0
               || LIBMATTI_GLFW_glfwWindowShouldClose(backend->window);
}

static void vulkan_get_window_size(const LIBMATTI_Renderer *renderer, int *width, int *height)
{
    const LIBMATTI_VulkanBackend *backend = renderer->backend;
    int w = renderer->width;
    int h = renderer->height;
    if (backend != NULL && backend->window != 0)
    {
        LIBMATTI_GLFW_glfwGetFramebufferSize(backend->window, &w, &h);
        if (w <= 0) w = renderer->width;
        if (h <= 0) h = renderer->height;
    }
    if (width != NULL) *width = w;
    if (height != NULL) *height = h;
}

static void vulkan_get_mouse_position(const LIBMATTI_Renderer *renderer, double *x, double *y)
{
    const LIBMATTI_VulkanBackend *backend = renderer->backend;
    if (x != NULL) *x = 0.0;
    if (y != NULL) *y = 0.0;
    if (backend == NULL || backend->window == 0)
        return;

    double cursor_x = 0.0, cursor_y = 0.0;
    int window_width = 0, window_height = 0;
    LIBMATTI_GLFW_glfwGetCursorPos(backend->window, &cursor_x, &cursor_y);
    LIBMATTI_GLFW_glfwGetWindowSize(backend->window, &window_width, &window_height);
    if (window_width <= 0 || window_height <= 0)
        return;
    if (x != NULL)
        *x = cursor_x * (double) renderer->width / (double) window_width;
    if (y != NULL)
        *y = cursor_y * (double) renderer->height / (double) window_height;
}

static int vulkan_is_mouse_down(const LIBMATTI_Renderer *renderer, int button)
{
    const LIBMATTI_VulkanBackend *backend = renderer->backend;
    if (backend == NULL || backend->window == 0)
        return 0;
    return LIBMATTI_GLFW_glfwGetMouseButton(backend->window, button) == LIBMATTI_GLFW_PRESS;
}

static int vulkan_is_key_down(const LIBMATTI_Renderer *renderer, int key)
{
    const LIBMATTI_VulkanBackend *backend = renderer->backend;
    if (backend == NULL || backend->window == 0)
        return 0;
    return LIBMATTI_GLFW_glfwGetKey(backend->window, key) == LIBMATTI_GLFW_PRESS;
}

static void vulkan_request_close(LIBMATTI_Renderer *renderer)
{
    const LIBMATTI_VulkanBackend *backend = renderer->backend;
    if (backend != NULL && backend->window != 0)
        LIBMATTI_GLFW_glfwSetWindowShouldClose(backend->window, 1);
}

static void copy_pixels_to_staging(const LIBMATTI_VulkanBackend *backend,
                                   const LIBMATTI_VulkanFrame *frame, const uint32_t *source,
                                   int width, int height)
{
    uint32_t *destination = frame->staging_mapped;
    if (destination == NULL)
        return;

    for (int y = 0; y < height; y++)
    {
        const uint32_t *source_row = source + (size_t) y * (size_t) width;
        uint32_t *destination_row = destination + (size_t) y * (size_t) width;
        if (backend->format_is_bgr)
        {



            memcpy(destination_row, source_row, (size_t) width * sizeof(uint32_t));
            continue;
        }
        for (int x = 0; x < width; x++)
        {
            uint32_t pixel = source_row[x];
            destination_row[x] = (pixel & 0xFF00FF00u) | ((pixel & 0x00FF0000u) >> 16)
                                 | ((pixel & 0x000000FFu) << 16);
        }
    }
}

static int record_frame(LIBMATTI_VulkanBackend *backend, LIBMATTI_VulkanFrame *frame,
                        uint32_t image_index, int width, int height)
{
    if (vkResetCommandBuffer(frame->command, 0) != VK_SUCCESS)
        return 0;

    VkCommandBufferBeginInfo begin = {0};
    begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if (vkBeginCommandBuffer(frame->command, &begin) != VK_SUCCESS)
        return 0;

    VkImageSubresourceRange range = {0};
    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseMipLevel = 0;
    range.levelCount = 1;
    range.baseArrayLayer = 0;
    range.layerCount = 1;



    VkImageMemoryBarrier barrier = {0};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = frame->texture_layout;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = frame->texture;
    barrier.subresourceRange = range;
    barrier.srcAccessMask = frame->texture_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
                                ? VK_ACCESS_TRANSFER_READ_BIT
                                : 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vkCmdPipelineBarrier(frame->command,
                         frame->texture_layout == VK_IMAGE_LAYOUT_UNDEFINED
                             ? VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
                             : VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

    VkBufferImageCopy copy = {0};
    copy.bufferOffset = 0;
    copy.bufferRowLength = 0;
    copy.bufferImageHeight = 0;
    copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy.imageSubresource.mipLevel = 0;
    copy.imageSubresource.baseArrayLayer = 0;
    copy.imageSubresource.layerCount = 1;
    copy.imageOffset.x = 0;
    copy.imageOffset.y = 0;
    copy.imageOffset.z = 0;
    copy.imageExtent.width = (uint32_t) width;
    copy.imageExtent.height = (uint32_t) height;
    copy.imageExtent.depth = 1;
    vkCmdCopyBufferToImage(frame->command, frame->staging, frame->texture,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);



    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    vkCmdPipelineBarrier(frame->command, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);



    barrier.oldLayout = backend->layouts[image_index];
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.image = backend->images[image_index];
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vkCmdPipelineBarrier(frame->command, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

    VkImageBlit blit = {0};
    blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit.srcSubresource.mipLevel = 0;
    blit.srcSubresource.baseArrayLayer = 0;
    blit.srcSubresource.layerCount = 1;
    blit.srcOffsets[0].x = 0;
    blit.srcOffsets[0].y = 0;
    blit.srcOffsets[0].z = 0;
    blit.srcOffsets[1].x = width;
    blit.srcOffsets[1].y = height;
    blit.srcOffsets[1].z = 1;
    blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit.dstSubresource.mipLevel = 0;
    blit.dstSubresource.baseArrayLayer = 0;
    blit.dstSubresource.layerCount = 1;
    blit.dstOffsets[0].x = 0;
    blit.dstOffsets[0].y = 0;
    blit.dstOffsets[0].z = 0;
    blit.dstOffsets[1].x = (int32_t) backend->extent.width;
    blit.dstOffsets[1].y = (int32_t) backend->extent.height;
    blit.dstOffsets[1].z = 1;
    vkCmdBlitImage(frame->command, frame->texture, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   backend->images[image_index], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit,
                   VK_FILTER_NEAREST);



    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = 0;
    vkCmdPipelineBarrier(frame->command, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL, 0, NULL, 1,
                         &barrier);

    return vkEndCommandBuffer(frame->command) == VK_SUCCESS;
}

static void vulkan_present(LIBMATTI_Renderer *renderer)
{
    LIBMATTI_VulkanBackend *backend = renderer->backend;
    if (backend == NULL || !backend->initialized || backend->failed)
        return;

    int window_width = 0, window_height = 0;
    LIBMATTI_GLFW_glfwGetFramebufferSize(backend->window, &window_width, &window_height);
    if (window_width <= 0 || window_height <= 0)
        return;


    if (backend->swapchain == VK_NULL_HANDLE || backend->need_recreate
        || (uint32_t) window_width != backend->extent.width
        || (uint32_t) window_height != backend->extent.height)
    {
        if (!recreate_swapchain(backend, window_width, window_height))
        {
            fprintf(stderr, "[vulkan] swapchain recreation failed: %s\n",
                    LIBMATTI_Render_GetLastError());
            backend->need_recreate = 1;
            return;
        }
        if (backend->swapchain == VK_NULL_HANDLE)
            return;
    }

    LIBMATTI_VulkanFrame *frame = &backend->frames[backend->frame_index];

    if (vkWaitForFences(backend->device, 1, &frame->in_flight, VK_TRUE, UINT64_MAX)
        != VK_SUCCESS)
    {
        LIBMATTI_Render_SetLastError("Vulkan frame fence wait failed");
        backend->failed = 1;
        return;
    }

    uint32_t image_index = 0;
    VkResult acquire = vkAcquireNextImageKHR(backend->device, backend->swapchain, UINT64_MAX,
                                             frame->image_available, VK_NULL_HANDLE,
                                             &image_index);
    if (acquire == VK_ERROR_OUT_OF_DATE_KHR)
    {
        backend->need_recreate = 1;
        return;
    }
    if (acquire != VK_SUCCESS && acquire != VK_SUBOPTIMAL_KHR)
    {
        LIBMATTI_Render_SetLastError("vkAcquireNextImageKHR failed (%d)", (int) acquire);
        backend->failed = acquire == VK_ERROR_DEVICE_LOST;
        return;
    }
    if (acquire == VK_SUBOPTIMAL_KHR)
        backend->need_recreate = 1;





    if (backend->image_in_flight[image_index] != VK_NULL_HANDLE)
    {
        if (vkWaitForFences(backend->device, 1, &backend->image_in_flight[image_index], VK_TRUE,
                            UINT64_MAX)
            != VK_SUCCESS)
        {
            LIBMATTI_Render_SetLastError("Vulkan image fence wait failed");
            backend->failed = 1;
            return;
        }
    }

    if (!ensure_frame_texture(backend, frame, renderer->width, renderer->height)
        || !ensure_staging(backend, frame,
                           (VkDeviceSize) renderer->width * (VkDeviceSize) renderer->height * 4u))
    {
        backend->need_recreate = 1;
        return;
    }

    copy_pixels_to_staging(backend, frame, renderer->pixels, renderer->width, renderer->height);

    if (!record_frame(backend, frame, image_index, renderer->width, renderer->height))
    {
        LIBMATTI_Render_SetLastError("Vulkan command recording failed");
        backend->need_recreate = 1;
        return;
    }

    if (vkResetFences(backend->device, 1, &frame->in_flight) != VK_SUCCESS)
    {
        LIBMATTI_Render_SetLastError("Vulkan fence reset failed");
        backend->need_recreate = 1;
        return;
    }

    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    VkSubmitInfo submit = {0};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &frame->image_available;
    submit.pWaitDstStageMask = &wait_stage;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &frame->command;
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &backend->render_finished[image_index];
    if (vkQueueSubmit(backend->queue, 1, &submit, frame->in_flight) != VK_SUCCESS)
    {
        LIBMATTI_Render_SetLastError("Vulkan queue submission failed");
        backend->failed = 1;
        return;
    }

    backend->image_in_flight[image_index] = frame->in_flight;
    backend->layouts[image_index] = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    frame->texture_layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    backend->frame_index = (backend->frame_index + 1) % VK_FRAMES_IN_FLIGHT;

    VkPresentInfoKHR present = {0};
    present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = &backend->render_finished[image_index];
    present.swapchainCount = 1;
    present.pSwapchains = &backend->swapchain;
    present.pImageIndices = &image_index;

    VkResult result = vkQueuePresentKHR(backend->queue, &present);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        backend->need_recreate = 1;
    else if (result != VK_SUCCESS)
    {
        LIBMATTI_Render_SetLastError("vkQueuePresentKHR failed (%d)", (int) result);
        backend->need_recreate = 1;
    }
}

static double vulkan_get_time(const LIBMATTI_Renderer *renderer)
{
    (void) renderer;
    return LIBMATTI_GLFW_glfwGetTime();
}

static void vulkan_set_title(LIBMATTI_Renderer *renderer, const char *title)
{
    LIBMATTI_VulkanBackend *backend = renderer->backend;
    if (backend != NULL && backend->window != 0)
        LIBMATTI_GLFW_glfwSetWindowTitle(backend->window, title);
}

static void *vulkan_get_native_window(const LIBMATTI_Renderer *renderer)
{
    const LIBMATTI_VulkanBackend *backend = renderer->backend;
    return backend != NULL ? (void *) (size_t) backend->window : NULL;
}

const LIBMATTI_RenderBackendOps *LIBMATTI_Render_VulkanOps(void)
{
    static const LIBMATTI_RenderBackendOps ops = {
        "vulkan",
        "Vulkan 1.0 (GLFW surface, swapchain blit, 2 frames in flight)",
        vulkan_is_available,
        vulkan_init,
        vulkan_shutdown,
        vulkan_poll_events,
        vulkan_should_close,
        vulkan_get_window_size,
        vulkan_present,
        vulkan_get_time,
        vulkan_set_title,
        vulkan_get_native_window,
        vulkan_get_mouse_position,
        vulkan_is_mouse_down,
        vulkan_is_key_down,
        vulkan_request_close,
    };
    return &ops;
}

#else


static int vulkan_is_available(void)
{
    return 0;
}

static int vulkan_init(LIBMATTI_Renderer *renderer)
{
    (void) renderer;
    LIBMATTI_Render_SetLastError(
        "Vulkan support was not compiled in (install the Vulkan headers/loader)");
    return 0;
}

static void vulkan_shutdown(LIBMATTI_Renderer *renderer)
{
    (void) renderer;
}

static void vulkan_poll_events(LIBMATTI_Renderer *renderer)
{
    (void) renderer;
}

static int vulkan_should_close(const LIBMATTI_Renderer *renderer)
{
    (void) renderer;
    return 1;
}

static void vulkan_get_window_size(const LIBMATTI_Renderer *renderer, int *width, int *height)
{
    if (width != NULL) *width = renderer->width;
    if (height != NULL) *height = renderer->height;
}

static void vulkan_present(LIBMATTI_Renderer *renderer)
{
    (void) renderer;
}

static double vulkan_get_time(const LIBMATTI_Renderer *renderer)
{
    (void) renderer;
    return 0.0;
}

static void vulkan_set_title(LIBMATTI_Renderer *renderer, const char *title)
{
    (void) renderer;
    (void) title;
}

static void *vulkan_get_native_window(const LIBMATTI_Renderer *renderer)
{
    (void) renderer;
    return NULL;
}

static void vulkan_get_mouse_position(const LIBMATTI_Renderer *renderer, double *x, double *y)
{
    (void) renderer;
    if (x != NULL) *x = 0.0;
    if (y != NULL) *y = 0.0;
}

static int vulkan_is_mouse_down(const LIBMATTI_Renderer *renderer, int button)
{
    (void) renderer;
    (void) button;
    return 0;
}

static int vulkan_is_key_down(const LIBMATTI_Renderer *renderer, int key)
{
    (void) renderer;
    (void) key;
    return 0;
}

static void vulkan_request_close(LIBMATTI_Renderer *renderer)
{
    (void) renderer;
}

const LIBMATTI_RenderBackendOps *LIBMATTI_Render_VulkanOps(void)
{
    static const LIBMATTI_RenderBackendOps ops = {
        "vulkan",
        "Vulkan (not compiled in)",
        vulkan_is_available,
        vulkan_init,
        vulkan_shutdown,
        vulkan_poll_events,
        vulkan_should_close,
        vulkan_get_window_size,
        vulkan_present,
        vulkan_get_time,
        vulkan_set_title,
        vulkan_get_native_window,
        vulkan_get_mouse_position,
        vulkan_is_mouse_down,
        vulkan_is_key_down,
        vulkan_request_close,
    };
    return &ops;
}

#endif
