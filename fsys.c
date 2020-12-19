/*
    David Monk        - DM584
    Operating Systems - MCOMD3PST
    Assignment 2      - 2019-20
*/
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include "fsys.h"

int outputFile(unsigned char *bytestream, const char *fileName); // Pass an array of data and save as a file to OUTPUT folder with passed filename
int storeFile(const char *longFilePath, int blockStart);         // Split file into chunks and store up to 512 bytes in next available block, starting with passed block number
unsigned char *readFileFromBlock(const char *fileName);          // Read a specific file from all blocks and compile, in order, into an unsigned char array
unsigned char *readFileBytes(const char *fileName);              // Read a file into an unsigned char array
char *getFileName(const char *filePath);                         // Pass filepath 'DOCUMENTS/letter1.txt' return filename 'letter1.txt
int deleteFile(const char *fileName);                            // Remove fileparts from blocks, remove empty blocks, remove appropriate data from diskSpace array, remove entry from fileIndex
int findFile(const char *fileName);                              // See if file exists in fileIndex.txt/ has been saved previously
int fileExists(const char *file);                                // See if file exists
void addToLog(char *logText);                                    // Add string to dated log with appropriate timestamps.
void createBackendFiles();                                       // Create neccesary files and directories. fileIndex.txt, DISK/, DOCUMENTS/, SpaceArray etc
void saveSpaceArray();                                           // Save state of diskspace array.
void showHelp();                                                 // Shows help in case of incorrect usage

int main(int argc, const char *argv[])
{
    int errorCode = 0;
    // create neccesary files and directories
    // fileIndex.txt, DISK/, DOCUMENTS/, SpaceArray, firstAvailableBlock.txt
    createBackendFiles();
    if (argc < 3)
    {
        // print help message to stderr if not enough arguments passed
        showHelp();
        // add log entry
        addToLog("Program called without enough arguments (error)");
        errorCode = 1;
    }
    else
    {
        // give parameters better variable names
        char *function;
        function = strdup(argv[1]);
        int i;
        for (i = 0; function[i]; i++) // Convert to lowercase
        {
            function[i] = tolower(function[i]);
        }
        char *file;
        file = strdup(argv[2]);

        // if-else statement to control flow of the program based on commmand line arguments

        // Store function
        if (strcmp(function, "store") == 0)
        {
            // read from a txt file that contains the number of the first available block
            char *temp = readFileBytes("firstAvailableBlock.txt");
            int firstAvailableBlock = atoi(temp);

            // Check that the file is being read from the DOCUMENTS directory.
            // The reason this doesn't read straight from the documents folder is so that tab autocomplete can be used on the command line
            char *locationCheck = strstr(file, "DOCUMENTS/");
            if (locationCheck == NULL)
            {
                fprintf(stderr, "Cannot read file from this directory. Please use DOCUMENTS/...");
                snprintf(tempString, sizeof(tempString), "%s is not in correct directory (error)\0", file);
                addToLog(tempString);
                errorCode = 1;
            }
            else
            {
                // getFilename returns char's after last slash/
                // e.g. if file = DOCUMENTS/letter2.txt
                // then fileName = letter2.txt
                char *fileName = getFileName(file);

                // Check if chosen file exists
                if (fileExists(file) == 1)
                {
                    fprintf(stderr, "File not found");
                    snprintf(tempString, sizeof(tempString), "%s not found (error)\0", file);
                    addToLog(tempString);
                    errorCode = 1;
                }
                // check file is not listed in FileIndex.txt
                else if (findFile(fileName) == 1)
                {
                    if (storeFile(file, firstAvailableBlock) == 0)
                    {
                        printf("Stored");
                        snprintf(tempString, sizeof(tempString), "%s stored correctly (informational)\0", file);
                        addToLog(tempString);
                    }
                    else
                    {
                        fprintf(stderr, "Error: File stored incorrectly");
                        snprintf(tempString, sizeof(tempString), "%s not stored correctly (error)\0", file);
                        addToLog(tempString);
                        errorCode = 1;
                    }
                }
                // if file already saved trigger overrite message
                else
                {
                    printf("File exists. Did you want to overwrite? y/n \n");
                    char userInput[1];
                    // get user input
                    scanf("%s", &userInput);
                    if (userInput[0] == 'y')
                    {
                        // delete currently stored file
                        if (deleteFile(fileName) == 0)
                        {
                            snprintf(tempString, sizeof(tempString), "Overwrite of %s: delete successful (informational)\0", file);
                            addToLog(tempString);

                            // get first available block
                            char *temp = readFileBytes("firstAvailableBlock.txt");
                            int firstAvailableBlock = atoi(temp);

                            // store new file
                            if (storeFile(file, firstAvailableBlock) == 0)
                            {
                                printf("Overwrite Complete");
                                snprintf(tempString, sizeof(tempString), "%s overwritten successfully (informational)\0", file);
                                addToLog(tempString);
                            }
                            else
                            {
                                // storing of new file error.
                                fprintf(stderr, "Overwrite Failed");
                                snprintf(tempString, sizeof(tempString), "Overwrite of %s not successful : Store (error)\0", file);
                                addToLog(tempString);
                                errorCode = 1;
                            }
                        }
                        else
                        {
                            // deletion of currently stored file error
                            fprintf(stderr, "Error: Overwrite not successful");
                            snprintf(tempString, sizeof(tempString), "Overwrite of %s not successful : Delete (error)\0", file);
                            addToLog(tempString);
                            errorCode = 1;
                        }
                    }
                    else
                    {
                        printf("Overwrite cancelled");
                        snprintf(tempString, sizeof(tempString), "Overwrite of %s cancelled (informational)\0", file);
                        addToLog(tempString);
                    }
                }
            }
        }
        // Retrieve Function
        else if (strcmp(function, "retrieve") == 0)
        {
            // check to see if file has previously been saved
            if (findFile(file) == 0)
            {
                // read file from blocks and output to OUTPUT directory.
                if (outputFile(readFileFromBlock(file), file) == 0)
                {
                    printf("Retrieved.\nFile saved at OUTPUT/%s", file);
                    snprintf(tempString, sizeof(tempString), "%s retrieved successfully (informational)\0", file);
                    addToLog(tempString);
                }
                else
                {
                    fprintf(stderr, "Error: Retrieve Failed");
                    snprintf(tempString, sizeof(tempString), "Retrieval of %s not successful (error)\0", file);
                    addToLog(tempString);
                    errorCode = 1;
                }
            }
            else
            {
                    fprintf(stderr, "Error: Retrieve Failed: File not found");
                    snprintf(tempString, sizeof(tempString), "Retrieval of %s not successful: File not found (error)\0", file);
                    addToLog(tempString);
                    errorCode = 1;
            }
        }
        // Delete Function
        else if (strcmp(function, "delete") == 0)
        {
            // check if file has previously been saved
            if (findFile(file) == 0)
            {
                // delete file
                if (deleteFile(file) == 0)
                {
                    printf("Deleted");
                    snprintf(tempString, sizeof(tempString), "%s deleted successfully (informational)\0", file);
                    addToLog(tempString);
                }
                else
                {
                    fprintf(stderr, "Error: Delete not successful");
                    snprintf(tempString, sizeof(tempString), "%s not deleted successfully (error)\0", file);
                    addToLog(tempString);
                    errorCode = 1;
                }
            }
            else
            {
                fprintf(stderr, "File not found");
                snprintf(tempString, sizeof(tempString), "%s not deleted : File Not Found (error)\0", file);
                addToLog(tempString);
                errorCode = 1;
            }
        }
        // Default case
        else
        {
            showHelp();
            addToLog("Program called with unknown arguments (error)");
            errorCode = 1;
        }
    }
    printf("\n");
    // Save state of diskspace array.
    saveSpaceArray();

    return errorCode;
}