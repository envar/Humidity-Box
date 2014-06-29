#ifndef _DataLog_H
#define _DataLog_H

#ifndef _CONFIG_H_

// Ensures that these defines ar available if the Config.h file is not present. These should be moved to their own file if you are modularizing

#define DISABLE_SDCARD_WRITING 1
#define DELETE_DATAFILE_ON_STARTUP 1
#define SERIAL_PRINT_ENABLE 1

#endif

class DataLog {
	public:
		DataLog(char * filename); // Adds the filename and deletes the old one if enabled
		template <class T> void addData(T data); // Add the data to the packet
		void addData(float data); // Add float type data
		void addData(double data); // Add double type data
		int saveData(); // Save the data to the SD card
		String packet; // Use of this object does cause a memory allocation whenever new data is added to the string, this results in the length of the data packet to now become limited to the size of the available ram rather than the 
	private:
		static char FileName[50]; // Storage for the filename
};
	
char DataLog::FileName[50]; // The filename written to the SD card, defined on startup

DataLog::DataLog(char * filename)
{
	// Save the filename
	for(uint16_t i = 0; i < strlen(filename)*sizeof(char); i++)
		FileName[i] = filename[i];
	
	// Reset the packet
	packet = "";
	
	// Delete the old file if the feature is enabled
	#if DELETE_DATAFILE_ON_STARTUP && !DISABLE_SDCARD_WRITING
	while(SD.exists(FileName))
	{
		/* If so then delete it */
		#if SERIAL_PRINT_ENABLE
		Serial.print(FileName);
		Serial.println(" already exists...DELETING");
		#endif
			
		SD.remove(FileName);
	}
	#endif
		
	#if SERIAL_PRINT_ENABLE && !DISABLE_SDCARD_WRITING
	Serial.println("Data init.");
	#endif
}

template <class T> void DataLog::addData(T data)
{
	// Add generic type data
	packet += String(data);
	packet += ","; // Add a comma
		
	#if SERIAL_PRINT_ENABLE
	Serial.println("Data added.");
	#endif
}
	
void DataLog::addData(float data)
{
	addData((double) data); // Cast the float as a double
}
	
void DataLog::addData(double data)
{
	// Convert a double to a cString with dtostrf on runtime
	char * cString = (char *) malloc(sizeof(char)*50); // Allocate memory to a C string
		
	dtostrf(data, 5, 4, cString); // Convert the inputted double into the C string
		
	addData(cString); // Add the C string to the data packet
	
	free(cString); // Unallocate the memeory for the C string
}

int DataLog::saveData()
{
	if(DISABLE_SDCARD_WRITING)
	{
		packet = ""; // Clear the packet string
		return 0;
	}
	
	#if SERIAL_PRINT_ENABLE
	Serial.print("Data save started: ");
	Serial.println(FileName);
	#endif
	
	// Open the file
	File dataFile = SD.open(FileName, FILE_WRITE);
		
	if(dataFile)
	{
		// Adding a packet length as the check digit to ensure that all data was written to the sd card sucesfully that was contained in the packet string
		packet += packet.length();
		packet += ",";
		
		// Write data to file
		dataFile.println(packet);
		dataFile.close(); // Close file
			
		packet = ""; // Resset packet
			
		#if SERIAL_PRINT_ENABLE
		Serial.println("Data save finished.");
		#endif
		
		return 0;
	}
	else
	{
		#if SERIAL_PRINT_ENABLE
		Serial.println("Data save failed.");
		#endif
		
		dataFile.close(); // Close the file
			
		return 1;
	}
}

#endif

