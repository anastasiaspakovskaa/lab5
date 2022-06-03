#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <iostream>
#include <string>
#include <codecvt>
#include <locale>
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
	int num;
	char name[10];
	double hours;
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
	DWORD cbRead, dwMode;

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

		if (hNamedPipe != INVALID_HANDLE_VALUE) {
			break;
		}
	}

	dwMode = PIPE_READMODE_MESSAGE;

	if (!SetNamedPipeHandleState(hNamedPipe, &dwMode, NULL, NULL))
	{
		std::cout << "SetNamedPipeHandleState failed.\n";
		return 0;
	}

	while (true) {
		std::cout << "1 - modification\n2 - reading\n3 - exit\n";
		int ans;
		std::cin >> ans;
		if (ans == 1) {
			boolean ok = false;
			while (!ok) {
				std::cout << "Enter ID\n";
				int id;
				std::cin >> id;
				HANDLE* mutex = new HANDLE[2];
				mutex[0] = OpenMutex(MUTEX_ALL_ACCESS, true, (LPWSTR)convertToWideString("r" + std::to_string(id)).c_str());
				if (mutex[0] != NULL) {
					ok = true;
					mutex[1] = OpenMutex(MUTEX_ALL_ACCESS, false, (LPWSTR)convertToWideString("w" + std::to_string(id)).c_str());
					std::cout << "Wait\n";
					WaitForMultipleObjects(2, mutex, true, INFINITE);
					message msg;
					msg.id = id;
					msg.type = 'w';
					employee emp;
					TransactNamedPipe(
						hNamedPipe,
						(char*)&msg,
						sizeof(msg),
						(char*)&emp,
						sizeof(emp),
						&cbRead,
						NULL);
					std::cout << "Name: " << emp.name << "\nHours: " << emp.hours << "\nModified name\n";
					std::cin >> emp.name;
					std::cout << "\nModified hours\n";
					std::cin >> emp.hours;
					DWORD dwRead;
					WriteFile(hNamedPipe, (char*)&emp, sizeof(emp), &dwRead, NULL);
					std::cout << "\nPress any key to finish\n";
					_getch();
					ReleaseMutex(mutex[0]);
					ReleaseMutex(mutex[1]);
				}
				else {
					std::cout << "\nEmployee not found\n";
				}
			}
		}
		else if (ans == 2) {
			boolean ok = false;
			while (!ok) {
				std::cout << "\nEnter ID\n";
				int id;
				std::cin >> id;
				HANDLE mutex;
				mutex = OpenMutex(MUTEX_ALL_ACCESS, false, (LPWSTR)convertToWideString("w" + std::to_string(id)).c_str());
				if (mutex != NULL) {
					ok = true;
					std::cout << "\nWait\n";
					WaitForSingleObject(mutex, INFINITE);
					message msg;
					msg.id = id;
					msg.type = 'r';
					employee emp;
					TransactNamedPipe(
						hNamedPipe,
						(char*)&msg,
						sizeof(msg),
						(char*)&emp,
						sizeof(emp),
						&cbRead,
						NULL);
					std::cout << "Name: " << emp.name << "\nHours: " << emp.hours << "\nPress any key to exit\n";
					_getch();
					ReleaseMutex(mutex);
				}
				else {
					std::cout << "\nEmployee not found\n";
				}
			}
		}
		else {
			return 0;
		}
	}
}