#include "Panels/DirectoryPanel.h"
#include <filesystem>
#include <algorithm>
#include "Editor/EditorPCH.h"


struct DirectoryPanel::FileNode
{
    std::string name;
    bool isDirectory;
    std::vector<std::unique_ptr<FileNode>> children;
    std::filesystem::path fullPath;
    GLuint texId{};
    bool isHovered{};

    FileNode(const std::string& n, bool dir, const std::filesystem::path& path, GLuint id = 0)
        : name(n), isDirectory(dir), fullPath(path), texId(id) {}
};

// --------------------------------------
// Constructor
// --------------------------------------
BOOM_INLINE DirectoryPanel::DirectoryPanel(AppInterface* c)
    : IWidget(c)
    , folderIcon("Icons/folder.png", false)
    , assetIcon("Icons/asset.png", false)
{
}

// --------------------------------------
// Init
// --------------------------------------
BOOM_INLINE void DirectoryPanel::Init()
{
    rootNode = BuildDirectoryTree();
    glfwSetDropCallback(context->GetWindowHandle().get(), OnDrop);
    treeNodeOpenStatus[ROOT_PATH.string()] = true;
}

// --------------------------------------
// Main OnShow
// --------------------------------------
BOOM_INLINE void DirectoryPanel::OnShow()
{
    dropTargetPath.clear();

    if (ImGui::Begin("Project", nullptr, ImGuiWindowFlags_HorizontalScrollbar))
    {
        ImGui::Separator();

        if (rootNode)
            RenderDirectoryTree(rootNode);

        RefreshUpdate();
        PrintSelectedInfo();
        DeleteUpdate();
    }
    ImGui::End();

    if (filesDropped && !droppedFiles.empty())
    {
        std::filesystem::path targetDir{ dropTargetPath.empty() ? ROOT_PATH : dropTargetPath };
        CopyFilesToDirectory(droppedFiles, targetDir);
        droppedFiles.clear();
        dropTargetPath.clear();
        filesDropped = false;
    }
}

// --------------------------------------
// Refresh logic
// --------------------------------------
void DirectoryPanel::RefreshUpdate()
{
    if (ImGui::Button("Refresh") || (rTimer += context->GetDeltaTime()) > AUTO_REFRESH_TIMER)
    {
        rootNode = BuildDirectoryTree();
        rTimer = 0.0;
    }
}

// --------------------------------------
// Delete logic
// --------------------------------------
void DirectoryPanel::DeleteUpdate()
{
    if (!selectedPath.empty() && ImGui::IsKeyPressed(ImGuiKey_Delete, false))
        showDeleteConfirm = true;

    if (showDeleteConfirm)
    {
        ImGui::OpenPopup("Confirm Delete");
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    }

    if (ImGui::BeginPopupModal("Confirm Delete", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Are you sure you want to delete:\n%s?", selectedPath.c_str());
        ImGui::Separator();

        if (ImGui::Button("Yes", ImVec2(120, 0)) || ImGui::IsKeyPressed(ImGuiKey_Enter, false))
        {
            if (!DeletePath(selectedPath))
            {
                showDeleteError = true;
                deleteErrorMessage = "Failed to delete: " + selectedPath;
            }
            selectedPath.clear();
            showDeleteConfirm = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("No", ImVec2(120, 0)) || ImGui::IsKeyPressed(ImGuiKey_Escape))
        {
            showDeleteConfirm = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if (showDeleteError)
    {
        ImGui::OpenPopup("Delete Error");
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    }

    if (ImGui::BeginPopupModal("Delete Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("%s", deleteErrorMessage.c_str());
        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0)) || ImGui::IsKeyPressed(ImGuiKey_Escape))
        {
            showDeleteError = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

// --------------------------------------
// Info display
// --------------------------------------
void DirectoryPanel::PrintSelectedInfo()
{
    ImGui::Separator();
    ImGui::Text("Selected: %s", selectedPath.empty() ? "None" : selectedPath.c_str());
    if (!selectedPath.empty() && std::filesystem::exists(selectedPath))
    {
        ImGui::Text("Size: %lld bytes", std::filesystem::file_size(selectedPath));
    }
}

// --------------------------------------
// Tree building
// --------------------------------------
std::unique_ptr<DirectoryPanel::FileNode> DirectoryPanel::BuildDirectoryTree()
{
    auto root = std::make_unique<FileNode>(ROOT_PATH.filename().string(), true, ROOT_PATH);

    std::function<void(FileNode&, uint32_t)> scanDir = [&](FileNode& node, uint32_t depth)
        {
            if (depth > MAX_DEPTH || !std::filesystem::exists(node.fullPath)) return;

            for (const auto& entry : std::filesystem::directory_iterator(node.fullPath))
            {
                const auto& path = entry.path();
                bool isDir = entry.is_directory();
                GLuint texId = isDir ? (GLuint)folderIcon : (GLuint)assetIcon;

                auto child = std::make_unique<FileNode>(path.filename().string(), isDir, path, texId);
                if (isDir)
                    scanDir(*child, depth + 1);

                node.children.push_back(std::move(child));
            }
        };

    scanDir(*root, 0);
    return root;
}

// --------------------------------------
// Render tree
// --------------------------------------
void DirectoryPanel::RenderDirectoryTree(std::unique_ptr<FileNode> const& root)
{
    std::stable_sort(root->children.begin(), root->children.end(),
        [](const auto& a, const auto& b)
        {
            return a->isDirectory > b->isDirectory || (a->isDirectory == b->isDirectory && a->name < b->name);
        });

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
    if (root->children.empty()) flags |= ImGuiTreeNodeFlags_Leaf;

    bool isSelected = (selectedPath == root->fullPath.string());
    if (isSelected) flags |= ImGuiTreeNodeFlags_Selected;

    ImGui::PushID(root->fullPath.string().c_str());

    if (root->texId)
    {
        ImGui::Image((void*)(intptr_t)root->texId, ImVec2(24, 24));
        ImGui::SameLine();
    }

    bool nodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)root.get(), flags, root->isDirectory ? "%s/" : "%s", root->name.c_str());

    if (ImGui::IsItemClicked())
        selectedPath = root->fullPath.string();

    if (nodeOpen)
    {
        for (auto& child : root->children)
            RenderDirectoryTree(child);
        ImGui::TreePop();
    }

    ImGui::PopID();
}

// --------------------------------------
// Filesystem ops
// --------------------------------------
void DirectoryPanel::CopyFilesToDirectory(const std::vector<std::string>& filePaths, const std::filesystem::path& targetDir)
{
    for (const auto& filePath : filePaths)
    {
        std::filesystem::path src(filePath);
        if (!std::filesystem::exists(src)) continue;

        std::filesystem::path dest = targetDir / src.filename();
        try
        {
            if (std::filesystem::exists(dest))
            {
                std::string base = dest.stem().string();
                std::string ext = dest.extension().string();
                int i = 1;
                do { dest = targetDir / (base + " (" + std::to_string(i++) + ")" + ext); } while (std::filesystem::exists(dest));
            }
            std::filesystem::copy(src, dest, std::filesystem::copy_options::recursive);
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            BOOM_ERROR("DirectoryPanel::CopyFilesToDirectory: {}", e.what());
        }
    }
    rootNode = BuildDirectoryTree();
}

bool DirectoryPanel::DeletePath(const std::filesystem::path& path)
{
    try
    {
        if (!std::filesystem::exists(path)) return false;
        if (std::filesystem::is_directory(path))
            std::filesystem::remove_all(path);
        else
            std::filesystem::remove(path);

        rootNode = BuildDirectoryTree();
        return true;
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        BOOM_ERROR("DirectoryPanel::DeletePath: {}", e.what());
        return false;
    }
}

void DirectoryPanel::OnDrop(GLFWwindow*, int count, const char** paths)
{
    droppedFiles.clear();
    for (int i = 0; i < count; ++i)
        droppedFiles.push_back(paths[i]);
    filesDropped = true;
}
