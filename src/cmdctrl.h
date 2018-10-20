#ifndef _CMD_CTRL
#define _CMD_CTRL

#include <iostream>
#include <string>
#include <map>
#include "nic.h"

#define COM_END 0x01
#define COM_EXIT 0x00
#define COM_ERR 0x11

using namespace std;

short ComSpoof(void){
    string newAddr;
    cout<<"Enter new MAC address: ";
    cin>>newAddr;

    DisableAllInterfaces();
    ChangeAddr(newAddr);
    EnableAllInterfaces();

    return COM_END;
}

short ComRestore(void) {
    cout<<"Restoring original MAC Addresses..."<<endl;

    DisableAllInterfaces();
    RestoreDefaultAddr();
    EnableAllInterfaces();

    return COM_END;
}

short ComList(void) {
    PrintMAC();
    return COM_END;
}

short ComHelp(void) {
    cout<<endl;
    cout<<"  spoof \t Changes the MAC address."<<endl;
    cout<<"  restore \t Restores the original MAC address."<<endl;
    cout<<"  list  \t Shows a list of MAC addresses."<<endl;
    cout<<"  help  \t Shows this list of valid commands."<<endl;
    cout<<"  exit  \t Exits the program."<<endl;
    cout<<endl;
    return COM_END;
}
short ComExit(void) { return COM_EXIT; }

typedef short (*handler)(void);
map<string, handler> funcMap = {
    {"spoof", ComSpoof},
    {"restore", ComRestore},
    {"list", ComList},
    {"help", ComHelp},
    {"exit", ComExit}
};

short ExecCom(string cmd) {
    short retcode;
    for(int i=0;i<cmd.length();i++) {
        cmd[i] = tolower(cmd[i]);
    }

    if(funcMap.find(cmd) == funcMap.end()) {
        cout<<"Command doesn't exist. Type \"help\" for a list of valid commands."<<endl;
        return COM_ERR;
    }

    handler h = funcMap[cmd];
    retcode = (*h)();
}

#endif
