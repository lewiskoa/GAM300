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

        BOOM_INFO("Editor::OnStart - Initializing GLFW for Editor DLL");
		//auto vwindow = GetWindowHandle();
        //ImGui_ImplGlfw_InitForOpenGL(vwindow, true);
        //ImGui_ImplOpenGL3_Init("#version 450");

        if (!glfwInit()) {
            BOOM_ERROR("Editor::OnStart - Failed to initialize GLFW!");
            return;
        }

        BOOM_INFO("Editor::OnStart - GLFW initialized successfully");

        // Don't try to force context switching here - let it happen during main loop
        GLFWwindow* engineWindow = GetWindowHandle();
        BOOM_INFO("Editor::OnStart - Engine window: {} (size check will happen in OnUpdate)", (void*)engineWindow);

        // Set flag to initialize ImGui when context becomes available
        m_WaitingForContext = true;

        BOOM_INFO("Editor::OnStart - Will initialize ImGui when engine context becomes current");
    }

    BOOM_INLINE void OnUpdate() override
    {
        static int frameCount = 0;
        frameCount++;

        // Check if engine has made its context current
        GLFWwindow* currentContext = glfwGetCurrentContext();
        GLFWwindow* engineWindow = GetWindowHandle();

        if (m_WaitingForContext && currentContext) {
            BOOM_INFO("Editor::OnUpdate [Frame {}] - Engine context is current: {}", frameCount, (void*)currentContext);
            // Force context current every frame until ImGui initializes
            glfwMakeContextCurrent(engineWindow);

            GLFWwindow* current = glfwGetCurrentContext();
            if (current == engineWindow) {
                BOOM_INFO("Editor forced context current at frame {}", frameCount);

                try {
                    CreateImGuiWithCurrentContext(current);
                    m_WaitingForContext = false;
                }
                catch (const std::exception& e) {
                    BOOM_ERROR("ImGui init failed: {}", e.what());
                }
            }
        }

        if (m_GuiContext && currentContext) {
            // Update ImGui only when we have a valid context
            m_GuiContext->OnUpdate();

            if (frameCount % 300 == 0) {
                BOOM_INFO("Editor::OnUpdate - ImGui running at frame {} with context {}",
                    frameCount, (void*)currentContext);
            }
        }
        else if (m_WaitingForContext) {
            // Still waiting for context
            if (frameCount % 180 == 0) { // Every 3 seconds
                BOOM_INFO("Editor::OnUpdate - Waiting for engine context... Frame {}, Current: {}",
                    frameCount, (void*)currentContext);
            }
        }
        else if (!currentContext) {
            // Lost context during runtime
            if (frameCount % 60 == 0) {
                BOOM_WARN("Editor::OnUpdate - Lost OpenGL context at frame {}", frameCount);
            }
        }
    }

    BOOM_INLINE ~Editor()
    {
        m_GuiContext.reset();
        BOOM_INFO("Editor::~Editor - Cleaned up");
    }

private:
    BOOM_INLINE void CreateImGuiWithCurrentContext(GLFWwindow* context)
    {
        BOOM_INFO("CreateImGuiWithCurrentContext - Using context: {}", (void*)context);

        // Get window dimensions for ImGui
        GLFWwindow* engineWindow = GetWindowHandle();
        int width, height;
        glfwGetWindowSize(engineWindow, &width, &height);
        BOOM_INFO("CreateImGuiWithCurrentContext - Window size: {}x{}", width, height);

        m_GuiContext = std::make_unique<GuiContextNoSwitch>();
        m_GuiContext->InitializeWithExistingContext(context);
        m_GuiContext->AttachWindow<ViewportWindow>();

        BOOM_INFO("CreateImGuiWithCurrentContext - Success!");
    }

private:
    std::unique_ptr<GuiContextNoSwitch> m_GuiContext;
    bool m_WaitingForContext = false;
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

        // Initialize GLFW in editor
        //if (!glfwInit()) {
        //    BOOM_ERROR("Failed to initialize GLFW in editor");
        //    return -1;
        //}

        // Get the engine window handle
        GLFWwindow* engineWindow = app->GetWindowHandle();
        BOOM_INFO("Got engine window handle: {}", (void*)engineWindow);

        if (engineWindow) {
            // Test window properties
            int width, height;
            glfwGetWindowSize(engineWindow, &width, &height);
            BOOM_INFO("Window size: {}x{}", width, height);

            // Try to make context current
            BOOM_INFO("Attempting to make context current...");
            glfwMakeContextCurrent(engineWindow);

            GLFWwindow* current = glfwGetCurrentContext();
            BOOM_INFO("Current context after switch: {}", (void*)current);

            if (current == engineWindow) {
                BOOM_INFO("SUCCESS! Context is current, initializing ImGui...");

                // Initialize ImGui
                IMGUI_CHECKVERSION();
                ImGuiContext* ctx = ImGui::CreateContext();
                BOOM_INFO("Created ImGui context: {}", (void*)ctx);

                ImGuiIO& io = ImGui::GetIO();
                io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
                io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

                // Initialize backends
                bool platformInit = ImGui_ImplGlfw_InitForOpenGL(engineWindow, true);
                bool rendererInit = ImGui_ImplOpenGL3_Init("#version 450");

                BOOM_INFO("Platform backend init: {}", platformInit);
                BOOM_INFO("Renderer backend init: {}", rendererInit);

                if (platformInit && rendererInit) {
                    BOOM_INFO("ImGui initialized successfully in main()!");

                    // Set style
                    ImGui::StyleColorsDark();

                    // Test a simple ImGui frame
                    ImGui_ImplOpenGL3_NewFrame();
                    ImGui_ImplGlfw_NewFrame();
                    ImGui::NewFrame();

                    ImGui::Begin("Test Window");
                    ImGui::Text("Hello from main()!");
                    ImGui::End();

                    ImGui::Render();
                    ImDrawData* drawData = ImGui::GetDrawData();
                    if (drawData && drawData->Valid) {
                        ImGui_ImplOpenGL3_RenderDrawData(drawData);
                        BOOM_INFO("ImGui test render successful!");
                    }

                    // Clean up
                    ImGui_ImplOpenGL3_Shutdown();
                    ImGui_ImplGlfw_Shutdown();
                    ImGui::DestroyContext();
                }
                else {
                    BOOM_ERROR("ImGui backend initialization failed");
                }
            }
            else {
                BOOM_ERROR("Failed to make context current. Expected: {}, Got: {}",
                    (void*)engineWindow, (void*)current);
            }
        }
        else {
            BOOM_ERROR("No engine window handle available");
        }

        // Run the application (remove the Editor layer for this test)
        // app->AttachLayer<Editor>();
        app->RunContext(false);

    }
    catch (const std::exception& e) {
        BOOM_ERROR("Application failed: {}", e.what());
        return -1;
    }

    return 0;
}