#pragma once
#include "Helpers.h"

// Debug macros for DLL boundary debugging
#define DEBUG_DLL_BOUNDARY(func_name) \
    BOOM_INFO("[DLL_DEBUG] Entering {} at {}", func_name, __FILE__ ":" + std::to_string(__LINE__))

#define DEBUG_POINTER(ptr, name) \
    BOOM_INFO("[PTR_DEBUG] {} = {} (valid: {})", name, (void*)ptr, (ptr != nullptr))

#define DEBUG_OPENGL_STATE() \
    { \
        GLFWwindow* current = glfwGetCurrentContext(); \
        BOOM_INFO("[GL_DEBUG] Current context: {}", (void*)current); \
        if (current) { \
            BOOM_INFO("[GL_DEBUG] Context visible: {}", glfwGetWindowAttrib(current, GLFW_VISIBLE)); \
            BOOM_INFO("[GL_DEBUG] Context focused: {}", glfwGetWindowAttrib(current, GLFW_FOCUSED)); \
        } \
    }

#define DEBUG_IMGUI_STATE() \
    { \
        ImGuiContext* ctx = ImGui::GetCurrentContext(); \
        BOOM_INFO("[IMGUI_DEBUG] Current ImGui context: {}", (void*)ctx); \
        if (ctx) { \
            ImGuiIO& io = ImGui::GetIO(); \
            BOOM_INFO("[IMGUI_DEBUG] Backend platform: {}", io.BackendPlatformName ? io.BackendPlatformName : "NULL"); \
            BOOM_INFO("[IMGUI_DEBUG] Backend renderer: {}", io.BackendRendererName ? io.BackendRendererName : "NULL"); \
            BOOM_INFO("[IMGUI_DEBUG] Fonts loaded: {}", io.Fonts->Fonts.Size); \
        } \
    }

// Enhanced debugging functions for your specific use case
namespace DebugHelpers
{
    // Debug function to validate window handle across DLL boundary
    inline void ValidateWindowHandle(GLFWwindow* window, const char* location)
    {
        BOOM_INFO("[WINDOW_DEBUG] Validating window handle at {}", location);
        DEBUG_POINTER(window, "Window handle");

        if (window) {
            BOOM_INFO("[WINDOW_DEBUG] Window size: {}x{}",
                glfwGetWindowAttrib(window, GLFW_CLIENT_API),
                glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR));

            int width, height;
            glfwGetWindowSize(window, &width, &height);
            BOOM_INFO("[WINDOW_DEBUG] Window dimensions: {}x{}", width, height);

            int fbWidth, fbHeight;
            glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
            BOOM_INFO("[WINDOW_DEBUG] Framebuffer dimensions: {}x{}", fbWidth, fbHeight);
        }
    }

    // Debug function to validate frame data
    inline void ValidateFrameData(uint32_t frameId, const char* location)
    {
        BOOM_INFO("[FRAME_DEBUG] Validating frame data at {}", location);
        BOOM_INFO("[FRAME_DEBUG] Frame ID: {}", frameId);

        if (frameId != 0) {
            GLint bound_texture;
            glGetIntegerv(GL_TEXTURE_BINDING_2D, &bound_texture);
            BOOM_INFO("[FRAME_DEBUG] Currently bound texture: {}", bound_texture);

            // Check if the texture is valid
            GLboolean isTexture = glIsTexture(frameId);
            BOOM_INFO("[FRAME_DEBUG] Frame ID {} is valid texture: {}", frameId, isTexture);

            if (isTexture) {
                glBindTexture(GL_TEXTURE_2D, frameId);
                GLint width, height, format;
                glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
                glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
                glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &format);
                BOOM_INFO("[FRAME_DEBUG] Texture dimensions: {}x{}, format: {}", width, height, format);
                glBindTexture(GL_TEXTURE_2D, bound_texture); // Restore previous binding
            }
        }
    }

    // Debug function to check ImGui initialization state
    inline void ValidateImGuiState(const char* location)
    {
        BOOM_INFO("[IMGUI_INIT_DEBUG] Checking ImGui state at {}", location);
        DEBUG_IMGUI_STATE();
        DEBUG_OPENGL_STATE();

        // Check if ImGui is properly initialized
        ImGuiContext* ctx = ImGui::GetCurrentContext();
        if (ctx) {
            BOOM_INFO("[IMGUI_INIT_DEBUG] ImGui context exists");

            // Check if backends are initialized
            ImGuiIO& io = ImGui::GetIO();
            bool platformInit = io.BackendPlatformUserData != nullptr;
            bool rendererInit = io.BackendRendererUserData != nullptr;

            BOOM_INFO("[IMGUI_INIT_DEBUG] Platform backend initialized: {}", platformInit);
            BOOM_INFO("[IMGUI_INIT_DEBUG] Renderer backend initialized: {}", rendererInit);

            if (!platformInit || !rendererInit) {
                BOOM_ERROR("[IMGUI_INIT_DEBUG] ImGui backends not properly initialized!");
            }
        }
        else {
            BOOM_ERROR("[IMGUI_INIT_DEBUG] No ImGui context found!");
        }
    }

    // Debug function to trace data flow
    inline void TraceDataFlow(const void* data, size_t size, const char* dataType, const char* location)
    {
        BOOM_INFO("[DATA_FLOW] {} data at {}: ptr={}, size={}", dataType, location, data, size);

        if (data && size > 0) {
            // Log first few bytes for debugging (be careful with this in production)
            const uint8_t* bytes = static_cast<const uint8_t*>(data);
            std::string hexDump;
            size_t dumpSize = std::min(size, size_t(16)); // Only dump first 16 bytes

            for (size_t i = 0; i < dumpSize; ++i) {
                char buffer[4];
                snprintf(buffer, sizeof(buffer), "%02X ", bytes[i]);
                hexDump += buffer;
            }

            BOOM_INFO("[DATA_FLOW] First {} bytes: {}", dumpSize, hexDump);
        }
    }
}