#pragma once
#include"Core.h"
#include "Application/Application.h"

using namespace Boom;   
//trying out
class BOOM_API MyEngineClass 
{
public:
    void whatup();

    std::unique_ptr<Application>CreateApp();
};

