#include <omnibazaar_util.hpp>

// Includes used to get hdd id and mac address of primary adapter.
#include <iostream>
#include <stdio.h>
#include <algorithm>
#include <sstream>
#include <vector>
#include <assert.h>
#include <cstring>
#if _WIN32 || _WIN64
#pragma comment(lib, "iphlpapi")
#include <Shlobj.h>
#include <windows.h>
#include <iphlpapi.h>
#include <Shellapi.h>
#elif __linux__
#include <sys/types.h>
#include <unistd.h>
#include <mntent.h>
#include <dirent.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sstream>
#include <stdlib.h>
#endif

namespace omnibazaar {

    fc::string util::get_harddrive_id()
    {
#if _WIN32 || _WIN64
        const BOOL asCmdProcess = FALSE;
        std::vector<std::string> stdOutLines = std::vector<std::string>();
        BOOL bSuccess = FALSE;
        LPTSTR cmdLine("wmic path win32_physicalmedia get SerialNumber");

        HANDLE g_hChildStd_OUT_Rd = NULL;
        HANDLE g_hChildStd_OUT_Wr = NULL;
        SECURITY_ATTRIBUTES saAttr;
        saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
        saAttr.bInheritHandle = TRUE;
        saAttr.lpSecurityDescriptor = NULL;
        bSuccess = CreatePipe(&g_hChildStd_OUT_Rd,
                              &g_hChildStd_OUT_Wr, &saAttr, 0);
        bSuccess = SetHandleInformation(g_hChildStd_OUT_Rd,
                                        HANDLE_FLAG_INHERIT, 0);

        PROCESS_INFORMATION piProcInfo;
        STARTUPINFO siStartInfo;
        ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
        ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
        siStartInfo.cb = sizeof(STARTUPINFO);
        siStartInfo.hStdError = g_hChildStd_OUT_Wr;
        siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
        siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

        bSuccess = CreateProcess(NULL,
                                 cmdLine,			  // command line
                                 NULL,                 // process security attributes
                                 NULL,                 // primary thread security attributes
                                 TRUE,                 // handles are inherited
                                 CREATE_NO_WINDOW,     // creation flags
                                 NULL,                 // use parent's environment
                                 NULL,                 // use parent's current directory
                                 &siStartInfo,         // STARTUPINFO pointer
                                 &piProcInfo);        // receives PROCESS_INFORMATION
        WaitForSingleObject(piProcInfo.hProcess, (DWORD)(-1L));
        CloseHandle(piProcInfo.hProcess);
        CloseHandle(piProcInfo.hThread);
        DWORD bytesInPipe = 0;
        while (bytesInPipe == 0)
        {
            bSuccess = PeekNamedPipe(g_hChildStd_OUT_Rd, NULL, 0, NULL,
                                     &bytesInPipe, NULL);
        }
        DWORD dwRead;
        CHAR *pipeContents = new CHAR[bytesInPipe];
        bSuccess = ReadFile(g_hChildStd_OUT_Rd, pipeContents,
                            bytesInPipe, &dwRead, NULL);
        std::stringstream stream(pipeContents);
        std::string res;
        while (getline(stream, res) && stdOutLines.size() < 2)
        {
            if (!res.empty())
            {
                res.erase(std::remove_if(res.begin(), res.end(), ::isspace), res.end());
            }
            stdOutLines.push_back(res);
        }
        return stdOutLines[1];
#elif __linux__
        static char* root_fs_path = NULL;
        static const char disk_by_uuid_dir[] = "/dev/disk/by-uuid/";

        FILE* mtab = setmntent("/etc/mtab", "r");

        // 1. read which entry is the root file system
        struct mntent* m_entry = NULL;

        // 2. find out the root file system
        while ((m_entry = getmntent(mtab)) != NULL)
        {
            if (strcmp(m_entry->mnt_dir, "/") == 0)
            {
                root_fs_path = strdup(m_entry->mnt_fsname);
                break;
            }
        }

        // 3. scan the directory containing UUID disks and return the one that point
        // to the root file system
        struct dirent** namelist = nullptr;

        const auto filter_func = [&](const struct dirent* d_entry){
            char link_path[512];
            sprintf(link_path, "%s%s", disk_by_uuid_dir, d_entry->d_name);

            // Removed statically allocated return buffer due to buffer overflow crashes.
            // Using nullptr for second parameter because it's not trivial to determine proper buffer size
            // for realpath() and it's safer to let it allocate return value on its own, see
            // http://man7.org/linux/man-pages/man3/realpath.3.html
            char *resolved_link_path = realpath(link_path, nullptr);

            const int result = strcmp(resolved_link_path, root_fs_path) == 0 ? 1 : 0;
            if(resolved_link_path)
            {
                free(resolved_link_path);
            }
            return result;
        };

        const int scanned_num = scandir(disk_by_uuid_dir, &namelist, filter_func, alphasort);

        fc::string result;
        if (scanned_num > 0)
        {
            result = namelist[0][0].d_name;
            for(int i = 0; i < scanned_num; ++i)
            {
                free(namelist[i]);
            }
            free(namelist);
        }
        return result;
#endif
    }

    fc::string util::get_primary_mac()
    {
#if _WIN32 || _WIN64
        char macAddress[20] = "";
        PIP_ADAPTER_INFO AdapterInfo;
        DWORD dwBufLen = sizeof(AdapterInfo);

        AdapterInfo = (IP_ADAPTER_INFO *)malloc(sizeof(IP_ADAPTER_INFO));
        assert(AdapterInfo != NULL);
        if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == ERROR_BUFFER_OVERFLOW)
        {
            AdapterInfo = (IP_ADAPTER_INFO *)malloc(dwBufLen);
            assert(AdapterInfo != NULL);
        }

        if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == NO_ERROR)
        {
            PIP_ADAPTER_INFO info = AdapterInfo;
            while (!strcmp(info->IpAddressList.IpAddress.String, "0.0.0.0"))
            {
                info = info->Next;
            }
            sprintf(macAddress, "%02X:%02X:%02X:%02X:%02X:%02X",
                    info->Address[0], info->Address[1],
                    info->Address[2], info->Address[3],
                    info->Address[4], info->Address[5]);
        }
        free(AdapterInfo);
        return macAddress;
#elif __linux__

        char buf[8192] = {0};
        struct ifconf ifc = {0};
        struct ifreq *ifr = NULL;
        int sck = 0;
        int nInterfaces = 0;
        int i = 0;
        char ip[INET6_ADDRSTRLEN] = {0};
        char macp[19];
        struct ifreq *item;
        struct sockaddr *addr;

        /* Get a socket handle. */
        sck = socket(PF_INET, SOCK_DGRAM, 0);
        if(sck < 0)
        {
            perror("socket");
            return "";
        }

        /* Query available interfaces. */
        ifc.ifc_len = sizeof(buf);
        ifc.ifc_buf = buf;
        if(ioctl(sck, SIOCGIFCONF, &ifc) < 0)
        {
            perror("ioctl(SIOCGIFCONF)");
            return "";
        }

        /* Iterate through the list of interfaces. */
        ifr = ifc.ifc_req;
        nInterfaces = ifc.ifc_len / sizeof(struct ifreq);

        for(i = 0; i < nInterfaces; i++)
        {
            item = &ifr[i];

            addr = &(item->ifr_addr);

            /* Get the IP address*/
            if(ioctl(sck, SIOCGIFADDR, item) < 0)
            {
                perror("ioctl(OSIOCGIFADDR)");
            }

            if (inet_ntop(AF_INET, &(((struct sockaddr_in *)addr)->sin_addr), ip, sizeof ip) == NULL) //vracia adresu interf
            {
                perror("inet_ntop");
                continue;
            }

            /* Get the MAC address */
            if(ioctl(sck, SIOCGIFHWADDR, item) < 0)
            {
                perror("ioctl(SIOCGIFHWADDR)");
                return "";
            }

            // skip zeros address
            if(!( item->ifr_hwaddr.sa_data[0]|
                  item->ifr_hwaddr.sa_data[1]|
                  item->ifr_hwaddr.sa_data[2]|
                  item->ifr_hwaddr.sa_data[3]|
                  item->ifr_hwaddr.sa_data[4]|
                  item->ifr_hwaddr.sa_data[5]))
                continue;
            // create mac address string
            sprintf(macp, " %02x:%02x:%02x:%02x:%02x:%02x",
                    (unsigned char)item->ifr_hwaddr.sa_data[0],
                    (unsigned char)item->ifr_hwaddr.sa_data[1],
                    (unsigned char)item->ifr_hwaddr.sa_data[2],
                    (unsigned char)item->ifr_hwaddr.sa_data[3],
                    (unsigned char)item->ifr_hwaddr.sa_data[4],
                    (unsigned char)item->ifr_hwaddr.sa_data[5]);

            return macp;
        }
#endif
    }

}
