//texture should all be already be in .dds file format(BC1/BC7) depending on no-alpha/alpha .png
//CompressTextureForEditor(in,out) should be called when importing new textures within editor in .png 
// to convert to .dds format
//

#pragma once
namespace Boom {
	struct BOOM_API Texture2D {

		Texture2D() = default;
		//file path starts from Textures folder
		Texture2D(std::string filename, bool isFlipY = true, bool isHDR = false);
		~Texture2D();
		void Use(int32_t uniform, int32_t unit);
		void Bind();
		void UnBind();

	public: //member manipulators
		operator uint32_t() const noexcept;
		int32_t Height() const noexcept;
		int32_t Width() const noexcept;

		//call this when importing new textures with the editor
		//contains exception handling due to memory manipulation
		void CompressTextureForEditor(std::string const& inputPng, std::string const& fullPath, bool forceAlpha = true);

	protected: //helpers
		std::string GetExtension(std::string const& filename);

		void LoadUnCompressed(std::string const& filename, bool isFlipY, bool isHDR);
		void LoadCompressed(std::string const& filename, bool isFlipY);
		void CompressTexture(std::string const& inputPng, std::string const& outputDDS, bool forceAlpha);

	private:
		int32_t height;
		int32_t width;
		uint32_t id;
	};

	using Texture = std::shared_ptr<Texture2D>;
}