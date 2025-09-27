#pragma once
#include "Widgets.h"
#include "DebugHelpers.h" // Include our debug helpers

struct GuiContext : AppInterface
{
    BOOM_INLINE virtual ~GuiContext()
    {
        DEBUG_DLL_BOUNDARY("GuiContext::~GuiContext");

        // Clean up ImGui in reverse order
        if (ImGui::GetCurrentContext()) {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        }
    }

    BOOM_INLINE virtual void OnGuiStart() {}
    BOOM_INLINE virtual void OnGuiFrame() {}

    BOOM_INLINE virtual void OnStart() override final
    {
        DEBUG_DLL_BOUNDARY("GuiContext::OnStart");

        std::shared_ptr<GLFWwindow> window = this->GetWindowHandle();
        if (!window) {
            BOOM_ERROR("GuiContext::OnStart - Invalid window handle!");
            return;
        }

        // CRITICAL: Ensure the engine's context is current
        if (!EnsureContextCurrent(window.get())) {
            BOOM_ERROR("GuiContext::OnStart - Failed to ensure context is current!");
            return;
        }

        DebugHelpers::ValidateWindowHandle(window.get(), "OnStart");
        DEBUG_OPENGL_STATE();

        // Initialize ImGui
        IMGUI_CHECKVERSION();

        // Check if ImGui context already exists
        ImGuiContext* existingCtx = ImGui::GetCurrentContext();
        if (existingCtx) {
            BOOM_WARN("GuiContext::OnStart - ImGui context already exists: {}", (void*)existingCtx);
        }
        else {
            ImGuiContext* ctx = ImGui::CreateContext();
            BOOM_INFO("GuiContext::OnStart - Created ImGui context: {}", (void*)ctx);
        }

        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigWindowsMoveFromTitleBarOnly = true;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

        // Initialize backends with validation
        bool platformNeedsInit = (io.BackendPlatformUserData == nullptr);
        bool rendererNeedsInit = (io.BackendRendererUserData == nullptr);

        BOOM_INFO("GuiContext::OnStart - Platform needs init: {}, Renderer needs init: {}",
            platformNeedsInit, rendererNeedsInit);

        bool platformInit = true, rendererInit = true;

        if (platformNeedsInit) {
            platformInit = ImGui_ImplGlfw_InitForOpenGL(window.get(), true);
            BOOM_INFO("GuiContext::OnStart - Platform backend init result: {}", platformInit);
        }

        if (rendererNeedsInit) {
            rendererInit = ImGui_ImplOpenGL3_Init(SHADER_VERSION);
            BOOM_INFO("GuiContext::OnStart - Renderer backend init result: {}", rendererInit);
        }

        if (!platformInit || !rendererInit) {
            BOOM_ERROR("GuiContext::OnStart - Failed to initialize ImGui backends!");
            return;
        }

        // Load fonts only if needed
        if (io.Fonts->Fonts.Size == 0) {
            LoadFonts();
        }

        // Set imgui style
        ImGui::StyleColorsDark();

        // Validate ImGui state after initialization
        DebugHelpers::ValidateImGuiState("After initialization");

        // Attach event callback
        AttachCallback<SelectEvent>([this](auto e)
            {
                DEBUG_DLL_BOUNDARY("SelectEvent callback");
                for (auto& window : m_Windows) {
                    if (window) {
                        window->OnSelect(ToEntt<Entity>(e.EnttID));
                    }
                }
            });

        // Store the window for context restoration
        m_EngineWindow = window;

        OnGuiStart();
        BOOM_INFO("GuiContext::OnStart completed successfully");
    }

    BOOM_INLINE void OnUpdate() override final
    {
        DEBUG_DLL_BOUNDARY("GuiContext::OnUpdate");

        // Ensure context is current before every frame
        if (!EnsureContextCurrent(m_EngineWindow.get())) {
            BOOM_ERROR("GuiContext::OnUpdate - Context lost and cannot be restored!");
            return;
        }

        // Validate ImGui state occasionally
        static int frameCount = 0;
        if (++frameCount % 300 == 0) { // Every 5 seconds at 60fps
            DebugHelpers::ValidateImGuiState("Periodic validation");
        }

        // Initialize ImGui for the frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Set up the main viewport
        static auto viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowPos(viewport->Pos);

        // Define main window flags
        static auto flags =
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoNavFocus |
            ImGuiWindowFlags_NoBackground |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_MenuBar;

        // Set window style
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 1));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);

        // Begin the main window
        bool windowOpen = ImGui::Begin("MainWindow", NULL, flags);
        if (windowOpen)
        {
            // Set up the main dockspace
            ImGui::DockSpace(ImGui::GetID("MainDockspace"), ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode);
            ImGui::PopStyleColor();
            ImGui::PopStyleVar(3);

            // Iterate through each window in the vector and call OnShow
            for (auto& window : m_Windows)
            {
                if (window) {
                    window->OnShow(this);
                }
            }

            // Show ImGui Demo Window for debugging purposes
            ImGui::ShowDemoWindow();

            // Interface update
            OnGuiFrame();
        }
        ImGui::End();

        // Render ImGui draw data
        ImGui::Render();

        ImDrawData* drawData = ImGui::GetDrawData();
        if (drawData && drawData->Valid) {
            ImGui_ImplOpenGL3_RenderDrawData(drawData);
        }
        else {
            BOOM_WARN("GuiContext::OnUpdate - Invalid draw data!");
        }
    }

    template<typename T, typename... Args>
    BOOM_INLINE void AttachWindow(Args&&... args)
    {
        BOOM_STATIC_ASSERT(std::is_base_of<IWidget, T>::value);
        auto window = std::make_unique<T>(this, std::forward<Args>(args)...);
        BOOM_INFO("GuiContext::AttachWindow - Created window: {}", (void*)window.get());
        m_Windows.push_back(std::move(window));
    }

    template<typename T, typename... Args>
    BOOM_INLINE auto CreateWidget(Args&&... args)
    {
        BOOM_STATIC_ASSERT(std::is_base_of<IWidget, T>::value);
        auto widget = std::make_unique<T>(this, std::forward<Args>(args)...);
        BOOM_INFO("GuiContext::CreateWidget - Created widget: {}", (void*)widget.get());
        return widget;
    }

private:
    // Helper function to ensure OpenGL context is current
    BOOM_INLINE bool EnsureContextCurrent(GLFWwindow* window)
    {
        if (!window) return false;

        GLFWwindow* currentContext = glfwGetCurrentContext();
        if (currentContext != window) {
            BOOM_INFO("GuiContext::EnsureContextCurrent - Restoring context: {} -> {}",
                (void*)currentContext, (void*)window);

            glfwMakeContextCurrent(window);

            currentContext = glfwGetCurrentContext();
            if (currentContext != window) {
                BOOM_ERROR("GuiContext::EnsureContextCurrent - Failed to restore context!");
                return false;
            }

            BOOM_INFO("GuiContext::EnsureContextCurrent - Context restored successfully");
        }

        return true;
    }

    // Helper function to load fonts
    BOOM_INLINE void LoadFonts()
    {
        BOOM_INFO("GuiContext::LoadFonts - Loading fonts...");

        ImGuiIO& io = ImGui::GetIO();
        ImFontConfig fontConfig;
        fontConfig.MergeMode = true;
        fontConfig.PixelSnapH = true;
        static const ImWchar iconRange[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };

        // Regular font and icon
        ImFont* regularFont = io.Fonts->AddFontFromFileTTF(FONT_FILE, REGULAR_FONT_SIZE);
        //ImFont* regularIcon = io.Fonts->AddFontFromFileTTF(ICON_FONT, REGULAR_FONT_SIZE, &fontConfig, iconRange);

        // Small font and icon
        ImFont* smallFont = io.Fonts->AddFontFromFileTTF(FONT_FILE, SMALL_FONT_SIZE);
        //ImFont* smallIcon = io.Fonts->AddFontFromFileTTF(ICON_FONT, SMALL_FONT_SIZE, &fontConfig, iconRange);

        BOOM_INFO("GuiContext::LoadFonts - Loaded fonts: regular={}, small={}",
            (void*)regularFont, (void*)smallFont);

        // Build font atlas
        io.Fonts->Build();
    }

private:
    std::vector<std::unique_ptr<IWidget>> m_Windows;
    std::shared_ptr<GLFWwindow> m_EngineWindow = nullptr;
};

struct GuiContextNoSwitch : AppInterface
{
    BOOM_INLINE virtual ~GuiContextNoSwitch()
    {
        if (ImGui::GetCurrentContext()) {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        }
        // NEVER call glfwTerminate() or glfwMakeContextCurrent()
    }

    BOOM_INLINE void InitializeWithExistingContext(GLFWwindow* window)
    {
        BOOM_INFO("GuiContextNoSwitch::InitializeWithExistingContext - Window: {}", (void*)window);

        // Don't call OnStart() - use this custom initialization instead
        m_Window = window;

        // Verify current context works
        GLFWwindow* current = glfwGetCurrentContext();
        if (!current) {
            BOOM_ERROR("InitializeWithExistingContext - No current context!");
            return;
        }

        BOOM_INFO("InitializeWithExistingContext - Current context: {}", (void*)current);

        // Test OpenGL state
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            BOOM_ERROR("InitializeWithExistingContext - OpenGL error: {}", error);
            return;
        }

        // Initialize ImGui with the current context
        IMGUI_CHECKVERSION();
        ImGuiContext* ctx = ImGui::CreateContext();
        BOOM_INFO("InitializeWithExistingContext - Created ImGui context: {}", (void*)ctx);

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigWindowsMoveFromTitleBarOnly = true;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

        // Initialize backends with the current window/context
        bool platformInit = ImGui_ImplGlfw_InitForOpenGL(current, true);
        bool rendererInit = ImGui_ImplOpenGL3_Init(SHADER_VERSION);

        BOOM_INFO("InitializeWithExistingContext - Platform: {}, Renderer: {}", platformInit, rendererInit);

        if (!platformInit || !rendererInit) {
            BOOM_ERROR("InitializeWithExistingContext - Backend initialization failed!");
            return;
        }

        // Load fonts
        LoadFonts();

        // Set style
        ImGui::StyleColorsDark();

        BOOM_INFO("InitializeWithExistingContext - Initialization complete!");
    }

    BOOM_INLINE void OnUpdate()
    {
        // Never try to switch contexts - just work with whatever is current
        GLFWwindow* current = glfwGetCurrentContext();
        if (!current) {
            BOOM_WARN("GuiContextNoSwitch::OnUpdate - No current context, skipping frame");
            return;
        }

        // Standard ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Create main dockspace window
        static auto flags = ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_MenuBar;

        auto viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowPos(viewport->Pos);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 1));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);

        if (ImGui::Begin("MainWindow", NULL, flags)) {
            ImGui::DockSpace(ImGui::GetID("MainDockspace"), ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode);
            ImGui::PopStyleColor();
            ImGui::PopStyleVar(3);

            // Show windows
            for (auto& vwindow : m_Windows) {
                if (vwindow) {
                    vwindow->OnShow(this);
                }
            }

            // Demo window for testing
            ImGui::ShowDemoWindow();
        }
        ImGui::End();

        // Render
        ImGui::Render();
        ImDrawData* drawData = ImGui::GetDrawData();
        if (drawData && drawData->Valid) {
            ImGui_ImplOpenGL3_RenderDrawData(drawData);
        }
    }

    template<typename T, typename... Args>
    BOOM_INLINE void AttachWindow(Args&&... args)
    {
        auto window = std::make_unique<T>(this, std::forward<Args>(args)...);
        m_Windows.push_back(std::move(window));
    }

private:
    BOOM_INLINE void LoadFonts()
    {
        ImGuiIO& io = ImGui::GetIO();
        ImFontConfig fontConfig;
        fontConfig.MergeMode = true;
        fontConfig.PixelSnapH = true;
        static const ImWchar iconRange[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };

        io.Fonts->AddFontFromFileTTF(FONT_FILE, REGULAR_FONT_SIZE);
        io.Fonts->AddFontFromFileTTF(ICON_FONT, REGULAR_FONT_SIZE, &fontConfig, iconRange);
        io.Fonts->AddFontFromFileTTF(FONT_FILE, SMALL_FONT_SIZE);
        io.Fonts->AddFontFromFileTTF(ICON_FONT, SMALL_FONT_SIZE, &fontConfig, iconRange);

        io.Fonts->Build();
    }

private:
    std::vector<std::unique_ptr<IWidget>> m_Windows;
    GLFWwindow* m_Window = nullptr;
};