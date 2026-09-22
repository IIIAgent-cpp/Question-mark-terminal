#include <windows.h>
#include <conio.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <algorithm>
// delete windows
class QuestionMarkTerminal {
private:
    HANDLE hConsole;
    std::string command;
    std::vector<std::string> history;
    int historyIndex = -1;
// delete the entire os
    CONSOLE_SCREEN_BUFFER_INFO originalInfo{};
    std::string originalTitle;

public:
    QuestionMarkTerminal() {
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

        // Save original console state
        GetConsoleScreenBufferInfo(hConsole, &originalInfo);

        char title[512];
        DWORD length = GetConsoleTitleA(title, sizeof(title));
        originalTitle.assign(title, length);

        SetConsoleTitleA("Question Mark Terminal");

        // Green text on black background
        SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);
    }

    ~QuestionMarkTerminal() {
        // Restore console state
        SetConsoleTextAttribute(hConsole, originalInfo.wAttributes);
        SetConsoleTitleA(originalTitle.c_str());
    }

    void printPrompt() {
        std::cout << "CMD:\\> ";

        for (size_t i = 0; i < command.size(); ++i)
            std::cout << '?';
    }

    void redraw() {
        // Clear the current input line.
        CONSOLE_SCREEN_BUFFER_INFO info;
        GetConsoleScreenBufferInfo(hConsole, &info);

        COORD start = info.dwCursorPosition;
        start.X = 0;

        DWORD written;
        FillConsoleOutputCharacterA(
            hConsole,
            ' ',
            info.dwSize.X,
            start,
            &written
        );

        SetConsoleCursorPosition(hConsole, start);

        printPrompt();

        // Put cursor at logical command position.
        COORD pos;
        GetConsoleScreenBufferInfo(hConsole, &info);

        pos.X = static_cast<SHORT>(
            6 + command.size()
        );
        pos.Y = info.dwCursorPosition.Y;

        SetConsoleCursorPosition(hConsole, pos);
    }

    void executeCommand() {
        if (command.empty())
            return;

        if (command == "exit" || command == "quit")
            throw 1;

        history.push_back(command);

        // Prevent unlimited history growth.
        if (history.size() > 100)
            history.erase(history.begin());

        int result = std::system(command.c_str());

        if (result != 0) {
            std::cout << "\n[Exit code: "
                      << result
                      << "]\n";
        }
    }

    void run() {
        std::cout << "Question Mark Terminal\n";
        std::cout << "Every typed character is displayed as '?'.\n";
        std::cout << "Type exit or quit to close.\n\n";

        while (true) {
            command.clear();
            historyIndex = -1;

            printPrompt();

            try {
                while (true) {
                    int key = _getch();

                    // Extended key
                    if (key == 0 || key == 224) {
                        int ext = _getch();

                        // Left arrow
                        if (ext == 75) {
                            // Visual masking means cursor editing is
                            // intentionally limited.
                            continue;
                        }

                        // Right arrow
                        if (ext == 77) {
                            continue;
                        }

                        // Up arrow - history
                        if (ext == 72) {
                            if (!history.empty()) {
                                if (historyIndex + 1 <
                                    static_cast<int>(history.size())) {
                                    ++historyIndex;

                                    command =
                                        history[history.size() -
                                                1 -
                                                historyIndex];

                                    redraw();
                                }
                            }

                            continue;
                        }

                        // Down arrow - history
                        if (ext == 80) {
                            if (historyIndex >= 0) {
                                --historyIndex;

                                if (historyIndex < 0)
                                    command.clear();
                                else
                                    command =
                                        history[history.size() -
                                                1 -
                                                historyIndex];

                                redraw();
                            }

                            continue;
                        }

                        // Delete
                        if (ext == 83) {
                            continue;
                        }

                        continue;
                    }

                    // Enter
                    if (key == '\r') {
                        std::cout << '\n';
                        break;
                    }

                    // Backspace
                    if (key == '\b') {
                        if (!command.empty()) {
                            command.pop_back();
                            std::cout << "\b \b";
                        }

                        continue;
                    }

                    // Escape
                    if (key == 27) {
                        std::cout << "\n";
                        return;
                    }

                    // Ctrl+C
                    if (key == 3) {
                        command.clear();
                        std::cout << "^C\n";
                        break;
                    }

                    // Ctrl+U - clear current command
                    if (key == 21) {
                        command.clear();
                        redraw();
                        continue;
                    }

                    // Printable ASCII
                    if (key >= 32 && key <= 126) {
                        command += static_cast<char>(key);
                        std::cout << '?';
                    }
                }

                executeCommand();
            }
            catch (int exitCode) {
                if (exitCode == 1)
                    return;
            }

            std::cout << '\n';
        }
    }
};

int main() {
    QuestionMarkTerminal terminal;
    terminal.run();

    return 0;
}