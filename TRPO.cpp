#include <iostream>
#include <fstream>
#include <conio.h>
#include <windows.h>
#include <sstream>
#include <string>
#include <bitset>
#include <vector>
#include <iomanip>
// Самописная библиотека для реализации интерфейса
#include "interface_lib.h"

// Флаг выбора типа вывода
bool choice = 0;
// Название входного файла
string filename_text;
// Количество дополнений
long long num_depo = 0;
// Флаг открытия тестового файла
bool test_file_open = false;

// Тексты интерфейса
const string intro = "Это программа для распознавания дополнений в тексте.\n-Для перемещения по пунктам используйте стрелки вверх/вниз\n"
"-Для подтверждения выбора нажмите ENTER или стрелку вправо\n"
"-Для выхода из программы нажмите на ESCAPE или стрелку влево\n\nДля продолжения нажмите на ENTER или стрелку вправо.";
const string main_menu_text = "Файл должен находиться в папке с программой. Выберите файл, который хотите проанализировать:\n";
const string menu_1_1_text = "Выберите тип вывода результатов в файл:\n";
string menu_1_1_list[] = {"Вывод дополнений в хронологии с текстом", "Вывод дополнений в отсортированном формате"};
const string menu_1_1_1_text = "Введите название файла, в который хотите вывести данные:\n\n";

// Структура для считывания текста с файла
struct STWord
{
    string word;
    STWord* pred_ptr;
    STWord* next_ptr;
};

// Структура для обработки слов по каждому предложению
struct Ending
{
    string name;
    //сущ|гл|прил(прич)|числ|мест|нар|деепр|предл
    int prt_of_spch = 0b00000000;
    string face = "-";
    string number = "-";
    string time = "-";
    string padezh = "-";
    Ending* pred_ptr;
    Ending* next_ptr;
};

// Структура дополнений
struct Depolnenie
{
    string name;
    int num_sent;
    Depolnenie* next_ptr;
};

//Структура дополнений со статистикой
struct WordStat
{
    string word;
    int count;
    vector<int> sentence_numbers;
    WordStat* next_ptr;
};

// Корень структуры слов текста
STWord* main_ptr = nullptr;
// Корень структуры окончаний
Ending* main_ending_ptr = nullptr;
// Корень структуры слов предложения, соотнесенных с окончаниями
Ending* word_to_ending = nullptr;
// Корень структуры предлогов
Ending* main_prep = nullptr;
// Корень структуры союзов, частиц и междометий
Ending* main_particles = nullptr;
// Корень структуры местоимений
Ending* main_pronoun = nullptr;
// Корень структуры числительных
Ending* main_numeral = nullptr;
// Корень структуры дополнений
Depolnenie* main_dep = nullptr;
// Корень структуры сбора статистики
WordStat* main_stat = nullptr;

// Функция очистки элемента
void free_node(Ending* node_to_delete) {
    if (node_to_delete == nullptr) return;

    // Обновляем связи соседних узлов
    if (node_to_delete == word_to_ending)
    {
        word_to_ending = node_to_delete->next_ptr;
        delete node_to_delete;
        return;
    }
    if (node_to_delete->pred_ptr != nullptr) {
        node_to_delete->pred_ptr->next_ptr = node_to_delete->next_ptr;
    }
    if (node_to_delete->next_ptr != nullptr) {
        node_to_delete->next_ptr->pred_ptr = node_to_delete->pred_ptr;
    }
}

//// Функция записи названия файла
//string WriteFilename() {
//    string filename;
//    // Флаг завершения записи
//    bool inputComplete = false;
//    while (!inputComplete) {
//        char ch = _getch();
//        // В случае нажатия esc - пустая строка
//        if (ch == 27)
//            return "";
//        else if (ch == 13)
//        {
//            if (!filename.empty())
//            {
//                cout << endl;
//                inputComplete = true;
//            }
//        }
//        // Удаление символа при нажатии backspace
//        else if (ch == 8) {
//            if (!filename.empty())
//            {
//                filename.pop_back();
//                cout << "\b \b";
//            }
//        }
//        else if (((ch >= 'а' && ch <= 'я') || (ch >= 'А' && ch <= 'Я') || ch == 'Ё' || ch == 'ё' || ch == ' ' || ch == '.' || ch == '-' || isalpha(ch) || isalnum(ch)) == 1)
//        {
//            filename += ch;
//            cout << ch;
//        }
//    }
//    return filename;
//}

//Функция сортировки
void SortDepolnenie() {
    int number_dop = 0;
    if (main_dep == nullptr || main_dep->next_ptr == nullptr)
        return;

    // Начало отсортированного списка
    Depolnenie* sorted = nullptr; 

    Depolnenie* current = main_dep;
    while (current != nullptr) {
        Depolnenie* next = current->next_ptr;
        // Вставляем current в отсортированный список
        if (sorted == nullptr || current->name < sorted->name) {
            // Вставляем в начало
            cout << "\rОтсортировано: " << setprecision(2) << fixed
                << (float)number_dop / num_depo * 100 << "%" << flush;
            number_dop++;
            current->next_ptr = sorted;
            sorted = current;
        }
        else {
            // Ищем место для вставки
            Depolnenie* temp = sorted;
            while (temp->next_ptr != nullptr && temp->next_ptr->name < current->name) {
                temp = temp->next_ptr;
            }
            current->next_ptr = temp->next_ptr;
            temp->next_ptr = current;
            cout << "\rОтсортировано: " << setprecision(2) << fixed
                << (float)number_dop / num_depo * 100 << "%" << flush;
            number_dop++;
        }
        current = next;
    }
    main_dep = sorted;
}

// Функция очистки дополнений
void FreeDep(Depolnenie* dep) {
    Depolnenie* loc_dep;
    // Удаление корневого указателя
    if (dep == main_dep)
    {
        main_dep = dep->next_ptr;
        dep->next_ptr = nullptr;
        delete dep;
    }
    // Удаление некорневого
    else
    {
        loc_dep = main_dep;
        while (loc_dep->next_ptr != dep)
        {
            loc_dep = loc_dep->next_ptr;
        }
        loc_dep->next_ptr = dep->next_ptr;
        dep->next_ptr = nullptr;
        delete dep;
    }
}

// Функция добавления статистики
WordStat* AddStat(Depolnenie* cur_dep)
{
    WordStat* ins_stat;
    WordStat* ptr;
    ins_stat = new WordStat;
    ins_stat->word = cur_dep->name;
    ins_stat->next_ptr = nullptr;
    // Добавление в корень
    if (main_stat != nullptr)
    {
        ptr = main_stat;
        while (ptr->next_ptr != nullptr)
        {
            ptr = ptr->next_ptr;
        }
        ptr->next_ptr = ins_stat;
    }
    // Добавление не в корень
    else
        main_stat = ins_stat;
    return ins_stat;
}

// Функция подсчета статистики
void Statistic()
{
    Depolnenie* cur_dep = main_dep;
    Depolnenie* loc_dep;
    Depolnenie* locloc_dep;
    WordStat* cur_stat;
    int count_repeat = 0;
    int number_dop = 0;
    while (cur_dep != nullptr)
    {
        // Подсчет номера предложения
        cur_stat = AddStat(cur_dep);
        cur_stat->count = 1;
        cur_stat->sentence_numbers.push_back(cur_dep->num_sent);
        loc_dep = cur_dep->next_ptr;
        while (loc_dep != nullptr)
        {
            // Поиск совпадений по дополнению
            if (cur_dep->name == loc_dep->name)
            {
                // Счетчик совпадений
                cur_stat->count++;
                cur_stat->sentence_numbers.push_back(loc_dep->num_sent);
                locloc_dep = loc_dep;
                loc_dep = loc_dep->next_ptr;
                FreeDep(locloc_dep);
                number_dop++;
            }
            else
                loc_dep = loc_dep->next_ptr;
        }
        cur_dep = cur_dep->next_ptr;
        // Отображение процесса подсчета
        cout << "\rПодсчет статистики: " << setprecision(2) << fixed
            << (float)number_dop / num_depo * 100 << "%" << flush;
        number_dop++;
    }
}

// Функция добавления дополнений
void AddDepolnenie(Ending* word, int num)
{
    num_depo++;
    Depolnenie* ins_dep;
    Depolnenie* ptr;
    ins_dep = new Depolnenie;
    ins_dep->name = word->name;
    ins_dep->num_sent = num;
    ins_dep->next_ptr = nullptr;
    // Для корневого узла
    if (main_dep != nullptr)
    {
        ptr = main_dep;
        while (ptr->next_ptr != nullptr)
        {
            ptr = ptr->next_ptr;
        }
        ptr->next_ptr = ins_dep;
    }
    // Для некорневого узла
    else
        main_dep = ins_dep;
}

// Функция проверки дополнений
void CheckDepolnenie(int num)
{
    if (word_to_ending == nullptr) return;
    Ending* cur_word = word_to_ending;
    bool pred_dop = false;
    bool cur_dop = false;
    //cout << num << endl;
    while (cur_word != nullptr)
    {
        // Если местоимение или существительное и перед стоит предлог
        if ((((cur_word->prt_of_spch & 0b10000000) == 0b10000000) || ((cur_word->prt_of_spch & 0b00010000) == 0b00010000) || (cur_word->prt_of_spch == 0b00001000)) && cur_word->padezh != "ип")
        {
            if (cur_word->pred_ptr != nullptr)
            {
                if (cur_word->pred_ptr->prt_of_spch == 0b00000001 && cur_word->pred_ptr->padezh == "доп")
                {
                    // Если есть вероятность, что наречие - пропускаем
                    if ((cur_word->prt_of_spch & 0b00000100) == 0b00000100) {
                        cur_word = cur_word->next_ptr;
                        pred_dop = false;
                        continue;
                    }
                    cur_word->name = cur_word->pred_ptr->name + " " + cur_word->name;
                    //cout << cur_word->name << " " << "После предлога\n";
                    AddDepolnenie(cur_word, num);
                    pred_dop = true;
                    cur_word = cur_word->next_ptr;
                    continue;
                }
            }
        }
        // Существительные после прилагательного
        if ((((cur_word->prt_of_spch & 0b10000000) == 0b10000000) && cur_word->padezh == "доп")) {
            // Если дальше идет глагол - значит подлежащее
            if (cur_dop == true || (cur_word->next_ptr != nullptr && cur_word->next_ptr->prt_of_spch == 0b01000000)) {
                cur_word = cur_word->next_ptr;
                pred_dop = false;
                continue;
            }
            // Если раньше идет обстоятельный предлог - пропускаем
            if (cur_word->pred_ptr != nullptr && cur_word->pred_ptr->prt_of_spch == 0b00000001 && cur_word->pred_ptr->padezh == "обст") {
                cur_word = cur_word->next_ptr;
                pred_dop = false;
                continue;
            }
            //cout << cur_word->name << " " << "После прил\n";
            AddDepolnenie(cur_word, num);
            pred_dop = true;
            cur_word = cur_word->next_ptr;
            continue;
        }
        // Неимнительные местоимения
        if ((cur_word->prt_of_spch == 0b00001000) && cur_word->padezh != "ип") {
            //cout << cur_word->name << " " << "мест\n";
            AddDepolnenie(cur_word, num);
            pred_dop = true;
            cur_word = cur_word->next_ptr;
            continue;
        }
        // После глагола
        if (((cur_word->prt_of_spch & 0b01000000) == 0b01000000) && cur_word->next_ptr != nullptr && ((((cur_word->next_ptr->prt_of_spch & 0b10000000) == 0b10000000) || (cur_word->next_ptr->prt_of_spch == 0b00001000)) && cur_word->next_ptr->padezh != "ип") && (cur_word->prt_of_spch != 0b11111111))
        {
            // Если есть вероятность, что наречие или предлог - пропускаем
            if (((cur_word->next_ptr->prt_of_spch & 0b00000100) == 0b00000100) || (cur_word->next_ptr->prt_of_spch == 0b00000001)) {
                cur_word = cur_word->next_ptr;
                pred_dop = false;
                continue;
            }
            //cout << cur_word->next_ptr->name << " " << "После глагола\n";
            AddDepolnenie(cur_word->next_ptr, num);
            cur_word = cur_word->next_ptr;  //пропустить слово, чтоб 2 раза не записывать
            pred_dop = true;
        }
        // После числительного
        else if ((cur_word->prt_of_spch == 0b00010000) && (cur_word->padezh == "нип") && (cur_word->next_ptr != nullptr) && (((cur_word->next_ptr->prt_of_spch & 0b10000000) == 0b10000000) || (cur_word->next_ptr->prt_of_spch == 0b00001000)) && (cur_word->next_ptr->padezh != "ип"))
        {
            //// Если дальше идет глагол - значит подлежащее
            //if (cur_word->next_ptr->next_ptr != nullptr && cur_word->next_ptr->next_ptr->prt_of_spch == 0b01000000) {
            //    cur_word = cur_word->next_ptr;
            //    pred_dop = false;
            //    continue;
            //}
            cur_word->name = cur_word->name + " " + cur_word->next_ptr->name;
            //cout << cur_word->name << " " << "После числительного\n";
            AddDepolnenie(cur_word, num);
            cur_word = cur_word->next_ptr;
            pred_dop = true;
        }
        // После деепричастия
        else if (((cur_word->prt_of_spch & 0b00000010) == 0b00000010) && (cur_word->next_ptr != nullptr) && (((cur_word->next_ptr->prt_of_spch & 0b10000000) == 0b10000000) || ((cur_word->next_ptr->prt_of_spch == 0b00001000))) && (cur_word->next_ptr->padezh != "ип") && (cur_word->prt_of_spch != 0b11111111))
        {
            //cout << cur_word->next_ptr->name << " " << "После деепр\n";
            AddDepolnenie(cur_word->next_ptr, num);
            cur_word = cur_word->next_ptr;  //пропустить слово, чтоб 2 раза не записывать
            pred_dop = true;
        }
        // После дополнения
        else if ((((cur_word->prt_of_spch == 0b00001000) && (cur_word->padezh != "ип")) || ((cur_word->prt_of_spch & 0b10000000) == 0b10000000)) && pred_dop && !cur_dop)
        {
            // Если есть вероятность, что наречие или предлог - пропускаем
            if (((cur_word->prt_of_spch & 0b00000100) == 0b00000100) || (cur_word->prt_of_spch == 0b00000001)) {
                cur_word = cur_word->next_ptr;
                pred_dop = false;
                continue;
            }
            //cout << cur_word->name << " " << "После доп\n";
            AddDepolnenie(cur_word, num);
        }
        else
        {
            pred_dop = false;
        }
        cur_dop = false;
        cur_word = cur_word->next_ptr;
    }
    //cout << endl;
}

// Функция добавления окончаний
void add_ending(Ending*& cur_ptr, string name, int prt_of_spch, string face, string number, string time, string padezh)
{
    Ending* ins_word;
    Ending* ptr;
    ins_word = new Ending;
    // Обновление всех полей
    ins_word->name = name;
    ins_word->prt_of_spch |= prt_of_spch;
    if (face != "-")
        ins_word->face = face;
    if (number != "-")
        ins_word->number = number;
    if (time != "-")
        ins_word->time = time;
    if (padezh != "-")
        ins_word->padezh = padezh;
    // Добавление в конец
    ins_word->pred_ptr = nullptr;
    ins_word->next_ptr = nullptr;
    if (cur_ptr != nullptr)
    {
        ptr = cur_ptr;
        while (ptr->next_ptr != nullptr)
        {
            ptr = ptr->next_ptr;
        }
        ptr->next_ptr = ins_word;
        ins_word->pred_ptr = ptr;
    }
    else
        cur_ptr = ins_word;
}

// Функция перевода в нижний регистр
string to_lower_case(string str) {
    string result = str;

    for (char& c : result) {
        // Русские заглавные буквы
        if (c >= 'А' && c <= 'Я') {
            c = c + 32; // Преобразуем в строчные
        }
        else if (c == 'Ё') {
            c = 'ё';
        }
        // Английские и другие символы
        else if (c >= 'A' && c <= 'Z') {
            c = c + 32;
        }
    }
    return result;
}

// Функция проверки окончаний
void check_endings()
{
    if (main_ptr == nullptr) return;
    STWord* word_ptr = main_ptr;
    Ending* cur_end = main_ending_ptr;
    while (word_ptr != nullptr)
    {
        bool found_spch = false;
        int max_size = 0;
        while (cur_end != nullptr)
        {
            // Пропускаем если окончание больше слова
            if ((word_ptr->word).length() < (cur_end->name).length())
            {
                cur_end = cur_end->next_ptr;
                continue;
            }
            int cur_size = 0;
            cur_size = (cur_end->name).length();
            word_ptr->word = to_lower_case(word_ptr->word);
            // Находим совпадения по окончанию слова
            if (word_ptr->word.substr((word_ptr->word).length() - (cur_end->name).length()) == cur_end->name)
            {
                // Учитываем только совпадения по наибольшим окончаниям
                if (max_size <= cur_size) {
                    // Первое совпадение - добавляем в структуру
                    if (!found_spch) {
                        add_ending(word_to_ending, word_ptr->word, cur_end->prt_of_spch, cur_end->face, cur_end->number, cur_end->time, cur_end->padezh);
                        found_spch = true;
                        max_size = cur_size;
                    }
                    // Не первое - меняем
                    else {
                        Ending* ptr;
                        ptr = word_to_ending;
                        while (ptr->next_ptr != nullptr)
                        {
                            ptr = ptr->next_ptr;
                        }
                        if (max_size == cur_size)
                            ptr->prt_of_spch |= cur_end->prt_of_spch;
                        else
                            ptr->prt_of_spch = cur_end->prt_of_spch;
                        if (cur_end->face != "-")
                            ptr->face = cur_end->face;
                        if (cur_end->number != "-")
                            ptr->number = cur_end->number;
                        if (cur_end->time != "-")
                            ptr->time = cur_end->time;
                        if (cur_end->padezh != "-")
                            ptr->padezh = cur_end->padezh;
                        max_size = cur_size;
                    }
                }
            }
            cur_end = cur_end->next_ptr;
        }
        // Если не нашли совпадения, отображаем равную вероятность по каждой части речи
        if (found_spch == false)
            add_ending(word_to_ending, word_ptr->word, 0b11111111, "-", "-", "-", "-");
        word_ptr = word_ptr->next_ptr;
        cur_end = main_ending_ptr;
    }
}

// Функция распознавания предлогов
void check_prep() {
    if (word_to_ending == nullptr) return;

    Ending* current = word_to_ending;

    while (current != nullptr)
    {
        Ending* next = current->next_ptr;
        bool processed = false;

        current->name = to_lower_case(current->name);

        // Проверка одиночных предлогов
        Ending* cur_prep = main_prep;
        while (cur_prep != nullptr && !processed) {
            if (current->name.length() <= cur_prep->name.length() &&
                current->name == cur_prep->name) {
                // Нашли одиночный предлог
                current->prt_of_spch = 0b00000001;
                current->face = "-";
                current->number = "-";
                current->time = "-";
                current->padezh = cur_prep->padezh;
                processed = true;
                break;
            }
            cur_prep = cur_prep->next_ptr;
        }

        // Проверка составных предлогов (только если текущий не обработан)
        if (!processed) {
            cur_prep = main_prep;
            while (cur_prep != nullptr && !processed) {
                string compound_word = "";
                Ending* compound_ptr = current;
                int i = 0;

                // Собираем составное слово из нескольких узлов
                while (compound_ptr != nullptr && i < 3) {
                    compound_word += compound_ptr->name;
                    if (compound_word == cur_prep->name) {
                        // Нашли составной предлог - помечаем все слова в последовательности
                        Ending* mark_ptr = current;
                        for (int j = 0; j <= i && mark_ptr != nullptr; j++) {
                            mark_ptr->prt_of_spch = 0b00000001;
                            mark_ptr->face = "-";
                            mark_ptr->number = "-";
                            mark_ptr->time = "-";
                            mark_ptr->padezh = cur_prep->padezh;
                            mark_ptr = mark_ptr->next_ptr;
                        }
                        processed = true;
                        break;
                    }
                    i++;
                    if (compound_ptr != nullptr) {
                        compound_ptr = compound_ptr->next_ptr;
                    }
                }
                cur_prep = cur_prep->next_ptr;
            }
        }

        current = next;
    }

    // Объединение последовательных предлогов в один узел
    current = word_to_ending;
    while (current != nullptr && current->next_ptr != nullptr)
    {
        Ending* next = current->next_ptr;

        if (current->prt_of_spch == 0b00000001 &&
            next->prt_of_spch == 0b00000001) {
            // Объединяем предлоги
            current->name = current->name + " " + next->name;

            // Удаляем следующий узел
            current->next_ptr = next->next_ptr;
            if (next->next_ptr != nullptr) {
                next->next_ptr->pred_ptr = current;
            }
            delete next;
        }
        else {
            current = current->next_ptr;
        }
    }
}

// Функция распознавания частиц, междометий и союзов
void check_particles() {
    if (word_to_ending == nullptr) return;

    Ending* current = word_to_ending;
    Ending* prev = nullptr;

    while (current != nullptr)
    {
        Ending* next = current->next_ptr;
        bool deleted = false;

        // Проверка одиночных частиц
        Ending* cur_particle = main_particles;
        while (cur_particle != nullptr && !deleted) {
            if (current->name == cur_particle->name) {
                // Удаляем текущий узел
                if (prev != nullptr) {
                    prev->next_ptr = next;
                }
                else {
                    word_to_ending = next;
                }
                if (next != nullptr) {
                    next->pred_ptr = prev;
                }
                delete current;
                deleted = true;
                break;
            }
            cur_particle = cur_particle->next_ptr;
        }

        // Проверка составных частиц (только если текущий не удален)
        if (!deleted) {
            cur_particle = main_particles;
            while (cur_particle != nullptr && !deleted) {
                string compound_word = "";
                Ending* compound_ptr = current;
                int i = 0;

                // Собираем составное слово
                while (compound_ptr != nullptr && i < 3) {
                    compound_word += compound_ptr->name;
                    if (compound_word == cur_particle->name) {
                        // Удаляем последовательность узлов
                        Ending* to_delete = current;
                        for (int j = 0; j <= i; j++) {
                            Ending* next_delete = to_delete->next_ptr;
                            if (prev != nullptr) {
                                prev->next_ptr = next_delete;
                            }
                            else {
                                word_to_ending = next_delete;
                            }
                            if (next_delete != nullptr) {
                                next_delete->pred_ptr = prev;
                            }
                            delete to_delete;
                            to_delete = next_delete;
                        }
                        deleted = true;
                        next = to_delete; // Обновляем next для продолжения
                        break;
                    }
                    i++;
                    compound_ptr = compound_ptr->next_ptr;
                }
                cur_particle = cur_particle->next_ptr;
            }
        }

        if (deleted) {
            current = next;
        }
        else {
            prev = current;
            current = next;
        }
    }
}

// Функция поиска местоимений
void check_pronoun() {
    if (word_to_ending == nullptr) return;
    Ending* word_ptr = word_to_ending;
    Ending* cur_pronoun = main_pronoun;
    while (word_ptr != nullptr)
    {
        while (cur_pronoun != nullptr)
        {
            // Если нашли совпадение - добавляем
            if (word_ptr->name == cur_pronoun->name) {
                word_ptr->prt_of_spch = 0b00001000;
                word_ptr->face = "-";
                word_ptr->number = "-";
                word_ptr->time = "-";
                word_ptr->padezh = cur_pronoun->padezh;
                break;
            }
            cur_pronoun = cur_pronoun->next_ptr;
        }
        cur_pronoun = main_pronoun;
        word_ptr = word_ptr->next_ptr;
    }
}

// Функция поиска числительных
void check_numaral() {
    if (word_to_ending == nullptr) return;
    Ending* word_ptr = word_to_ending;
    Ending* cur_numeral = main_numeral;
    while (word_ptr != nullptr)
    {
        // Если нашли - добавляем
        while (cur_numeral != nullptr)
        {
            if (word_ptr->name == cur_numeral->name) {
                word_ptr->prt_of_spch = 0b00010000;
                word_ptr->face = "-";
                word_ptr->number = "-";
                word_ptr->time = "-";
                word_ptr->padezh = cur_numeral->padezh;
                break;
            }
            cur_numeral = cur_numeral->next_ptr;
        }
        cur_numeral = main_numeral;
        word_ptr = word_ptr->next_ptr;
    }
}

// Функция удаления прилагательных
void delete_adj() {
    if (word_to_ending == nullptr) return;
    Ending* current = word_to_ending;
    Ending* to_delete = nullptr;
    while (current != nullptr) {
        Ending* next = current->next_ptr;
        // Если нашли однозначно прилагательное - удаляем
        if (current->prt_of_spch == 0b00100000) {
            to_delete = current;
            if (current->next_ptr != nullptr && (((current->next_ptr->prt_of_spch & 0b10000000) == 0b10000000) || (current->next_ptr->prt_of_spch == 0b00001000)))
                current->next_ptr->padezh = "доп";
            // Обновляем связи
            if (current->pred_ptr != nullptr) {
                current->pred_ptr->next_ptr = current->next_ptr;
            }
            else {
                word_to_ending = current->next_ptr;
            }

            if (current->next_ptr != nullptr) {
                current->next_ptr->pred_ptr = current->pred_ptr;
            }
            delete to_delete;
        }
        current = next;
    }
}

// Функция вывода
void Output()
{
    if (main_ptr == nullptr) return;
    STWord* ins_word;
    ins_word = main_ptr;
    Depolnenie* ins_dep = main_dep;
    // Вывод слов предложения
    while (ins_word != nullptr)
    {
        cout << ins_word->word << endl;
        ins_word = ins_word->next_ptr;
    }
    cout << endl << endl;
    // Вывод дополнений
    if (main_dep != nullptr)
    {
        cout << "Дополнения\n";
        while (ins_dep != nullptr)
        {
            cout << ins_dep->name << endl;
            ins_dep = ins_dep->next_ptr;
        }
        cout << endl << endl;
    }
    else
        cout << "Дополнения\n-\n\n";
}

// Функция удаления структуры текста
void free_struct_stword()
{
    STWord* ins_word;
    STWord* loc_ptr;
    ins_word = main_ptr;
    while (ins_word != nullptr)
    {
        loc_ptr = ins_word;
        ins_word = ins_word->next_ptr;
        loc_ptr->pred_ptr = nullptr;
        loc_ptr->next_ptr = nullptr;
        delete loc_ptr;
    }
    main_ptr = nullptr;
}

// Функция удаления структуры статистики дополнений
void free_struct_statistic()
{
    WordStat* ins_word;
    WordStat* loc_ptr;
    ins_word = main_stat;
    while (ins_word != nullptr)
    {
        loc_ptr = ins_word;
        ins_word = ins_word->next_ptr;
        loc_ptr->next_ptr = nullptr;
        delete loc_ptr;
    }
    main_stat = nullptr;
}

// Функция удаления структуры дополнений
void free_struct_depolnenie()
{
    Depolnenie* ins_word;
    Depolnenie* loc_ptr;
    ins_word = main_dep;
    while (ins_word != nullptr)
    {
        loc_ptr = ins_word;
        ins_word = ins_word->next_ptr;
        loc_ptr->next_ptr = nullptr;
        delete loc_ptr;
    }
    main_dep = nullptr;
}

// Функция удаления структуры окончаний
void free_struct_endings(Ending*& cur_ptr)
{
    Ending* ins_word;
    Ending* loc_ptr;
    ins_word = cur_ptr;
    while (ins_word != nullptr)
    {
        loc_ptr = ins_word;
        ins_word = ins_word->next_ptr;
        loc_ptr->pred_ptr = nullptr;
        loc_ptr->next_ptr = nullptr;
        delete loc_ptr;
    }
    cur_ptr = nullptr;
}

// Функция загрузки слова в структуру
void LoadWord(string word)
{
    STWord* ins_word;
    STWord* ptr;
    ins_word = new STWord;
    ins_word->word = word;
    ins_word->pred_ptr = nullptr;
    ins_word->next_ptr = nullptr;
    // Если корень непустой
    if (main_ptr != nullptr)
    {
        ptr = main_ptr;
        while (ptr->next_ptr != nullptr)
        {
            ptr = ptr->next_ptr;
        }
        ptr->next_ptr = ins_word;
        ins_word->pred_ptr = ptr;
    }
    // Если пустой
    else
        main_ptr = ins_word;
}

// Функция вывода окончаний
void out_ending()
{
    if (word_to_ending == nullptr) return;
    ofstream out;
    if (test_file_open == false) {
        out.open("test.txt", ios::out);
        test_file_open = true;
    }
    else
        out.open("test.txt", ios::app);
    if (out.is_open())
    {
        Ending* ins_word;
        ins_word = word_to_ending;
        while (ins_word != nullptr)
        {
            out << ins_word->name << " " << bitset<8>(ins_word->prt_of_spch) << " " << ins_word->face << " " << ins_word->number << " " << ins_word->time << " " << ins_word->padezh << endl;
            ins_word = ins_word->next_ptr;
        }
    }
    out.close();
}

// Функция проверки, состоит ли строка только из пробелов
bool only_space(string word) {
    for (int i = 0; i < word.length(); i++) {
        if (word[i] != ' ')
            return 0;
    }
    return 1;
}

// Проверяет, является ли символ буквой (русской/английской) или разрешённым символом
bool is_alphabetic(char symbol) {
    if (('А' <= symbol and symbol <= 'я') or symbol == 'ё' or symbol == 'Ё' or
        symbol == '-')
        return 1;
    else
        return 0;
}

// Функция обработки текста по предложениям
void Analize_text(string filename) {
    // Открытие файла на чтение
    free_struct_depolnenie();
    free_struct_statistic();
    int num_sent = 0;
    cout << "\nПодождите, текст анализируется...\n";
    ifstream fin(filename);
    if (!fin.is_open()) {
        cout << "Ошибка открытия файла!\n";
        system("pause");
        return;
    }
    ofstream fout(filename.erase(filename.find(".txt")) + " (с нумерацией).txt");
    if (!fout.is_open()) {
        cout << "Ошибка открытия файла!\n";
        system("pause");
        return;
    }
    string line;
    while (getline(fin, line)) {
        // Заменяем все лишние знаки препинания на пробелы
        istringstream iss(line);
        string word;
        while (iss >> word) {
            //for (char& c : word) {
            //    if ((ispunct(c) || c == '\"' || c == '\'') && c != '.' && c != '?' && c != '!') {
            //        c = ' ';
            //    }
            //}
            // Проверяем, заканчивается ли слово на .!?
            if (!word.empty() && (word.back() == '.' || word.back() == '?' || word.back() == '!')) {
                // Убираем знак препинания
                //string clean_word = word.substr(0, word.length() - 1);
                //if (word.back() == '.' && word[word.length() - 2] == '.')
                //    clean_word = word.substr(0, word.length() - 3);

                ////Добавляем слово только если оно не пустое
                //if (!clean_word.empty()) {
                    num_sent++;
                //}
            }
        }
    }
    fin.clear();
    fin.seekg(0, ios::beg);
    int sentence_number = 1;
    bool has_words_in_sentence = false;
    string sentance = "";
    while (getline(fin, line)) {
        // Заменяем все лишние знаки препинания на пробелы
        istringstream iss(line);
        string word = "";
        while (iss >> word) {
            if (sentance == "")
                sentance = word;
            else
                sentance = sentance + " " + word;
            //for (char& c : word) {
            //    if ((ispunct(c) || c == '\"' || c == '\'') && c != '.' && c != '?' && c != '!') {
            //        c = ' ';
            //    }
            //}
            // Проверяем, заканчивается ли слово на .!?
            if (!word.empty() && (word.back() == '.' || word.back() == '?' || word.back() == '!')) {
                //// Убираем знак препинания
                //string clean_word = word.substr(0, word.length() - 1);
                //// Проверяем на многоточие
                //if (word.size() >= 3 && word[word.size() - 1] == '.' && word[word.size() - 2] == '.' && word[word.size() - 3] == '.') {
                //    clean_word = word.substr(0, word.length() - 3);
                //}
                for (int i = 0; i < word.length(); i++) {
                    if (!is_alphabetic(word[i]) || ((i == word.length()-1) && word[i] == '-')) {
                        word.erase(i, 1);
                        i--;
                    }
                }   
                
                // Добавляем слово только если оно не пустое
                if (!word.empty() && word[0]) {
                    LoadWord(word);
                    has_words_in_sentence = true;
                }
                // Обрабатываем предложение только если в нем были слова
                if (has_words_in_sentence) {
                    fout << sentence_number << "." << sentance << "\n";
                    sentance = "";
                    check_endings();
                    check_particles();
                    check_prep();
                    check_pronoun();
                    check_numaral();
                    //out_ending();
                    delete_adj();
                    CheckDepolnenie(sentence_number);
                    //Output();
                    free_struct_stword();
                    free_struct_endings(word_to_ending);
                    cout << "\rПроанализировано: " << setprecision(2) << fixed
                       << (float)sentence_number / num_sent * 100 << "%" << flush;
                    sentence_number++;
                    has_words_in_sentence = false;
                }
            }
            else {
                // Обычное слово без знаков конца предложения
                for (int i = 0; i < word.length(); i++) {
                    if (!is_alphabetic(word[i]) || ((i == word.length() - 1) && word[i] == '-')) {
                        word.erase(i, 1);
                        i--;
                    }
                }
                if (!word.empty() && word[0]) {
                    LoadWord(word);
                }
                has_words_in_sentence = true;
            }
        }
    }
    // Обработка последнего предложения если оно осталось
    if (has_words_in_sentence) {
        fout << sentence_number << "." << sentance << " ";
        sentance = "";
        check_endings();
        check_particles();
        check_prep();
        check_pronoun();
        check_numaral();
        //out_ending();
        delete_adj();
        CheckDepolnenie(sentence_number);
        //Output();
        free_struct_stword();
        free_struct_endings(word_to_ending);
    }
    fin.close();
    fout.close();
    test_file_open = false;
    cout << "\r";
    cout << "Текст успешно проанализирован.                    \n\n";
    system("pause");
}

// Функция считывания правил окончаний
bool parser_endings() {
    string line;
    ifstream endings("Окончания.txt");
    if (endings.is_open()) {
        while (getline(endings, line)) {
            // Для прилагательных
            if (line == "прил:") {
                while (line != "/") {
                    getline(endings, line);
                    if (line == "/")
                        break;
                    add_ending(main_ending_ptr, line, 0b00100000, "-", "-", "-", "-");
                }
            }
            // Для глаголов
            if (line == "гл:") {
                string word;
                endings >> word;
                string list[9];
                while (word != "/") {
                    int i = 0;
                    while (word != ";") {
                        list[i] = word;
                        endings >> word;
                        i++;
                    }
                    string face, number, time;
                    endings >> face;
                    endings >> number;
                    endings >> time;
                    for (int j = 0; j < i; j++) {
                        add_ending(main_ending_ptr, list[j], 0b01000000, face, number, time, "-");
                    }
                    endings >> word;
                }
            }
            // Для деепричастий
            if (line == "деепр:") {
                string word;
                endings >> word;
                string list[5];
                while (word != "/") {
                    int i = 0;
                    while (word != ";") {
                        list[i] = word;
                        endings >> word;
                        i++;
                    }
                    string time;
                    endings >> time;
                    for (int j = 0; j < i; j++) {
                        add_ending(main_ending_ptr, list[j], 0b00000010, "-", "-", time, "-");
                    }
                    endings >> word;
                }
            }
            // Для наречий
            if (line == "нар:") {
                while (line != "/") {
                    getline(endings, line);
                    if (line == "/")
                        break;
                    add_ending(main_ending_ptr, line, 0b00000100, "-", "-", "-", "-");
                }
            }
            // Для существительных
            if (line == "сущ:") {
                while (line != "/") {
                    getline(endings, line);
                    if (line == "/")
                        break;
                    add_ending(main_ending_ptr, line, 0b10000000, "-", "-", "-", "-");
                }
            }
        }
    }
    else
        return 0;
    endings.close();     // закрываем файл
    return 1;
}

// Функция считывания правил предлогов
bool parser_prep() {
    string line;
    ifstream prep("Предлоги.txt");
    if (prep.is_open()) {
        while (getline(prep, line)) {
            string member;
            getline(prep, member);
            add_ending(main_prep, line, 0b00000001, "-", "-", "-", member);
        }
    }
    else
        return 0;
    prep.close();
    return 1;
}

// Функция считывания правил союзов, частиц и междометий
bool parser_particles() {
    string line;
    ifstream particle("Союзы, частицы, междометия.txt");
    if (particle.is_open()) {
        while (getline(particle, line)) {
            add_ending(main_particles, line, 0b00000000, "-", "-", "-", "-");
        }
    }
    else
        return 0;
    particle.close();
    return 1;
}

// Функция считывания местоимений
bool parser_pronoun() {
    string line;
    ifstream pronoun("Местоимения.txt");
    if (pronoun.is_open()) {
        while (getline(pronoun, line)) {
            string prt_of_spch;
            getline(pronoun, prt_of_spch);
            add_ending(main_pronoun, line, 0b00001000, "-", "-", "-", prt_of_spch);
        }
    }
    else
        return 0;
    pronoun.close();
    return 1;
}

// Функция считывания числительных
bool parser_numeral() {
    string line;
    ifstream numeral("Числительные.txt");
    if (numeral.is_open()) {
        while (getline(numeral, line)) {
            string prt_of_spch;
            getline(numeral, prt_of_spch);
            add_ending(main_numeral, line, 0b00010000, "-", "-", "-", prt_of_spch);
        }
    }
    else
        return 0;
    numeral.close();
    return 1;
}

// Функция выбора файлов
int file_choose() {
    int position = 0;
    while (position > -1) {
        string* clean_file_names;
        string* file_names;
        int num_clean_files = 0;
        int num_files = 0;
        // Получаем список txt-файлов в директории программы
        string program_dir = get_program_dir();
        file_names = list_files(program_dir, num_files);
        for (int i = 0; i < num_files; i++) {
            if (file_names[i] == "Числительные.txt" || file_names[i] == "Местоимения.txt" ||
                file_names[i] == "Окончания.txt" || file_names[i] == "Предлоги.txt" ||
                file_names[i] == "Союзы, частицы, междометия.txt" || file_names[i].find("(с нумерацией).txt") != string::npos) {
                continue;
            }
            num_clean_files++;
        }

        if (num_clean_files == 0) {
            cout << "\nВ папке нет файлов. Добавьте нужные файлы и вернитесь в это меню.\n\n";
            system("pause");
            free_struct_depolnenie();
            free_struct_statistic();
            return -1;
        }

        clean_file_names = new string[num_clean_files];
        int j = 0;
        for (int i = 0; i < num_files; i++) {
            if (file_names[i] == "Числительные.txt" || file_names[i] == "Местоимения.txt" ||
                file_names[i] == "Окончания.txt" || file_names[i] == "Предлоги.txt" ||
                file_names[i] == "Союзы, частицы, междометия.txt" || file_names[i].find("(с нумерацией).txt") != string::npos) {
                j++;
                continue;
            }
            clean_file_names[i-j] = file_names[i];
        }
        // Пользователь выбирает файл
        dynamic_menu(position, clean_file_names, num_clean_files, main_menu_text);
        if (position < 0) {
            free_struct_depolnenie();
            free_struct_statistic();
            return position;
        }

        // Формируем полный путь к файлу
        program_dir = program_dir + "\\" + clean_file_names[position];
        ifstream fin(program_dir);

        if (!fin.is_open()) {  // Если не удалось открыть файл
            cout << "\nНе удалость открыть файл. Повторите еще раз или используйте дугой.\n\n";
            system("pause");
            continue;
        }
        Analize_text(program_dir);
        return 0;
    }
}

// Функция проверки на специальные символы в имени файла
// 0 - нет спецсимволов; 1 - есть
bool check_special_sym(char symbol) {
    return (symbol == '\\' or symbol == '/' or symbol == ':' or symbol == '*' or symbol == '?' or symbol == '\"' or symbol == '<' or symbol == '>' or symbol == '|');
}

// Функция обработки backspace (удаление последнего символа)
void backspace(string& string) {
    if (string == "") return; // Если строка пустая, игнорируем backspace
    printf("\b \b");  // Удаление символа из консоли
    string.erase((string.length()) - 1, 1);  // Удаление символа из строки
    return;
}

// Чтение слова с клавиатуры с обработкой специальных клавиш
// 0 - ENTER; -1 - стрелка влево; -2 - ESCAPE
int read_word(string& word) {
    char symbol = 0;
    while (symbol != ENTER) {
        symbol = _getch();

        // Обработка стрелок
        if (_kbhit()) {
            char arrow = _getch();
            if (arrow == 75)  // Стрелка влево - выход
                return -1;
            if (arrow == 77) {  // Стрелка вправо - подтверждение
                break;
            }
            else
                continue;
        }
        if (symbol == TAB) continue;  // Игнорируем Tab
        if (symbol == ENTER) break;   // Enter - завершение ввода
        if (symbol == ESCAPE) return -2;  // Escape - выход
        if (check_special_sym(symbol)) {  // Игнорируем спецсимволы
            continue;
        }
        if (symbol == BSPACE) {  // Backspace - удаление символа
            backspace(word);
            continue;
        }

        cout << symbol;
        word += symbol;  // Добавляем символ к строке
    }
    return 0;
}

// Функция вывода по хронологии
int hrono_out() {
    system("cls");
    // Отображения процесса подсчета
    cout << "Подождите, идет подсчет статистики дополнений...\n";
    Statistic();
    system("cls");
    cout << menu_1_1_1_text;
    // Подсчет количества дополнений
    Depolnenie* current = main_dep;
    int count = 0;
    while (current != nullptr)
    {
        count++;
        current = current->next_ptr;
    }
    // Открытие файла для вывода
    string filename;
    bool wrong_entry = false;
    ofstream file;
    while (!wrong_entry) {
        wrong_entry = true;
        int result = read_word(filename);
        if (result < 0)
            return result;
        // Проверка длины имени файла
        if (filename.length() >= 255) {
            cout << "Название слишком большое. Введите более короткое.\n";
            filename.clear();
            wrong_entry = false;
            continue;
        }
        // Проверка на пустое имя
        if (filename.length() == 0) {
            cout << "Название не может быть пустым. Введите другое.\n";
            filename.clear();
            wrong_entry = false;
            continue;
        }
        // Проверка на имя из пробелов
        if (only_space(filename)) {
            cout << "\nНазвание не может состоять только из пробелов. Введите другое.\n";
            filename.clear();
            wrong_entry = false;
            continue;
        }
        // Добавляем расширение .txt, если его нет
        if (filename.find(".txt") == filename.npos) filename += ".txt";

        // Пытаемся открыть файл
        file.open(filename);
        if (!file.is_open()) {
            cout << "Не удалость открыть или создать файл. Попробуйте еще раз.\n";
            filename.clear();
            wrong_entry = false;
            continue;
        }
    }
    file << "Текст из файла:" << filename_text << endl;
    file << "=================================\n";
    // Для вывода по хронологии
    if (choice == 0)
    {
        file << "Дополнения в хронологии с текстом\n";
    }
    // Для вывода по алфавиту
    if (choice == 1)
    {
        file << "Дополнения, отсортированные по алфавиту\n";
    }

    file << "=================================\n";

    // Цикл вывода в файл
    WordStat* cur_stat = main_stat;
    while (cur_stat != nullptr)
    {
        file << cur_stat->word << " встречается " << cur_stat->count << " раз(а) в следующих предложениях: ";
        // По хронологии
        if (choice == 0)
        {
            int num_common = 1;
            bool is_first = true;

            for (int i = 0; i < cur_stat->sentence_numbers.size(); i++)
            {
                // Проверяем, не последний ли это элемент и совпадает ли со следующим
                if (i + 1 < cur_stat->sentence_numbers.size() &&
                    cur_stat->sentence_numbers[i] == cur_stat->sentence_numbers[i + 1]) {
                    num_common++;
                    continue;
                }

                // Выводим разделитель (кроме первого элемента)
                if (!is_first) {
                    file << ", ";
                }
                is_first = false;

                // Выводим число
                file << cur_stat->sentence_numbers[i];

                // Выводим количество повторений в скобках, если больше 1
                if (num_common > 1) {
                    file << "[" << num_common << "]";
                }

                // Сбрасываем счетчик для следующего числа
                num_common = 1;
            }
        }
        // По алфавиту
        if (choice == 1)
        {
            int num_common = 1;
            bool is_first = true;

            // Начинаем с последнего элемента
            for (int i = cur_stat->sentence_numbers.size() - 1; i >= 0; i--)
            {
                // Проверяем, не первый ли это элемент и совпадает ли с предыдущим (в обратном порядке)
                if (i - 1 >= 0 && cur_stat->sentence_numbers[i] == cur_stat->sentence_numbers[i - 1]) {
                    num_common++;
                    continue;
                }

                // Выводим разделитель (кроме первого выводимого элемента)
                if (!is_first) {
                    file << ", ";
                }
                is_first = false;

                // Выводим число
                file << cur_stat->sentence_numbers[i];

                // Выводим количество повторений в скобках, если больше 1
                if (num_common > 1) {
                    file << "[" << num_common << "]";
                }

                // Сбрасываем счетчик для следующего числа
                num_common = 1;
            }
        }
        file << endl;
        cur_stat = cur_stat->next_ptr;
    }

    file << "==========================\n";
    file << "Всего: " << count << " дополнений\n";

    file.close();
    cout << "\nРезультат записан в " << filename << endl;
    system("pause");
    system("cls");
    // Очистка ненужных структур
    free_struct_depolnenie();
    free_struct_statistic();
    num_depo = 0;
    return -2;
}

// Функция вывода в отсортированном формате
int sorted_out() {
    // Изменение флага типа вывода
    choice = 1;
    system("cls");
    cout << "Подождите, идет сортировка дополнений...\n";
    // Сортировка
    SortDepolnenie();
    system("cls");
    // Аналогичный вывод
    int result = hrono_out();
    choice = 0;
    return result;
}

int main()
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    // Добавление всех правил
    while (!parser_endings()) {
        cout << "Файл окончаний не найден, добавьте его в папку с программой, иначе невозможно продолжить работу программы.\n\n";
        system("pause");
        system("cls");
    }
    while (!parser_prep()) {
        cout << "Файл предлогов не найден, добавьте его в папку с программой, иначе невозможно продолжить работу программы.\n\n";
        system("pause");
        system("cls");
    }
    while (!parser_particles()) {
        cout << "Файл союзов, частиц и междометий не найден, добавьте его в папку с программой, иначе невозможно продолжить работу программы.\n\n";
        system("pause");
        system("cls");
    }
    while (!parser_pronoun()) {
        cout << "Файл местоимений не найден, добавьте его в папку с программой, иначе невозможно продолжить работу программы.\n\n";
        system("pause");
        system("cls");
    }
    while (!parser_numeral()) {
        cout << "Файл числительных не найдены, добавьте их в папку с программой, иначе невозможно продолжить работу программы.\n\n";
        system("pause");
        system("cls");
    }
    // Создаем главное меню
    root = create_node("main_menu", intro, nullptr, 0, nullptr);
    add_node("main_menu", "1", "", nullptr, 0, file_choose);
    add_node("1", "1.1", menu_1_1_text, menu_1_1_list, size(menu_1_1_list), nullptr);
    add_node("1.1", "1.1.1", menu_1_1_1_text, nullptr, 0, hrono_out);
    add_node("1.1", "1.1.2", menu_1_1_1_text, nullptr, 0, sorted_out);
    cycle();
    return 0;
}
