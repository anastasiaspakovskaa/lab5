#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <iostream>
#include <conio.h>

std::wstring convertToWideString(const std::string& str) {
	const char* c_str = str.c_str();
	wchar_t* wstr = new wchar_t[str.length() + 1];
	mbstowcs(wstr, c_str, str.size() + 1);
	std::wstring wide(wstr);
	delete[] wstr;
	return wide;
}

struct employee
{
    int num; // ????????????????? ????? ??????????
    char name[10]; // ??? ??????????
    double hours; // ?????????? ???????????? ?????
};

struct message {
    char type;
    int id;
};

int main()
{
	char c; 
	HANDLE hNamedPipe;
	std::string pipeName = "\\\\.\\pipe\\demo_pipe";

	while (true) {
		WaitNamedPipe(convertToWideString(pipeName).c_str(), INFINITE);
		hNamedPipe = CreateFile(
			convertToWideString(pipeName).c_str(),
			GENERIC_READ | GENERIC_WRITE,
			0,
			(LPSECURITY_ATTRIBUTES)NULL,
			OPEN_EXISTING,
			0,
			(HANDLE)NULL
		);

		if (hNamedPipe == INVALID_HANDLE_VALUE)
		{
			std::cerr << "Connection with the named pipe failed." << std::endl
				<< "The last error code: " << GetLastError() << std::endl;
			std::cout << "Press any char to finish the client: ";
			std::cin >> c;
			return 0;
		}
		else {
			break;
		}
	}
    while (true) {
        std::cout << "1 - modification\n2 - reading\n3 - exit\n";
        int answer;
        std::cin >> answer;
        if (answer == 1) {
            boolean ok = false;
            while (!ok) {
                std::cout << "Enter ID\n";
                int id;
                std::cin >> id;
                message msg;
                msg.id = id;
                msg.type = 'w';
                employee emp;
                TransactNamedPipe(
                    hNamedPipe,                  // pipe handle 
                    (char*)&msg,              // message to server
                    sizeof(msg), // message length 
                    (char*)&emp,              // buffer to receive reply
                    sizeof(emp),  // size of read buffer
                    &cbRead,                // bytes read
                    NULL);                  // not overlapped 
                std::cout << "Name: " << emp.name << "\nHours: " << emp.hours << "\nModified name:\n";
                std::cin >> emp.name;
                std::cout << "\nModified hours:\n";
                std::cin >> emp.hours;
                DWORD dwRead;
                WriteFile(hNamedPipe,
                    (char*)&emp,
                    sizeof(emp),
                    &dwRead,
                    NULL);
                std::cout << "\nPress any key to finish\n";
                _getch();
            }
        else {
            std::cout << "\nEmploee not found\n";
        }
        }
        else if (answer == 2) {
            boolean ok = false;
            while (!ok) {
                cout << "\n??????? ID\n";
                int id;
                cin >> id;
                HANDLE mutex;
                mutex = OpenMutex(MUTEX_ALL_ACCESS, false, (LPWSTR)wstring_convert<codecvt_utf8<wchar_t>>().from_bytes("w" + to_string(id)).c_str());
                if (mutex != NULL) {
                    ok = true;
                    cout << "\n?????????, ?????? ? ?????? ????? ???? ????????????...\n";
                    WaitForSingleObject(mutex, INFINITE);
                    message msg;
                    msg.id = id;
                    msg.type = 'r';
                    employee emp;
                    TransactNamedPipe(
                        hPipe,                  // pipe handle 
                        (char*)&msg,              // message to server
                        sizeof(msg), // message length 
                        (char*)&emp,              // buffer to receive reply
                        sizeof(emp),  // size of read buffer
                        &cbRead,                // bytes read
                        NULL);                  // not overlapped 
                    cout << "??? ??????????: " << emp.name << "\n????? ???????????? ?????: " << emp.hours << "\n??????? ????? ???????, ????? ????????? ??????\n";
                    _getch();
                    ReleaseMutex(mutex);
                }
                else {
                    cout << "\n?????????? ? ????? ??????? ???\n";
                }
            }
        }
        else {
            break;
        }
    }
	/*for (int i = 0; i < 10; i++)
	{
		DWORD dwBytesWritten;
		if (!WriteFile(
			hNamedPipe, 
			&i, 
			sizeof(i), 
			&dwBytesWritten, 
			(LPOVERLAPPED)NULL 
		))
		{

			std::cerr << "Writing to the named pipe failed: " << std::endl
				<< "The last error code: " << GetLastError() << std::endl;
			std::cout << "Press any char to finish the client: ";
			std::cin >> c;
			CloseHandle(hNamedPipe);
			return 0;
		}
	
		std::cout << "The number " << i << " is written to the named pipe." << std::endl;
		Sleep(1000);
	}*/
	
	CloseHandle(hNamedPipe);

	std::cout << "The data are written by the client." << std::endl
		<< "Press any char to finish the client: ";
	std::cin >> c;
	return 0;
}