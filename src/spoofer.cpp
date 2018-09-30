#include <iostream>
#include <windows.h>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

const string TEMP_PATH = "\%USERPROFILE\%\\AppData\\Local\\Temp";

bool isBlankChar(char c) {
    if((!isalnum(c)) && (c != '-') && (c != '_') && (c != '\r') && (c != '\n')) return true;
    return false;
}

string ltrimStr(string str) {
  int i;
  for(i=0;i<str.length();i++){
    if(((str[i] >=48) &&(str[i] <=57)) || ((str[i] >= 65) && (str[i] <= 90)) || ((str[i] >= 97) && (str[i] <= 122))) {
      break;
    }
  }

  string trimmed = str.substr(i, str.length()-i);
  return trimmed;
}

string rtrimStr(string str) {
    int i;
    int space = -1;

    for(i=0;i<str.length();i++) {
        if((isBlankChar(str[i])) && (space == -1)) {
            space = i;
        } else if(isalnum(str[i])) {
           space = -1;
        }

    }

    if(space != -1)
        str = str.substr(0, space);

    return str;
}

string GetCurrentName(void) {

    char *path = new char[MAX_PATH];
    GetTempPathA(MAX_PATH, path);
    strcat (path, "\\output.txt");

    /*   Note: wmic outputs in unicode so there will be extra null bytes.
     *   This is fixed by adding | find /v "" to the command.
     *   Alternatively, wmic's output parameter could be used.
    **/
    uint16_t cmd_len = strlen("wmic computersystem get caption | find /v \"\" > ") + strlen(path);
    char *cmd = new char[cmd_len];

    strcpy(cmd, "wmic computersystem get caption | find /v \"\" > ");
    strcat(cmd, path);

    system(cmd);

    ifstream outputFile {path};

    if(!outputFile.is_open()) {
        return " ";
    }

    string current_name { istreambuf_iterator<char>(outputFile), istreambuf_iterator<char>() };

    current_name.erase(0, 8); //Remove the word "Caption"
    current_name = ltrimStr(current_name);
    outputFile.close();

    return current_name;
}

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
    DisableAllInterfaces();
    system("reg delete HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\0000 /v NetworkAddress /f");
    system("reg delete HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\0001 /v NetworkAddress /f");
    system("reg delete HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\0002 /v NetworkAddress /f");
    EnableAllInterfaces();
}


void PrintInterfaces() {
    vector<string> interface_names = GetInterfaceNames();
    for(vector<string>::iterator it=interface_names.begin(); it!=interface_names.end(); it++)
        cout<<*it<<endl;
}

int main(int argc, char **argv) {
    string new_name;
    string new_mac;
    char c;

    char *cmd = new char[256];

    /*cout<<"Getting PC name..."<<endl;
    string current_name = GetCurrentName();
    if(current_name == " ") {
        cout<<"Warning! PC name couldn't be retrieved."<<endl;
    } else {
        cout<<"Current name: "<<current_name;
    }
    */

    /*cout<<"Enter new name: ";
    cin>>new_name;*/

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

  //Execute command to change computer name
  //system("WMIC computersystem where caption=’"+ current_name +"‘ rename " + new_name);

    DisableAllInterfaces();

    for(int i=0;i<=2;i++) {
        //Change the MAC Address of the first 2 interfaces listed
        cout<<"Changing address of interface #" <<i<<endl;
        strcpy(cmd, "reg add HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\000");
        strcat(cmd, to_string(i).c_str());
        strcat(cmd, " /v NetworkAddress /d ");
        strcat(cmd, new_mac.c_str());
        strcat(cmd, " /f");

        system(cmd);
    }

    EnableAllInterfaces();

    PrintMAC();

    system("pause>nul");
    return 0;
}
