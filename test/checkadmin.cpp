/* checkadmin.cpp
 *
 * Test for using NetSessionEnum() to check if process
 * is running with administrator privileges in Windows.
 *
 * By t-andrew
 *
 */

#pragma comment(lib, "Netapi32.lib")

#include <stdio.h>
#include <windows.h>
#include <lm.h>

int main() {
    //These need to be declared to prevent other errors.
    DWORD dwEntriesRead = 0;
    DWORD dwTotalEntries = 0;
    DWORD dwResumeHandle = 0;
    LPSESSION_INFO_10 pBuf = NULL;

    DWORD dwLevel = 1; //Level 1 or 2 calls can only be performed by Administrators.

    NET_API_STATUS nStatus;

    nStatus = NetSessionEnum(NULL, NULL, NULL, dwLevel, (LPBYTE*)&pBuf,
                             MAX_PREFERRED_LENGTH, &dwEntriesRead, &dwTotalEntries, &dwResumeHandle);

    if(nStatus == ERROR_ACCESS_DENIED) {
        printf("Access is denied. Try running as admin");
    } else if(nStatus == NERR_Success) {
        printf("Process is running as admin");
    } else {
        printf("Another error has occurred");
    }

    system("pause>nul");

    return 0;
}
