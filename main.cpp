#include <iostream>
#include <string>
#include <cstdlib>
#include <filesystem> // Мастхэв из C++17 для работы с путями

#include "src/lab2_files/ArraySequence.hpp"
#include "src/LazySequence.hpp"
#include "src/StateMachine.hpp"
#include "src/Stream.hpp"

int ReadInt(int min, int max) {
    int value;
    std::cin >> value;

    while ( std::cin.fail() || value < min || value > max ) {
        std::cin.clear(); 
        std::cin.ignore(32767, '\n'); 
        std::cout << "Ошибка! Введите корректное число от " << min << " до " << max << ": ";
        std::cin >> value;
    }
    return value;
}

// === Функции-условия для автомата ===
bool IsValidNameChar( const char& c ) {
    bool isLower = ( c >= 'a' && c <= 'z' );
    bool isUpper = ( c >= 'A' && c <= 'Z' );
    bool isDigit = ( c >= '0' && c <= '9' );
    bool isSpecial = ( c == '_' || c == '-' );
    return isLower || isUpper || isDigit || isSpecial;
}

bool IsLetter( const char& c ) {
    bool isLower = ( c >= 'a' && c <= 'z' );
    bool isUpper = ( c >= 'A' && c <= 'Z' );
    return isLower || isUpper;
}

bool IsAtSymbol( const char& c ) { return c == '@'; }
bool IsDotSymbol( const char& c ) { return c == '.'; }

void SetupEmailFSM( fsm::StateMachine<char>& machine ) {
    machine.AddState( "START", false );
    machine.AddState( "NAME", false );
    machine.AddState( "AT", false );
    machine.AddState( "DOMAIN", false );
    machine.AddState( "DOT", false );
    machine.AddState( "SUCCESS", true ); 

    machine.SetInitialState( "START" );

    machine.AddTransition( "START", "NAME", IsValidNameChar );
    machine.AddTransition( "NAME", "NAME", IsValidNameChar );
    machine.AddTransition( "NAME", "AT", IsAtSymbol );
    
    machine.AddTransition( "AT", "DOMAIN", IsValidNameChar );
    machine.AddTransition( "DOMAIN", "DOMAIN", IsValidNameChar );
    machine.AddTransition( "DOMAIN", "DOT", IsDotSymbol );

    machine.AddTransition( "DOT", "SUCCESS", IsLetter );
    machine.AddTransition( "SUCCESS", "SUCCESS", IsLetter );
}

// Добавляем аргументы argc и argv, чтобы узнать путь запуска
int main(int argc, char* argv[]) {
    fsm::StateMachine<char> email_validator;
    SetupEmailFSM( email_validator );

    // === Умный поиск файла тестов (C++17) ===
    // Получаем абсолютный путь к запущенному app_main
    std::filesystem::path app_path = std::filesystem::absolute(argv[0]);
    // Получаем папку, в которой он лежит (твоя папка build)
    std::filesystem::path dir_path = app_path.parent_path();
    // Формируем точный путь к соседнему файлу app_tests
    std::filesystem::path tests_path = dir_path / "app_tests";

    int choice = 0;
    while (choice != 3) {
        std::cout << std::endl;
        std::cout << "==== Лабораторная работа №4: Машина состояний (FSM) ====" << std::endl;
        std::cout << "  1. Проверить Email-адрес (Ручной ввод)" << std::endl;
        std::cout << "  2. Запустить автоматические тесты (Google Test)" << std::endl;
        std::cout << "  3. Выход" << std::endl;
        std::cout << "Выберите действие: ";
        
        choice = ReadInt( 1, 3 );

        if ( choice == 1 ) {
            std::cout << "\nВведите Email-адрес для проверки: ";
            std::string input;
            std::cin >> input;

            Sequence<char>* char_seq = new MutableArraySequence<char>();
            for ( char c : input ) {
                char_seq->Append( c );
            }

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

        } else if ( choice == 2 ) {
            std::cout << "\n===========================================================" << std::endl;
            std::cout << " ЗАПУСК GOOGLE TESTS..." << std::endl;
            std::cout << "===========================================================\n" << std::endl;
            
            // Запускаем тесты по надежному абсолютному пути
            std::string command = "\"" + tests_path.string() + "\"";
            int result = system(command.c_str());
            
            if (result != 0) {
                std::cout << "\n[ВНИМАНИЕ] Не удалось запустить тесты по пути: \n" << tests_path << std::endl;
                std::cout << "Убедитесь, что app_tests успешно скомпилирован." << std::endl;
            }
        }
    }

    std::cout << "Программа завершена. До свидания!" << std::endl;
    return 0;
}
