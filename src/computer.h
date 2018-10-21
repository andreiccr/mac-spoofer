#ifndef _COMPUTER_MAN
#define _COMPUTER_MAN

#include <windows.h>
#include <string>

short GetCurrentName(char *name, short verbose=0) {

    char *n = (char*)malloc(sizeof(char) * (MAX_COMPUTERNAME_LENGTH+1));
    DWORD s = sizeof(n) * (MAX_COMPUTERNAME_LENGTH + 1);
    BOOL result = GetComputerNameA(n, &s);
    if(result == 0) {
        if(verbose)
            cerr<<"Couldn't obtain the computer name. Error code: "<< hex<<GetLastError()<<endl;
        return 0;
    }
    strcpy(name, n);
    free(n);
    return 1;
}


void PrintCurrentName() {
    char* current_name = new char[MAX_COMPUTERNAME_LENGTH+1];
    short res = GetCurrentName(current_name);

    if(res != 0) {
        cout<<current_name;
    }

    delete[] current_name;
}

void ChangeName(string currentName, string newName) {
    string cmd = "WMIC computersystem where caption=’"+ currentName +"‘ rename " + newName;
    system(cmd.c_str());
}
#endif
