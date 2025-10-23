//replica of the windows explorer for resources browsing
#pragma once

#include "Context/Context.h"

struct DirectoryWindow : IWidget {
private:
	const std::filesystem::path ROOT_PATH{ "Resources" };
	const uint32_t MAX_DEPTH{ 7 };
public:
	BOOM_INLINE DirectoryWindow(AppInterface* context)
		: IWidget{ context }
		, rootNode{ BuildDirectoryTree(ROOT_PATH, MAX_DEPTH) }
		, selectedPath{}
	{}

	BOOM_INLINE void OnShow(AppInterface*) override {
		if (ImGui::Begin("Project", nullptr, ImGuiWindowFlags_HorizontalScrollbar)) {
			ImGui::Separator();

			//render the tree
			if (rootNode) {
				RenderDirectoryTree(rootNode);
			}

			//refresh button to reload directory linked list
			if (ImGui::Button("Refresh")) {
				rootNode = BuildDirectoryTree(ROOT_PATH, MAX_DEPTH);
			}

			//preview selection with text info
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Text("Selected: %s", selectedPath.empty() ? "None" : selectedPath.c_str());
			if (!selectedPath.empty() && std::filesystem::exists(selectedPath)) {
				ImGui::Text("Size: %lld bytes", std::filesystem::file_size(selectedPath));
				// Add more details, e.g., preview image if it's a texture
			}
		}
		ImGui::End();
	}

private: //k-tree linked list of nodes containing children directories and files
	struct FileNode {
		std::string name;
		bool isDirectory;
		std::vector<std::unique_ptr<FileNode>> children;
		std::filesystem::path fullPath;

		FileNode(const std::string& n, bool dir, const std::filesystem::path& path)
			: name(n), isDirectory(dir), fullPath(path) {
		}
	};

	//function will build a tree according to the current directory path based on its children
	//2 object type to take note of: directories(folders) & files(assets)
	//object of type FileNode
	//default LIMIT of directory processed is DEPTH 7 from inputed node
	BOOM_INLINE std::unique_ptr<FileNode> BuildDirectoryTree(std::filesystem::path const& rootPath, uint32_t maxDepth) {
		auto root = std::make_unique<FileNode>(rootPath.filename().string(), true, rootPath);
		std::function<void(FileNode&, uint32_t)> scanDir = [&](FileNode& node, uint32_t depth) {
			if (depth > maxDepth || !std::filesystem::exists(node.fullPath)) return;

			for (const auto& entry : std::filesystem::directory_iterator(node.fullPath)) {
				const auto& path = entry.path();
				bool isDir = entry.is_directory();
				auto child = std::make_unique<FileNode>(path.filename().string(), isDir, path);
				if (isDir) {
					scanDir(*child, depth + 1);  // Recurse into subdirs
				}
				node.children.push_back(std::move(child));
			}
			};
		scanDir(*root, 0);
		return root;
	}

	//this renders the directories from the current directory file node
	BOOM_INLINE void RenderDirectoryTree(std::unique_ptr<FileNode> const& root) {
		// Sort children: directories first
		std::stable_sort(root->children.begin(), root->children.end(),
			[](const auto& a, const auto& b) {
				return a->isDirectory > b->isDirectory || (a->isDirectory == b->isDirectory && a->name < b->name);
			});

		// Render root (or skip if you want to start from a subfolder)
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
		if (root->children.empty()) {
			flags |= ImGuiTreeNodeFlags_Leaf;
		}

		bool isSelected = (selectedPath == root->fullPath.string());
		if (isSelected) {
			flags |= ImGuiTreeNodeFlags_Selected;
		}

		bool nodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)root.get(), flags, root->isDirectory ? "%s/" : "%s", root->name.c_str());
		if (ImGui::IsItemClicked()) {
			selectedPath = root->fullPath.string();  // Select on click (like Unity)
		}

		if (nodeOpen) {
			for (auto& child : root->children) {
				RenderDirectoryTree(child);  // Recurse
			}
			ImGui::TreePop();
		}
	}

public:
	std::unique_ptr<FileNode> rootNode;
	std::string selectedPath;
};