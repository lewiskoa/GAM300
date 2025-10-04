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

	protected: //helpers
		std::string GetExtension(std::string const& filename);

		void LoadUnCompressed(std::string const& filename, bool isFlipY, bool isHDR);
		void LoadCompressed(std::string const& filename, bool isFlipY);

	private:
		int32_t height;
		int32_t width;
		uint32_t id;
	};

	using Texture = std::shared_ptr<Texture2D>;
}