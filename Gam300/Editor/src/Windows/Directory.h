//replica of the windows explorer for resources browsing
#pragma once

#include "Context/Context.h"
#include "Auxiliaries/Assets.h"

struct DirectoryWindow : IWidget {
private:
	const std::filesystem::path ROOT_PATH{ "Resources" };
	const uint32_t MAX_DEPTH{ 7 };
	const double AUTO_REFRESH_TIMER{ 3.0 }; //count in seconds
	const std::string_view CUSTOM_PAYLOAD_TYPE{"_GLFW_DROP"};
public:
	BOOM_INLINE DirectoryWindow(AppInterface* c)
		: IWidget{ c }
		, folderIcon{ "Icons/folder.png", false }
		, assetIcon{ "Icons/asset.png", false }
		, rootNode{}
		, selectedPath{}
		, rTimer{}
		, dropTargetPath{}
		, showDeleteConfirm{}
		, showDeleteError{}
		, deleteErrorMessage{}
	{}

	//must be called to use drag and drop feature
	BOOM_INLINE void Init() {
		rootNode = BuildDirectoryTree();
		glfwSetDropCallback(context->GetWindowHandle().get(), OnDrop);
		treeNodeOpenStatus[ROOT_PATH.string()] = true;
	}

	BOOM_INLINE void OnShow() override {
		dropTargetPath.clear();
		if (ImGui::Begin("Project", nullptr, ImGuiWindowFlags_HorizontalScrollbar)) {
			ImGui::Separator();

			//render the tree
			if (rootNode) {
				RenderDirectoryTree(rootNode);
			}
			RefreshUpdate();
			PrintSelectedInfo();
			DeleteUpdate();
		}
		ImGui::End();

		//process dropped files
		if (filesDropped && !droppedFiles.empty()) {
			std::filesystem::path targetDir{ dropTargetPath.empty() ? ROOT_PATH : dropTargetPath };
			CopyFilesToDirectory(droppedFiles, targetDir);
			droppedFiles.clear();
			dropTargetPath.clear();
			filesDropped = false;
		}
	}

private: //seperated imgui logic
	BOOM_INLINE void RefreshUpdate() {
		//refresh button to reload directory linked list
		if (ImGui::Button("Refresh") || (rTimer += context->GetDeltaTime()) > AUTO_REFRESH_TIMER) {
			rootNode = BuildDirectoryTree();
			rTimer = 0.0;
		}
	}
	BOOM_INLINE void DeleteUpdate() {
		//delete option for selected file/directory
		if (!selectedPath.empty() && ImGui::IsKeyPressed(ImGuiKey_Delete, false)) {
			showDeleteConfirm = true;
		}
		if (showDeleteConfirm) {
			ImGui::OpenPopup("Confirm Delete");
			ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		}
		if (ImGui::BeginPopupModal("Confirm Delete", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::Text("Are you sure you want to delete:\n%s?", selectedPath.c_str());
			ImGui::Separator();
			if (ImGui::Button("Yes", ImVec2(120, 0)) || ImGui::IsKeyPressed(ImGuiKey_Enter, false)) {
				if (DeletePath(selectedPath)) {
					selectedPath.clear(); // Clear selection after deletion
				}
				else {
					showDeleteError = true;
					deleteErrorMessage = "Failed to delete: " + selectedPath;
				}
				showDeleteConfirm = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("No", ImVec2(120, 0)) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
				showDeleteConfirm = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		// Error popup
		if (showDeleteError) {
			ImGui::OpenPopup("Delete Error");
			ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		}
		if (ImGui::BeginPopupModal("Delete Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::Text("%s", deleteErrorMessage.c_str());
			ImGui::Separator();
			if (ImGui::Button("OK", ImVec2(120, 0)) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
				showDeleteError = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}
	BOOM_INLINE void PrintSelectedInfo() {
		//preview selection with text info
		ImGui::Separator();
		ImGui::Text("Selected: %s", selectedPath.empty() ? "None" : selectedPath.c_str());
		if (!selectedPath.empty() && std::filesystem::exists(selectedPath)) {
			ImGui::Text("Size: %lld bytes", std::filesystem::file_size(selectedPath));
			// Add more details, e.g., preview image if it's a texture
		}
	}

private: //directory tree
	//k-tree linked list of nodes containing children directories and files
	//every fileNode should have an icon to represent what it is
	// sprite for folders, textures
	// everything else represented by asset.png
	struct FileNode {
		std::string name;
		bool isDirectory;
		std::vector<std::unique_ptr<FileNode>> children;
		std::filesystem::path fullPath;
		GLuint texId;
		bool isHovered; //for drag & drop code logic

		FileNode(const std::string& n, bool dir, const std::filesystem::path& path, GLuint id = 0)
			: name{ n }, isDirectory{ dir }, fullPath{ path }, texId{ id }, isHovered{} {
		}
	};

	//function will build a tree according to the current directory path based on its children
	//2 object type to take note of: directories(folders) & files(assets)
	//object of type FileNode
	//default LIMIT of directory processed is DEPTH 7 from inputed node
	BOOM_INLINE std::unique_ptr<FileNode> BuildDirectoryTree() {
		auto root{ std::make_unique<FileNode>(ROOT_PATH.filename().string(), true, ROOT_PATH) };

		std::function<void(FileNode&, uint32_t)> scanDir = [&](FileNode& node, uint32_t depth) {
			if (depth > MAX_DEPTH || !std::filesystem::exists(node.fullPath)) return;

			for (const auto& entry : std::filesystem::directory_iterator(node.fullPath)) {
				const auto& path{ entry.path() };
				bool isDir{ entry.is_directory() };
				GLuint texId{ isDir ? (GLuint)folderIcon : (GLuint)assetIcon }; // Assign icon based on type
				
				//custom sprites
				if (!isDir) {
					std::string ext{ path.extension().string() };
					if (ext == ".dds" || ext == ".png") {
						//finds the texture id to draw
						context->AssetTextureView([&path, &texId](TextureAsset* tex){
							std::string texPath{ std::string(CONSTANTS::TEXTURES_LOCATION) + tex->source };
							if (texPath == path.generic_string()) {
								texId = static_cast<GLuint>(*tex->data.get());
								return;
							}
						});
					}
				}

				auto child{ std::make_unique<FileNode>(path.filename().string(), isDir, path, texId) };
				if (isDir) {
					scanDir(*child, depth + 1); // Recurse into subdirs
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

		//Render root
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
		if (root->children.empty()) {
			flags |= ImGuiTreeNodeFlags_Leaf;
		}

		bool isSelected{ selectedPath == root->fullPath.string() };
		if (isSelected) {
			flags |= ImGuiTreeNodeFlags_Selected;
		}

		ImGui::PushID(root->fullPath.string().c_str());

		//keep memory of open states for auto refreshing
		bool isOpen{ treeNodeOpenStatus[root->fullPath.string()] };
		//new directories
		if (root->isDirectory && treeNodeOpenStatus.find(root->fullPath.string()) == treeNodeOpenStatus.end()) {
			isOpen = (root->fullPath == ROOT_PATH);
		}
		//assets
		if (!root->isDirectory) {
			isOpen = false;
		}
		if (isOpen) {
			flags |= ImGuiTreeNodeFlags_DefaultOpen;
		}

		root->isHovered = false;

		if (root->texId) {
			ImGui::Image((void*)(intptr_t)root->texId, ImVec2(32, 32)); // Icon size: 16x16
			ImGui::SameLine();
		}

		bool nodeOpen{ ImGui::TreeNodeEx((void*)(intptr_t)root.get(), flags, root->isDirectory ? "%s/" : "%s", root->name.c_str()) };
		
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly | ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
			root->isHovered = true;
			if (root->isDirectory) {
				dropTargetPath = root->fullPath;
				ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.3f, 0.3f, 0.6f, 0.4f));
			}
			else {
				dropTargetPath = root->fullPath.parent_path(); //output to same directory
			}
		}
		
		if (ImGui::IsItemClicked()) {
			selectedPath = root->fullPath.string();  // Select on click (like Unity)
		}

		if (root->isDirectory) {
			treeNodeOpenStatus[root->fullPath.string()] = nodeOpen;
		}

		if (nodeOpen) {
			for (auto& child : root->children) {
				RenderDirectoryTree(child);  // Recurse
			}
			ImGui::TreePop();
		}

		if (root->isHovered && root->isDirectory) {
			ImGui::PopStyleColor();
		}
		ImGui::PopID();
	}

private: //filesystem logic
	//logic behind copying of files into specified directory
	BOOM_INLINE void CopyFilesToDirectory(const std::vector<std::string>& filePaths, const std::filesystem::path& targetDir) {
		for (const auto& filePath : filePaths) {
			std::filesystem::path srcPath(filePath);
			if (!std::filesystem::exists(srcPath)) continue;

			std::filesystem::path destPath = targetDir / srcPath.filename();
			try {
				if (std::filesystem::exists(destPath)) {
					// Handle conflict: append a number to filename (e.g., "file (1).txt")
					std::string baseName{ destPath.stem().string() };
					std::string ext{ destPath.extension().string() };
					int counter{ 1 };
					do {
						destPath = targetDir / (baseName + " (" + std::to_string(counter) + ")" + ext);
						counter++;
					} while (std::filesystem::exists(destPath));
				}
				std::filesystem::copy(srcPath, destPath, std::filesystem::copy_options::recursive);
			}
			catch (const std::filesystem::filesystem_error& e) {
				char const* dodo{ e.what() };
				BOOM_ERROR("Directory.h_CopyFilesToDirectory:{}", dodo);
				continue;
			}
		}
		// Rebuild tree to reflect new files
		rootNode = BuildDirectoryTree();
	}

	BOOM_INLINE bool DeletePath(const std::filesystem::path& path) {
		try {
			if (!std::filesystem::exists(path)) return false;
			if (std::filesystem::is_directory(path)) {
				std::filesystem::remove_all(path); // Delete directory and contents
			}
			else {
				std::filesystem::remove(path); // Delete file
			}
			rootNode = BuildDirectoryTree(); // Refresh tree
			return true;
		}
		catch (const std::filesystem::filesystem_error& e) {
			char const* dodo{ e.what() };
			BOOM_ERROR("Directory.h_DeletePath:{}", dodo);
			return false;
		}
	}

private: //glfw callback for drag and drop
	BOOM_INLINE static void OnDrop(GLFWwindow*, int32_t count, char const** paths) {
		droppedFiles.clear();
		for (int i{}; i < count; ++i) {
			droppedFiles.push_back(paths[i]);
		}
		filesDropped = true;
	}

private:
	//default icon textures(TODO: should be loaded into asset manager instead of directly init here)
	Texture2D folderIcon; //folder/directories
	Texture2D assetIcon; //represents other no-sprite icons

	//directory
	std::unique_ptr<FileNode> rootNode;
	std::string selectedPath;

	//auto refresh
	double rTimer;
	std::unordered_map<std::string, bool> treeNodeOpenStatus;

	//drag & drop
	inline static std::vector<std::string> droppedFiles{}; //paths of files to be dropped
	inline static bool filesDropped{ false }; //forced inline not allowed
	std::filesystem::path dropTargetPath;

	//deletion
	bool showDeleteConfirm;
	bool showDeleteError;
	std::string deleteErrorMessage;
};