#include "Time.h"
#include "LiquidCrystal.h"
#include "Button.h"

class Event
{
    private:
        Event * previous_event;
        Event * next_event;
        float set_point;
        time_t event_time;
        bool save_event_dialog();
        
    public:
        Event();
        ~Event();
        void add_next(Event * next);
        void add_previous(Event * previous);
        Event * get_next();
        Event * get_previous();
        void add_time(time_t t);
        time_t get_time();
        void add_set_point(float sp);
        float get_set_point();
        int check();
};

Event::Event()
{
    previous_event = 0;
    next_event = 0;
}

Event::~Event()
{
}

void Event::add_next(Event * next)
{
    next_event = next;
}

void Event::add_previous(Event * previous)
{
    previous_event = previous;
}

Event * Event::get_next()
{
    return next_event;
}

Event * Event::get_previous()
{
    return previous_event;
}

void Event::add_time(time_t t)
{
    event_time = t;
}

time_t Event::get_time()
{
    return event_time;
}

void Event::add_set_point(float sp)
{
    set_point = sp;
}

float Event::get_set_point()
{
    return set_point;
}

int Event::check()
{
    time_t current_time = time(NULL);

    if (current_time > event_time)
    {                             // TODO: Must have a way to change valve
        return 1;
    }
    else 
    {
        return 0;
    }
}

class Event_Handler
{
    private:
        Event * current_event;
        Event * first_event;
        Event * last_event;
        Event * events;
        time_t t;
        time_t new_t;
        tmElement_t tm;
        float RH_new;
        void (*current_action)(int);
        int selection;
        bool first_time; // keeps track if the function is being run for the first time

        int event_counter;
        void print2digits();
        void print_time(time_t t);
        void print_time(tmElement_t tm);
        void add_remove_prompt();
        void add_event_();

    public:
        Event_Handler();
        int add_remove_exit_prompt(int input);
        void add_event(Event * event);
        void remove_event(Event * event);
        void print_events();
        void navigate(int input);
        void change_current_action(void (*action)(int));
}

Event_Handler::Event_Handler()
{
    event_counter = 0;
    first_time = true;
    change_current_action(&navigate);
}

void Event_Handler::print2digits(int number)
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

void Event_Handler::add_event(Event * event)
{
    if (event_counter == 0)
    {
        first_event = last_event = current_event = event;
        event_counter++;
    }
    else
    {
        current_event = first_event;

        int n = event_counter;

        for (i = 0; i < n; i++)
        {
            if (event->get_time() < current_event->get_time())
            {
                first_event->add_previous(event);
                event->add_next(first_event);
                first_event = event;
                event_counter++;
                break;
            }
            else if (event->get_time() > last_event->get_time()
            {
                last_event->add_next(event);
                event->add_previous(last_event);
                last_event = event;
                event_counter++;
                break;
            }
            else
            {
                if (event->get_time() < current_event->get_next()->get_time())
                {
                    current_event->get_next()->add_previous(event);
                    event->add_next(current_event->get_next());
                    current_event->add_next(event);
                    event->add_previous(current_event);
                    event_counter++;
                    break;
                }
                else
                {
                    current_event = current_event->get_next();
                }
            }
        }
    }
}

void Event_Handler::remove_event(Event * event)
{
    if(event->get_previous() == 0) // Check if this is the first event
    {
        event->get_next()->add_previous(0);
    }
    else if ((event->get_previous() != 0) && (event->get_next() != 0)) // Check if event in middle
    {
        event->get_previous->add_next(event->get_next());
        event->get_next()->add_previous(event->get_previous());
    }
    else if (event->get_next() == 0) // Check if event is the last event
    {
        event->get_previous()->add_next(0);
    }

    delete event;
    event_counter--;
}

void Event_Handler::print_time(time_t t)
{
    tmElements_t tm;
    breaktime(t, tm);

    print2digits(tm.Hour);
    lcd.print(":");
    print2digits(tm.Minute);
    lcd.print(" ");
    print2digits(tm.Month);
    lcd.print("/");
    print2digits(tm.Day);
    lcd.print("/");
    print2digits(tm.Year % 100);
}

void Event_Handler::print_time(tmElements_t tm)
{
    print2digits(tm.Hour);
    lcd.print(":");
    print2digits(tm.Minute);
    lcd.print(" ");
    print2digits(tm.Month);
    lcd.print("/");
    print2digits(tm.Day);
    lcd.print("/");
    print2digits(tm.Year % 100);
}

void Event_Handler::print_events()
{
    lcd.clear();
    print_time(current_event->get_time());
    lcd.print(" RH:");
    lcd.print(current_event->get_set_point(), 0);
    lcd.setCursor(16, 0);
    lcd.print("<");
    lcd.setCursor(0, 1);
    print_time(current_event->get_next()->get_time());
    lcd.print(" RH:");
    lcd.print(current_event->get_next()->get_set_point(), 0); 
}

void Event_Handler::navigate(int input=-1)
{
    if (virgin)
    {
        virgin = false;
    }

    switch (input)
    {
        case -1:
            current_event = first_event;
            print_events();
            break;
        case 1: // Up Key
            current_event = current_event->get_previous();
            print_events();
            break;
        case 2: // Down Key
            current_event = current_event->get_next();
            print_events();
            break;
        case 4: // Select Key - add, remove, or exit
            virgin = true;
            selection = 3;
            change_current_action(&add_remove_exit_prompt);
            current_action();
            break;
        default:
            break;
    }
}

int Event_Handler::add_remove_exit_prompt(int input=-1)
{
    if (virgin)
    {
        lcd.clear();
        lcd.print("Add      Remove ");
        lcd.setCursor(0, 1);
        lcd.print("      Exit      ");

        virgin = false;
        selection = 3;
    } 

    // Selection: 1 is add, 2 is remove, 3 is exit

    switch (input)
    {
        case -1:
            lcd.setCursor(10, 1);
            lcd.print("<");
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
                    virgin = true;
                    selection = 1;
                    change_current_action(&add_event_prompt);
                    break;
                case 2:
                    virgin = true;
                    remove_event(current_event);
                    change_current_action(&navigate);
                    navigate();
                    break;
                case 3:
                    virgin = true;
                    change_current_action(&navigate);
                    navigate();
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

int Event_Handler::add_event_prompt(int input=-1)
{
    if (virgin)
    {
        selection = 1;
        if (RTC.get)
        {
            t = now();
            breakTime(t, tm); // break time into a structure of elements
        }
        RH_new = 0;

        lcd.clear();
        print_time();
        lcd.setCursor(0, 1);
        lcd.print("RH: ");
        lcd.print(RH_new, 0);
    }
    
    switch (input)
    {
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
                    tm.Day;
                    if (tm.Day > 31) tm.Day = 31;
                    break;
                case 5: // year selected
                    tm.Year++;
                    break;
                case 6: // RH selected
                    RH_new = RH_new + 1;
                    break;
                default:
                    break;
            }
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
                    RH_new = RH_new - 1;
                    break;
                default:
                    break;
            }
        case 3: // Left Key
            selection--;
            if (selection < 1) selection = 1;
            break;
        case 4: // Select Key
            bool choice = save_event_dialog();
            if (choice == true)
            {
                new_t = makeTime(tm);
                Event * event = new Event();
                event->add_time(new_t);
                event->add_set_point(RH_new);
                Event_Handler->add_event(event);
            }
            break;
        default:
            break;
    }
}

void Event_Handler::change_current_action(void (*action)(int))
{
    current_action = action;
}
