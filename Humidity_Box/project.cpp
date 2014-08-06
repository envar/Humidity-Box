#include "Arduino.h"
#include "avr/pgmspace.h"
// #include "ctype.h"
#include "EEPROM.h"
#include "LiquidCrystal.h"
#include "SD.h"
#include "Menu.h"
#include "Time.h"
#include "Events.h"
#include "Wire.h"
#include "NESpad.h"
#include "MemoryFree.h"
#include "../libraries/Enums/Enums.h"

// NINTENDO CONTROLLER
NESpad nintendo = NESpad(A3, A2, A1);
uint8_t check_button()
{
    uint8_t state = nintendo.buttons();
    if (state & NES_A) return NES_A;
    else if (state & NES_RIGHT) return NES_RIGHT;
    else if (state & NES_UP) return NES_UP;
    else if (state & NES_DOWN) return NES_DOWN;
    else if (state & NES_LEFT) return NES_LEFT;
    else return 0;
}

// Set up LCD
LiquidCrystal lcd(7, 6, 5, 4, 2, 1);
uint8_t left_arrow[8] = {
    0b00000,
    0b00100,
    0b01000,
    0b11111,
    0b01000,
    0b00100,
    0b00000,
    0b00000,
};

void print_free_mem(int line)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(line);
    lcd.setCursor(0, 1);
    lcd.print(freeMemory());

    delay(2000);
}

// Addresses for sensors
uint8_t HIH6130_address = 0x27; // temperature humidity sensor address
uint8_t DS1307RTC_address = 0x68; // real time clock address

// Initiate variables for sensor data
uint8_t status;

float RH, T_C;
float RH_sp = 0; // set point for humidity

// get the temperature and humidity
uint8_t get_humidity_temperature()
{
    byte Hum_H, Hum_L, Temp_H, Temp_L, status;
    unsigned int H_dat, T_dat;

    Wire.beginTransmission(HIH6130_address);
    Wire.write(0);
    Wire.endTransmission();
    delay(100);

    Wire.requestFrom(HIH6130_address, (int)4);
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

byte decToBcd(byte val)
{
  return ( (val/10*16) + (val%10) );
}
 
// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
  return ( (val/16*10) + (val%16) );
}

time_t getTimeDS1307()
{
    tmElements_t t;
    // Reset the register pointer
    Wire.beginTransmission(DS1307RTC_address);
    Wire.write(0);
    Wire.endTransmission();
   
    Wire.requestFrom(DS1307RTC_address, 7);
   
    // A few of these need masks because certain bits are control bits
    t.Second     = bcdToDec(Wire.read() & 0x7f);
    t.Minute     = bcdToDec(Wire.read());
    t.Hour       = bcdToDec(Wire.read() & 0x3f);  // Need to change this if 12 hour am/pm
    t.Wday       = bcdToDec(Wire.read());
    t.Day        = bcdToDec(Wire.read());
    t.Month      = bcdToDec(Wire.read());
    t.Year       = y2kYearToTm(bcdToDec(Wire.read()));
    
    return makeTime(t);

}

// Valve position
uint8_t valve1_pin = 3;  // Dry stream;
uint8_t valve1_pos = 150; // this should be 0 - 255
uint8_t valve2_pin = 9;  // Wet stream;
uint8_t valve2_pos = 150; // start off with full open

void change_valve_position(uint8_t sp)
{
    uint16_t addr = (sp * 2) / 5;
    valve1_pos = EEPROM.read(addr);
    // lcd.clear();
    // lcd.print("sp: ");
    // lcd.print(RH_sp);
    // delay(2000);
    // lcd.clear();
    // lcd.print(valve1_pos);
    // delay(2000);
    addr++;
    valve2_pos = EEPROM.read(addr);
    // lcd.clear();
    // lcd.print(valve2_pos);
    // delay(2000);

    analogWrite(valve1_pin, valve1_pos);
    analogWrite(valve2_pin, valve2_pos);
}

// initiate actions: keeps track of action to be executed when a key is pressed
void (*current_action)(ModeOption, uint8_t);
void (*last_action)(ModeOption, uint8_t);

void change_current_action(void (*action)(ModeOption, uint8_t))
{
    current_action = action;
}

void change_last_action(void (*action)(ModeOption, uint8_t))
{
    last_action = action;
}

// Helper print functions
void print2digits(uint8_t number)
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

void print3digits(uint8_t n)
{
    if (n < 10)
    {
        lcd.print("  ");
        lcd.print(n);
    }
    else if (n >= 10 && n < 100)
    {
        lcd.print(" ");
        lcd.print(n);
    }
    else
    {
        lcd.print(n);
    }
}

void print3digits(uint16_t n)
{
    if (n < 10)
    {
        lcd.print("  ");
        lcd.print(n);
    }
    else if (n >= 10 && n < 100)
    {
        lcd.print(" ");
        lcd.print(n);
    }
    else
    {
        lcd.print(n);
    }
}

void print3digits(float n)
{
    if (n < 10)
    {
        lcd.print("  ");
        lcd.print(n, 0);
    }
    else if (n >= 10 && n < 100)
    {
        lcd.print(" ");
        lcd.print(n, 0);
    }
    else
    {
        lcd.print(n, 0);
    }
}

void print_date_time(time_t t) {
    print2digits(hour(t));
    lcd.print(":");
    print2digits(minute(t));
    lcd.print(" ");
    print2digits(month(t));
    lcd.print("/");
    print2digits(day(t));
    lcd.print("/");
    lcd.print(year(t));
}

void print_time(time_t t) {
    print2digits(hour(t));
    lcd.print(":");
    print2digits(minute(t));
    lcd.print(":");
    print2digits(second(t));
}

// initiate menu
Menu * main_menu = new Menu();

void print_menu()
{
    char * first_row;
    char * second_row;

    lcd.clear();
    // check if this is the last item in the list
    if (main_menu->current_item != main_menu->current_item->get_next())
    {
        lcd.setCursor(0, 0);
        first_row = main_menu->current_item->get_title();
        second_row = main_menu->current_item->get_next()->get_title();
        lcd.print(first_row);
        lcd.setCursor(15, 0);
        lcd.write(uint8_t(0));
        lcd.setCursor(0, 1);
        lcd.print(second_row);
    }
    else
    {
        first_row = main_menu->current_item->get_previous()->get_title();
        second_row = main_menu->current_item->get_title();
        lcd.print(first_row);
        lcd.setCursor(0, 1);
        lcd.print(second_row);
        lcd.setCursor(15, 1);
        lcd.write(uint8_t(0));
    }
}

void navigate_menu(ModeOption mode, uint8_t input)
{
    switch (mode) {
        case INIT: 
            main_menu->current_item = main_menu->first_item;
            print_menu();
            break;
        case LOOP:
            switch (input)
            {
                case NES_RIGHT: // Right Key
                    main_menu->current_item = main_menu->current_item->get_child();
                    print_menu();
                    break;
                case NES_UP: // Up Key
                    main_menu->current_item = main_menu->current_item->get_previous();
                    print_menu();
                    break;
                case NES_DOWN: // Down Key
                    main_menu->current_item = main_menu->current_item->get_next();
                    print_menu();
                    break;
                case NES_LEFT: // Left Key
                    main_menu->current_item = main_menu->current_item->get_parent();
                    print_menu();
                    break;
                case NES_A: // Select Key
                    change_current_action(main_menu->current_item->get_action());
                    current_action(INIT, 0);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

// Dialogs
void save_changes_dialog(ModeOption mode, uint8_t input)
{
    static SelectionOption selection = NO;

    if (mode == INIT)
    {
        lcd.clear();
        lcd.print(F("Save changes"));
        lcd.setCursor(0, 1);
        lcd.print(F(" Yes  No"));
        lcd.write(uint8_t(0));
        selection = NO;
    }

    switch (input)
    {
        case NES_RIGHT: // Right Key
            lcd.setCursor(4, 1);
            lcd.print(" ");
            lcd.setCursor(8, 1);
            lcd.write(uint8_t(0));
            selection = NO;
            break;
        case NES_LEFT: // Left Key
            lcd.setCursor(8, 1);
            lcd.print(" ");
            lcd.setCursor(4, 1);
            lcd.write(uint8_t(0));
            selection = YES;
            break;
        case NES_A: // Select Key
            lcd.clear();
            if (selection == YES)
            {
                lcd.print(F("Changes saved"));
                delay(2000);
                change_current_action(last_action);
                current_action(RET_YES, 0);
            }
            else
            {
                lcd.print(F("Changes aborted"));
                change_current_action(last_action);
                current_action(RET_NO, 0);
            }
            break;
        default:
            break;
    }
}

void Display(ModeOption mode, uint8_t input)
{
    time_t now;

    switch (mode)
    {
        case INIT:
            lcd.clear();
            break;
        case LOOP:
            switch (input)
            {
                case NES_LEFT: case NES_B:
                    change_current_action(&navigate_menu);
                    current_action(INIT, 0);
                    break;
                default:
                    now = getTimeDS1307();

                    lcd.setCursor(0, 0);
                    print_time(now);

                    status = get_humidity_temperature();

                    if (status == 0)
                    {
                        lcd.print(F(" T:"));
                        lcd.print(T_C, 1);
                        lcd.setCursor(0, 1);
                        lcd.print(F("RH:"));
                        print3digits(RH);
                        lcd.print(F(" RHsp:"));
                        print3digits(RH_sp);
                    }
                    else
                    {
                        lcd.setCursor(0, 1);
                        lcd.print(F("Failed:"));
                        lcd.print(status);
                    }
                    break;
            }
            break;
        default:
            break;
    }
}

// Manually change the humidity
void change_humidity(ModeOption mode, uint8_t input)
{
    static uint8_t RH_sp_new;

    switch (mode)
    {
        case INIT:
            RH_sp_new = RH_sp;

            lcd.clear();
            lcd.print(F("Change Humidity "));
            lcd.setCursor(0, 1);
            lcd.print(F("RH:"));
            print3digits(RH_sp_new);
            break;

        case RET_YES:
            RH_sp = RH_sp_new;
            change_current_action(&navigate_menu);
            current_action(INIT, 0);
            break;

        case RET_NO:
            change_current_action(&navigate_menu);
            current_action(INIT, 0);
            break;

        case LOOP:
            switch (input)
            {
                case NES_UP: // Up Key
                    RH_sp_new++;
                    if (RH_sp_new > 100) RH_sp_new = 100;
                    lcd.setCursor(3, 1);
                    print3digits(RH_sp_new);
                    break;
                case NES_DOWN: // Down Key
                    if (RH_sp_new > 0) RH_sp_new -= 1;
                    lcd.setCursor(3, 1);
                    print3digits(RH_sp_new);
                    break;
                case NES_A: case NES_B:
                    change_current_action(&save_changes_dialog);
                    change_last_action(&change_humidity);
                    current_action(INIT, 0);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

// Add Events and navigate events list
Event_Handler * event_handler = new Event_Handler();

void navigate_events(ModeOption mode, uint8_t input);

void add_event_prompt(ModeOption mode, uint8_t input)
{
    static time_t t;
    static tmElements_t event_t;
    static uint8_t selection;
    static float RH_sp_new;

    switch (mode) {
        case INIT: 
            {
            t = getTimeDS1307();
            breakTime(t, event_t);
            selection = 1;
            RH_sp_new = 0;

            lcd.clear();
            print_date_time(t);
            lcd.setCursor(0, 1);
            lcd.print(F("RH: "));
            print3digits(RH_sp_new);
            }
            break;
            
        case LOOP:
            switch (input) {
                case NES_RIGHT: // Right Key
                    selection++;
                    if (selection > 6) selection = 6;
                    break;
                case NES_UP: // Up Key
                    switch (selection) {
                        case 1: // Hour selected
                            event_t.Hour++;
                            if (event_t.Hour > 24) event_t.Hour = 24;
                            break;
                        case 2: // Minute selected
                            event_t.Minute++;
                            if (event_t.Minute > 60) event_t.Minute = 60;
                            break;
                        case 3: // Month selected
                            event_t.Month++;
                            if (event_t.Month > 12) event_t.Month = 12;
                            break;
                        case 4: // day selected
                            event_t.Day++;
                            if (event_t.Day > 31) event_t.Day = 31;
                            break;
                        case 5: // Year selected
                            event_t.Year++;
                            break;
                        case 6: // RH selected
                            if (RH_sp_new < 100) RH_sp_new += 5;
                            break;
                        default:
                            break;
                    }
                    lcd.setCursor(0, 0);
                    t = makeTime(event_t);
                    print_date_time(t);
                    lcd.setCursor(4, 1);
                    print3digits(RH_sp_new);
                    break;
                case NES_DOWN: // Down Key
                    switch (selection)
                    {
                        case 1: // Hour selected
                            event_t.Hour--;
                            if (event_t.Hour < 0) event_t.Hour = 0;
                            break;
                        case 2: // Minute selected
                            event_t.Minute--;
                            if (event_t.Minute < 0) event_t.Minute = 0;
                            break;
                        case 3: // Month selected
                            event_t.Month--;
                            if (event_t.Month < 0) event_t.Month = 0;
                            break;
                        case 4: // day selected
                            event_t.Day--;
                            if (event_t.Day < 0) event_t.Day = 0;
                            break;
                        case 5: // Year selected
                            event_t.Year--;
                            break;
                        case 6: // RH selected
                            if (RH_sp_new > 0) RH_sp_new -= 5;
                            break;
                        default:
                            break;
                    }
                    lcd.setCursor(0, 0);
                    t = makeTime(event_t);
                    print_date_time(t);
                    lcd.setCursor(4, 1);
                    print3digits(RH_sp_new);
                    break;
                case NES_LEFT: // Left Key
                    selection--;
                    if (selection < 1) selection = 1;
                    break;
                case NES_A: case NES_B: // Select Key
                    change_current_action(&save_changes_dialog);
                    change_last_action(&add_event_prompt);
                    current_action(INIT, 0);
                    break;
                default:
                    break;
            }
            break;
            
        case RET_YES:
            {
            Event * event = new Event();
            time_t t = makeTime(event_t);
            event->add_time(t);
            event->add_set_point(RH_sp_new);
            event_handler->add_event(event);

            change_current_action(&navigate_events);
            current_action(INIT, 0);
            }
            break;
        case RET_NO:
            change_current_action(&navigate_events);
            current_action(INIT, 0);
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
            lcd.write(uint8_t(0));
            lcd.print("RH:");
            print3digits(event_handler->current_event->get_set_point());
            lcd.print(" ");
            print_date_time(event_handler->current_event->get_time());
            // lcd.setCursor(15, 0);
            // lcd.write(uint8_t(0));
            lcd.setCursor(0, 1);
            lcd.print(" ");
            lcd.print("RH:");
            print3digits(event_handler->current_event->get_next()->get_set_point());
            lcd.print(" ");
            print_date_time(event_handler->current_event->get_next()->get_time());
        }
        else if (event_handler->current_event->get_previous())
        {
            lcd.print(" ");
            lcd.print("RH:");
            print3digits(event_handler->current_event->get_previous()->get_set_point());
            lcd.print(" ");
            print_date_time(event_handler->current_event->get_previous()->get_time());
            lcd.setCursor(0, 1);
            lcd.write(uint8_t(0));
            lcd.print("RH:");
            print3digits(event_handler->current_event->get_set_point());
            lcd.print(" ");
            print_date_time(event_handler->current_event->get_time());
            // lcd.setCursor(15, 1);
            // lcd.write(uint8_t(0));
        }
        else
        {
            lcd.write(uint8_t(0));
            lcd.print("RH:");
            print3digits(event_handler->current_event->get_set_point());
            lcd.print(" ");
            print_date_time(event_handler->current_event->get_time());
            // lcd.setCursor(15, 0);
            // lcd.write(uint8_t(0));
        }
    }
}

void add_remove_exit_prompt(ModeOption mode, uint8_t input);

void navigate_events(ModeOption mode, uint8_t input)
{
    static uint8_t scroll_pos;
    uint8_t scroll_offset = 5;
    uint8_t scroll_len = 10;
    switch (mode)
    {
        case INIT:
            event_handler->current_event = event_handler->first_event;
            print_events();
            scroll_pos = 0;
            break;
        case LOOP:
            switch (input)
            {
                case NES_UP: // Up Key
                    if (event_handler->current_event->get_previous())
                    {
                        event_handler->current_event = event_handler->current_event->get_previous();
                        lcd.home();
                        print_events();
                    }
                    scroll_pos = 0;
                    break;
                case NES_DOWN: // Down Key
                    if (event_handler->current_event->get_next())
                    {
                        event_handler->current_event = event_handler->current_event->get_next();
                        lcd.home();
                        print_events();
                    }
                    scroll_pos = 0;
                    break;
                case NES_A: // Select Key - add, remove, or exit
                    change_current_action(&add_remove_exit_prompt);
                    current_action(INIT, 0);
                    break;
                case NES_B: case NES_LEFT:
                    change_current_action(&navigate_menu);
                    current_action(INIT, 0);
                    break;
                default:
                    if (scroll_pos < scroll_offset) {
                        scroll_pos++;
                    }
                    else if (scroll_pos < scroll_len + scroll_offset) {
                        lcd.scrollDisplayLeft();
                        scroll_pos++;
                    }
                    else {
                        navigate_events(INIT, 0);
                    }
                    break;
            }
            break;
        default:
            break;
    }
}

void add_remove_exit_prompt(ModeOption mode, uint8_t input)
{
    static SelectionOption selection;

    switch (mode)
    {
        case INIT:
            lcd.clear();
            lcd.print(F("Add      Remove "));
            lcd.setCursor(0, 1);
            lcd.print(F("      Exit      "));
            lcd.setCursor(10, 1);
            lcd.write(uint8_t(0));
            selection = EXIT;
            break;
        case LOOP:
            switch (input)
            {
                case NES_RIGHT: // Right key
                    if (selection == ADD)
                    {
                        lcd.setCursor(3, 0);
                        lcd.print(" ");
                        lcd.setCursor(15, 0);
                        lcd.write(uint8_t(0));
                        selection = REMOVE;
                    }
                    break;
                case NES_UP: // Up key
                    if (selection == EXIT)
                    {
                        lcd.setCursor(10, 1);
                        lcd.print(" ");
                        lcd.setCursor(3, 0);
                        lcd.write(uint8_t(0));
                        selection = ADD;
                    }
                    break;
                case NES_DOWN: // Down key
                    if ((selection == ADD) || (selection == REMOVE))
                    {
                        lcd.setCursor(3, 0);
                        lcd.print(" ");
                        lcd.setCursor(15, 0);
                        lcd.print(" ");
                        lcd.setCursor(10, 1);
                        lcd.write(uint8_t(0));
                        selection = EXIT;
                    }
                    lcd.setCursor(10, 1);
                    lcd.write(uint8_t(0));
                    break;
                case NES_LEFT: // Left key
                    if (selection == REMOVE)
                    {
                        lcd.setCursor(15, 0);
                        lcd.print(" ");
                        lcd.setCursor(3, 0);
                        lcd.write(uint8_t(0));
                        selection = ADD;
                    }
                    break;
                case NES_A: // Select Key
                    switch (selection)
                    {
                        case ADD:
                            change_current_action(&add_event_prompt);
                            current_action(INIT, 0);
                            break;
                        case REMOVE:
                            event_handler->remove_event(event_handler->current_event);
                            change_current_action(&navigate_events);
                            current_action(INIT, 0);
                            break;
                        case EXIT:
                            change_current_action(&navigate_menu);
                            current_action(INIT, 0);
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
        default:
            break;
    }
}

void remove_all(ModeOption mode, uint8_t input)
{
    event_handler->remove_all_events();
    change_current_action(&navigate_events);
    current_action(INIT, 0);
}

void event_wizard(ModeOption mode, uint8_t input)
{
    static uint8_t RH_sp_start;
    static uint8_t RH_sp_end;

    static uint8_t steps;

    static uint16_t t_dwell;

    static uint8_t selection;

    switch (mode)
    {
        case INIT:
            RH_sp_start = RH_sp_end = 0;
            steps = 1;
            t_dwell = 60;
            selection = 1;
            lcd.clear();
            lcd.print("RH Start:");
            print3digits(RH_sp_start);
            break;
        case LOOP:
            switch (input)
            {
                case NES_UP: // Up Key
                    switch (selection)
                    {
                        case 1: 
                            if (RH_sp_start < 100) RH_sp_start +=5;
                            lcd.setCursor(9,0);
                            print3digits(RH_sp_start);
                            break;
                        case 2: 
                            if (RH_sp_end < 100) RH_sp_end +=5;
                            lcd.setCursor(8,0);
                            print3digits(RH_sp_end);
                            break;
                        case 3:
                            if (steps < 100) steps++;
                            lcd.setCursor(6, 0);
                            print3digits(steps);
                            break;
                        case 4: 
                            t_dwell++;
                            lcd.setCursor(12,0);
                            print3digits(t_dwell);
                            break;
                    }
                    break;
                case NES_DOWN: 
                    switch (selection)
                    {
                        case 1: 
                            if (RH_sp_start > 0) RH_sp_start -=5;
                            lcd.setCursor(9,0);
                            print3digits(RH_sp_start);
                            break;
                        case 2: 
                            if (RH_sp_end > 0) RH_sp_end -=5;
                            lcd.setCursor(8,0);
                            print3digits(RH_sp_end);
                            break;
                        case 3:
                            if (steps > 0) steps--;
                            lcd.setCursor(6, 0);
                            print3digits(steps);
                            break;
                        case 4: 
                            if (t_dwell > 0) t_dwell--;
                            lcd.setCursor(12,0);
                            print3digits(t_dwell);
                            break;
                        default:
                            break;
                    }
                    break;
                case NES_LEFT: // Left Key
                    if (selection > 1) {
                        selection--;
                        switch (selection) 
                        {
                            case 1:
                                lcd.clear();
                                lcd.print("RH Start:");
                                print3digits(RH_sp_start);
                                break;
                            case 2:
                                lcd.clear();
                                lcd.print("RH Stop:");
                                print3digits(RH_sp_start);
                                break;
                            case 3:
                                lcd.clear();
                                lcd.print("Steps:");
                                print3digits(steps);
                                break;
                            case 4:
                                lcd.clear();
                                lcd.print("Dwell (min):");
                                print3digits(t_dwell);
                                break;
                            default:
                                break;
                        }
                    }
                    break;
                case NES_RIGHT: // Right Key
                    if (selection < 4) {
                        selection++;
                        switch (selection) 
                        {
                            case 1:
                                lcd.clear();
                                lcd.print("RH Start:");
                                print3digits(RH_sp_start);
                                break;
                            case 2:
                                lcd.clear();
                                lcd.print("RH Stop:");
                                print3digits(RH_sp_end);
                                break;
                            case 3:
                                lcd.clear();
                                lcd.print("Steps:");
                                print3digits(steps);
                                break;
                            case 4:
                                lcd.clear();
                                lcd.print("Dwell (min):");
                                print3digits(t_dwell);
                                break;
                            default:
                                break;
                        }
                    }
                    break;
                case NES_A: // Select Key
                    if (selection == 4) {
                        change_current_action(&save_changes_dialog);
                        change_last_action(&event_wizard);
                        current_action(INIT, 0);
                    }
                    else if (selection < 4) {
                        selection++;
                        switch (selection) 
                        {
                            case 1:
                                lcd.clear();
                                lcd.print("RH Start:");
                                print3digits(RH_sp_start);
                                break;
                            case 2:
                                lcd.clear();
                                lcd.print("RH Stop:");
                                print3digits(RH_sp_start);
                                break;
                            case 3:
                                lcd.clear();
                                lcd.print("Steps:");
                                print3digits(steps);
                                break;
                            case 4:
                                lcd.clear();
                                lcd.print("Dwell (min):");
                                print3digits(t_dwell);
                                break;
                            default:
                                break;
                        }
                    }
                    break;
                default:
                    break;
            }
            break;
        case RET_YES:
            {
                event_handler->remove_all_events();
                uint8_t RH_sp_new = RH_sp_start;
                time_t t = getTimeDS1307() + 10;

                int8_t RH_step = (RH_sp_end - RH_sp_start) / steps;

                for (uint8_t i = 0; i < (steps+1); i++) 
                {
                    Event * event = new Event();
                    event->add_time(t);
                    event->add_set_point(RH_sp_new);
                    event_handler->add_event(event);

                    t += t_dwell*60;
                    RH_sp_new += RH_step;
                }

                change_current_action(&navigate_events);
                current_action(INIT, 0);
                
            }
            break;
        case RET_NO:
            change_current_action(&navigate_events);
            current_action(INIT, 0);
            break;
        default:
            break;
    }
}

void calibration(ModeOption mode, uint8_t input)
{
    static uint16_t addr;

    static uint8_t RH_cal;
    static uint8_t valve1_cal;
    static uint8_t valve2_cal;

    switch (mode)
    {
        case INIT:
            addr = 0;

            RH_cal = 0;
            valve1_cal = 255;
            valve2_cal = 255;

            lcd.clear();
            lcd.print("Set RH using");
            lcd.setCursor(0, 1);
            lcd.print("Valve 1 and 2");
            delay(2000);
            lcd.clear();
            lcd.print("L/R for V1");
            lcd.setCursor(0, 1);
            lcd.print("U/D for V2");
            delay(2000);
            lcd.clear();
            lcd.print("Set RH = ");
            print3digits(RH_cal);
            delay(2000);
            lcd.clear();
            lcd.print("RH:");
            print3digits(RH);
            lcd.print(" Goal:");
            print3digits(RH_cal);
            lcd.setCursor(0, 1);
            lcd.print("V1:");
            print3digits(valve1_cal);
            lcd.print(" V2:");
            print3digits(valve2_cal);
            break;
        case LOOP:
            status = get_humidity_temperature();

            switch (input)
            {
                case NES_RIGHT:
                    if (valve1_cal < 255) valve1_cal++;
                    lcd.setCursor(3, 1);
                    print3digits(valve1_cal);
                    break;
                case NES_UP:
                    if (valve2_cal < 255) valve2_cal++;
                    lcd.setCursor(10, 1);
                    print3digits(valve2_cal);
                    break;
                case NES_DOWN:
                    if (valve2_cal > 0) valve2_cal--;
                    lcd.setCursor(10, 1);
                    print3digits(valve2_cal);
                    break;
                case NES_LEFT:
                    if (valve1_cal > 0) valve1_cal--;
                    lcd.setCursor(3, 1);
                    print3digits(valve1_cal);
                    break;
                case NES_A:
                    current_action(RET_YES, 0);
                    break;
                default:
                    break;
            }
            analogWrite(valve1_pin, valve1_cal);
            analogWrite(valve2_pin, valve2_cal);

            break;
        case RET_YES:
            EEPROM.write(addr, valve1_cal);
            addr++;
            EEPROM.write(addr, valve2_cal);
            addr++;
            if (RH_cal == 100)
            {
                change_current_action(&navigate_menu);
                current_action(INIT, 0);
            }
            else 
            {
                RH_cal += 5;

                lcd.clear();
                lcd.print("Set RH = ");
                lcd.print(RH_cal);
                delay(2000);
                lcd.clear();
                lcd.print("RH:");
                print3digits(RH);
                lcd.print(" Goal:");
                print3digits(RH_cal);
                lcd.setCursor(0, 1);
                lcd.print("V1:");
                print3digits(valve1_cal);
                lcd.print(" V2:");
                print3digits(valve2_cal);
            }
            break;
        case RET_NO:
            change_current_action(&navigate_menu);
            current_action(INIT, 0);
            break;
        default:
            break;
    }
}

void manual_valve(ModeOption mode, uint8_t input)
{
    switch (mode)
    {
        case INIT:
            lcd.clear();
            lcd.print("L/R for V1");
            lcd.setCursor(0, 1);
            lcd.print("U/D for V2");
            delay(2000);
            lcd.clear();
            lcd.print("RH:");
            print3digits(RH);
            lcd.print(" ");
            lcd.setCursor(0, 1);
            lcd.print("V1:");
            print3digits(valve1_pos);
            lcd.print(" V2:");
            print3digits(valve2_pos);
            break;
        case LOOP:
            switch (input)
            {
                case NES_RIGHT:
                    if (valve1_pos < 255) valve1_pos++;
                    lcd.setCursor(3, 1);
                    print3digits(valve1_pos);
                    break;
                case NES_UP:
                    if (valve2_pos < 255) valve2_pos++;
                    lcd.setCursor(10, 1);
                    print3digits(valve2_pos);
                    break;
                case NES_DOWN:
                    if (valve2_pos > 0) valve2_pos--;
                    lcd.setCursor(10, 1);
                    print3digits(valve2_pos);
                    break;
                case NES_LEFT:
                    if (valve1_pos > 0) valve1_pos--;
                    lcd.setCursor(3, 1);
                    print3digits(valve1_pos);
                    break;
                case NES_A: case NES_B:
                    change_current_action(&navigate_menu);
                    current_action(INIT, 0);
                    return;
                    break;
                default:
                    break;
            }

            lcd.setCursor(3, 0);
            print3digits(RH);
            lcd.print(" ");

            analogWrite(valve1_pin, valve1_pos);
            analogWrite(valve2_pin, valve2_pos);
            break;
        default:
            break;
    }

}

void setup()
{
    // Initialize LCD screen
    lcd.begin(16, 2);
    lcd.createChar(0, left_arrow);

    // Initialize I2C library for communication with Sensor and RTC
    Wire.begin();

    // Initialize valves to default position
    analogWrite(valve1_pin, valve1_pos);
    analogWrite(valve2_pin, valve2_pos);

    // Welcome Message
    lcd.setCursor(0, 0);
    lcd.print(F("Humidity Control"));
    lcd.setCursor(0, 1);
    lcd.print(F("by Michal Nicpon"));
    delay(2000);

    Item * display_item = new Item("Display         ");
    display_item->add_action(&Display);
    Item * change_humidity_item = new Item("Change Humidity ");
    change_humidity_item->add_action(&change_humidity);
    Item * events_item = new Item("Events          ");
    events_item->add_action(&navigate_events);
    Item * event_wizard_item = new Item("Event Wizard    ");
    event_wizard_item->add_action(&event_wizard);
    Item * remove_all_item = new Item("Remove All Events");
    remove_all_item->add_action(&remove_all);
    Item * manual_mode_item = new Item("Manual Mode     ");
    manual_mode_item->add_action(&manual_valve);
    Item * calibration_item = new Item("Calibration Mode");
    calibration_item->add_action(&calibration);
    
    main_menu->add_item_down(display_item);
    main_menu->add_item_down(change_humidity_item);
    main_menu->add_item_down(events_item);
    main_menu->add_item_down(event_wizard_item);
    main_menu->add_item_down(remove_all_item);
    main_menu->add_item_down(manual_mode_item);
    main_menu->add_item_down(calibration_item);

    pinMode(valve1_pin, OUTPUT);
    pinMode(valve2_pin, OUTPUT);

    /*
    // Initialize SD card
    pinMode(10, OUTPUT);

    if (!SD.begin(10))
    {
        lcd.setCursor(0, 0);
        lcd.print(F("Card failed, or not present"));
        delay(3000);
        return;
    }

    print_free_mem(__LINE__);

    lcd.setCursor(0, 0);
    lcd.print(F("Card initialized."));
    delay(2000);

    if (SD.exists("data.log"))
    {
        SD.remove("data.log");
    }

    File dataFile = SD.open("data.log", FILE_WRITE);
    if (dataFile)
    {
        dataFile.println("Time,RH,T_C");
        dataFile.close();
    }
    else
    {
        lcd.clear();
        lcd.print(F("Create log fail"));
    }

    print_free_mem(__LINE__);
    */

    change_current_action(&navigate_menu);
    current_action(INIT, 0);
}

void loop()
{
    static time_t now;
    static uint8_t key;
    static uint8_t last_key;
    static uint32_t last_key_time;
    static uint8_t num_key_hold;

    now = getTimeDS1307();
    
    if (!event_handler->is_empty()) {
        if (now > event_handler->first_event->get_time()) {
            RH_sp = event_handler->first_event->get_set_point();
            change_valve_position(RH_sp);
            event_handler->remove_event(event_handler->first_event);
        }
    }

    last_key = key;
    key = nintendo.buttons();
    if (key) { 
        if (key==last_key) {
            if (num_key_hold < 4) {
                current_action(LOOP, key);
                delay(200);
                num_key_hold += 1;
            }
            else if (num_key_hold >= 4) {
                current_action(LOOP, key);
                delay(50);
                num_key_hold += 1;
            }
        }    
        else {
            current_action(LOOP, key);
            delay(200);
        }
    }
    else if ((millis() - last_key_time) > 500) {
        last_key_time = millis();
        current_action(LOOP, key);
    }
    else {
        num_key_hold = 0;
    }

    status = get_humidity_temperature();
    
    //String dataString = "1";
    /*
    dataString += String(unixTime(now));
    dataString += ",";
    char * cString = (char *) malloc(sizeof(char)*10); 
    dtostrf((double) RH, 4, 1, cString);
    dataString += cString;
    free(cString);
    dataString += ",";
    cString = (char *) malloc(sizeof(char)*10); 
    dtostrf((double) T_C, 4, 1, cString);
    dataString += cString;
    free(cString);
    */
    
    /*
    File dataFile = SD.open("data.log", FILE_WRITE);
    if (dataFile)
    {
        dataFile.println(dataString);
        dataFile.close();

        lcd.clear();
        lcd.print(F("Write Successful"));
        delay(2000);
        print_free_mem(__LINE__);
    }
    else
    {
        lcd.clear();
        lcd.print(F("Error"));
        delay(2000);
        print_free_mem(__LINE__);
    }
    */

}

