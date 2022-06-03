#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <windows.h>
#include <fstream>
#include <codecvt>
#include <string>
#include <vector>

std::wstring convertToWideString(const std::string& str) {
	const char* c_str = str.c_str();
	wchar_t* wstr = new wchar_t[str.length() + 1];
	mbstowcs(wstr, c_str, str.size() + 1);
	std::wstring wide(wstr);
	delete[] wstr;
	return wide;
}

void replace(std::string& str, const std::string& from, const std::string& to) {
	if (from.empty())
		return;
	std::string::size_type start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}
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

int processesNumber, recordsNumber;
std::fstream file;
employee emp;
HANDLE* processes;

void thread(HANDLE hNamedPipe);

int main(int argc, char* argv[])
{
	std::cout << "Enter file name: ";
	std::string filename;
	std::cin >> filename;

	std::cout << "Enter number of records: ";
	std::cin >> recordsNumber;

	file = std::fstream(filename, std::ios::trunc | std::ios::binary | std::ios::in | std::ios::out);

	HANDLE* readAccess = new HANDLE[recordsNumber];
	HANDLE* writeAccess = new HANDLE[recordsNumber];

	for (int i = 0; i < recordsNumber; i++) {
		std::cout << "Write num: ";
		std::cin >> emp.num;
		std::cout << "Write name: ";
		std::cin >> emp.name;
		std::cout << "Write hours: ";
		std::cin >> emp.hours;
		file.write((char*)&emp, sizeof(emp));
		std::string read = "r" + std::to_string(emp.num);
		std::string write = "w" + std::to_string(emp.num);
		readAccess[i] = CreateMutex(NULL, 0, (LPWSTR)convertToWideString(read).c_str());
		writeAccess[i] = CreateMutex(NULL, 0, (LPWSTR)convertToWideString(write).c_str());
	}
	std::cout << "\n" << filename << "\n";

	file.clear();
	file.seekg(0);

	for (int i = 0; i < recordsNumber; i++) {
		file.read((char*)&emp, sizeof(emp));
		std::cout << emp.num << " " << emp.name << " " << emp.hours << "\n";
	}

	std::cout << "Enter number of processes: ";
	std::cin >> processesNumber;
	if (processesNumber < 1) {
		std::cerr << "wrong number of processes\n";
		return 0;
	}

	std::vector<STARTUPINFO> si(processesNumber);
	std::vector<PROCESS_INFORMATION> procInfo(processesNumber);
	std::string cmdLine;
	std::string path = argv[0];
	processes = new HANDLE[processesNumber];
	replace(path, "Server.exe", "");
	cmdLine = path + "Client.exe ";

	for (int i = 0; i < processesNumber; ++i) {

		ZeroMemory(&si[i], sizeof(STARTUPINFO));
		si[i].cb = sizeof(STARTUPINFO);

		if (!CreateProcess(
			NULL,
			(LPWSTR)convertToWideString(cmdLine).c_str(),
			NULL,
			NULL,
			FALSE,
			CREATE_NEW_CONSOLE,
			NULL, NULL,
			&si[i],
			&procInfo[i])) {

			std::cerr << "error creating process";
			std::cerr << GetLastError();
			return 0;
		}
		else {
			std::cout << "start process\n";
		}

		processes[i] = procInfo[i].hProcess;
	}

	HANDLE hNamedPipe;
	std::string pipeName = "\\\\.\\pipe\\demo_pipe";
	DWORD* idthread = new DWORD[processesNumber];
	HANDLE* threads = new HANDLE[processesNumber];
	for (int i = 0; i < processesNumber; i++) {
		hNamedPipe = CreateNamedPipe(
			convertToWideString(pipeName).c_str(),
			PIPE_ACCESS_DUPLEX,
			PIPE_TYPE_MESSAGE | PIPE_WAIT,
			processesNumber,
			0,
			0,
			INFINITE,
			(LPSECURITY_ATTRIBUTES)NULL
		);
		threads[i] = CreateThread(NULL, 0, LPTHREAD_START_ROUTINE(thread), (void*)hNamedPipe, 0, &idthread[i]);
	}

	WaitForMultipleObjects(processesNumber, threads, true, INFINITE);

	system("pause");
}

void thread(HANDLE hNamedPipe) {
	ConnectNamedPipe(hNamedPipe, NULL);

	while (WaitForMultipleObjects(processesNumber, processes, true, 0)) {
		message msg;
		DWORD dwRead;
		ReadFile(hNamedPipe, (char*)&msg, sizeof(msg), &dwRead, NULL);
		file.clear();
		file.seekg(0);
		int pos;
		for (int i = 0; i < recordsNumber; i++) {
			pos = file.tellg();
			file.read((char*)&emp, sizeof(emp));
			if (emp.num == msg.id) {
				break;
			}
		}
		WriteFile(hNamedPipe, &emp, sizeof(emp), &dwRead, NULL);
		if (msg.type == 'w') {
			ReadFile(hNamedPipe, &emp, sizeof(emp), &dwRead, NULL);
			for (int i = 0; i < recordsNumber; i++) {
				file.clear();
				file.seekg(pos);
				file.write((char*)&emp, sizeof(emp));
			}
		}
	}
}