#include "BoomEngine.h"
#include "Vendors/imgui/imgui.h"
#include "Windows/Inspector.h"
#include "Windows/Hierarchy.h"
#include "Windows/Resource.h"
#include "Windows/Viewport.h"
#include "Windows/MenuBar.h"
#include "Context/DebugHelpers.h"
using namespace Boom;

//struct Editor : AppInterface {};






class EditorDebugTest
{
public:
    void TestDLLBoundary()
    {
        BOOM_INFO("=== Starting DLL Boundary Debug Test ===");

        // Test 1: Validate engine context
        TestEngineContext();

        // Test 2: Test ImGui context sharing
        TestImGuiContextSharing();

        // Test 3: Test frame data transfer
        TestFrameDataTransfer();

        // Test 4: Test widget creation
        TestWidgetCreation();

        BOOM_INFO("=== Debug Test Complete ===");
    }

public:
    void TestEngineContext()
    {
        BOOM_INFO("--- Testing Engine Context ---");

        // Assuming you have access to your engine application
        // Replace 'yourEngineApp' with your actual engine instance
        /*
        auto* app = GetYourEngineApplication(); // Replace with your method

        if (app) {
            DEBUG_POINTER(app, "Engine Application");

            GLFWwindow* window = app->GetWindowHandle();
            DebugHelpers::ValidateWindowHandle(window, "TestEngineContext");

            uint32_t frame = app->GetSceneFrame();
            DebugHelpers::ValidateFrameData(frame, "TestEngineContext");
        } else {
            BOOM_ERROR("TestEngineContext - No engine application available!");
        }
        */

        BOOM_INFO("TestEngineContext - Replace with your engine access code");
    }

    void TestGuiContextAfterEngine(Application* app)
    {
        if (!app) {
            BOOM_ERROR("TestGuiContextAfterEngine - No application provided!");
            return;
        }

        BOOM_INFO("=== Testing GuiContext with Running Engine ===");

        // Validate engine state
        GLFWwindow* engineWindow = app->GetWindowHandle();
        DebugHelpers::ValidateWindowHandle(engineWindow, "Engine window");

        uint32_t frameId = app->GetSceneFrame();
        DebugHelpers::ValidateFrameData(frameId, "Engine frame");

        // Test creating GuiContext with engine running
        try {
            BOOM_INFO("Creating GuiContext...");
            auto guiContext = std::make_unique<GuiContext>();

            BOOM_INFO("Calling OnStart...");
            guiContext->OnStart();

            BOOM_INFO("Testing widget creation...");
            auto viewport = guiContext->CreateWidget<ViewportWindow>();

            if (viewport) {
                BOOM_INFO("ViewportWindow created successfully!");
                viewport->DebugViewportState();

                // Test a few OnShow calls
                for (int i = 0; i < 3; ++i) {
                    BOOM_INFO("Testing OnShow call {}", i);
                    viewport->OnShow(guiContext.get());
                }
            }

            BOOM_INFO("GuiContext test completed successfully!");

        }
        catch (const std::exception& e) {
            BOOM_ERROR("GuiContext test failed: {}", e.what());
        }

        BOOM_INFO("=== GuiContext Test Complete ===");
    }

    void TestImGuiContextSharing()
    {
        BOOM_INFO("--- Testing ImGui Context Sharing ---");

        // Check if ImGui context exists before creating GuiContext
        ImGuiContext* beforeCtx = ImGui::GetCurrentContext();
        BOOM_INFO("ImGui context before GuiContext creation: {}", (void*)beforeCtx);

        // Test creating GuiContext (this should be your main test)
        try {
            // Replace this with your actual GuiContext creation
            /*
            auto guiContext = std::make_unique<GuiContext>();
            guiContext->OnStart(); // This should trigger the debugging
            */
            BOOM_INFO("GuiContext creation test - Add your creation code here");

        }
        catch (const std::exception& e) {
            BOOM_ERROR("GuiContext creation failed: {}", e.what());
        }

        // Check context after
        ImGuiContext* afterCtx = ImGui::GetCurrentContext();
        BOOM_INFO("ImGui context after GuiContext creation: {}", (void*)afterCtx);

        if (beforeCtx != afterCtx) {
            BOOM_WARN("ImGui context changed during GuiContext creation!");
        }
    }

    void TestFrameDataTransfer()
    {
        BOOM_INFO("--- Testing Frame Data Transfer ---");

        // Test getting frame data multiple times to check consistency
        /*
        auto* app = GetYourEngineApplication(); // Replace with your method

        if (app) {
            for (int i = 0; i < 5; ++i) {
                uint32_t frame = app->GetSceneFrame();
                BOOM_INFO("Frame data test {}: ID={}", i, frame);
                DebugHelpers::ValidateFrameData(frame, "TestFrameDataTransfer iteration");

                // Add small delay to see if frame changes
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
        */

        BOOM_INFO("TestFrameDataTransfer - Add your frame data access code here");
    }

    void TestWidgetCreation()
    {
        BOOM_INFO("--- Testing Widget Creation ---");

        try {
            // Test creating a simple widget without GuiContext first
            /*
            auto widget = std::make_unique<ViewportWindow>(nullptr);
            if (widget) {
                BOOM_INFO("Widget created successfully (null context test)");
            }
            */

            // Then test with actual GuiContext
            /*
            auto guiContext = std::make_unique<GuiContext>();
            auto widget = guiContext->CreateWidget<ViewportWindow>();
            if (widget) {
                BOOM_INFO("Widget created successfully with GuiContext");

                // Test the widget's debug function
                widget->DebugViewportState();
            }
            */

            BOOM_INFO("TestWidgetCreation - Add your widget creation code here");

        }
        catch (const std::exception& e) {
            BOOM_ERROR("Widget creation failed: {}", e.what());
        }
    }
};

// Function to call from your editor's main function or initialization
void RunImGuiDebugTest()
{
    BOOM_INFO("=== Starting DLL Boundary Debug Test ===");

    // Test OpenGL context state BEFORE creating GuiContext
    {
        BOOM_INFO("--- Testing Initial OpenGL State ---");
        GLFWwindow* currentContext = glfwGetCurrentContext();
        DEBUG_POINTER(currentContext, "Current OpenGL Context");

        if (currentContext) {
            DebugHelpers::ValidateWindowHandle(currentContext, "Initial state");
            DEBUG_OPENGL_STATE();
        }
        else {
            BOOM_ERROR("No OpenGL context is current! This will cause ImGui initialization to fail.");
        }
    }

    // Test ImGui context state
    {
        BOOM_INFO("--- Testing Initial ImGui State ---");
        ImGuiContext* ctx = ImGui::GetCurrentContext();
        DEBUG_POINTER(ctx, "Current ImGui Context");

        if (ctx) {
            ImGuiIO& io = ImGui::GetIO();
            BOOM_INFO("Platform backend: {}", io.BackendPlatformName ? io.BackendPlatformName : "NULL");
            BOOM_INFO("Renderer backend: {}", io.BackendRendererName ? io.BackendRendererName : "NULL");
            BOOM_INFO("Fonts loaded: {}", io.Fonts->Fonts.Size);
        }
    }

    BOOM_INFO("=== Debug Test Complete ===");
}

// Example of how to integrate this into your editor's main loop
/*
int main()
{
    // Your engine initialization...

    // Run the debug test
    RunImGuiDebugTest();

    // Your main loop...
    while (running) {
        // Engine update...

        // ImGui update with debugging
        if (guiContext) {
            DebugHelpers::ValidateImGuiState("Before OnUpdate");
            guiContext->OnUpdate();
            DebugHelpers::ValidateImGuiState("After OnUpdate");
        }

        // Rest of your loop...
    }

    return 0;
}
*/

class Editor : public AppInterface
{
public:
    BOOM_INLINE Editor() = default;

    BOOM_INLINE void OnStart() override
    {
        BOOM_INFO("Editor::OnStart - Starting WITHOUT context switching");

        m_InitialWindow = GetWindowHandle();
        m_LastValidWindow = m_InitialWindow;

        GLFWwindow* currentContext = glfwGetCurrentContext();

        BOOM_INFO("Editor::OnStart - Current context: {}", (void*)currentContext);
        BOOM_INFO("Editor::OnStart - Engine window: {}", (void*)m_InitialWindow);

        if (currentContext) {
            CreateImGuiWithCurrentContext(currentContext);
        }
        else if (m_InitialWindow) {
            BOOM_WARN("Editor::OnStart - Deferring ImGui creation to OnUpdate()");
            m_DeferredInit = true;
        }
        else {
            BOOM_ERROR("Editor::OnStart - No context and no engine window!");
            return;
        }

        BOOM_INFO("Editor::OnStart - Completed successfully");
    }

    BOOM_INLINE void OnUpdate() override
    {
        static int frameCount = 0;
        frameCount++;

        GLFWwindow* currentWindow = GetWindowHandle();

        if (m_DeferredInit) {
            GLFWwindow* currentContext = glfwGetCurrentContext();

            if (frameCount % 60 == 0) {
                BOOM_INFO("Editor::OnUpdate [Frame {}] - Deferred init check: current={}, window={}",
                    frameCount, (void*)currentContext, (void*)currentWindow);
            }

            if (currentContext) {
                BOOM_INFO("Editor::OnUpdate - Context available for deferred init: {}", (void*)currentContext);
                CreateImGuiWithCurrentContext(currentContext);
                m_DeferredInit = false;
            }
            else if (currentWindow) {
                if (frameCount % 60 == 0) {
                    BOOM_INFO("Editor::OnUpdate - Attempting to fix window and force context...");

                    // Check window properties
                    int width, height;
                    glfwGetWindowSize(currentWindow, &width, &height);
                    int visible = glfwGetWindowAttrib(currentWindow, GLFW_VISIBLE);
                    int focused = glfwGetWindowAttrib(currentWindow, GLFW_FOCUSED);

                    BOOM_INFO("Editor::OnUpdate - Window state: size={}x{}, visible={}, focused={}",
                        width, height, visible, focused);

                    // If window has zero size, try to fix it
                    if (width == 0 || height == 0) {
                        BOOM_INFO("Editor::OnUpdate - Fixing window size...");

                        // Make window visible first
                        if (!visible) {
                            BOOM_INFO("Editor::OnUpdate - Making window visible...");
                            glfwShowWindow(currentWindow);
                        }

                        // Set a reasonable size
                        BOOM_INFO("Editor::OnUpdate - Setting window size to 1280x720...");
                        glfwSetWindowSize(currentWindow, 1280, 720);

                        // Force window to front
                        glfwFocusWindow(currentWindow);

                        // Check if it worked
                        glfwGetWindowSize(currentWindow, &width, &height);
                        BOOM_INFO("Editor::OnUpdate - New window size: {}x{}", width, height);
                    }

                    if (width > 0 && height > 0) {
                        BOOM_INFO("Editor::OnUpdate - Window is valid, forcing context...");

                        // Now try to make context current
                        glfwMakeContextCurrent(currentWindow);

                        GLFWwindow* newCurrent = glfwGetCurrentContext();
                        if (newCurrent == currentWindow) {
                            BOOM_INFO("Editor::OnUpdate - Successfully forced engine context current!");
                            CreateImGuiWithCurrentContext(newCurrent);
                            m_DeferredInit = false;
                        }
                        else {
                            BOOM_ERROR("Editor::OnUpdate - Failed to force engine context current");
                        }
                    }
                    else {
                        BOOM_ERROR("Editor::OnUpdate - Still can't fix window size");
                    }
                }
            }
            else {
                if (frameCount % 300 == 0) {
                    BOOM_ERROR("Editor::OnUpdate - No window available after {} frames", frameCount);
                }
                return;
            }
        }

        if (m_GuiContext) {
            GLFWwindow* currentContext = glfwGetCurrentContext();
            if (currentContext) {
                m_GuiContext->OnUpdate();

                if (frameCount % 300 == 0) {
                    BOOM_INFO("Editor::OnUpdate - ImGui running successfully at frame {}", frameCount);
                }
            }
            else {
                BOOM_WARN("Editor::OnUpdate - Lost OpenGL context during runtime!");
            }
        }
    }

private:
    BOOM_INLINE void CreateImGuiWithCurrentContext(GLFWwindow* context)
    {
        BOOM_INFO("CreateImGuiWithCurrentContext - Using context: {}", (void*)context);

        try {
            GLenum error = glGetError();
            if (error != GL_NO_ERROR) {
                BOOM_ERROR("CreateImGuiWithCurrentContext - OpenGL error before init: {}", error);
                return;
            }

            m_GuiContext = std::make_unique<GuiContextNoSwitch>();
            m_GuiContext->InitializeWithExistingContext(context);
            m_GuiContext->AttachWindow<ViewportWindow>();

            BOOM_INFO("CreateImGuiWithCurrentContext - Success!");

        }
        catch (const std::exception& e) {
            BOOM_ERROR("CreateImGuiWithCurrentContext - Failed: {}", e.what());
        }
    }

private:
    std::unique_ptr<GuiContextNoSwitch> m_GuiContext;
    bool m_DeferredInit = false;
    GLFWwindow* m_InitialWindow = nullptr;
    GLFWwindow* m_LastValidWindow = nullptr;
};

int32_t main()
{
    try {
        MyEngineClass engine;
        engine.whatup();
        BOOM_INFO("Editor Started");

        // Create application
        auto app = std::make_unique<Application>();
        app->PostEvent<WindowTitleRenameEvent>("Boom Editor - Press 'Esc' to quit. 'WASD' to pan camera");

        // DON'T do the context killing test - just attach the editor and run
        app->AttachLayer<Editor>();

        // This is the key line you were missing - actually run the application!
        app->RunContext(false);

    }
    catch (const std::exception& e) {
        BOOM_ERROR("Application failed: {}", e.what());
        return -1;
    }

    return 0;
}