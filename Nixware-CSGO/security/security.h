#pragma once
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "../helpers/utils.hpp"

#pragma comment(lib,"ws2_32")

class CSecurity : public Singleton<CSecurity>
{
public:
	bool LoadConfig();
	bool Auth();
	bool Connect();
	std::string GetHWID();
};