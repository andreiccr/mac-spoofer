#include <iostream>
#include <windows.h>
#include <string>
#include "nic.h"

using namespace std;

const string TEMP_PATH = "\%USERPROFILE\%\\AppData\\Local\\Temp";

int main(int argc, char **argv) {
    string new_mac;
    char c;

    cout<<"List of MAC Addresses: ";
    PrintMAC();

    cout<<endl;
    cout<<"Spoof or Restore? (s/r) ";
    cin>>c;

    if(c == 'r') {
        cout<<"Restoring original MAC Addresses..."<<endl;
        RestoreDefaultAddr();
        cout<<"Done.";
        return 0;
    } else if (c != 's') {
        cout<<"Invalid operation!";
        return 0;
    }

    cout<<"Enter new MAC: ";
    cin>>new_mac;

    DisableAllInterfaces();

    ChangeAddr(new_mac);

    EnableAllInterfaces();

    PrintMAC();

    system("pause>nul");
    return 0;
}
