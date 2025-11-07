#include "interface_lib.h"

// Глобальный корень дерева
tree_node* root = nullptr;

// Создание нового узла
tree_node* create_node(string name, string text, string* list, int size_list, fun_ptr fun) {
    tree_node* new_node = new tree_node;
    new_node->name = name;
    new_node->text = text;
    new_node->list = list;
    new_node->size_list = size_list;
    new_node->function = fun;
    new_node->first_child = nullptr;
    new_node->next_brother = nullptr;
    new_node->prev_brother = nullptr;
    new_node->father = nullptr;
    return new_node;
}

// Поиск узла по значению в дереве
tree_node* find_node(tree_node* current, string value) {
    // Если дошли до конца
    if (current == nullptr)
        return nullptr;

    // Если нашли по значению, возвращаем его
    if (current->name == value)
        return current;

    // Рекурсивно ищем в детях
    tree_node* child = current->first_child;
    while (child != nullptr) {
        tree_node* found = find_node(child, value);
        // Если нашли значение
        if (found != nullptr)
            return found;
        // Переходим к следующему брату
        child = child->next_brother;
    }
    // Если ничего не нашли, возвращаем nullptr
    return nullptr;
}

// Освобождение памяти поддерева
void free_subtree(tree_node* node) {
    if (node == nullptr) return;

    // Рекурсивно удаляем детей
    tree_node* child = node->first_child;
    while (child != nullptr) {
        tree_node* next_child = child->next_brother;
        free_subtree(child);
        child = next_child;
    }

    delete node;
}

// Добавление узла как потомка указанного родителя
// 0 - родитель не найден; 1 - узел добавлен
bool add_node(string parent_name, string new_name, string new_text, string* list, int size_list, fun_ptr fun) {
    // Если дерево пустое, создаем корень
    if (root == nullptr) {
        root = create_node(new_name, new_text, list, size_list, fun);
        return true;
    }

    // Находим родительский узел
    tree_node* parent_node = find_node(root, parent_name);
    if (parent_node == nullptr) {
        return false; // Родитель не найден
    }

    // Создаем новый узел
    tree_node* new_node = create_node(new_name, new_text, list, size_list, fun);
    new_node->father = parent_node;

    // Если у родителя нет детей, новый узел становится первым ребенком
    if (parent_node->first_child == nullptr) {
        parent_node->first_child = new_node;
    }
    else {
        // Ищем последнего ребенка родителя
        tree_node* last_child = parent_node->first_child;
        while (last_child->next_brother != nullptr) {
            last_child = last_child->next_brother;
        }

        // Добавляем новый узел как следующего брата
        last_child->next_brother = new_node;
        new_node->prev_brother = last_child;
    }

    return true;
}

// Удаление узла и всего его поддерева
// 0 - узел не найден; 1 - узел удален
bool remove_node(string value) {
    if (root == nullptr) return false;

    // Если удаляем корень
    if (root->name == value) {
        free_subtree(root);
        root = nullptr;
        return true;
    }

    // Находим узел для удаления
    tree_node* node_to_remove = find_node(root, value);
    if (node_to_remove == nullptr) {
        return false; // Узел не найден
    }

    tree_node* parent = node_to_remove->father;

    // Перестраиваем связи между братьями
    if (node_to_remove->prev_brother != nullptr) {
        node_to_remove->prev_brother->next_brother = node_to_remove->next_brother;
    }
    if (node_to_remove->next_brother != nullptr) {
        node_to_remove->next_brother->prev_brother = node_to_remove->prev_brother;
    }

    // Если удаляемый узел был первым ребенком, обновляем указатель родителя
    if (parent != nullptr && parent->first_child == node_to_remove) {
        parent->first_child = node_to_remove->next_brother;
    }

    // Освобождаем память поддерева
    free_subtree(node_to_remove);

    return true;
}

// Вспомогательная функция для печати дерева
void print_tree(tree_node* node, int level = 0) {
    if (node == nullptr) return;

    for (int i = 0; i < level; i++) {
        cout << "  ";
    }
    cout << node->name << endl;

    tree_node* child = node->first_child;
    while (child != nullptr) {
        print_tree(child, level + 1);
        child = child->next_brother;
    }
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
void dynamic_menu(int& position, string items[], int size_items, string text) {
    char symbol;
    move_arr(position, items, size_items, text);  // Первоначальное отображение меню
    do {
        symbol = _getch();  // Получение нажатой клавиши
        if (symbol == ENTER) {
            move_arr(position, items, size_items, text);
            return;
        }
        if (symbol == ESCAPE) {
            position = -2;  // Выход из меню
            return;
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
                return;
            }
            if (sub_symbol == 77) {  // Стрелка вправо
                move_arr(position, items, size_items, text);
                return;
            }
        }
    } while (symbol != ENTER);
}

// Меню нет/да
// 0 - нет; 1 - да
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

// Цикл для прохождения по стурктуре меню
void cycle() {
    tree_node* menu_pointer = root;
    int position = 0;
    while (position > -1) { 
        // Отображение меню
        dynamic_menu(position, menu_pointer->list, menu_pointer->size_list, menu_pointer->text);
        // Если нажат ESCAPE - выходим в главное меню
        if (position == -2) {
            // Если нажат ESCAPE в главном меню - уточняем о выходе
            if (menu_pointer == root and no_or_yes("Вы уверены, что хотите выйти?\n") == 1)
                return;
            menu_pointer = root;
            position = 0;
            continue;
        }
        // Если нажата стребка влево - выходим на меню назад
        if (position == -1) {
            // Если нажата стрелка влево в главном меню - уточняем о выходе
            if (menu_pointer == root and no_or_yes("Вы уверены, что хотите выйти?\n") == 1)
                return;
            else if (menu_pointer != root) {
                menu_pointer = menu_pointer->father;
                position = 0;
                continue;
            }
            position = 0;
            continue;
        }
        else {
            // Переход к выбранному меню
            if (menu_pointer->first_child == nullptr) {
                position = 0;
                continue;
            }
            menu_pointer = menu_pointer->first_child;
            if (menu_pointer != 0) {
                for (int i = 0; i < position; i++) {
                    menu_pointer = menu_pointer->next_brother;
                }
            }
            // Если в узле есть переданная функция для выполнения - выполняем
            if (menu_pointer->function != nullptr) {
                int result = menu_pointer->function();
                // Переход на меню назад
                if (result == -1)
                    menu_pointer = menu_pointer->father;
                // Переход в главное меню
                if (result == -2)
                    menu_pointer = root;
                // Переход в главное меню с сохранением указателя на пункт
                if (result == -3) {
                    menu_pointer = root;
                    continue;
                }
            }
            position = 0;
        }
    }
    return;
}
