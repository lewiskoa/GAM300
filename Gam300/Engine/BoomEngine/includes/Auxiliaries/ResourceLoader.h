#include "Core.h"
#include "GlobalConstants.h"

namespace Boom {
	namespace fs = std::filesystem;
	//logic behind auto loading from the resource folder
	//negates hard coding strings to loads data
	class ResourseLoader {
	public:
		//AssetProcessFn is of params(std::string path, std::string extension)
		//RenderFn is of param(double percent) [0.0, 1.0] range
		template <class AssetProcessFn, class RenderFn>
		BOOM_INLINE void ResourcesLoad(AssetProcessFn apf, RenderFn rf) {
			std::vector<fs::path> files;

			//gather path to all files located within resources folder
			for (auto const& entry : fs::recursive_directory_iterator(CONSTANTS::RESOURCES_LOCATION)) {
				if (entry.is_regular_file()) {
					files.push_back(entry.path());
				}
			}

			if (files.empty()) {
				BOOM_DEBUG("No files detected in {} folder", CONSTANTS::RESOURCES_LOCATION);
				return;
			}

			totalFileCount = static_cast<uint64_t>(files.size());

			//file.extension will determine the type of asset manager load by apf
			//apf will parse these extension and load by itself
			//fn will then draw the appropriate loading screen progress
			for (fs::path const& file : files) {
				apf(file.string(), file.extension().string());
				++fileCount;
				rf(GetProgressPercent());
			}
		}

	private:
		//[0.0, 1.0] range
		[[nodiscard]] BOOM_INLINE double GetProgressPercent() const noexcept {
			BOOM_ASSERT(totalFileCount == 0 && "divide by zero");
			return (double)fileCount/(double)totalFileCount;
		}
	private:
		uint64_t fileCount;
		uint64_t totalFileCount;
	};
}