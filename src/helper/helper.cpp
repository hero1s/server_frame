

#include "helper/helper.h"
#include <iomanip>
#include <sstream>

#include "file/filehelper.h"
#include "svrlib.h"
#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

using namespace svrlib;

namespace {

};

size_t CHelper::GetDirectoryFiles(std::string const& oPathDir, std::vector<std::string>& oFileNames)
{
    oFileNames.clear();

    std::string oPath = oPathDir;
    if (oPath.empty()) {
        return 0;
    }
    if (oPath[oPath.size() - 1] != '/') {
        oPath += "/";
    }

    DIR* dp;
    if ((dp = opendir(oPath.c_str())) == NULL) {
        return 0;
    }
    struct dirent* dirp;
    while ((dirp = readdir(dp)) != NULL) {
        struct dirent temp = (*dirp);
        struct stat buf;
        std::string oTempPath = oPath;
        oTempPath += temp.d_name;
        if (lstat(oTempPath.c_str(), &buf) < 0) {
            continue;
        }
        if (S_ISDIR(buf.st_mode)) {
            continue;
        } else {
            oFileNames.push_back(temp.d_name);
        }
    }
    closedir(dp);
    return oFileNames.size();
}

size_t CHelper::GetAllDirectories(std::string const& oPathDir, std::vector<std::string>& oDirNames)
{
    oDirNames.clear();

    std::string oPath = oPathDir;
    if (oPath.empty()) {
        return 0;
    }
    if (oPath[oPath.size() - 1] != '/') {
        oPath += "/";
    }

    DIR* dp;
    if ((dp = opendir(oPath.c_str())) == NULL) {
        return 0;
    }
    struct dirent* dirp;
    while ((dirp = readdir(dp)) != NULL) {
        struct dirent temp = (*dirp);
        struct stat buf;
        std::string oTempPath = oPath;
        oTempPath += temp.d_name;
        if (lstat(oTempPath.c_str(), &buf) < 0) {
            continue;
        }
        std::string strDirName = temp.d_name;
        if (S_ISDIR(buf.st_mode) && strDirName != "." && strDirName != "..") {
            oDirNames.push_back(temp.d_name);
        }
    }
    closedir(dp);
    return oDirNames.size();
}

std::string CHelper::GetExeDir()
{

    const int MAXBUFSIZE = 1024;

    char szFileName[MAXBUFSIZE] = { 0 };

    int count;
    count = readlink("/proc/self/exe", szFileName, sizeof(szFileName));
    char* p = strrchr(szFileName, '/');
    if (p)
        *(p + 1) = 0;
    return szFileName;
}

std::string CHelper::GetExeFileName()
{
    const int MAXBUFSIZE = 1024;
    char szFileName[MAXBUFSIZE] = { 0 };

    int count;
    count = readlink("/proc/self/exe", szFileName, sizeof(szFileName));
    return szFileName;
}

struct stIP {
    union {
        uint32_t uiIP;
        uint8_t arIP[4];
    };
};

bool CHelper::IsLanIP(uint32_t uiIP)
{
    stIP oIP;
    oIP.uiIP = uiIP;
    if (oIP.arIP[0] == 10) // 10.0.0.0 - 10.255.255.255
    {
        return true;
    }
    if (oIP.arIP[0] == 172 && (oIP.arIP[1] >= 16 && oIP.arIP[1] <= 31)) {
        return true;
    }
    if (oIP.arIP[0] == 192 && oIP.arIP[1] == 168) {
        return true;
    }
    if (oIP.arIP[0] == 169 && oIP.arIP[1] == 254) {
        return true;
    }
    return false;
}

size_t CHelper::GetAllHostIPs(std::vector<uint32_t>& oIPs)
{

    enum {
        MAXINTERFACES = 16,
    };
    int fd = 0;
    int intrface = 0;
    struct ifreq buf[MAXINTERFACES];
    struct ifconf ifc;
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0) {
        ifc.ifc_len = sizeof(buf);
        ifc.ifc_buf = (caddr_t)buf;
        if (!ioctl(fd, SIOCGIFCONF, (char*)&ifc)) {
            intrface = ifc.ifc_len / sizeof(struct ifreq);
            while (intrface-- > 0) {
                if (!(ioctl(fd, SIOCGIFADDR, (char*)&buf[intrface]))) {
                    uint32_t uiIP = ((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr.s_addr;
                    if (uiIP != 0 && uiIP != inet_addr("127.0.0.1")) {
                        oIPs.push_back(uiIP);
                    }
                }
            }
        }
    }
    close(fd);

    return oIPs.size();
}

std::string CHelper::GetLanIP()
{
    std::vector<uint32_t> oIPs;
    if (GetAllHostIPs(oIPs) > 0) {
        for (size_t i = 0; i < oIPs.size(); ++i) {
            if (IsLanIP(oIPs[i])) {
                struct in_addr stAddr;
                stAddr.s_addr = oIPs[i];
                return inet_ntoa(stAddr);
            }
        }
    }
    // 返回无效的IP
    return "1.1.0.1";
}

std::string CHelper::GetNetIP()
{
    uint32_t uiNetIP = 0;
    std::vector<uint32_t> oIPs;
    if (GetAllHostIPs(oIPs) > 0) {
        for (size_t i = 0; i < oIPs.size(); ++i) {
            if (!IsLanIP(oIPs[i])) {
                if (uiNetIP == 0) {
                    uiNetIP = oIPs[i];
                } else {
                    // 如果有多个外网ip，则返回ip 0
                    break;
                }
            }
        }
    }
    struct in_addr stAddr;
    stAddr.s_addr = uiNetIP;
    return inet_ntoa(stAddr);
}

bool CHelper::IsHaveNetIP()
{
    std::vector<uint32_t> oIPs;
    if (GetAllHostIPs(oIPs) > 0) {
        for (size_t i = 0; i < oIPs.size(); ++i) {
            if (!IsLanIP(oIPs[i])) {
                return true;
            }
        }
    }
    return false;
}

// ip数字字符串转行
uint32_t CHelper::IPToValue(const std::string& strIP)
{
    uint32_t a[4];
    std::string IP = strIP;
    std::string strTemp;
    size_t pos;
    size_t i = 3;
    do {
        pos = IP.find(".");
        if (pos != std::string::npos) {
            strTemp = IP.substr(0, pos);
            a[i] = atoi(strTemp.c_str());
            i--;
            IP.erase(0, pos + 1);
        } else {
            strTemp = IP;
            a[i] = atoi(strTemp.c_str());
            break;
        }

    } while (1);

    uint32_t nResult = (a[3]) + (a[2] << 8) + (a[1] << 16) + (a[0] << 24);
    return nResult;
}

std::string CHelper::ValueToIP(uint32_t ulAddr)
{
    char strTemp[20];
    memset(strTemp, 0, sizeof(strTemp));
    sprintf(strTemp, "%d.%d.%d.%d", (ulAddr & 0x000000ff), (ulAddr & 0x0000ff00) >> 8, (ulAddr & 0x00ff0000) >> 16, (ulAddr & 0xff000000) >> 24);
    return string(strTemp);
}
