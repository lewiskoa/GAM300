#pragma once
#ifndef CONTEXT_H
#define CONTEXT_H
#include "AppWindow.h"
#include "Graphics/Renderer.h"
#include "GlobalConstants.h"
#include "Auxiliaries/Assets.h"
#include "ECS/ECS.hpp"
#include "Physics/Context.h"

namespace Boom
{
	// Forward declaration of the base interface
	struct AppInterface;

	
	/**
	* @brief Holds global state and owns all attached layers.
	*/
	struct AppContext
	{
		/// BOOM_INLINE hints to the compiler to inline destructor calls
		/// reducing function-call overhead in the engine’s core update loop
		BOOM_INLINE AppContext()
			: dispatcher{}
			, window{ std::make_unique<AppWindow>(&dispatcher, CONSTANTS::WINDOW_WIDTH, CONSTANTS::WINDOW_HEIGHT, "Boom Engine") }
			, renderer{ std::make_unique<GraphicsRenderer>(CONSTANTS::WINDOW_WIDTH, CONSTANTS::WINDOW_HEIGHT) }
			, assets{ std::make_unique<AssetRegistry>() }
			, Physics{ std::make_unique<PhysicsContext>() }
			, scene{}
		{
		}


		/** @brief Destructor that deletes and nulls out all layer pointers. */ 
		~AppContext();
		//BOOM_INLINE ~AppContext()
		//{
		//	// Iterate and delete each layer, then null out pointer
		//	for (AppInterface*& layer : Layers)
		//	{
		//		BOOM_DELETE(layer);
		//	}
		//}

		/**
		 * @brief Container of all active layers in the application.
		 *
		 * Stores pointers to AppInterface-derived layers. Pointers
		 * are managed manually and cleaned up in the destructor.
		 */
		std::vector<AppInterface*> Layers; //Use of pointers within containers to prevent memory leaks and promote safe practice
		EventDispatcher dispatcher;
		std::unique_ptr<AppWindow> window;
		std::unique_ptr<GraphicsRenderer> renderer;
		std::unique_ptr<AssetRegistry> assets;
		std::unique_ptr<PhysicsContext> Physics; //physics context
		double DeltaTime;
		EntityRegistry scene;
	};

}// namespace Boom


#endif // CONTEXT_H
