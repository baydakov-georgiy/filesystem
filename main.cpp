#include "FileSystem.h"
#include <iostream>
#include <sstream>
#include <cstdlib>

using namespace std;

vector<string> tokenize(const string& line) {
    vector<string> tokens;
    stringstream ss(line);
    string token;
    
    while (ss >> token) {
        tokens.push_back(token);
    }
    
    return tokens;
}

struct Command {
    vector<string> args;
    string redirect_out;
    string redirect_append;
    bool has_redirect = false;
    bool is_append = false;
};

Command parseCommand(const string& line) {
    Command cmd;
    stringstream ss(line);
    string token;
    bool redirect_mode = false;
    
    while (ss >> token) {
        if (token == ">") {
            redirect_mode = true;
            cmd.has_redirect = true;
            cmd.is_append = false;
        } else if (token == ">>") {
            redirect_mode = true;
            cmd.has_redirect = true;
            cmd.is_append = true;
        } else if (redirect_mode) {
            if (cmd.is_append) {
                cmd.redirect_append = token;
            } else {
                cmd.redirect_out = token;
            }
            redirect_mode = false;
        } else {
            cmd.args.push_back(token);
        }
    }
    
    return cmd;
}

void runTerminal(FileSystem& fs) {
    string line;
    
    cout << "\n" << endl;
    cout << "Linux-подобный терминал файловой системы" << endl;
    cout << "Введите 'help' для списка команд, 'exit' для выхода" << endl;
    
    while (true) {
        const char* user = getenv("USER");
        if (!user) user = "user";
        cout << "\033[1;32m" << user << "@filesystem"  
             << "\033[0m:\033[1;34m" << fs.getCurrentPath() 
             << "\033[0m$ ";
        
        if (!getline(cin, line)) break;
        
        if (line.empty()) continue;
        
        Command cmd = parseCommand(line);
        if (cmd.args.empty()) continue;
        
        string command = cmd.args[0];
        
        if (command == "exit" || command == "quit") {
            cout << "Выход из терминала..." << endl;
            break;
        }
        else if (command == "help") {
            cout << "Доступные команды:" << endl;
            cout << "  pwd              - показать текущий путь" << endl;
            cout << "  ls [-l]          - список файлов" << endl;
            cout << "  cd <path>        - перейти в директорию" << endl;
            cout << "  mkdir <name>     - создать директорию" << endl;
            cout << "  touch <name>     - создать пустой файл" << endl;
            cout << "  cat <file>       - вывести содержимое файла" << endl;
            cout << "  echo <text>      - вывести текст (можно с > file или >> file)" << endl;
            cout << "  rm <name>        - удалить файл" << endl;
            cout << "  rm -r <name>     - удалить директорию рекурсивно" << endl;
            cout << "  chmod <mode> <f> - изменить права (например: chmod 755 file)" << endl;
            cout << "  find <pattern>   - найти файлы по шаблону" << endl;
            cout << "  tree             - показать дерево файловой системы" << endl;
            cout << "  clear            - очистить экран" << endl;
            cout << "  debug            - переключить режим отладки" << endl;
            cout << "  ed <f> <op> [...] - редактор (insert/delete/append/find)" << endl;
            cout << "  exit             - выход" << endl;
        }
        else if (command == "pwd") {
            cout << fs.getCurrentPath() << endl;
        }
        else if (command == "ls") {
            bool showDetails = false;
            string path = "";
            
            for (size_t i = 1; i < cmd.args.size(); i++) {
                if (cmd.args[i] == "-l") {
                    showDetails = true;
                } else {
                    path = cmd.args[i];
                }
            }
            
            if (!path.empty()) {
                fs.ls(path, showDetails);
            } else {
                fs.ls(showDetails);
            }
        }
        else if (command == "cd") {
            if (cmd.args.size() < 2) {
                fs.changeDirectory("/");
            } else {
                fs.changeDirectory(cmd.args[1]);
            }
        }
        else if (command == "mkdir") {
            if (cmd.args.size() < 2) {
                cout << "mkdir: отсутствует операнд" << endl;
            } else {
                fs.mkdir(cmd.args[1]);
            }
        }
        else if (command == "touch") {
            if (cmd.args.size() < 2) {
                cout << "touch: отсутствует операнд" << endl;
            } else {
                fs.touch(cmd.args[1]);
            }
        }
        else if (command == "cat") {
            if (cmd.args.size() < 2) {
                cout << "cat: отсутствует операнд" << endl;
            } else {
                fs.cat(cmd.args[1]);
            }
        }
        else if (command == "echo") {
            if (cmd.args.size() < 2) {
                cout << endl;
            } else {
                string text;
                for (size_t i = 1; i < cmd.args.size(); i++) {
                    if (i > 1) text += " ";
                    text += cmd.args[i];
                }
                
                if (cmd.has_redirect) {
                    text += "\n";
                    if (cmd.is_append) {
                        fs.appendFile(cmd.redirect_append, text);
                    } else {
                        fs.writeFile(cmd.redirect_out, text);
                    }
                } else {
                    cout << text << endl;
                }
            }
        }
        else if (command == "rm") {
            if (cmd.args.size() < 2) {
                cout << "rm: отсутствует операнд" << endl;
            } else if (cmd.args.size() == 2) {
                fs.rm(cmd.args[1], false);
            } else if (cmd.args[1] == "-r" && cmd.args.size() >= 3) {
                fs.rm(cmd.args[2], true);
            } else {
                fs.rm(cmd.args[1], false);
            }
        }
        else if (command == "chmod") {
            if (cmd.args.size() < 3) {
                cout << "chmod: отсутствует операнд" << endl;
            } else {
                fs.chmod(cmd.args[1], cmd.args[2]);
            }
        }
        else if (command == "find") {
            if (cmd.args.size() < 2) {
                cout << "find: отсутствует операнд" << endl;
            } else {
                fs.findFiles(cmd.args[1]);
            }
        }
        else if (command == "tree") {
            fs.visualize();
        }
        else if (command == "clear") {
            cout << "\033[2J\033[1;1H";
        }
        else if (command == "debug") {
            fs.toggleDebug();
        }
        else if (command == "ed") {
            if (cmd.args.size() < 3) {
                cout << "ed: использование: ed <file> <operation> [args...]" << endl;
                cout << "  операции:" << endl;
                cout << "    insert <pos> <text> - вставить текст на позицию" << endl;
                cout << "    delete <substring>  - удалить первое вхождение подстроки" << endl;
                cout << "    append <text>       - добавить текст в конец" << endl;
                cout << "    find <substring>    - найти позицию подстроки" << endl;
            } else {
                string filename = cmd.args[1];
                string operation = cmd.args[2];
                
                if (operation == "insert" && cmd.args.size() >= 5) {
                    int pos = stoi(cmd.args[3]);
                    string text;
                    for (size_t i = 4; i < cmd.args.size(); i++) {
                        if (i > 4) text += " ";
                        text += cmd.args[i];
                    }
                    if (fs.insertInFile(filename, pos, text)) {
                        cout << "Текст вставлен на позицию " << pos << endl;
                    }
                } else if (operation == "delete" && cmd.args.size() >= 4) {
                    string substr;
                    for (size_t i = 3; i < cmd.args.size(); i++) {
                        if (i > 3) substr += " ";
                        substr += cmd.args[i];
                    }
                    if (fs.deleteFromFile(filename, substr)) {
                        cout << "Подстрока удалена" << endl;
                    }
                } else if (operation == "append" && cmd.args.size() >= 4) {
                    string text;
                    for (size_t i = 3; i < cmd.args.size(); i++) {
                        if (i > 3) text += " ";
                        text += cmd.args[i];
                    }
                    if (fs.appendFile(filename, text)) {
                        cout << "Текст добавлен" << endl;
                    }
                } else if (operation == "find" && cmd.args.size() >= 4) {
                    string substr;
                    for (size_t i = 3; i < cmd.args.size(); i++) {
                        if (i > 3) substr += " ";
                        substr += cmd.args[i];
                    }
                    int pos = fs.findInFile(filename, substr);
                    if (pos >= 0) {
                        cout << "Найдено на позиции: " << pos << endl;
                    } else {
                        cout << "Не найдено" << endl;
                    }
                } else {
                    cout << "ed: неверная операция или аргументы" << endl;
                }
            }
        }
        else {
            cout << command << ": команда не найдена" << endl;
        }
    }
}

int main() {
    FileSystem fs;

    fs.createDirectory("/home");
    fs.createDirectory("/home/user");
    fs.createDirectory("/etc");
    fs.createDirectory("/var");
    fs.createDirectory("/tmp");
    
    runTerminal(fs);
    
    return 0;
}
