#include <stdio.h>
#include <windows.h>
#include <psapi.h>

// Entry point function
int main(void)
{
    int i;
    int strpath;

    unsigned long processIdArray[2000];
    unsigned long returnSize = 0;

    BOOL status;
    status = EnumProcesses(processIdArray, sizeof(processIdArray), &returnSize);

    if (status == 0) // Errror...
    {
        printf("EnumProcesses failed\n");
    }
    unsigned long processCount = returnSize / sizeof(unsigned long);

    printf("\t\nTotal process running currently: %lu\n\n", processCount);

    printf("_______________________________________________________________________________________________________________________________________________________________________\n||\n||\t\t\t\t\tTASK MANAGER\n||");

    printf("_____________________________________________________________________________________________________________________________________________________________________\n||\n|| || PID ||\t\t || Name || \n||____________________________________________________________________________________________________________________________________________________________________\n");

    HMODULE hmod;
    HANDLE hprocess;
    unsigned long epmSize;
    int returnValue = 0;

    char lpBaseName[2000];
    DWORD Namefilereturn;

    for (int i = 0; i < processCount; i++)
    {
        hprocess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processIdArray[i]);

		if (NULL != hprocess)
        {
            returnValue = EnumProcessModules(hprocess, &hmod, sizeof(hmod), &epmSize);
            if (returnValue != 0)
            {
                GetModuleBaseName(hprocess, hmod, lpBaseName, 2000);
                printf("||\t%lu\t|=|\t%s\n", processIdArray[i], lpBaseName);
            }
        }

        // Namefilereturn = GetModuleFileNameExA(hprocess,hmod, lpBaseName, 2000);

        // printf("\n\t%s\n", lpBaseName);/// full path of the file

        ///
        // DWORD fileAttributes;
        // fileAttributes = GetFileAttributesA(lpBaseName);/// exe file name

        // printf("%s\n",lpBaseName);
    }

    return (0);
}
