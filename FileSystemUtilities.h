#include <FS.h> // Library to use the filesystem

class FileSystemUtilities
{
public:
    boolean init()
    {
        return SPIFFS.begin();
    }

public:
    String readFile(fs::FS &fs, const char *path) // read the content of a file
    {
        Serial.printf("Reading file: %s\r\n", path); // Debug message
        File file = fs.open(path, "r");              // Open the file
        if (!file || file.isDirectory())             // If the file doesn't exist or is a directory
        {
            Serial.println("- empty file or failed to open file"); // Debug message
            return String();                                       // Return an empty string
        }
        Serial.println("- read from file:"); // Debug message
        String fileContent;                  // Create a string to hold the content of the file
        while (file.available())             // While there is data to read
        {
            fileContent += String((char)file.read()); // Add the character to the string
        }
        file.close();                // Close the file
        Serial.println(fileContent); // Debug message
        return fileContent;          // Return the content of the file
    }

public:
    void writeFile(fs::FS &fs, const char *path, const char *message) // write to a file
    {
        Serial.printf("Writing file: %s\r\n", path); // Debug message
        File file = fs.open(path, "w");              // Open the file
        if (!file)                                   // If the file doesn't exist
        {
            Serial.println("- failed to open file for writing"); // Debug message
            return;
        }
        if (file.print(message)) // Write the message to the file
        {
            Serial.println("- file written"); // Debug message
        }
        else
        {
            Serial.println("- write failed"); // Debug message
        }
        file.close(); // Close the file
    }
};