#pragma once
#ifndef CONTEXT_H
#define CONTEXT_H
#include "AppWindow.h"
#include "Graphics/Renderer.h"
#include "GlobalConstants.h"
//include ecs

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
			, renderer{ std::make_unique<GraphicsRenderer>(CONSTANTS::WINDOW_WIDTH, CONSTANTS::WINDOW_HEIGHT) }
			, window{ std::make_unique<AppWindow>(&dispatcher, CONSTANTS::WINDOW_WIDTH, CONSTANTS::WINDOW_HEIGHT, "Boom Engine") }
		{
		}


		/** @brief Destructor that deletes and nulls out all layer pointers. */ 
		BOOM_INLINE ~AppContext()
		{
			// Iterate and delete each layer, then null out pointer
			for (AppInterface*& layer : Layers)
			{
				BOOM_DELETE(layer);
			}
		}

		/**
		 * @brief Container of all active layers in the application.
		 *
		 * Stores pointers to AppInterface-derived layers. Pointers
		 * are managed manually and cleaned up in the destructor.
		 */
		std::vector<AppInterface*> Layers; //Use of pointers within containers to prevent memory leaks and promote safe practice
		EventDispatcher dispatcher;
		std::unique_ptr<GraphicsRenderer> renderer;
		std::unique_ptr<AppWindow> window;
		//scene
	};

}// namespace Boom


#endif // CONTEXT_H
