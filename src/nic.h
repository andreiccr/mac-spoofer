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

void PrintMAC() {
    char *path = new char[PATH_MAX];
    GetTempPathA(MAX_PATH, path);
    strcat(path, "\\macs.txt");

    uint16_t cmd_len = strlen("getmac /fo table /nh > ") + strlen(path);
    char *cmd = new char[cmd_len];
    strcpy(cmd, "getmac /fo table /nh > ");
    strcat(cmd, path);

    system(cmd);

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
    char *path = new char[MAX_PATH];
    GetTempPathA(MAX_PATH, path);
    strcat(path, "\\intnames.txt");

    uint16_t cmd_len = strlen("wmic nic get NetConnectionID | find /v \"\" > ") + strlen(path);
    char *cmd = new char[cmd_len];
    strcpy(cmd, "wmic nic get NetConnectionID | find /v \"\" > ");
    strcat(cmd, path);
    system(cmd);

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
    system("reg delete HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\0000 /v NetworkAddress /f");
    system("reg delete HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\0001 /v NetworkAddress /f");
    system("reg delete HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\0002 /v NetworkAddress /f");
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

    //Change the MAC Address of the first 2 interfaces listed
    for(int i=0;i<=2;i++) {
        cout<<"Changing address of interface #" <<i<<endl;
        string cmd = "reg add HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\000" + to_string(i) + " /v NetworkAddress /d " + newAddr + " /f";

        system(cmd.c_str());
    }
}

#endif
