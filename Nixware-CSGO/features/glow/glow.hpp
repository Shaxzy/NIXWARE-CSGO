#pragma once
#include "../../helpers/singleton.hpp"

class CGlow : public Singleton<CGlow>
{
public:
    void Run();
    void Shutdown();
};