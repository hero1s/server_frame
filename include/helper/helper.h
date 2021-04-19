
#pragma once

#include "fundamental/common.h"
#include "utility/basic_types.h"
#include <arpa/inet.h>
#include <asm/unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

namespace svrlib {
class CHelper {
    typedef std::vector<unsigned char> BUF;

public:
    static size_t GetDirectoryFiles(std::string const& oPathDir, std::vector<std::string>& oFileNames);

    static size_t GetAllDirectories(std::string const& oPathDir, std::vector<std::string>& oDirNames);

    static std::string GetExeDir();

    static std::string GetExeFileName();

    static std::string GetLanIP();

    static std::string GetNetIP();

    static bool IsHaveNetIP();

    static bool IsLanIP(uint32_t uiIP);

    static size_t GetAllHostIPs(std::vector<uint32_t>& oIPs);

    //ip???????
    static uint32_t IPToValue(const std::string& strIP);

    static std::string ValueToIP(uint32_t ulAddr);
    //???????
    static void* LoadSoFunction(const std::string& soPath, const std::string& funcName);
};
}
