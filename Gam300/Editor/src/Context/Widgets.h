#pragma once
#include "Events.h"
#include "Inputs.h"

//forward declaration
struct GuiContext;

struct IWidget
{
	BOOM_INLINE IWidget(AppInterface* c) : context{ c } {};

	BOOM_INLINE virtual ~IWidget() = default;
	BOOM_INLINE virtual void OnSelect(Entity) {}
	BOOM_INLINE virtual void OnShow() {}
	BOOM_INLINE virtual void SetTitle(const char*) {}

protected:
	AppInterface* context;
};

// widget type definition
using Widget = std::unique_ptr<IWidget>;

