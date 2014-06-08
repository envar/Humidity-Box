#include "Arduino.h"
#include "LiquidCrystal.h"
#include "Menu.h"
#include "Events.h"
#include "DS1307RTC.h"
#include "Time.h"
#include "Wire.h"
#include "math.h"
#include "Button.h"

// Set up LCD
LiquidCrystal lcd(8, 13, 9, 4, 5, 6, 7);
byte left_arrow[8] = {
    0b00000,
    0b00100,
    0b01000,
    0b11111,
    0b01000,
    0b00100,
    0b00000,
    0b00000,
};


// Addresses for sensors
byte HIH6130_address = 0x27; // temperature humidity sensor address
byte DS1307RTC_address = 0x68; // real time clock address

// Initiate variables for sensor data
byte status;

float RH, T_C;
float RH_sp = 0; // set point for humidity
float RH_sp_new = 0;

// Valve position
int valve1_pin = 10;  // Dry stream;
int valve1_level = 0; // this should be a percentage
int valve1_sp = 0;    // 
int valve2_pin = 11;  // Wet stream;
int valve2_level = 0;
int valve2_sp = 0;

void check_valves()
{
    int level;

    if (valve1_level != valve1_sp)
    {
        level = map(valve1_sp, 0, 100, 0, 255);
        analogWrite(valve1_pin, level);
        valve1_level = valve1_sp;
    }
    if (valve2_level != valve2_sp)
    {
        level = map(valve2_sp, 0, 100, 0, 255);
        analogWrite(valve2_pin, level);
        valve2_level = valve2_sp;
    }
}

// initiate actions: keeps track of action to be executed when a key is pressed
void (*current_action)(int);
void (*last_action)(int);

void change_current_action(void (*action)(int))
{
    current_action = action;
}

void change_last_action(void (*action)(int))
{
    last_action = action;
}

// initiate menu
Menu * main_menu = new Menu();

void print_menu()
{
    char * first_row;
    char * second_row;

    // check if this is the last item in the list
    if (main_menu->current_item != main_menu->current_item->get_next())
    {
        lcd.setCursor(0, 0);
        first_row = main_menu->current_item->get_title();
        second_row = main_menu->current_item->get_next()->get_title();
        lcd.print(first_row);
        lcd.setCursor(15, 0);
        lcd.write(byte(0));
        lcd.setCursor(0, 1);
        lcd.print(second_row);
    }
    else
    {
        first_row = main_menu->current_item->get_previous()->get_title();
        second_row = main_menu->current_item->get_title();
        lcd.print(first_row);
        lcd.setCursor(15, 0);
        lcd.print(" ");
        lcd.setCursor(0, 1);
        lcd.print(second_row);
        lcd.setCursor(15, 1);
        lcd.write(byte(0));
    }
}

void navigate_menu(int input) 
{
    switch (input)
    {
        case -9:
            main_menu->current_item = main_menu->first_item;
            print_menu();
            break;
        case -1: // No button
            break;
        case 0: // Right Key
            main_menu->current_item = main_menu->current_item->get_child();
            print_menu();
            break;
        case 1: // Up Key
            main_menu->current_item = main_menu->current_item->get_previous();
            print_menu();
            break;
        case 2: // Down Key
            main_menu->current_item = main_menu->current_item->get_next();
            print_menu();
            break;
        case 3: // Left Key
            main_menu->current_item = main_menu->current_item->get_parent();
            print_menu();
            break;
        case 4: // Select Key
            change_current_action(main_menu->current_item->get_action());
            current_action(-9);
            break;
        default:
            break;
    }
}

// Dialogs
int selection;

void save_changes_dialog(int input)
{
    switch (input)
    {
        case -9:
            lcd.clear();
            lcd.print("Save changes");
            lcd.setCursor(0, 1);
            lcd.print(" Yes  No<");
            selection = 0;
            break;
        case 0: // Right Key
            lcd.setCursor(4, 1);
            lcd.print(" ");
            lcd.setCursor(8, 1);
            lcd.print("<");
            selection = 0;
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
            if (selection == 1) 
            {
                lcd.print("Changes saved");
                delay(2000);
                change_current_action(last_action);
                current_action(9);
            }
            else
            {
                lcd.print("No changes made");       
                change_current_action(last_action);
                current_action(9);
            }
            break;
        default:
            break;
    }
}

// get the temperature and humidity
byte get_humidity_temperature()
{
    byte Hum_H, Hum_L, Temp_H, Temp_L, status;
    unsigned int H_dat, T_dat;

    Wire.beginTransmission(HIH6130_address);
    Wire.write(0);
    Wire.endTransmission();
    delay(100);
    
    Wire.requestFrom(HIH6130_address, (byte)4);
    Hum_H = Wire.read();
    Hum_L = Wire.read();
    Temp_H = Wire.read();
    Temp_L = Wire.read();

    status = (Hum_H >> 6) & 0x03;
    Hum_H = Hum_H & 0x3f;
    H_dat = (((unsigned int)Hum_H) << 8) | Hum_L;
    T_dat = (((unsigned int)Temp_H) << 8) | Temp_L;
    T_dat = T_dat >> 2;
    RH = H_dat * 0.00610388817;
    T_C = T_dat * 0.01007141549 - 40;
    return(status);
}

// Display the time, humidity, and temperature in a nice way
void print2digits(int number)
{
    if (number >= 0 && number < 10)
    {
        lcd.print("0");
        lcd.print(number);
    }
    else
    {
        lcd.print(number);
    }
}

void Display(int input)
{
    // digital clock display of the time

    switch (input)
    {
        case -9:
            lcd.clear();
            break;
        case 3:
            change_current_action(&navigate_menu);
            current_action(-9);
            break;
        default:
            time_t t = now();
            tmElements_t tm;
            breakTime(t, tm);

            lcd.setCursor(0, 0);
            print2digits(tm.Hour);
            lcd.print(":");
            print2digits(tm.Minute);
            lcd.print(":");
            print2digits(tm.Second);
                    
            status = get_humidity_temperature();
            
            if (status == 0)
            {
                lcd.print(" T:");
                lcd.print(T_C, 1);
                lcd.setCursor(0, 1);
                lcd.print("RH:");
                lcd.print(RH, 0);
                lcd.print(" RHsp:");
                lcd.print(RH_sp, 0);
            }
            else
            {
                lcd.setCursor(0, 1);
                lcd.print("failed: ");
                lcd.print(status);
            }
            break;
    }
}

// Manually change the humidity

void print3digits(float n)
{
    if (n < 10) 
    {
        lcd.setCursor(3, 1);
        lcd.print("  ");
        lcd.print(n);
    }
    else if (n >= 10 && n < 100)
    {
        lcd.setCursor(3, 1);
        lcd.print(" ");
        lcd.print(n, 0);
    }
    else
    {
        lcd.setCursor(3, 1);
        lcd.print(n, 0);
    }

}

void change_humidity(int input)
{
    switch (input)
    {
        case -9: // First time run
            lcd.clear();
            lcd.print("Change Humidity ");
            lcd.setCursor(0, 1);
            lcd.print("RH:");
            print3digits(RH_sp_new);
            break;
        case -1: // No button
            break;
        case 1: // Up Key
            RH_sp_new++;
            if (RH_sp_new > 100) RH_sp_new = 100;
            print3digits(RH_sp_new);
            break;
        case 2: // Down Key
            RH_sp_new--;
            if (RH_sp_new < 0) RH_sp_new = 0;
            print3digits(RH_sp_new);
            break;
        case 4: // Select Key
            change_current_action(&save_changes_dialog);
            change_last_action(&change_humidity);
            current_action(-9);
            break;
        case 9: // Recieved selection from menu save dialog
            if (selection == 1) // If "yes"
            {
                RH_sp = RH_sp_new; // commit change
            }
            change_current_action(&navigate_menu);
            current_action(-9);
            break;
        default:
            break;
    }
}

// Add Events and navigate events list
Event_Handler * event_handler = new Event_Handler();

time_t t;
tmElements_t tm;

void navigate_events(int input);

void print_time(time_t t)
{
    tmElements_t tm;
    breakTime(t, tm);

    print2digits(tm.Hour);
    lcd.print(":");
    print2digits(tm.Minute);
    lcd.print(" ");
    print2digits(tm.Month);
    lcd.print("/");
    print2digits(tm.Day);
    lcd.print("/");
    print2digits((tm.Year+1970) % 100);
}

void print_time(tmElements_t tm)
{
    print2digits(tm.Hour);
    lcd.print(":");
    print2digits(tm.Minute);
    lcd.print(" ");
    print2digits(tm.Month);
    lcd.print("/");
    print2digits(tm.Day);
    lcd.print("/");
    print2digits((tm.Year+1970) % 100);
}

void add_event_prompt(int input)
{
    switch (input)
    {
        case -9:
            selection = 1;
            if (RTC.get)
            {
                t = now();
                breakTime(t, tm); // break time into a structure of elements
            }
            RH_sp_new = 0;

            lcd.clear();
            print_time(tm);
            lcd.setCursor(0, 1);
            lcd.print("RH: ");
            lcd.print(RH_sp_new, 0);
            break;
        case 0: // Right Key
            selection++;
            if (selection > 6) selection = 6;
            break;
        case 1: // Up Key
            switch (selection)
            {
                case 1: // hour selected
                    tm.Hour++;
                    if (tm.Hour > 24) tm.Hour = 24;
                    break;
                case 2: // minute selected
                    tm.Minute++;
                    if (tm.Minute > 60) tm.Minute = 60;
                    break;
                case 3: // month selected
                    tm.Month++;
                    if (tm.Month > 12) tm.Month = 12;
                    break;
                case 4: // day selected
                    tm.Day++;
                    if (tm.Day > 31) tm.Day = 31;
                    break;
                case 5: // year selected
                    tm.Year++;
                    break;
                case 6: // RH selected
                    RH_sp_new = RH_sp_new + 1;
                    break;
                default:
                    break;
            }
            lcd.setCursor(0, 0);
            print_time(tm);
            lcd.setCursor(4, 1);
            lcd.print(RH_sp_new, 0);
            break;
        case 2: // Down Key
            switch (selection)
            {
                case 1: // hour selected
                    tm.Hour--;
                    if (tm.Hour < 0) tm.Hour = 0;
                    break;
                case 2: // minute selected
                    tm.Minute--;
                    if (tm.Minute < 0) tm.Minute = 0;
                    break;
                case 3: // month selected
                    tm.Month--;
                    if (tm.Month < 0) tm.Month = 0;
                    break;
                case 4: // day selected
                    tm.Day--;
                    if (tm.Day < 0) tm.Day = 0;
                    break;
                case 5: // year selected
                    tm.Year--;
                    break;
                case 6: // RH selected
                    RH_sp_new = RH_sp_new - 1;
                    break;
                default:
                    break;
            }
            lcd.setCursor(0, 0);
            print_time(tm);
            lcd.setCursor(4, 1);
            lcd.print(RH_sp_new, 0);
            break;
        case 3: // Left Key
            selection--;
            if (selection < 1) selection = 1;
            break;
        case 4: // Select Key
            change_current_action(&save_changes_dialog);
            change_last_action(&add_event_prompt);
            current_action(-9);
            break;
        case 9:
            if (selection == 1)
            {
                t = makeTime(tm);
                Event * event = new Event();
                event->add_time(t);
                event->add_set_point(RH_sp_new);
                event_handler->add_event(event);
            }
            change_current_action(&navigate_events);
            current_action(-9);
            break;
        default:
            break;
    }
}

void print_events()
{
    if (event_handler->is_empty())
    {    
        lcd.clear();
        lcd.print("No Events");
    }
    else
    {
        lcd.clear();
        if (event_handler->current_event->get_next())
        {
            print_time(event_handler->current_event->get_time());
            lcd.setCursor(15, 0);
            lcd.print("<");
            lcd.setCursor(0, 1);
            print_time(event_handler->current_event->get_next()->get_time());
        }
        else if (event_handler->current_event->get_previous())
        {
            print_time(event_handler->current_event->get_previous()->get_time());
            lcd.setCursor(0, 1);
            print_time(event_handler->current_event->get_time());
            lcd.setCursor(15, 1);
            lcd.print("<");
        }
        else
        {
            print_time(event_handler->current_event->get_time());
            lcd.setCursor(15, 0);
            lcd.print("<");
        }
    }
}

void add_remove_exit_prompt(int input);

void navigate_events(int input)
{
    switch (input)
    {
        case -9:
            event_handler->current_event = event_handler->first_event;
            print_events();
            break;
        case 1: // Up Key
            if (event_handler->current_event->get_previous())
            {
                event_handler->current_event = event_handler->current_event->get_previous();
                print_events();
            }
            break;
        case 2: // Down Key
            if (event_handler->current_event->get_next())
            {
                event_handler->current_event = event_handler->current_event->get_next();
                print_events();
            }
            break;
        case 4: // Select Key - add, remove, or exit
            change_current_action(&add_remove_exit_prompt);
            current_action(-9);
            break;
        default:
            break;
    }
}

void add_remove_exit_prompt(int input)
{
    // Selection: 1 is add, 2 is remove, 3 is exit
    switch (input)
    {
        case -9:
            lcd.clear();
            lcd.print("Add      Remove ");
            lcd.setCursor(0, 1);
            lcd.print("      Exit      ");
            selection = 3;
            break;
        case -1:
            break;
        case 0: // Right key
            if (selection == 1) 
            {
                lcd.setCursor(3, 0);
                lcd.print(" ");
                lcd.setCursor(15, 0);
                lcd.print("<");
                selection = 2;
            }
            break;
        case 1: // Up key
            if (selection == 3)
            {
                lcd.setCursor(10, 1);
                lcd.print(" ");
                lcd.setCursor(3, 0);
                lcd.print("<");
                selection = 1;
            }
            break;
        case 2: // Down key
            if ((selection == 1) || (selection == 2))
            {
                lcd.setCursor(3, 0);
                lcd.print(" ");
                lcd.setCursor(15, 0);
                lcd.print(" ");
                lcd.setCursor(10, 1);
                lcd.print("<");
                selection = 3;
            }
            lcd.setCursor(10, 1);
            lcd.print("<");
            break;
        case 3: // Left key
            if (selection == 2)
            {
                lcd.setCursor(15, 0);
                lcd.print(" ");
                lcd.setCursor(3, 0);
                lcd.print("<");
                selection = 1;
            }
            break;
        case 4: // Select Key
            switch (selection)
            {
                case 1:
                    change_current_action(&add_event_prompt);
                    current_action(-9);
                    break;
                case 2:
                    event_handler->remove_event(event_handler->current_event);
                    change_current_action(&navigate_events);
                    current_action(-9);
                    break;
                case 3:
                    change_current_action(&navigate_menu);
                    current_action(-9);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}





void setup()
{
    setSyncProvider(RTC.get);
    lcd.begin(16, 2);
    lcd.createChar(0, left_arrow);
    lcd.setCursor(0, 0);
    lcd.print("Humidity Control");
    lcd.setCursor(0, 1);
    lcd.print("by Michal Nicpon");
    delay(3000);
    Wire.begin();

    Item * display_item = new Item("Display         ");
    display_item->add_action(&Display);
    Item * change_humidity_item = new Item("Change Humidity ");
    change_humidity_item->add_action(&change_humidity);
    Item * events_item = new Item("Events          ");
    events_item->add_action(&navigate_events);
    
    main_menu->add_item_down(display_item);
    main_menu->add_item_down(change_humidity_item);
    main_menu->add_item_down(events_item);
    
    pinMode(valve1_pin, OUTPUT);
    pinMode(valve2_pin, OUTPUT);

    change_current_action(&navigate_menu);
    current_action(-9);
}



void loop()
{
    check_valves();

    key = check_button();
    current_action(key);

    delay(100);
}


