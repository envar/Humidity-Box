#include "Time.h"

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


class Event_Handler
{
    private:
        Event * last_event;
        Event * events;
        int event_counter;
    public:
        Event_Handler();
        Event * first_event;
        Event * current_event;
        bool is_empty();
        void add_event(Event * event);
        void remove_event(Event * event);
};

Event_Handler::Event_Handler()
{
    event_counter = 0;
}

bool Event_Handler::is_empty()
{
    if (event_counter)
    {
        return false;
    }
    else
    {
        return true;
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

        for (int i = 0; i < n; i++)
        {
            if (event->get_time() < current_event->get_time())
            {
                first_event->add_previous(event);
                event->add_next(first_event);
                first_event = event;
                event_counter++;
                break;
            }
            else if (event->get_time() > last_event->get_time())
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
        event->get_previous()->add_next(event->get_next());
        event->get_next()->add_previous(event->get_previous());
    }
    else if (event->get_next() == 0) // Check if event is the last event
    {
        event->get_previous()->add_next(0);
    }

    delete event;
    event_counter--;
}

