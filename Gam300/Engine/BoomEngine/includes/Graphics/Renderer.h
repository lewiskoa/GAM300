#pragma once
#include "Core.h"
//#include "Application/Interface.h"

namespace Boom {
	class BOOM_API GraphicsRenderer { //: public AppInterface {
	public:
		GraphicsRenderer() = delete;

		BOOM_INLINE GraphicsRenderer(int32_t w, int32_t h);

	public: //interface derive
		BOOM_INLINE void OnStart();// override;
		BOOM_INLINE void OnUpdate();// override;
		BOOM_INLINE ~GraphicsRenderer();// override;

	private:
		BOOM_INLINE void PrintSpecs();
	private:
		int32_t width;
		int32_t height;
	};
}