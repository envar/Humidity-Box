#include "Arduino.h"

class Item
{
    private:
        static const int screen_len = 16;
        char * title;
        Item * previous_item;
        Item * next_item;
        Item * parent_item;
        Item * child_item;
        typedef void (*function)(int);

    public:
        void (*item_function)(int);
        Item(const char * item_name);
        ~Item();
        void add_previous(Item * previous);
        void add_next(Item * next);
        void add_parent(Item * parent);
        void add_child(Item * parent);
        Item * get_previous();
        Item * get_next();
        Item * get_parent();
        Item * get_child();
        char * get_title();
        void add_action(void (*myfunction)(int));
        function get_action();
        void execute(int input);
};

Item::Item(const char * item_name)
{

    previous_item = 0;
    next_item = 0;
    parent_item = 0;
    child_item = 0;

    title = new char[screen_len +1]();
    for (int i = 0; i < screen_len; i++)
    {
        if (item_name[i] == '\0')
        {
            break;
        }

        title[i] = item_name[i];
    }
}

Item::~Item()
{
    if (next_item)
    {
        delete next_item;
    }

    delete title;
}

void Item::add_previous(Item * previous)
{
    previous_item = previous;
}

void Item::add_next(Item * next)
{
    next_item = next;
}

void Item::add_parent(Item * parent)
{
    parent_item = parent;
}

void Item::add_child(Item * child)
{
    child_item = child;
}

Item * Item::get_previous()
{
    return previous_item;
}

Item * Item::get_next()
{
    return next_item;
}

Item * Item::get_parent()
{
    return parent_item;
}

Item * Item::get_child()
{
    return child_item;
}

char * Item::get_title()
{
    return title;
}


void Item::add_action(void (*myfunction)(int))
{
    item_function = myfunction;
}

Item::function Item::get_action()
{
    return item_function;
}

void Item::execute(int input=-1)
{
    item_function(input);
}

class Menu
{
private:
    static const int screen_len = 16;
    int item_counter;
    // void save_changes_dialog(int);
    bool is_empty;
    int selection;
public:
    Menu();
    ~Menu();
    Item * first_item;
    Item * current_item; // where you are in the menu
    //void (*current_action)(int);
    //void (*last_action)(int);
    void add_item_down(Item * item); // Add item below the last created item
    void add_item_right(Item * item);
    //void navigate(int input);
    //void navigate_to_top();
    //void navigate_to(Item * item);
    //void select(char action);
    //void print_menu();
    //void change_current_action(void (*action)(int));
    //void change_last_action(void (*action)(int));
    //int get_selection();
};

Menu::Menu()
{
    current_item = 0;
    item_counter = 0;
    is_empty = true;
}

Menu::~Menu()
{
}

void Menu::add_item_down(Item * item)
{
    if (is_empty)
    {
        item->add_previous(item);
        item->add_next(item);
        item->add_parent(item);
        item->add_child(item);
        current_item = first_item = item;
        is_empty = false;
    }
    else
    {
        item->add_next(item);
        item->add_previous(current_item);
        item->add_parent(current_item->get_parent()); // inheret parent from item above so that you can quickly go back
        item->add_child(item); // new items have no child - loops back
        current_item->add_next(item);
        current_item = item;
    }
    item_counter++;
}

void Menu::add_item_right(Item * item) // TODO: Add in catch if there are no items
{
    item->add_previous(item);
    item->add_next(item);
    item->add_parent(current_item);
    item->add_child(item);
    current_item->add_child(item);
    current_item = item;
    item_counter++;
}

/*
void Menu::navigate(int input=-1)
{
    switch (input)
    {
        case -1:
            current_item = first_item;
            print_menu();
        case 0: // Right Key
            current_item = current_item->get_child();
            print_menu();
            break;
        case 1: // Up Key
            current_item = current_item->get_previous();
            print_menu();
            break;
        case 2: // Down Key
            current_item = current_item->get_next();
            print_menu();
            break;
        case 3: // Left Key
            current_item = current_item->get_parent();
            print_menu();
            break;
        case 4: // Select Key
            change_current_action(current_item->get_action());
            current_action(-1);
            break;
        default:
            break;
    }
}

void Menu::navigate_to_top()
{
    current_item = first_item;
    print_menu();
}

void Menu::navigate_to(Item * item)
{
    current_item = item;
    print_menu();
}

void Menu::print_menu()
{
    char * first_row;
    char * second_row;

    first_row = current_item->get_title();
    if (current_item != current_item->get_next())
    {
        second_row = current_item->get_next()->get_title();
    }
    else
    {
        second_row = (char *)"                ";
    }

    lcd.setCursor(0, 0);
    lcd.print(first_row);
    lcd.setCursor(15, 0);
    lcd.print("<");
    lcd.setCursor(0, 1);
    lcd.print(second_row);
}

void Menu::save_changes_dialog(int input=-1)
{
    switch (input)
    {
        case -1:
            lcd.clear();
            lcd.print("Save changes");
            lcd.setCursor(0, 1);
            lcd.print(" Yes  No<");
            selection = 2;
            break;
        case 0: // Right Key
            lcd.setCursor(4, 1);
            lcd.print(" ");
            lcd.setCursor(8, 1);
            lcd.print("<");
            selection = 2;
            break;
        case 3: // Left Key
            lcd.setCursor(8, 1);
            lcd.print(" ");
            lcd.setCursor(4, 1);
            lcd.print("<");
            selection = 1;
            break;
        case 4: // Select Key
            lcd.clear();
            if (selection == 1) lcd.print("Changes saved");
            else lcd.print("No changes made");
            last_action(9);
            change_current_action(last_action);
            break;
        default:
            break;
    }

}

void Menu::change_current_action(void (*action)(int))
{
    current_action = action;
}

int Menu::get_selection()
{
    return selection;
}
*/
