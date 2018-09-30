#ifndef _COMPUTER_MAN
#define _COMPUTER_MAN

#include <string>
#include "text.h"

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


void PrintCurrentName() {
    string current_name = GetCurrentName();
    if(current_name == " ") {
        cout<<"PC name couldn't be retrieved."<<endl;
    } else {
        cout<<"Current name: "<<current_name<<endl;
    }
}

void ChangeName(string currentName, string newName) {
    string cmd = "WMIC computersystem where caption=’"+ currentName +"‘ rename " + newName;
    system(cmd.c_str());
}
#endif
