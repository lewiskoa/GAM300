#pragma once
namespace Boom {
	struct Texture2D {
		Texture2D() = default;
		Texture2D(std::string const& filename);
		~Texture2D();

		bool Load(std::string const& filename);

		void Use(int32_t uniform, int32_t unit);
		void Bind();
		void UnBind();

	public: //member manipulators
		operator uint32_t() const noexcept;
		int32_t Height() const noexcept;
		int32_t Width() const noexcept;

	private:
		int32_t height;
		int32_t width;
		uint32_t id;
	};

	using Texture = std::shared_ptr<Texture2D>;
}