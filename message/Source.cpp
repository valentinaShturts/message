#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "C:/Users/Valentina/Desktop/json.hpp"
#include "C:/Users/Valentina/Desktop/httplib.h"
#include <windows.h>

using json = nlohmann::json;
using namespace std;

const string HOST = "shturts.atwebpages.com";
const string ADD_PATH = "/dotnet/upload.php";
const string GET_PATH = "/dotnet/get_messages.php";

// перетворення Windows-1251 у UTF-8
string win1251_to_utf8(const string& win1251_str) {
    // спочатку в wstring (UTF-16)
    int wsize = MultiByteToWideChar(CP_ACP, 0, win1251_str.c_str(), -1, nullptr, 0);
    wstring wstr(wsize, 0);
    MultiByteToWideChar(CP_ACP, 0, win1251_str.c_str(), -1, &wstr[0], wsize);

    // потім у UTF-8
    int usize = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    string utf8_str(usize, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &utf8_str[0], usize, nullptr, nullptr);
    return utf8_str;
}

// перетворення UTF-8 у wstring
wstring utf8_to_wstring(const string& str) {
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    if (size == 0) {
        wcout << L"Помилка перетворення UTF-8 у wstring\n";
        return L"";
    }
    wstring wstr(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], size);
    return wstr;
}

void sendMessage(const string& nickname, const string& messageText) 
{
    httplib::Client cli(HOST);

    time_t now = time(0);
    tm* ltm = localtime(&now);
    char timeStr[30];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", ltm);

    string finalMessage = "[" + string(timeStr) + "] " + nickname + ": " + messageText;

    json j;
    j["message"] = finalMessage;

    string json_str = j.dump();

    httplib::Headers headers = { {"Content-Type", "application/json"} };
    if (auto res = cli.Post(ADD_PATH, headers, json_str, "application/json")) {
        wcout << L"Сервер відповів: " << utf8_to_wstring(res->body) << endl;
    }
    else {
        wcout << L"Помилка: сервер не відповів\n";
    }
}
void getMessages() {
    httplib::Client cli(HOST);

    if (auto res = cli.Get(GET_PATH)) {
        if (res->status == 200 && !res->body.empty()) {
            try {
                json j = json::parse(res->body);
                for (const auto& msg : j) {
                    wcout << utf8_to_wstring(msg) << endl;
                }
            }
            catch (const std::exception& e) {
                wcout << L"Помилка розбору повідомлень: " << utf8_to_wstring(e.what()) << endl;
            }
        }
        else {
            wcout << L"Сервер повернув пусту відповідь або помилку.\n";
        }
    }
    else {
        wcout << L"Сервер не відповідає.\n";
    }
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    _setmode(_fileno(stdout), _O_U16TEXT);

    wcout << L"Введіть ваш нікнейм: ";
    string nickname;
    getline(cin, nickname);

    getMessages();

    wcout << L"Введіть повідомлення (Ctrl+C для виходу):\n";
    string text;
    while (true) {
        getline(cin, text);
        if (!text.empty()) {
            sendMessage(nickname, text);
        }
    }

    return 0;
}