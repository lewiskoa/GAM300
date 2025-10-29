#pragma once
#include "Context/Context.h"
#include "Auxiliaries/Assets.h"

namespace Boom { class Application; }


// Replica of Windows Explorer for browsing resources.
class DirectoryPanel : public IWidget
{
public:
    BOOM_INLINE DirectoryPanel(AppInterface* context);
    BOOM_INLINE void Init();
    BOOM_INLINE void OnShow() override;

private:
    // --- Helpers ---
    void RefreshUpdate();
    void DeleteUpdate();
    void PrintSelectedInfo();

    // --- Directory tree logic ---
    struct FileNode;
    std::unique_ptr<FileNode> BuildDirectoryTree();
    void RenderDirectoryTree(std::unique_ptr<FileNode> const& root);

    // --- Filesystem operations ---
    void CopyFilesToDirectory(const std::vector<std::string>& filePaths, const std::filesystem::path& targetDir);
    bool DeletePath(const std::filesystem::path& path);

    // --- Drag-and-drop callback ---
    static void OnDrop(GLFWwindow*, int count, const char** paths);

private:
    // --- Config ---
    const std::filesystem::path ROOT_PATH{ "Resources" };
    const uint32_t MAX_DEPTH{ 7 };
    const double AUTO_REFRESH_TIMER{ 3.0 };
    const std::string_view CUSTOM_PAYLOAD_TYPE{ "_GLFW_DROP" };

    // --- UI Data ---
    Texture2D folderIcon;
    Texture2D assetIcon;
    std::unique_ptr<FileNode> rootNode;
    std::string selectedPath;

    double rTimer{};
    std::unordered_map<std::string, bool> treeNodeOpenStatus;

    // Drag and drop
    inline static std::vector<std::string> droppedFiles{};
    inline static bool filesDropped{ false };
    std::filesystem::path dropTargetPath;

    // Delete handling
    bool showDeleteConfirm{};
    bool showDeleteError{};
    std::string deleteErrorMessage;
};
