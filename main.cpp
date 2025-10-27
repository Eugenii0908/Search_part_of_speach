#include <iostream>
#include <fstream>
#include <conio.h>
#include <windows.h>
#include <sstream>
#include <string>
#include <bitset>

using namespace std;

// Определение констант для клавиш
const char ESCAPE = 27;   // Клавиша ESC
const char ENTER = 13;    // Клавиша Enter
const char BSPACE = 8;    // Клавиша Backspace

const string intro = "Это программа для распознавания дополнений в тексте.\n";
const string menu_choose_file = "Выберете файл, из которого хотите считать. Предварительно файл должен быть загружен в папку с программой.\n-Для перемещения по пунктам используйте стрелки вверх/вниз\n-Для подтверждения выбора нажмите ENTER или стрелку вправо\n-Для выхода в главное меню нажмите ESCAPE или на стрелку влево\n\n";

struct STWord
{
    string word;
    STWord* pred_ptr;
    STWord* next_ptr;
};

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

struct Depolnenie
{
    string name;
    Depolnenie *next_ptr;
};

STWord* main_ptr = nullptr;
Ending* main_ending_ptr = nullptr;
Ending* word_to_ending = nullptr;
Ending* main_prep = nullptr;
Ending* main_particles = nullptr;
Ending* main_pronoun = nullptr;
Ending* main_numeral = nullptr;
Depolnenie* main_dep;


bool test_file_open = false;

void AddDepolnenie(Ending *word)
{
    Depolnenie *ins_dep;
    Depolnenie *ptr;
    ins_dep = new Depolnenie;
    ins_dep->name = word->name;
    ins_dep->next_ptr = nullptr;
    if (main_dep != nullptr)
    {
        ptr = main_dep;
        while (ptr->next_ptr != nullptr)
        {
            ptr = ptr->next_ptr;
        }
        ptr->next_ptr = ins_dep;
    }
    else
        main_dep = ins_dep;
}

void CheckDepolnenie()
{
    Ending *cur_word = word_to_ending;
    while(cur_word != nullptr)
    {
        if(cur_word->padezh == "нип")
        {
            //cout<<cur_word->padezh;
            if(cur_word->pred_ptr != nullptr)
            {
                if((cur_word->pred_ptr->prt_of_spch == 0b00000001 && cur_word->pred_ptr->padezh == "доп") || cur_word->pred_ptr->prt_of_spch != 0b00000001)
                {
                    AddDepolnenie(cur_word);
                }
            }
        }
        if(cur_word->prt_of_spch == 0b01000000 && cur_word->next_ptr != nullptr && cur_word->next_ptr->prt_of_spch & 0b10000000)
        {
            AddDepolnenie(cur_word->next_ptr);
            cur_word = cur_word->next_ptr;
        }
        if(cur_word->prt_of_spch == 0b00010000 && cur_word->padezh == "нип" && cur_word->next_ptr != nullptr)   //следующее слово после числительного
        {
            AddDepolnenie(cur_word->next_ptr);
            cur_word = cur_word->next_ptr;
        }
        cur_word = cur_word->next_ptr;
    }
}

void add_ending(Ending*& cur_ptr, string name, int prt_of_spch, string face, string number, string time, string padezh)
{
    Ending* ins_word;
    Ending* ptr;
    ins_word = new Ending;
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

void check_endings()
{
    /*ifstream file_ending;
    ifstream file_preposition;
    string ending;

    file_ending.open("окончания.txt");
    file_preposition.open("предлоги.txt");
*/
    STWord* word_ptr = main_ptr;
    Ending* cur_end = main_ending_ptr;
    while (word_ptr != nullptr)
    {
        bool found_spch = false;
        int max_size = 0;
        while (cur_end != nullptr)
        {
            if ((word_ptr->word).length() < (cur_end->name).length())
            {
                cur_end = cur_end->next_ptr;
                continue;
            }
            int cur_size = 0;
            cur_size = (cur_end->name).length();
            if (word_ptr->word.substr((word_ptr->word).length() - (cur_end->name).length()) == cur_end->name)
            {
                if (max_size <= cur_size) {
                    if (!found_spch) {
                        //cout << word_ptr->word << " по окончанию " << cur_end->name << endl;
                        add_ending(word_to_ending, word_ptr->word, cur_end->prt_of_spch, cur_end->face, cur_end->number, cur_end->time, cur_end->padezh);
                        found_spch = true;
                        max_size = cur_size;
                    }
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
        if (found_spch == false)
            add_ending(word_to_ending, word_ptr->word, 0b11111111, "-", "-", "-", "-");
        word_ptr = word_ptr->next_ptr;
        cur_end = main_ending_ptr;
        //file_ending.seekg(0, ios::beg);
    }
}

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

void free_node(Ending* node_to_delete) {
    if (node_to_delete == nullptr) return;

    // Обновляем связи соседних узлов
    if (node_to_delete->pred_ptr != nullptr) {
        node_to_delete->pred_ptr->next_ptr = node_to_delete->next_ptr;
    }
    if (node_to_delete->next_ptr != nullptr) {
        node_to_delete->next_ptr->pred_ptr = node_to_delete->pred_ptr;
    }

    delete node_to_delete;
}

void check_prep() {
    //Максимальное количество слов в союзе - 5
    Ending* word_ptr = word_to_ending;
    Ending* cur_prep = main_prep;
    while (word_ptr != nullptr)
    {
        word_ptr->name = to_lower_case(word_ptr->name);
        while (cur_prep != nullptr)
        {
            if ((word_ptr->name).length() > (cur_prep->name).length())
            {
                cur_prep = cur_prep->next_ptr;
                continue;
            }
            if (word_ptr->name == cur_prep->name) {
                word_ptr->prt_of_spch = 0b00000001;
                word_ptr->face = "-";
                word_ptr->number = "-";
                word_ptr->time = "-";
                word_ptr->padezh = cur_prep->padezh;
                break;
            }
            string compound_word = "";
            Ending* compound_ptr = word_ptr;
            bool is_found = false;
            int i = 0;
            while (compound_ptr != nullptr and i < 3) {
                compound_word += compound_ptr->name;
                if (compound_word == cur_prep->name) {
                    compound_ptr = word_ptr;
                    for (int j = 0; j < i + 1; j++) {
                        compound_ptr->prt_of_spch = 0b00000001;
                        compound_ptr->face = "-";
                        compound_ptr->number = "-";
                        compound_ptr->time = "-";
                        compound_ptr->padezh = cur_prep->padezh;
                        compound_ptr = compound_ptr->next_ptr;
                    }
                    word_ptr = compound_ptr;
                    is_found = true;
                    break;
                }
                i++;
                compound_ptr = compound_ptr->next_ptr;
            }
            if (is_found)
                break;
            cur_prep = cur_prep->next_ptr;
        }
        cur_prep = main_prep;
        word_ptr = word_ptr->next_ptr;
    }

    word_ptr = word_to_ending;
    while (word_ptr != nullptr)
    {
        if (word_ptr->prt_of_spch == 0b00000001 and word_ptr->next_ptr != nullptr and word_ptr->next_ptr->prt_of_spch == 0b00000001) {
            word_ptr->name = word_ptr->name + "_" + word_ptr->next_ptr->name;
            free_node(word_ptr->next_ptr);
            continue;
        }
        word_ptr = word_ptr->next_ptr;
    }
}

void check_particles() {
    //Максимальное количество слов в союзе - 5
    Ending* word_ptr = word_to_ending;
    Ending* cur_particle = main_particles;
    Ending* loc_ptr;
    while (word_ptr != nullptr)
    {
        while (cur_particle != nullptr)
        {
            if (word_ptr->name == cur_particle->name)
            {
                if(word_ptr == word_to_ending)
                    word_to_ending = word_ptr->next_ptr;
                loc_ptr = word_ptr;
                word_ptr = word_ptr->next_ptr;
                free_node(loc_ptr);
                //break;
            }
            string compound_word = "";
            Ending* compound_ptr = word_ptr;
            bool is_found = false;
            int i = 0;
            while (compound_ptr != nullptr and i < 5) {
                compound_word += compound_ptr->name;
                if (compound_word == cur_particle->name) {
                    compound_ptr = word_ptr;
                    for (int j = 0; j < i + 1; j++)
                    {
                        if(word_ptr == word_to_ending)
                            word_to_ending = word_ptr->next_ptr;
                        loc_ptr = word_ptr;
                        word_ptr = word_ptr->next_ptr;
                        free_node(word_ptr);

                    }
                    word_ptr = compound_ptr;
                    is_found = true;
                    break;
                }
                i++;
                compound_ptr = compound_ptr->next_ptr;
            }
            if (is_found)
                break;
            cur_particle = cur_particle->next_ptr;
        }
        cur_particle = main_particles;
        word_ptr = word_ptr->next_ptr;
    }
}

void check_pronoun() {
    Ending* word_ptr = word_to_ending;
    Ending* cur_pronoun = main_pronoun;
    while (word_ptr != nullptr)
    {
        while (cur_pronoun != nullptr)
        {
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

void check_numaral() {
    Ending* word_ptr = word_to_ending;
    Ending* cur_numeral = main_numeral;
    while (word_ptr != nullptr)
    {
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

void delete_adj() {
    Ending* current = word_to_ending;
    Ending* to_delete = nullptr;
    while (current != nullptr) {
        Ending* next = current->next_ptr;
        if (current->prt_of_spch == 0b00100000) {
            to_delete = current;

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

void Output()
{
    STWord* ins_word;
    ins_word = main_ptr;
    Depolnenie *ins_dep = main_dep;
    while (ins_word != nullptr)
    {
        cout << ins_word->word << endl;
        ins_word = ins_word->next_ptr;
    }
    cout << endl<<endl;
    if(main_dep != nullptr)
    {
        cout<<"Дополнения\n";
        while (ins_dep != nullptr)
        {
            cout << ins_dep->name << endl;
            ins_dep = ins_dep->next_ptr;
        }
        cout << endl<<endl;
    }

}

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

void LoadWord(string word)
{
    STWord* ins_word;
    STWord* ptr;
    ins_word = new STWord;
    ins_word->word = word;
    ins_word->pred_ptr = nullptr;
    ins_word->next_ptr = nullptr;
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
    else
        main_ptr = ins_word;
}



void out_ending()
{
    ofstream out;          // поток для записи
    if (test_file_open == false) {
        out.open("test.txt", ios::out);      // открываем файл для записи
        test_file_open = true;
    }
    else
        out.open("test.txt", ios::app);      // открываем файл для записи
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

//void DevideSentence(string filename) {
//    ifstream fin;
//    string line;
//    string keyword = "";
//    fin.open(filename, ios::ate);
//    if (!fin.is_open()) {
//        system("cls");
//        cout << "Ошибка! Файл не открыт. Выберите другой файл\n";
//        system("pause");
//        return;
//    }
//    else if (fin.tellg() == 0) {  //проверка файла на пустоту
//        system("cls");
//        fin.close();
//        cout << "Файл пуст! Выберите другой файл\n";
//        system("pause");
//        return;
//    }
//    else {
//        fin.seekg(0, ios::beg);     //перемещение указателя в начало файла
//    }
//    int i = 1;
//    cout << i << endl;
//    while (!fin.eof())
//    {
//        bool flag = false;
//        getline(fin, line);
//        if (keyword != "")
//            line = keyword + ' ' + line;
//        //cout<<line<<endl;
//        istringstream iss(line);
//        while (getline(iss, keyword, ' '))
//        {
//            bool containsNewline = (keyword.find('\n') != -1);
//            if (keyword.empty())
//                continue;
//            //cout<<sentence<<endl;
//            LoadWord(keyword);
//            if ((keyword[keyword.length() - 1] == '.') || (keyword[keyword.length() - 1] == '?') || (keyword[keyword.length() - 1] == '!'))
//            {
//                Output();
//                //Search();   //здесь запускается функция, которая будет обрабатывать предложение
//                FreeStruck();   //функция очистки структуры (предложение мы типа обработали и больше не нужно)
//                i++;
//                cout << i << endl;
//                if (!containsNewline && iss.peek() != EOF)
//                    flag = true;
//                break;
//            }
//        }
//        if (flag)
//        {
//            getline(iss, keyword);
//        }
//        else
//            keyword = "";
//    }
//    system("pause");
//}

void DevideSentence(string filename) {
    ifstream fin(filename);
    if (!fin.is_open()) {
        cout << "Ошибка открытия файла!\n";
        system("pause");
        return;
    }

    string line;
    int sentence_number = 1;
    bool has_words_in_sentence = false;

    while (getline(fin, line)) {
        // Заменяем все лишние знаки препинания на пробелы
        for (char& c : line) {
            if (ispunct(c) && c != '.' && c != '?' && c != '!') {
                c = ' ';
            }
        }

        istringstream iss(line);
        string word;

        while (iss >> word) {
            // Проверяем, заканчивается ли слово на .!?
            if (!word.empty() && (word.back() == '.' || word.back() == '?' || word.back() == '!')) {
                // Убираем знак препинания
                string clean_word = word.substr(0, word.length() - 1);

                // Добавляем слово только если оно не пустое
                if (!clean_word.empty()) {
                    LoadWord(clean_word);
                    has_words_in_sentence = true;
                }

                // Обрабатываем предложение только если в нем были слова
                if (has_words_in_sentence) {
                    check_endings();
                    check_particles();
                    check_prep();
                    check_pronoun();
                    check_numaral();
                    delete_adj();
                    CheckDepolnenie();
                    Output();
                    out_ending();
                    free_struct_stword();
                    free_struct_endings(word_to_ending);
                    cout << "Обработка предложения " << sentence_number++ << endl;
                    has_words_in_sentence = false;
                }
            }
            else {
                // Обычное слово без знаков конца предложения
                LoadWord(word);
                has_words_in_sentence = true;
            }
        }
    }

    // Обработка последнего предложения если оно осталось
    if (has_words_in_sentence) {
        check_endings();
        check_particles();
        check_prep();
        check_pronoun();
        check_numaral();
        delete_adj();
        CheckDepolnenie();
        Output();
        out_ending();
        free_struct_stword();
        free_struct_endings(word_to_ending);
        cout << "Обработка предложения " << sentence_number << endl;
    }

    fin.close();
    test_file_open = false;
    system("pause");
}

string WriteFilename() {                 //функция записи названия файла
    string filename;
    bool inputComplete = false;         //флаг завершения записи
    while (!inputComplete) {
        char ch = _getch();
        if (ch == 27)
            return "";         //в случае нажатия esc - пустая строка
        else if (ch == 13)
        {
            if (!filename.empty())
            {
                cout << endl;
                inputComplete = true;
            }
        }
        else if (ch == 8) {               //удаление символа при нажатии backspace
            if (!filename.empty())
            {
                filename.pop_back();
                cout << "\b \b";
            }
        }
        else if (((ch >= 'а' && ch <= 'я') || (ch >= 'А' && ch <= 'Я') || ch == 'Ё' || ch == 'ё' || ch == ' ' || ch == '.' || ch == '-' || isalpha(ch) || isalnum(ch)) == 1)
        {
            filename += ch;
            cout << ch;
        }
    }
    return filename;
}

// Функция для отображения меню с выделенным текущим пунктом
void move_arr(int position, string items[], int size_items, string text) {
    system("cls");  // Очистка экрана
    cout << text;   // Вывод текста меню
    for (int i = 0; i < size_items; i++) {
        if (i == position)
            cout << " " << char(155) << char(155) << "  ";  // Выделение текущего пункта
        else
            cout << "   ";
        cout << items[i] << '\n';  // Вывод пункта меню
    }
}

// Функция для навигации по меню с помощью клавиш
int dynamic_menu(int& position, string items[], int size_items, string text) {
    char symbol;
    move_arr(position, items, size_items, text);  // Первоначальное отображение меню
    if (items == NULL) {
        cout << "\nФайлы не найдены. Добавьте файл(-ы) в папку с программой.\n";
        system("pause");
        system("cls");
        return -1;
    }
    do {
        symbol = _getch();  // Получение нажатой клавиши
        if (symbol == ENTER) {
            move_arr(position, items, size_items, text);
            return 0;
        }
        if (symbol == ESCAPE) {
            position = -2;  // Выход из меню
            return 0;
        }
        if (symbol == -32 && _kbhit()) {  // Обработка стрелок
            char sub_symbol;
            sub_symbol = _getch();
            if (sub_symbol == 72) {  // Стрелка вверх
                if (position > 0) {
                    position--;
                    move_arr(position, items, size_items, text);
                }
                else {
                    position = size_items - 1;
                    move_arr(position, items, size_items, text);
                }

            }
            if (sub_symbol == 80) {  // Стрелка вниз
                if (position < size_items - 1) {
                    position++;
                    move_arr(position, items, size_items, text);
                }
                else {
                    position = 0;
                    move_arr(position, items, size_items, text);
                }
            }
            if (sub_symbol == 75) {  // Стрелка влево
                position = -1;
                return 0;
            }
            if (sub_symbol == 77) {  // Стрелка вправо
                move_arr(position, items, size_items, text);
                return 0;
            }
        }
    } while (symbol != ENTER);
}

// Меню Да/Нет
int no_or_yes(string menu) {
    string list[2] = { "Нет", "Да" };
    int position = 0;
    system("cls");
    dynamic_menu(position, list, size(list), menu);
    return position;
}

// Получает список txt-файлов в указанной папке
string* list_files(const string& folder_path, int& num_files) {
    WIN32_FIND_DATAA file_data;
    HANDLE h_find = FindFirstFileA((folder_path + "\\*.txt").c_str(), &file_data);

    if (h_find == INVALID_HANDLE_VALUE) {
        num_files = 0;
        return nullptr; // Папка пуста или не существует
    }

    // Первый проход: подсчёт количества файлов
    num_files = 0;
    do {
        if (!(file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            num_files++;
        }
    } while (FindNextFileA(h_find, &file_data));

    // Выделяем массив под имена файлов
    string* file_names = new string[num_files];

    // Второй проход: заполнение массива (перезапускаем поиск)
    FindClose(h_find);
    h_find = FindFirstFileA((folder_path + "\\*.txt").c_str(), &file_data);

    int index = 0;
    do {
        if (!(file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            file_names[index++] = file_data.cFileName;
        }
    } while (FindNextFileA(h_find, &file_data));

    FindClose(h_find);
    return file_names;
}

// Получает путь к директории, где находится программа
string get_program_dir() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);  // Получаем полный путь к исполняемому файлу
    string exe_path = buffer;
    return exe_path.substr(0, exe_path.find_last_of("\\/"));  // Возвращаем только директорию
}

bool parser_endings() {
    string line;
    ifstream endings("Окончания.txt"); // окрываем файл окончаний
    if (endings.is_open()) {
        while (getline(endings, line)) {
            if (line == "прил:") {
                while (line != "/") {
                    getline(endings, line);
                    if (line == "/")
                        break;
                    add_ending(main_ending_ptr, line, 0b00100000, "-", "-", "-", "-");
                }
            }
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
            if (line == "нар:") {
                while (line != "/") {
                    getline(endings, line);
                    if (line == "/")
                        break;
                    add_ending(main_ending_ptr, line, 0b00000100, "-", "-", "-", "-");
                }
            }
            /*if (line == "числ:") {
                string word;
                endings >> word;
                string list[8];
                while (word != "/") {
                    int i = 0;
                    while (word != ";") {
                        list[i] = word;
                        endings >> word;
                        i++;
                    }
                    string padezh;
                    endings >> padezh;
                    for (int j = 0; j < i; j++) {
                        add_ending(main_ending_ptr, list[j], 0b00010000, "-", "-", "-", padezh);
                    }
                    endings >> word;
                }
            }*/
            /*if (line == "мест:") {
                string word;
                endings >> word;
                string list[6];
                while (word != "/") {
                    int i = 0;
                    while (word != ";") {
                        list[i] = word;
                        endings >> word;
                        i++;
                    }
                    string padezh;
                    endings >> padezh;
                    for (int j = 0; j < i; j++) {
                        add_ending(main_ending_ptr, list[j], 0b00001000, "-", "-", "-", padezh);
                    }
                    endings >> word;
                }
            }*/
            if (line == "сущ:") {
                while (line != "/") {
                    getline(endings, line);
                    if (line == "/")
                        break;
                    add_ending(main_ending_ptr, line, 0b10000000, "-", "-", "-", "-");
                }
            }
        }
        //out_ending();
    }
    else
        return 0;
    endings.close();     // закрываем файл
    return 1;
}

bool parser_prep() {
    string line;
    ifstream prep("Предлоги.txt"); // окрываем файл окончаний
    if (prep.is_open()) {
        while (getline(prep, line)) {
            string member;
            getline(prep, member);
            add_ending(main_prep, line, 0b00000001, "-", "-", "-", member);
        }
    }
    else
        return 0;
    prep.close();     // закрываем файл
    return 1;
}

bool parser_particles() {
    string line;
    ifstream particle("Союзы, частицы, междометия.txt"); // окрываем файл окончаний
    if (particle.is_open()) {
        while (getline(particle, line)) {
            add_ending(main_particles, line, 0b00000000, "-", "-", "-", "-");
        }
    }
    else
        return 0;
    particle.close();     // закрываем файл
    return 1;
}

bool parser_pronoun() {
    string line;
    ifstream pronoun("Местоимения.txt"); // окрываем файл окончаний
    if (pronoun.is_open()) {
        while (getline(pronoun, line)) {
            string prt_of_spch;
            getline(pronoun, prt_of_spch);
            add_ending(main_pronoun, line, 0b00001000, "-", "-", "-", prt_of_spch);
        }
    }
    else
        return 0;
    pronoun.close();     // закрываем файл
    return 1;
}

bool parser_numeral() {
    string line;
    ifstream numeral("Числительные.txt"); // окрываем файл окончаний
    if (numeral.is_open()) {
        while (getline(numeral, line)) {
            string prt_of_spch;
            getline(numeral, prt_of_spch);
            add_ending(main_numeral, line, 0b00010000, "-", "-", "-", prt_of_spch);
        }
    }
    else
        return 0;
    numeral.close();     // закрываем файл
    return 1;
}

int main()
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    cout << intro;
    system("pause");
    system("cls");
    int position = 0;
    while (!parser_endings() or !parser_prep() or !parser_particles() or !parser_pronoun() or !parser_numeral())
    {
        cout << "Файлы правил не найдены, добавьте их в папку с программой, иначе невозможно продолжить работу программы.\n\n";
        system("pause");
        system("cls");
    }
    while (position > -1) {
        //system("pause");
        //system("cls");
        string* file_names;
        int num_files;

        // Получаем список txt-файлов в директории программы
        string program_dir = get_program_dir();
        file_names = list_files(program_dir, num_files);

        // Пользователь выбирает файл
        int empty_list = dynamic_menu(position, file_names, num_files, menu_choose_file);
        if (empty_list == -1)
            continue;



        if (position < 0) {
            string out_confirm = "Вы точно хотите выйти из программы?\n";
            if (no_or_yes(out_confirm) == 1)  // Подтверждение выхода
                return 0;
            else
                position = 0;
            continue;
        }

        // Формируем полный путь к файлу
        program_dir = program_dir + "\\" + file_names[position];
        //program_dir = WriteFilename();
        /*if (program_dir == "")
            break;
        else if (program_dir.find(".txt") == -1) {
            program_dir += ".txt";
            DevideSentence(program_dir);
        }
        else
            DevideSentence(program_dir);*/
        DevideSentence(program_dir);
    }
    /*cout<<"Введите название текстового файла\nНажмите клавишу esc для выхода\n";  //переформулировать
    bool exit = false;
    string filename;
    while(!exit)
    {
        filename = WriteFilename();
        if(filename == "")
            break;
        else if(filename.find(".txt")==-1){
            filename+=".txt";
            DevideSentence(filename);
        }
        else
            DevideSentence(filename);
    }*/
    return 0;
}
