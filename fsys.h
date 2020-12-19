/*
    David Monk        - DM584
    Operating Systems - MCOMD3PST
    Assignment 2      - 2019-20
*/
#define BLOCKSIZE 512

int diskSpace[1000000]; // Persistant array keeps track of how much space is being used in each block (Up to 512MB can be stored)
unsigned int fileSize;  // Global Variable for sharing data between two functions
char tempString[512];   // Re-usable char array for log outputting

int updateLatestBlock(int latest);         // Save earliest block number with less than 512 bytes to firstAvailableBlock.txt
int getFileSize(const char *longFilePath); // Obtain size of a file in bytes
int getBlockNumber(char *blockNumber);     // Pass a string formatted like so, 'BLOCK000051', and return an int of '51'
void makeDirectory(const char *name);      // Create a directory either in windows or linux
int findFirstSpace();                      // Find first available block that data can be stored

void makeDirectory(const char *name)
{
#ifdef _WIN32
    _mkdir(name);
#else
    mkdir(name, 0750);
#endif
}

void saveSpaceArray()
{
    FILE *f = fopen("spaceArray", "wb");
    fwrite(diskSpace, sizeof(char), sizeof(diskSpace), f);
    fclose(f);
}

void addToLog(char *logText)
{
    makeDirectory("LOGS");
    char buf[256] = {0};
    time_t rawtime = time(NULL);
    struct tm *ptm = localtime(&rawtime);
    strftime(buf, 256, "LOGS/%Y-%m-%d.txt", ptm);
    FILE *logFile = fopen(buf, "a");
    strftime(buf, 256, "[%d/%m/%Y %H:%M:%S]", ptm);
    fprintf(logFile, "%s - %s\n", buf, logText);
    fclose(logFile);
}

void createBackendFiles() //Create all neccessary backend file and directories if they don't exist
{
    makeDirectory("FILEINDEX");
    makeDirectory("DISK");
    makeDirectory("DOCUMENTS");
    if (fileExists("FILEINDEX/fileIndex.txt") == 1)
    {
        FILE *file = fopen("FILEINDEX/fileIndex.txt", "w");
        fclose(file);
    }
    if (fileExists("firstAvailableBlock.txt") == 1)
    {
        FILE *file = fopen("firstAvailableBlock.txt", "w");
        fprintf(file, "0");
        fclose(file);
    }
    if (fileExists("spaceArray") == 0)
    {
        FILE *file = fopen("spaceArray", "r");
        fseek(file, 0, SEEK_END);
        long len = ftell(file);
        fseek(file, 0, SEEK_SET);
        fread(diskSpace, 1, len, file);
        fclose(file);
    }
}

void showHelp()
{
    fprintf(stderr, "\nHELP: How to use fsys.\n\
    This program has just 3 functions, store, retrieve, and delete\n\
    Usage examples:\n\
    store:      ./fsys store DOCUMENTS/[filename]\n\
    retrieve:   ./fsys retrieve [filename]\n\
    delete:     ./fsys delete [filename]\n\
    \n\
    If a file has already been 'stored' with the same filename as the one you are trying to store, \n\
    an option for overwriting will be displayed. A 'y' or 'n' is required at this point.\
    ");
}

int findFile(const char *fileName)
{
    int returnInt = 1;
    char *param2;
    param2 = strdup(fileName);
    FILE *file = fopen("FILEINDEX/fileIndex.txt", "r");
    char line[10000];
    int param2Length = strlen(param2);

    // Go line by line looking for the filename. If found returns 0 else returns 1.
    while (fgets(line, sizeof(line), file) != NULL)
    {
        char *asterisk;
        int index;
        asterisk = strchr(line, 42); //ascii number for *
        index = (int)(asterisk - line);

        char temp[index + 1];
        temp[index] = '\0';
        int i;
        for (i = 0; i < index; i++)
        {
            temp[i] = line[i];
        }
        if (strcmp(param2, temp) == 0)
        {
            returnInt = 0;
            break;
        }
    }
    fclose(file);
    return returnInt;
}

unsigned char *readFileFromBlock(const char *fileName) // Read data in blocks to a char array and return it
{
    //unsigned char arrays are used so I can store any filetype, not just .txt.
    unsigned char *dataFromDisk;
    char *param2;
    param2 = strdup(fileName);
    FILE *file = fopen("FILEINDEX/fileIndex.txt", "r");
    char line[1000];
    char *asterisk;
    int index;
    int loopCount = 0;
    unsigned long offset = 0;

    // read line by line of fileIndex
    while (fgets(line, sizeof(line), file) != NULL)
    {
        // Store all text before the * in a line as temp
        asterisk = strchr(line, 42);
        index = (int)(asterisk - line);
        char temp[index + 1];
        temp[index] = '\0';
        int i;
        for (i = 0; i < index; i++)
        {
            temp[i] = line[i];
        }

        if (strcmp(param2, temp) == 0)
        {
            // If temp and param2(fileName) are equal, the first line will contain the size of the file for memory allocation
            // e.g. letter2.txt*1798
            if (loopCount < 1)
            {
                char size[100];

                int lineSize = strlen(line);
                int indexNumber;
                int k = 0;
                for (indexNumber = index + 1; indexNumber < lineSize - 1; indexNumber++)
                {
                    size[k++] = line[indexNumber];
                }
                fileSize = atol(size);
                dataFromDisk = (char *)malloc(fileSize + 1);
                dataFromDisk[fileSize] = '\0';
            }
            else
            {
                // For each following line read the file located in block stated, then placing it in the char arrray offset by
                // the amount already read.
                // e.g. letter2.txt*BLOCK000000
                char blockNumber[100];
                int lineSize = strlen(line);
                int indexNumber;
                int k = 0;
                for (indexNumber = index + 1; indexNumber < lineSize - 1; indexNumber++)
                {
                    blockNumber[k++] = line[indexNumber];
                }
                char buffer[500];
                snprintf(buffer, sizeof(buffer), "DISK/%s/%s\0", blockNumber, param2);
                FILE *file = fopen(buffer, "rb");
                fseek(file, 0, SEEK_END);
                unsigned long len = ftell(file);
                fseek(file, 0, SEEK_SET);
                fread(dataFromDisk + offset, len, sizeof(unsigned char), file);
                offset = offset + len;
                fclose(file);
            }
            loopCount++;
        }
    }
    fclose(file);
    return dataFromDisk;
}

unsigned char *readFileBytes(const char *fileName) //read file to char array
{
    FILE *file = fopen(fileName, "rb");
    fseek(file, 0, SEEK_END);
    long len = ftell(file);
    unsigned char *data = malloc(len + 1);
    data[len] = '\0';
    fseek(file, 0, SEEK_SET);
    fread(data, 1, len, file);
    fclose(file);
    return data;
}

int updateLatestBlock(int latest) //store number(earliest available block) in txt file
{
    FILE *file = fopen("firstAvailableBlock.txt", "w");
    fprintf(file, "%d", latest);
    fclose(file);
}

int getFileSize(const char *longFilePath) //retrieve a file's size
{
    FILE *file = fopen(longFilePath, "r");
    int sz;
    fseek(file, 0L, SEEK_END);
    sz = ftell(file);
    fclose(file);
    return sz;
}

char *getFileName(const char *filePath) // Remove file path, returning just the filename
{
    char *buf = malloc(400);
    strcpy(buf, filePath);
    char *fileName;
    int delimiter = '/';
    fileName = strrchr(buf, delimiter);
    return fileName + 1;
}

int getBlockNumber(char *blockNumber) // Pass a string formatted like so, 'BLOCK000051', and return an int of '51'
{
    char *ptr;
    int ch = 'K';
    ptr = strrchr(blockNumber, ch);
    int number = atoi(ptr + 1);
    return number;
}

int storeFile(const char *longFilePath, int blockStart)
{
    char *fileName;
    fileName = getFileName(longFilePath);
    int sizeOfFile = getFileSize(longFilePath);
    int FileNameStringLength = strlen(fileName);

    FILE *fileIndex = fopen("FILEINDEX/fileIndex.txt", "a"); // Append new file 'stored in' data to fileIndex
    fwrite(fileName, 1, FileNameStringLength, fileIndex);
    fprintf(fileIndex, "*%d\n", sizeOfFile);
    char *bytestream = readFileBytes(longFilePath);

    int filePart = 0;
    int i = sizeOfFile;
    int amountWritten = 0;
    int j = blockStart;
    char buf[256];
    while (i > 0)
    {
        int written = 0;
        while (written == 0)
        {
            int amntToWrite;
            int availableSpace = BLOCKSIZE - diskSpace[j];
            if (i <= availableSpace)
            {
                amntToWrite = i;
            }
            else
            {
                amntToWrite = availableSpace;
            }
            if (diskSpace[j] == 0) // if there is no thing saved in this block number, create a directory(block)
            {
                snprintf(buf, sizeof(buf), "DISK/BLOCK%06d", j);
                makeDirectory(buf);
            }
            if (diskSpace[j] < BLOCKSIZE) // if there is space in the block number, start saving how much space is available in the block
            {
                snprintf(buf, sizeof(buf), "DISK/BLOCK%06d/%s", j, fileName);
                FILE *fp2 = fopen(buf, "w");
                fwrite(bytestream + (sizeOfFile - i), 1, amntToWrite, fp2);
                fclose(fp2);
                fprintf(fileIndex, "%s*BLOCK%06d\n", fileName, j);
                i = i - amntToWrite;
                diskSpace[j] = diskSpace[j] + amntToWrite;
                amountWritten += amntToWrite;
                snprintf(tempString, sizeof(tempString), "%s: part %d: %d bytes - Saved in DISK/BLOCK%06d (informational)\0", fileName, ++filePart, amntToWrite, j);
                addToLog(tempString);
                j++;
                written = 1;
            }
            else //if no space available in block, try the next one.
            {
                j++;
            }
        }
    }

    if (j - 1 < BLOCKSIZE) // if there was space left in the last block written to, set latest block as this
    {
        updateLatestBlock(j - 1);
    }
    else // else, find the first available block. 
    {
        updateLatestBlock(findFirstSpace());
    }

    fclose(fileIndex);

    if (sizeOfFile == amountWritten) // check that the correct amount of bytes were saved
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

int findFirstSpace() // In case a file fills up exactly 512 bytes, this is called to find the first available block space.
{
    int i;

    for (i = 0; i < sizeof(diskSpace); i++)
    {
        if (diskSpace[i] < 512)
        {
            return i;
        }
    }
}

int outputFile(unsigned char *bytestream, const char *fileName) // Save the char array as a file in OUTPUT
{
    makeDirectory("OUTPUT");
    char buf[10000];
    snprintf(buf, sizeof(buf), "OUTPUT/%s", fileName);
    FILE *fp2 = fopen(buf, "wb");
    fwrite(bytestream, fileSize, 1, fp2);
    fclose(fp2);
    if (getFileSize(buf) == fileSize)
        return 0;
    else
        return 1;
}

int fileExists(const char *file)
{
    if (access(file, F_OK) != -1)
        return 0;
    else
        return 1;
}

int deleteFile(const char *fileName)
{
    char *fileIndex = malloc(getFileSize("FILEINDEX/fileIndex.txt"));
    char *param2;
    param2 = strdup(fileName);
    FILE *file = fopen("FILEINDEX/fileIndex.txt", "r");
    char line[1000];
    char *asterisk;
    int index;
    int loopCount = 0;
    int fileIndexIndex = 0;
    while (fgets(line, sizeof(line), file) != NULL) // Go through fileIndex and find entries that match filename
    {
        asterisk = strchr(line, 42);
        index = (int)(asterisk - line);
        char temp[index + 1];
        temp[index] = '\0';
        int i;
        for (i = 0; i < index; i++)
        {
            temp[i] = line[i];
        }

        if (strcmp(param2, temp) == 0) // If filename matches delete the file from stated block and remove filesize from diskspace array
        {
            if (loopCount > 0)
            {
                char blockNumber[100];
                int lineSize = strlen(line);
                int indexNumber;
                int k = 0;
                for (indexNumber = index + 1; indexNumber < lineSize - 1; indexNumber++)
                {
                    blockNumber[k++] = line[indexNumber];
                }

                int blockNumberint = getBlockNumber(blockNumber);
                char *fabString = readFileBytes("firstAvailableBlock.txt");
                int firstAvailableBlock = atoi(fabString);
                if (loopCount == 1 && firstAvailableBlock > blockNumberint)
                {
                    updateLatestBlock(blockNumberint);
                }
                char buffer[500];
                snprintf(buffer, sizeof(buffer), "DISK/%s/%s", blockNumber, param2);
                diskSpace[blockNumberint] = diskSpace[blockNumberint] - getFileSize(buffer);

                remove(buffer);
                if (diskSpace[blockNumberint] == 0) // If not files in block, delete block
                {
                    snprintf(buffer, sizeof(buffer), "DISK/%s", blockNumber, param2);
                    remove(buffer);
                }
            }
            loopCount++;
        }
        else // for each line in fileIndex if filename not matched, add this line to fileIndex char array, thus removing deleted file from fileIndex 
        {
            int k;
            for (k = 0; k < strlen(line); k++) 
            {
                fileIndex[fileIndexIndex + k] = line[k];
            }
            fileIndexIndex += strlen(line);
        }
    }
    fileIndex[fileIndexIndex] = '\0';
    fclose(file);
    file = fopen("FILEINDEX/fileIndex.txt", "w");
    fputs(fileIndex, file);
    fclose(file);

    if (findFile(fileName) == 1)
        return 0;
    else
        return 1;
}