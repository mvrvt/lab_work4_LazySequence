#include <iostream>
#include <string>

#include "src/lab2_files/ArraySequence.hpp"
#include "src/LazySequence.hpp"
#include "src/Stream.hpp"
#include "src/StateMachine.hpp"

int ReadInt(int min, int max) {
    int value;
    std::cin >> value;

    // Пока ввод некорректен (ввели букву) или число выходит за границы
    while ( std::cin.fail() || value < min || value > max ) {
        std::cin.clear(); // Сбрасываем флаг ошибки cin
        std::cin.ignore(32767, '\n'); // Очищаем буфер ввода до конца строки
        std::cout << "Ошибка! Введите корректное число от " << min << " до " << max << ": ";
        std::cin >> value;
    }
    return value;
}

// === Функции-условия для автомата ===

// Проверка: является ли символ допустимым для имени ящика (буквы, цифры, дефис, подчеркивание)
bool IsValidNameChar( const char& c ) {
    bool isLower = ( c >= 'a' && c <= 'z' );
    bool isUpper = ( c >= 'A' && c <= 'Z' );
    bool isDigit = ( c >= '0' && c <= '9' );
    bool isSpecial = ( c == '_' || c == '-' );
    
    return isLower || isUpper || isDigit || isSpecial;
}

// Проверка: является ли символ только буквой (для домена верхнего уровня, типа .com, .ru)
bool IsLetter( const char& c ) {
    bool isLower = ( c >= 'a' && c <= 'z' );
    bool isUpper = ( c >= 'A' && c <= 'Z' );
    
    return isLower || isUpper;
}

// Проверка: является ли символ собачкой '@'
bool IsAtSymbol( const char& c ) {
    return c == '@';
}

// Проверка: является ли символ точкой '.'
bool IsDotSymbol( const char& c ) {
    return c == '.';
}

// Настройка автомата (FSM) для Email
void SetupEmailFSM( fsm::StateMachine<char>& machine ) {
    // 1. Создаем состояния
    machine.AddState( "START", false );
    machine.AddState( "NAME", false );
    machine.AddState( "AT", false );
    machine.AddState( "DOMAIN", false );
    machine.AddState( "DOT", false );
    machine.AddState( "SUCCESS", true ); // Только это состояние означает валидный Email

    machine.SetInitialState( "START" );

    // 2. Настраиваем переходы, передавая наши простые функции
    machine.AddTransition( "START", "NAME", IsValidNameChar );
    machine.AddTransition( "NAME", "NAME", IsValidNameChar );
    machine.AddTransition( "NAME", "AT", IsAtSymbol );
    
    machine.AddTransition( "AT", "DOMAIN", IsValidNameChar );
    machine.AddTransition( "DOMAIN", "DOMAIN", IsValidNameChar );
    machine.AddTransition( "DOMAIN", "DOT", IsDotSymbol );

    machine.AddTransition( "DOT", "SUCCESS", IsLetter );
    machine.AddTransition( "SUCCESS", "SUCCESS", IsLetter );
}

int main() {
    // Создаем и настраиваем автомат
    fsm::StateMachine<char> email_validator;
    SetupEmailFSM( email_validator );

    int choice = 0;
    while (choice != 2) {
        std::cout << std::endl;
        std::cout << "==== Лабораторная работа №4: Машина состояний (FSM) ====" << std::endl;
        std::cout << "  1. Проверить Email-адрес (Ручной ввод)" << std::endl;
        std::cout << "  2. Выход" << std::endl;
        std::cout << "Выберите действие: ";
        
        choice = ReadInt( 1, 2 );

        if ( choice == 1 ) {
            std::cout << "\nВведите Email-адрес для проверки: ";
            std::string input;
            std::cin >> input;

            // Оборачиваем строку пользователя в Sequence для автомата
            Sequence<char>* char_seq = new MutableArraySequence<char>();
            for ( char c : input ) {
                char_seq->Append( c );
            }

            // Запуск автомата
            bool is_valid = email_validator.Process( char_seq );
            
            std::cout << "-----------------------------------------------------------" << std::endl;
            if (is_valid) {
                std::cout << "  РЕЗУЛЬТАТ: Email '" << input << "' ВАЛИДЕН!" << std::endl;
            } else {
                std::cout << "  РЕЗУЛЬТАТ: Ошибка! Неверный формат Email." << std::endl;
                std::cout << "  Автомат остановился в состоянии: " << email_validator.GetCurrentState() << std::endl;
            }
            std::cout << "-----------------------------------------------------------" << std::endl;

            delete char_seq;
        }
    }

    std::cout << "Программа завершена. До свидания!" << std::endl;
    return 0;
}
