//replica of the windows explorer for resources browsing
#pragma once

#include "Context/Context.h"
#include "Auxiliaries/Assets.h"

struct DirectoryWindow : IWidget {
private:
	const std::filesystem::path ROOT_PATH{ "Resources" };
	const uint32_t MAX_DEPTH{ 7 };
	const double AUTO_REFRESH_TIMER{ 3.0 }; //count in seconds
public:
	BOOM_INLINE DirectoryWindow(AppInterface* c)
		: IWidget{ c }
		, rootNode{}
		, selectedPath{}
		, folderIcon{ "Icons/folder.png", false }
		, assetIcon{ "Icons/asset.png", false }
		, rTimer{}
	{}

	BOOM_INLINE void Init() {
		rootNode = BuildDirectoryTree();
	}

	BOOM_INLINE void OnShow() override {
		if (ImGui::Begin("Project", nullptr, ImGuiWindowFlags_HorizontalScrollbar)) {
			ImGui::Separator();

			//render the tree
			if (rootNode) {
				RenderDirectoryTree(rootNode);
			}

			//refresh button to reload directory linked list
			if (ImGui::Button("Refresh") || (rTimer += context->GetDeltaTime()) > AUTO_REFRESH_TIMER ) {
				rootNode = BuildDirectoryTree();
				rTimer = 0.0;
			}

			//preview selection with text info
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
	//every fileNode should have an icon to represent what it is
	// sprite for folders, textures
	// everything else represented by asset.png
	struct FileNode {
		std::string name;
		bool isDirectory;
		std::vector<std::unique_ptr<FileNode>> children;
		std::filesystem::path fullPath;
		GLuint texId;

		FileNode(const std::string& n, bool dir, const std::filesystem::path& path, GLuint id = 0)
			: name{n}, isDirectory{dir}, fullPath{path}, texId{id} {
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
						std::string pathStr{ path.generic_string() };
						context->AssetTextureView([&path, &texId](TextureAsset* tex){
							std::string tmp{ std::string(CONSTANTS::TEXTURES_LOCATION) + tex->source };
							std::string tmp2{ path.generic_string() };
							if (tmp == tmp2) {
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
			isOpen = false;
		}
		//assets
		if (!root->isDirectory) {
			isOpen = false;
		}
		if (isOpen) {
			flags |= ImGuiTreeNodeFlags_DefaultOpen;
		}


		if (root->texId) {
			ImGui::Image((void*)(intptr_t)root->texId, ImVec2(32, 32)); // Icon size: 16x16
			ImGui::SameLine();
		}

		bool nodeOpen{ ImGui::TreeNodeEx((void*)(intptr_t)root.get(), flags, root->isDirectory ? "%s/" : "%s", root->name.c_str()) };
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
		ImGui::PopID();
	}

private:
	std::unique_ptr<FileNode> rootNode;
	std::string selectedPath;

	Texture2D folderIcon; //folder/directories
	Texture2D assetIcon; //represents other no-sprite icons

	double rTimer;

	std::unordered_map<std::string, bool> treeNodeOpenStatus;
};