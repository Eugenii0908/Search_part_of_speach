#pragma once
#include <iostream>
#include <conio.h>
#include <windows.h>
using namespace std;

// Определение констант для клавиш
const char ESCAPE = 27;   // Клавиша ESC
const char ENTER = 13;    // Клавиша Enter
const char BSPACE = 8;    // Клавиша Backspace
const char TAB = 9;       // Клавиша Tab

// Указатель на функцию для передачи в структуру
using fun_ptr = int(*)();

// Структура узла сильноветвящегося дерева
struct tree_node {
    string name;
    string text;
    string* list;
    int size_list;
    fun_ptr function;
    struct tree_node* first_child;
    struct tree_node* next_brother;
    struct tree_node* prev_brother;
    struct tree_node* father;
};

// Глобальный корень дерева
extern tree_node* root;

// Создание нового узла
tree_node* create_node(string name, string text, string* list, int size_list, fun_ptr fun);
// Поиск узла по значению в дереве
tree_node* find_node(tree_node* current, string value);
// Освобождение памяти поддерева
void free_subtree(tree_node* node);
// Добавление узла как потомка указанного родителя
// 0 - родитель не найден; 1 - узел добавлен
bool add_node(string parent_name, string new_name, string new_text, string* list, int size_list, fun_ptr fun);
// Удаление узла и всего его поддерева
// 0 - узел не найден; 1 - узел удален
bool remove_node(string value);
// Вспомогательная функция для печати дерева
void print_tree(tree_node* node, int level);
// Функция для отображения меню с выделенным текущим пунктом
void move_arr(int position, string items[], int size_items, string text);
// Функция для навигации по меню с помощью клавиш
void dynamic_menu(int& position, string items[], int size_items, string text);
// Меню Да/Нет
// 0 - нет; 1 - да
int no_or_yes(string menu);
// Получает список txt-файлов в указанной папке
string* list_files(const string& folder_path, int& num_files);
// Получает путь к директории, где находится программа
string get_program_dir();
// Цикл для прохождения по структуре меню
void cycle();