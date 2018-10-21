#include <iostream>
#include <string>
#include "cmdctrl.h"
#include "computer.h"

using namespace std;

const string TEMP_PATH = "\%USERPROFILE\%\\AppData\\Local\\Temp";

int main(int argc, char **argv) {

    string cmd;
    short retcode = COM_END;

    while(retcode != COM_EXIT) {
        PrintCurrentName();
        cout<<":>";
        cin>>cmd;

        retcode = ExecCom(cmd);
        cin.clear();
        cin.ignore(100, '\n');
    }

    return 0;
}
