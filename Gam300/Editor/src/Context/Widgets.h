#pragma once
#include "Events.h"
#include "Inputs.h"

//forward declaration
struct GuiContext;

struct IWidget
{
	BOOM_INLINE IWidget(AppInterface*) {};

	BOOM_INLINE virtual ~IWidget() = default;
	BOOM_INLINE virtual void OnSelect(Entity) {}
	BOOM_INLINE virtual void OnShow(AppInterface*) {}
	BOOM_INLINE virtual void SetTitle(const char*) {}

};

// widget type definition
using Widget = std::unique_ptr<IWidget>;

