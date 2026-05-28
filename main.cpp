#include <iostream>
#include <string>
#include <cctype>

// Подключаем наши классы
#include "src/lab2_files/ArraySequence.hpp"
#include "src/LazySequence.hpp"
#include "src/Stream.hpp"
#include "src/StateMachine.hpp"

using namespace std;

// ==========================================
// НАСТРОЙКА АВТОМАТА (FSM) ДЛЯ EMAIL
// ==========================================
void SetupEmailFSM(fsm::StateMachine<char>& machine) {
    // 1. Создаем состояния
    machine.AddState("START", false);
    machine.AddState("NAME", false);
    machine.AddState("AT", false);
    machine.AddState("DOMAIN", false);
    machine.AddState("DOT", false);
    machine.AddState("SUCCESS", true); // Только это состояние означает валидный Email

    machine.SetInitialState("START");

    // Вспомогательные функции-условия
    auto is_alnum = [](const char& c) { return std::isalnum(c) || c == '_' || c == '-'; };
    auto is_alpha = [](const char& c) { return std::isalpha(c); };

    // 2. Настраиваем переходы (Transitions)
    // Из START в NAME, если буква или цифра
    machine.AddTransition("START", "NAME", is_alnum);
    // Крутимся в NAME, пока идут буквы/цифры/символы
    machine.AddTransition("NAME", "NAME", is_alnum);
    // Из NAME в AT, если встретили '@'
    machine.AddTransition("NAME", "AT", [](const char& c) { return c == '@'; });
    
    // Из AT в DOMAIN
    machine.AddTransition("AT", "DOMAIN", is_alnum);
    machine.AddTransition("DOMAIN", "DOMAIN", is_alnum);
    // Из DOMAIN в DOT при встрече точки '.'
    machine.AddTransition("DOMAIN", "DOT", [](const char& c) { return c == '.'; });

    // Из DOT в SUCCESS (например, 'com', 'ru')
    machine.AddTransition("DOT", "SUCCESS", is_alpha);
    // Крутимся в SUCCESS для оставшихся букв домена
    machine.AddTransition("SUCCESS", "SUCCESS", is_alpha);
}

// ==========================================
// ГЛАВНОЕ МЕНЮ (UI)
// ==========================================
int main() {
    fsm::StateMachine<char> email_validator;
    SetupEmailFSM(email_validator);

    int choice = 0;
    while (choice != 2) {
        cout << "\n=== Лабораторная работа №4: Машина состояний (FSM) ===" << endl;
        cout << "1. Проверить Email-адрес (Ручной ввод)" << endl;
        cout << "2. Выход" << endl;
        cout << "Выберите действие: ";
        
        // Защита от ввода букв вместо цифр
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(10000, '\n');
            continue;
        }

        if (choice == 1) {
            cout << "\nВведите Email-адрес для проверки: ";
            string input;
            cin >> input;

            // Оборачиваем строку пользователя в нашу Sequence для автомата
            Sequence<char>* char_seq = new MutableArraySequence<char>();
            for (char c : input) {
                char_seq = char_seq->Append(c);
            }

            // Запускаем автомат!
            bool is_valid = email_validator.Process(char_seq);
            
            cout << "----------------------------------------" << endl;
            if (is_valid) {
                cout << "  РЕЗУЛЬТАТ: Email '" << input << "' ВАЛИДЕН!" << endl;
            } else {
                cout << "  РЕЗУЛЬТАТ: Ошибка! Неверный формат Email." << endl;
                cout << "  Автомат остановился в состоянии: " << email_validator.GetCurrentState() << endl;
            }
            cout << "----------------------------------------" << endl;

            delete char_seq;
        }
    }

    cout << "Программа завершена. Удачи на защите!" << endl;
    return 0;
}
