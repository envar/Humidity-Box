#include "Arduino.h"
#include "LiquidCrystal.h"

// Set up LCD
LiquidCrystal lcd(8, 13, 9, 4, 5, 6, 7);

#include "Menu.h"
#include "DS1307RTC.h"
#include "Time.h"
#include "Wire.h"
#include "math.h"
#include "Button.h"

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

// initiate menu
Menu * main_menu = new Menu();

// get the temperature and humidity
byte get_humidity_temperature()
{
    byte Hum_H, Hum_L, Temp_H, Temp_L, status;
    unsigned int H_dat, T_dat;

    Wire.beginTransmission(HIH6130_address);
    Wire.write(0);
    Wire.endTransmission();
    delay(100);
    
    Wire.requestFrom(HIH6130_address, 4);
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

void Display(int input=-1)
{
    // digital clock display of the time
    time_t t = now();
    tmElements_t tm;
    breakTime(t, tm);

    lcd.clear();
    print2digits(tm.Hour);
    lcd.print(":");
    print2digits(tm.Minute);
    lcd.print(":");
    print2digits(tm.Second);
            
    status = get_humidity_temperature();
    
    if (status == 0)
    {
        lcd.setCursor(0, 1);
        lcd.print("RH:");
        lcd.print(RH, 1);
        lcd.print("T:");
        lcd.print(T_C, 1);
    }
    else
    {
        lcd.setCursor(0, 1);
        lcd.print("failed: ");
        lcd.print(status);
    }

    if (input == 3)
    {
        main_menu->change_current_action(main_menu::navigate);
    }
}

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

void change_humidity(int input=-1)
{
    switch (input)
    {
        case -1:
            RH_sp_new = RH_sp;

            lcd.clear();
            lcd.print("Change Humidity ");
            lcd.setCursor(0, 1);
            lcd.print("RH:");
            lcd.print(RH_sp_new, 0);
            break;
        case 1: // Up Key
            RH_sp_new++;
            if (RH_sp_new > 100) RH_sp_new = 100;
            lcd.setCursor(3, 1);
            lcd.print(RH_sp_new, 0);
            break;
        case 2: // Down Key
            RH_sp_new--;
            if (RH_sp_new < 0) RH_sp_new = 0;
            lcd.setCursor(3, 1);
            lcd.print(RH_sp_new, 0);
            break;
        case 4: // Select Key
            main_menu->change_current_action(&(main_menu->save_changes_dialog));
            main_menu->change_last_action(change_humidity);
            break;
        case 9: // Recieved selection from menu save dialog
            if (main_menu->get_selection() == 1) // If "yes"
            {
                RH_sp = RH_sp_new; // commit change
            }
            main_menu->change_current_action(main_menu->navigate);
 
            break;
        default:
            break;
    }


}

void setup()
{
    setSyncProvider(RTC.get);
    lcd.begin(16, 2);
    lcd.print("Hello World");
    delay(5000);
    Wire.begin();

    Item * display_item = new Item("Display         ");
    display_item->add_action(&Display);
    Item * change_humidity_item = new Item("Change Humidity ");
    change_humidity_item->add_action(&change_humidity);
    //Item * events_item = new Item("Events          ");
    
    main_menu->add_item_down(display_item);
    main_menu->add_item_down(change_humidity_item);
    //main_menu->add_item_down(events_item);
    
    main_menu->navigate_to_top();

    pinMode(valve1_pin, OUTPUT);
    pinMode(valve2_pin, OUTPUT);
}



void loop()
{
    static const int where_am_I = 1; // 1 is main menu;
                                     // 2 is event_handler
    check_valves();

    key = check_button();
    if (key >= 0)
    {
        main_menu->current_action(key);
    }

    delay(100);
}


