/*
 * This program demonstrate how many memory hogs Windows app, and how fast it loads
 * 
 * compiling> gcc timeloading.cpp -lpsapi
 * (c) MSDN examples & Siemargl small piece of work
 * 
 */

#include <windows.h>
#include <psapi.h>
#include <stdio.h>
#include <tchar.h>

void PrintInfo(HANDLE hproc); // print memory usage and so on
void SendQuitCommand(DWORD dwProcessId, DWORD dwThreadId); // send WM_QUIT to all proc threads
BOOL WindowPresent(DWORD dwProcessId); // true if found any named window for that process

int _tmain(int argc, TCHAR* argv[])
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	LARGE_INTEGER StartingTime, ReadyTime, EndingTime, ElapsedMicroseconds1, ElapsedMicroseconds2;
	LARGE_INTEGER Frequency;

	QueryPerformanceFrequency(&Frequency); 

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    if( argc < 2 )
    {
        printf("Usage: %s [-quit] [-splash] [cmdline]\n", argv[0]);
        return -1;
    }
    
    BOOL isQuit = FALSE, isJava = FALSE, skipSplash = FALSE;
    // some rails for running javaw -jar ....
    TCHAR CmdLine[1024] = {0}, argn[256];
    for ( int i = 1; i < argc; i++ )
    {
		if ( strstr( argv[i], "javaw" ) ) isJava = TRUE;
		if ( !strcmp( argv[i], "-quit" ) )
		{
			isQuit = TRUE;
			continue;
		}
		if ( !strcmp( argv[i], "-splash" ) )
		{
			skipSplash = TRUE;
			continue;
		}
		if ( *argv[i] == '-' )
			sprintf(argn, "%s ", argv[i]);
		else
			sprintf(argn, "\"%s\" ", argv[i]);
		strcat(CmdLine, argn);  // not UNICODE RDY
	}
    
    //printf("%s\n", CmdLine); 

	QueryPerformanceCounter(&StartingTime);

    // Start the child process. 
    if( !CreateProcess( NULL,   // No module name (use command line)
        CmdLine,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
    ) 
    {
        printf( "CreateProcess failed (%d).\n", GetLastError() );
        return -1;
    }

	printf( "Started successfully, waiting for closing child...\n" );
	if ( isJava ) printf( "Java workarounds ON\n" );
	
	// Wait to init message queue
	if( WAIT_FAILED == WaitForInputIdle(pi.hProcess, INFINITE) )
	{
        printf( "WaitForInputIdle failed (Console apps not supported).\n" );
        return -1;
	}
	// Java or Splash - wait for app window creation
	if ( isJava || skipSplash )
		while( !WindowPresent(pi.dwProcessId) );
	
	QueryPerformanceCounter(&ReadyTime);
	ElapsedMicroseconds1.QuadPart = ReadyTime.QuadPart - StartingTime.QuadPart;

    // Wait until child process exits. Print Memory usage 
    DWORD rc = 0;
    do {
		PrintInfo(pi.hProcess);

		if (isQuit)
		{ 
			SendQuitCommand(pi.dwProcessId, pi.dwThreadId);
			// Java - wait for app window destruction, then kill javaw.exe
			if ( isJava )
			{
				while( WindowPresent( pi.dwProcessId ) );
				TerminateProcess( pi.hProcess, 0 );
			}
		}
		rc = WaitForSingleObject( pi.hProcess, 3000 /*INFINITE*/ ); //ms

	} while ( rc == WAIT_TIMEOUT );

	QueryPerformanceCounter(&EndingTime);
	ElapsedMicroseconds2.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;

    // Close process and thread handles. 
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );

	// Print results
	ElapsedMicroseconds1.QuadPart *= 1000000;
	ElapsedMicroseconds1.QuadPart /= Frequency.QuadPart;

	ElapsedMicroseconds2.QuadPart *= 1000000;
	ElapsedMicroseconds2.QuadPart /= Frequency.QuadPart;

	printf( "Starting time is %.6g ms\n", ElapsedMicroseconds1.QuadPart/1000.0 );
	printf( "Full run time is %.8g ms\n", ElapsedMicroseconds2.QuadPart/1000.0 );
    
    return 0;
}

void PrintInfo(HANDLE hProcess)
// print memory usage and so on
{
    PROCESS_MEMORY_COUNTERS pmc;
    // Print information about the memory usage of the process.

    if ( GetProcessMemoryInfo( hProcess, &pmc, sizeof(pmc)) )
    {
/*		
        printf( "\tPageFaultCount: 0x%08X\n", pmc.PageFaultCount );
        printf( "\tPeakWorkingSetSize: 0x%08X\n", 
                  pmc.PeakWorkingSetSize );
        printf( "\tWorkingSetSize: 0x%08X\n", pmc.WorkingSetSize );
        printf( "\tQuotaPeakPagedPoolUsage: 0x%08X\n", 
                  pmc.QuotaPeakPagedPoolUsage );
        printf( "\tQuotaPagedPoolUsage: 0x%08X\n", 
                  pmc.QuotaPagedPoolUsage );
        printf( "\tQuotaPeakNonPagedPoolUsage: 0x%08X\n", 
                  pmc.QuotaPeakNonPagedPoolUsage );
        printf( "\tQuotaNonPagedPoolUsage: 0x%08X\n", 
                  pmc.QuotaNonPagedPoolUsage );
        printf( "\tPagefileUsage: 0x%08X\n", pmc.PagefileUsage ); 
        printf( "\tPeakPagefileUsage: 0x%08X\n", 
                  pmc.PeakPagefileUsage );
        printf( "\t--------------------\n" );
*/
        printf( "DiskIO: %7.5gMB", pmc.PageFaultCount * 4 / 1024.0 );
        printf( "\tWorkingSetSize: %7.5gMB", pmc.WorkingSetSize / 1024 / 1024.0 );
        printf( "\tPagefileUsage: %7.5gMB\n", pmc.PagefileUsage / 1024 / 1024.0 );
    }
}

BOOL CALLBACK EnumThreadWndProcQuit(HWND hWnd, LPARAM lParam)
{
	DWORD dwWindowProcessID;
	DWORD dwOurProcID = (DWORD)lParam;
	DWORD dwThreadID = GetWindowThreadProcessId(hWnd, &dwWindowProcessID);
	if (dwWindowProcessID == dwOurProcID)
	{
//		printf("Send close to %d\n", dwOurProcID);
		PostThreadMessage( dwThreadID, WM_QUIT, 0, 0 );
	}
//		PostMessage( hWnd, WM_QUIT, 0, 0 );
//		PostMessage( hWnd, WM_CLOSE, 0, 0 );
   return TRUE; 
}

void SendQuitCommand(DWORD dwProcessId, DWORD dwThreadId) 
// send WM_QUIT to all proc threads
{
//	EnumThreadWindows( dwThreadId, EnumThreadWndProc, 0 ); // not working for java apps
	EnumWindows( EnumThreadWndProcQuit, dwProcessId ); 
}

BOOL CALLBACK EnumThreadWndProcFind(HWND hWnd, LPARAM lParam)
{
	DWORD dwWindowProcessID;
	DWORD dwOurProcID = (DWORD)lParam;
	DWORD dwThreadID = GetWindowThreadProcessId(hWnd, &dwWindowProcessID);
	WINDOWINFO wi;
	
	if (dwWindowProcessID == dwOurProcID)
	{ 
		GetWindowInfo(hWnd, &wi);
		if (IsWindowVisible(hWnd))
		{
			char title[256];
			GetWindowText(hWnd, title, sizeof title);
//			printf("found window <%s>\n", title);
			if ( strlen(title) > 0 )
				return FALSE;
		}
	}
	
   return TRUE; 
}


BOOL WindowPresent(DWORD dwProcessId) 
// true if found any named window for that process
{
	return !EnumWindows( EnumThreadWndProcFind, dwProcessId ); 
}

