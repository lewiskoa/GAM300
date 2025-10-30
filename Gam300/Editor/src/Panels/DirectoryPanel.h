#pragma once
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>

namespace Boom { class AppContext; }
struct GLFWwindow; // forward declare

namespace EditorUI {

    class Editor;

    class DirectoryPanel {
    public:
        explicit DirectoryPanel(Editor* owner);
        void Init();
        void OnShow();                 // Editor.cpp calls OnShow()
        void Render() { OnShow(); }    // wrapper

    private:
        // Helpers
        void RefreshUpdate();
        void DeleteUpdate();
        void PrintSelectedInfo();

        // Directory tree
        struct FileNode;
        std::unique_ptr<FileNode> BuildDirectoryTree();
        void RenderDirectoryTree(std::unique_ptr<FileNode> const& root);

        // Filesystem operations
        void CopyFilesToDirectory(const std::vector<std::string>& filePaths,
            const std::filesystem::path& targetDir);
        bool DeletePath(const std::filesystem::path& path);

        // Drag-and-drop callback
        static void OnDrop(GLFWwindow*, int count, const char** paths);

    private:
        // Owner/context
        Editor* m_Owner = nullptr;
        Boom::AppContext* m_Ctx = nullptr;

        // Config
        const std::filesystem::path ROOT_PATH{ "Resources" };
        const uint32_t  MAX_DEPTH = 7;
        const double    AUTO_REFRESH_SEC = 3.0;
        const std::string_view CUSTOM_PAYLOAD_TYPE{ "_GLFW_DROP" };

        // UI state
        std::unique_ptr<FileNode>      rootNode;
        std::string                    selectedPath;
        double                         rTimer = 0.0;
        std::unordered_map<std::string, bool> treeNodeOpenStatus;

        // Drag & drop
        inline static std::vector<std::string> droppedFiles{};
        inline static bool filesDropped{ false };
        std::filesystem::path dropTargetPath;

        // Delete handling
        bool        showDeleteConfirm{};
        bool        showDeleteError{};
        std::string deleteErrorMessage;
    };

} // namespace EditorUI
