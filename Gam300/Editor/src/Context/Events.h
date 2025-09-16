#pragma once
#include "Helpers.h"

struct SelectEvent
{
	BOOM_INLINE SelectEvent(EntityID entity) : EnttID(entity) {}

	EntityID EnttID;
};