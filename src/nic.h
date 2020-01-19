#ifndef _NIC_MAN
#define _NIC_MAN

#ifndef WINVER
    #define WINVER 0x0600 //Needed for Win32 API newer features
#endif

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <windows.h>
#include <iphlpapi.h>
#include "text.h"

using namespace std;

/* 
  The first n interfaces as they appear in registry, will be modified.
  This is now used only if subkeys counting fails
*/
const int nicNum = 3; 

vector<string> activeAdapters; //This must store the active adapters names.

/* Redirects the output of the command in a temporary location in a file specified by 'filename' */
const char* systemfout(const char *ocmd, const char* filename) {
    char *path = new char[MAX_PATH];
    GetTempPathA(MAX_PATH, path);
    strcat(path, "\\");
    strcat(path, filename);

    uint16_t cmd_len = strlen(ocmd) + strlen(path) + strlen(" > ");
    char *cmd = new char[cmd_len];
    strcpy(cmd, ocmd);
    strcat(cmd, " > ");
    strcat(cmd, path);
    system(cmd);
    return path;
}

void PrintMAC() {

    IP_ADAPTER_ADDRESSES AdapterAddr[64]; //First 64 NICs
    DWORD bufLen = sizeof(AdapterAddr);
    ULONG result;

    result = GetAdaptersAddresses(AF_INET, GAA_FLAG_SKIP_ANYCAST, NULL, AdapterAddr, &bufLen);

    if(result != ERROR_SUCCESS) {
        cerr<<"Couldn't obtain adapter information"<<endl;
        cerr<<"Failed to list physical addresses"<<endl;
        return;
    }

    PIP_ADAPTER_ADDRESSES pAdapterAddr = AdapterAddr;

    do {
        //Print each BYTE given by PhysicalAddress array, because I'm too lazy to find a more elegant way.
        for(int i=0;i<pAdapterAddr->PhysicalAddressLength;i++) {
            if(pAdapterAddr->PhysicalAddress[i] <= 0xf)
                printf("0");
            printf("%x", pAdapterAddr->PhysicalAddress[i]);
        }

        wcout<<"  "<<pAdapterAddr->FriendlyName<<endl;
        pAdapterAddr = pAdapterAddr->Next;

    } while(pAdapterAddr);

}

string pwcharToString(PWCHAR wstr) {
    wstring ws(wstr);
    string str(ws.begin(), ws.end());
    return str;
}


vector<string> GetInterfaceNames() {
    vector<string> nicNames;
    vector<string>::iterator it = nicNames.begin();

     IP_ADAPTER_ADDRESSES AdapterAddr[64]; //First 64 NICs
    DWORD bufLen = sizeof(AdapterAddr);
    ULONG result;

    result = GetAdaptersAddresses(AF_INET, GAA_FLAG_SKIP_ANYCAST, NULL, AdapterAddr, &bufLen);

    if(result != ERROR_SUCCESS) {
        cerr<<"Couldn't obtain adapter information"<<endl;
        cerr<<"Failed to get adapter names"<<endl;
        nicNames.insert(it, "###");
        return nicNames;
    }

    PIP_ADAPTER_ADDRESSES pAdapterAddr = AdapterAddr;

    do {
        it = nicNames.insert(it, pwcharToString(pAdapterAddr->FriendlyName));
        pAdapterAddr = pAdapterAddr->Next;

    } while(pAdapterAddr);

    return nicNames;
}

void DisableAllInterfaces() {
    //Call GetInterfaceNames() before disabling all adapters and save them in activeAdapters.
    activeAdapters = GetInterfaceNames();

    for(vector<string>::iterator it=activeAdapters.begin(); it!=activeAdapters.end(); it++) {
        string cmd = "netsh interface set interface \""+ *it +"\" disable";
        system(cmd.c_str());
        //cout<<"Disabled " << *it << " with command " << cmd <<endl;
    }
}

void EnableAllInterfaces() {

    for(vector<string>::iterator it=activeAdapters.begin(); it!=activeAdapters.end(); it++) {
        string cmd = "netsh interface set interface \""+ *it +"\" enable";
        system(cmd.c_str());
        //cout<<"Enabled " << *it << " with command " << cmd <<endl;
    }
}

//Returns the number of keys representing interfaces. Returns 0 on failure.
DWORD CountInterfaces() {
	HKEY hKey;
    LONG res;
	DWORD subKeys=0; //Number of keys representing interfaces. Substract by 2.
	
	res = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}" , 0, KEY_ALL_ACCESS, &hKey);
	
	if(res == ERROR_SUCCESS) {
		RegQueryInfoKey(hKey, NULL, NULL, NULL, &subKeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	}
	
	return (subKeys>1)?(subKeys-2):0;
}


void RestoreDefaultAddr() {
    HKEY hKey;
    LONG res;
	string path = "";
	DWORD subKeys = CountInterfaces();
	if(subKeys==0) subKeys = nicNum; // Keys counting failed
	
    for(int i=0;i<subKeys;i++) {
		if(i>=10)
			path = "SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\00" + to_string(i);
		else
			path = "SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\000" + to_string(i);
		
        res = RegOpenKeyEx(HKEY_LOCAL_MACHINE, path.c_str() , 0, KEY_ALL_ACCESS, &hKey);
        if (res == ERROR_SUCCESS) {
            RegDeleteValueA(hKey, "NetworkAddress");
            RegCloseKey(hKey);
        }
    }
}


void PrintInterfaces() {
    vector<string> interface_names = GetInterfaceNames();
    for(vector<string>::iterator it=interface_names.begin(); it!=interface_names.end(); it++)
        cout<<*it<<endl;
}

void ChangeAddr(string newAddr) {
    for(int i=0;i<newAddr.length();i++) {
        newAddr[i] = tolower(newAddr[i]);
    }

    if(!regex_match(newAddr, regex("([a-f]|\\d){12}"))) {

        if(regex_match(newAddr, regex("(([a-f]|\\d){2}-){5}([a-f]|\\d){2}"))) {
            //Separated by -
            for(int i=2;i<=10;i+=2)
                newAddr.erase(newAddr.begin() + i);

        } else if(regex_match(newAddr, regex("(([a-f]|\\d){2}:){5}([a-f]|\\d){2}"))) {
            //Separated by :
            for(int i=2;i<=10;i+=2)
                newAddr.erase(newAddr.begin() + i);

        } else {
            //Not a valid MAC Address
            cout<<"Error: This MAC Address is invalid."<<endl;
            return;
        }
    }

    HKEY hKey;
    LONG res;
	string cmd = "";
	
	DWORD subKeys = CountInterfaces();
	if(subKeys==0) subKeys = nicNum; // Keys counting failed
	
    for(int i=0;i<subKeys;i++) {
        #ifdef _UNSTABLE
            string path = "SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\000" + to_string(i);
            res = RegOpenKeyEx(HKEY_LOCAL_MACHINE, path.c_str() , 0, KEY_WRITE, &hKey);
            if (res == ERROR_SUCCESS) {
                res = RegSetValueEx(hKey, "NetworkAddress" , 0, REG_SZ, (BYTE *)newAddr.c_str(), sizeof(newAddr));
                if(res == ERROR_SUCCESS) {
                    cout<<"Changed MAC address of interface #"<<i<<" to "<<newAddr<<endl;
                } else {
                    cerr<<"Couldn't change MAC address of interface #"<<i<<"  Reason: "<<res<<endl;
                }
                RegCloseKey(hKey);
            }
        #else
			if(i>=10)
				cmd = "reg add HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\00" + to_string(i) + " /v NetworkAddress /d " + newAddr + " /f";
			else
				cmd = "reg add HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\000" + to_string(i) + " /v NetworkAddress /d " + newAddr + " /f";
            system(cmd.c_str());
        #endif // _DEBUG
    }


}

#endif
