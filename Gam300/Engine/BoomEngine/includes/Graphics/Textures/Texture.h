//texture should all be already be in .dds file format(BC1/BC7) depending on no-alpha/alpha .png
//CompressTextureForEditor(in,out) should be called when importing new textures within editor in .png 
// to convert to .dds format

#include "BoomProperties.h"

#pragma once
namespace Boom {
	struct BOOM_API Texture2D {
		Texture2D();
		//file path starts from Textures folder
		//textures that shouldn't compress are often already really small in size, like icons
		Texture2D(std::string filename, bool shouldCompress = true);

		//use this overload when importing new textures(.png) with the editor
		//will output a (BC7 .dds) compressed texture and load it setting ID
		//contains exception handling due to memory manipulation
		Texture2D(std::string const& inputPngPath, std::string const& outputDDSPath);
		~Texture2D();
		void Use(int32_t uniform, int32_t unit);
		void Bind();
		void UnBind();

	public: //member manipulators
		operator uint32_t() const noexcept;
		int32_t Height() const noexcept;
		int32_t Width() const noexcept;

	protected: //helpers
		std::string GetExtension(std::string const& filename);

		void LoadUnCompressed(std::string const& filename);
		void LoadCompressed(std::string const& filename);
		void CompressTexture(std::string const& inputPng, std::string const& outputDDS);

	private:
		int32_t height;
		int32_t width;
		uint32_t id;

	public: //descriptions
		float quality;
		int32_t alphaThreshold;
		int32_t mipLevel; //will not be enforced if too big
		bool isGamma;
		//std::string assetFilePath;
		//std::string assetName;

	public:
		/*
		XPROPERTY_DEF(
			"Texture", Texture2D,
			obj_member<"quality", &Texture2D::quality>,
			obj_member<"alphaThreshold", &Texture2D::alphaThreshold>,
			obj_member<"mipLevel", &Texture2D::mipLevel>,
			obj_member<"isGamma", &Texture2D::isGamma>
		)*/
	};

	using Texture = std::shared_ptr<Texture2D>;
}