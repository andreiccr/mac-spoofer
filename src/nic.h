#ifndef _NIC_MAN
#define _NIC_MAN

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <windows.h>
#include "text.h"

using namespace std;

const int nicNum = 3; // The first n interfaces as they appear in registry, will be modified.

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
    const char *path = systemfout("getmac /fo table /nh", "macs.txt");

    ifstream outputFile {path};
    string macs { istreambuf_iterator<char>(outputFile), istreambuf_iterator<char>() };
    cout<<macs<<endl;

}

vector<string> ParseInterfaceNames(string interfaces) {
    int pos;
    char buffer[256];
    int i;
    bool charfound, finished;
    vector<string> ints ;
    vector<string>::iterator it;

    it = ints.begin();

    finished = false;

    while(!finished) {
        pos = interfaces.find('\n');
        i = pos;

        charfound = false;
        while(!charfound) {
            if(isalnum(interfaces[i])) {
                //Add everything in the array from this position to the right until a /r/n is found.
                charfound = true;
            }
            else {
                if(i+1 >= interfaces.length()) {
                    finished = true;
                    break;
                }

                i++;
            }
        }

        if(finished) break;

        interfaces = interfaces.substr(i);
        i = 0;

        if(charfound) {
            while (interfaces[i] != '\r') {
                buffer[i] = interfaces[i];
                i++;
            }

            string element = buffer;
            element = element.substr(0,i); //Keep only the text, not the entire 256 byte content.
            element = rtrimStr(element);

            it = ints.insert(it, element.substr(0,i));
        }
    }

    return ints;

}

vector<string> GetInterfaceNames() {
    const char *path = systemfout("wmic nic get NetConnectionID | find /v \"\"", "intnames.txt");

    ifstream interfaceFile {path};
    string inames { istreambuf_iterator<char>(interfaceFile), istreambuf_iterator<char>() };

    vector<string> ints = ParseInterfaceNames(inames);

    return ints;
}

void DisableAllInterfaces() {

    vector<string> inames = GetInterfaceNames();
    for(vector<string>::iterator it=inames.begin(); it!=inames.end(); it++) {
        string cmd = "netsh interface set interface \""+ *it +"\" disable";
        system(cmd.c_str());
        //cout<<"Disabled " << *it << " with command " << cmd <<endl;
    }
}

void EnableAllInterfaces() {

    vector<string> inames = GetInterfaceNames();
    for(vector<string>::iterator it=inames.begin(); it!=inames.end(); it++) {
        string cmd = "netsh interface set interface \""+ *it +"\" enable";
        system(cmd.c_str());
    }
}


void RestoreDefaultAddr() {
    HKEY hKey;
    LONG res;
    for(int i=0;i<nicNum;i++) {
        string path = "SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\000" + to_string(i);
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

    for(int i=0;i<nicNum;i++) {
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
            string cmd = "reg add HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\000" + to_string(i) + " /v NetworkAddress /d " + newAddr + " /f";
            system(cmd.c_str());
        #endif // _DEBUG
    }


}

#endif
