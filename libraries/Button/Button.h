// Set up buttons
int key = -1;
int oldkey = -1;

// Convert ADC value to key number
int get_key(int input)
{
    int adc_key_val[5] = {50, 200, 400, 600, 800};
    int NUM_KEYS = 5;
    
    int k;
    for (k = 0; k < NUM_KEYS; k++)
    {
      if (input < adc_key_val[k])
      {
        return k;
      }
    }   
    if (k >= NUM_KEYS)
    {
        k = -1;  // No valid key pressed
    }
    return k;
}

// Check if a button has been pressed
int check_button()
{  
    int key;
    int adc_key_in;

    adc_key_in = analogRead(0);     // read the value from the sensor 
    key = get_key(adc_key_in);      // convert into key press
    if (key != oldkey)              // if keypress is detected
    {
        delay(50);                  // wait for debounce time
        adc_key_in = analogRead(0); // read the value from the sensor 
        key = get_key(adc_key_in);  // convert into key press
        if (key != oldkey)    
        {   
            oldkey = key;
        }
    }
    return key;
}

