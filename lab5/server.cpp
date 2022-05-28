#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <Windows.h>
#include <vector>
#include <string>
#include <list>

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
std::vector<HANDLE> processes;
char c;

void thread(HANDLE hNamedPipe);

int main(int argc, char* argv[]) {
    std::cout << "Enter file name: ";
    std::string filename;
    std::cin >> filename;

    std::cout << "Enter number of records: ";
    std::cin >> recordsNumber;

    std::fstream file = std::fstream(filename, std::ios::trunc | std::ios::binary | std::ios::in | std::ios::out);
    
    for (int i = 0; i < recordsNumber; i++) {
        std::cout << "Write num: ";
        std::cin >> emp.num;
        std::cout << "Write name: ";
        std::cin >> emp.name;
        std::cout << "Write hours: ";
        std::cin >> emp.hours;
        file.write((char*)&emp, sizeof(emp));
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
        std::cerr << "wrong number of threads\n";
        return 0;
    }
    
    std::vector<STARTUPINFO> si(processesNumber);
    std::vector<PROCESS_INFORMATION> procInfo(processesNumber);
    std::string cmdLine;
    std::string path = argv[0];
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
    }
    /////////////////////////////////////////////////
    HANDLE hNamedPipe;
    std::string pipeName = "\\\\.\\pipe\\demo_pipe";

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
        if (hNamedPipe == INVALID_HANDLE_VALUE)
        {
            std::cerr << "Creation of the named pipe failed." << std::endl
                << "The last error code: " << GetLastError() << std::endl;
            std::cout << "Press any char to finish server: ";
            std::cin >> c;
            return 0;
        }

        std::cout << "The server is waiting for connection with a client." << std::endl;
        
       
        
    }
    /*std::cout << "Enter r to read or any other string to exit\n";
    std::string input = "r";
    std::string message;
    std::ifstream fin{};
    std::string cachedFile;

    while (true) {
        std::cout << "enter next action: ";
        std::cin >> input;
        if (input != "r") {
            break;
        }

        fin.open(filename, std::ifstream::binary);
        std::getline(fin, message);
        std::cout << "new message: " << message << '\n';

        while (std::getline(fin, message)) {
            cachedFile += message + '\n';
        }
        fin.close();
        file.open(filename, std::ofstream::binary | std::ofstream::trunc);
        file << cachedFile;
        file.close();
        cachedFile = "";
    }

    for (int i = 0; i < processesNumber; ++i) {
        CloseHandle(procInfo[i].hThread);
        CloseHandle(procInfo[i].hProcess);
    }*/
}
void thread(HANDLE hNamedPipe) {
    if (!ConnectNamedPipe(
        hNamedPipe,
        (LPOVERLAPPED)NULL
    ))
    {
        std::cerr << "The connection failed." << std::endl
            << "The last error code: " << GetLastError() << std::endl;
        CloseHandle(hNamedPipe);
        std::cout << "Press any char to finish the server: ";
        std::cin >> c;
    }
    else {
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
}