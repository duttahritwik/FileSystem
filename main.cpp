#include<iostream>
#include<string>
#include<vector>
#include<unordered_map>
using namespace std;
class File {
    public:
        int blockNumberPreviousBlock; // 4 bytes
        int blockNumberNextBlock; // 4 bytes
        string userData; // 504 bytes
        string filePath;
        File(string filePath, string fileData) {
            this -> blockNumberPreviousBlock = this -> blockNumberPreviousBlock = 0;
            this -> userData = fileData.substr(0, 504);
            this -> filePath = filePath;
        }
};
class SubDirectory {
    public:
        char directoryType; // F - Free, D - Directory, U - User Data
        string directoryName; // Name of Directory
        int blockNumber; // block number of first block of file
        string directoryPath;
        vector<File> fileList;
        SubDirectory (string directoryName, int blockNumber, char directoryType, string directoryPath) {
            this -> directoryType = directoryType; // initially all directories are unassigned/free
            this -> directoryName = directoryName;
            this -> blockNumber = blockNumber;
            this -> directoryPath = directoryPath;
            this -> fileList = vector<File> ();
        }
        SubDirectory(){}
};
class Directory {
    public:
        int backBlock;
        int secondaryDirectoryBlock;
        int firstUnusedBlock;
        bool unusedDirectory;
        bool isRootDirectory;
        vector<SubDirectory> directoryList;
        Directory (bool isRoot = false) {
           this -> isRootDirectory = isRoot;
           this -> unusedDirectory = true;
           this -> backBlock = this -> secondaryDirectoryBlock = this -> firstUnusedBlock = 0;
           this -> directoryList = vector<SubDirectory> ();
        }
};
// MainMemory class is used just to simulate and allocate memory,
// actual storage occurs in the objects of type File and Directory
class MainMemory {
    public:
    vector<vector<bool>> memoryBlocks; // this is to allocate memory
    // directory/file name : { block number in memory, start address of block, end address of block }
    unordered_map<string, vector<int> > memoryTracker;
    MainMemory() {
        // true means initially all blocks are unallocated/empty.
        this -> memoryBlocks.resize(100, vector<bool> (512, true));
    }
};
 
MainMemory mainMemory;
Directory rootDirectory = new Directory(true);
 
bool checkForDirectory(string directoryName) {
    vector<SubDirectory> directoriesInRoot = rootDirectory.directoryList;
    
    for(auto directory : directoriesInRoot) {
        string nameOfDirectory = directory.directoryName;
        
        if (nameOfDirectory == directoryName)
            return true;
    }
    
    return false;
}

vector<string> splitString(string filePath, char delimiter) {
    vector<string> result;
    string subPath = "";
    char currentChar;
    
    for (int i = 1; i < filePath.size(); ++i) {
        
        currentChar = filePath[i];
        
        if (currentChar == delimiter) {
            result.push_back(subPath);
            subPath = "";
        }
        else {
            subPath.push_back(currentChar);
        }
    }
    
    result.push_back(subPath);
    return result;
}
 
int checkForFreeMemorySpace() {
    // if there is no free space in memory, return -1
    // otherwise return the block number of the first free block
    
    vector<vector<bool>> memoryBlocks = mainMemory.memoryBlocks;
    bool memoryAvailable = false;
    int firstFreeBlock = -1;
    
    for(int i = 0; i < memoryBlocks.size(); i++)
        if (memoryBlocks[i][0] == true) {
            memoryAvailable = true;
            firstFreeBlock = i;
            break;
        }
    
    return firstFreeBlock;
}

void updateMemory(int firstFitBlock, string directoryName, string fileName, string pathOfCurrentFile) {
    // allocate the memory
    for(int i = 0; i < mainMemory.memoryBlocks[firstFitBlock].size(); i++)
        mainMemory.memoryBlocks[firstFitBlock][i] = false;
    
    // update the hashmap to reflect which file/directory is allocated which block
    mainMemory.memoryTracker[pathOfCurrentFile] = { firstFitBlock, 0, 511 };
}

void createFileInDirectory(string fileName, string directoryName, string userData, string pathOfCurrentDirectory, string pathOfCurrentFile) {
    // creates file in directory "directoryName", assuming "directoryName" already exists
    int firstFitBlock = checkForFreeMemorySpace();
    if (firstFitBlock == -1)
        cout<<"Memory is full!\n";
    else {
        
        updateMemory(firstFitBlock, directoryName, fileName, pathOfCurrentFile);
        
        for(auto &subdirectory : rootDirectory.directoryList) {
            if (subdirectory.directoryName == directoryName) {
                File newFile = File(pathOfCurrentFile, userData);
                subdirectory.fileList.push_back(newFile);
            }
        }
    }
}
 
void createFileWithDirectory(string fileName, string directoryName, string userData, string pathOfCurrentDirectory, string pathOfCurrentFile) {
    // creates file "fileName" in directory "directoryName", while also creating directory "directoryName"
    int firstFitBlock = checkForFreeMemorySpace();
    if (firstFitBlock == -1)
        cout<<"Memory is full!\n";
    else {
        
        updateMemory(firstFitBlock, directoryName, fileName, pathOfCurrentFile);
        
        // update the rootDirectory object and directoryList vector
        rootDirectory.firstUnusedBlock = firstFitBlock + 1;
        rootDirectory.unusedDirectory = false;
        
        // update the File and SubDirectory objects to reflect the changes
        SubDirectory subDirectory = SubDirectory(directoryName, firstFitBlock, 'U', pathOfCurrentDirectory);
        File file = File(pathOfCurrentFile, userData);
        subDirectory.fileList.push_back(file);
        rootDirectory.directoryList.push_back(subDirectory);
    }
}

string readFile(string fileName) {
    // return contents of file "fileName"
    string fileContent = "";
    vector<SubDirectory> directoriesInRoot = rootDirectory.directoryList;
    
    for(auto subdirectory : directoriesInRoot) {
        vector<File> fileList = subdirectory.fileList;
        for(auto file : fileList) {
            if (file.filePath.size()) {
                vector<string> filePath = splitString(file.filePath, '/');
                string existingFileName = filePath[filePath.size() - 1];
                
                if (existingFileName == fileName) {
                    fileContent = file.userData;
                    break;
                }
            }
        }
    }
    
    if (!fileContent.size())
        cout<<"File "<<fileName<<" not found\n";
    return fileContent;
}

void writeOrAppendToFile(string fileName, string fileNewContent, bool isAppend = false) {
    bool fileFound = false;
    
    for(auto &directory : rootDirectory.directoryList) {
        for(auto &file : directory.fileList) {
            
            vector<string> filePath = splitString(file.filePath, '/');
            string existingFileName = filePath[filePath.size() - 1];
            
            if (existingFileName == fileName) {
                fileFound = true;
                if (isAppend)
                    file.userData += fileNewContent;
                else
                    file.userData = fileNewContent;
            }
        }
    }
    
    if (fileFound)
        cout<<"Contents of file "<<fileName<<" changed to "<<readFile(fileName)<<endl;
    else
        cout<<"File "<<fileName<<"  not found\n";
}

void deleteFile(string fileName) {
    // delete the file
    int directoryIndex = -1, fileIndex = -1;
    
    for(int i = 0; i < rootDirectory.directoryList.size(); i++) {
        for(int j = 0; j < rootDirectory.directoryList[i].fileList.size(); j++) {

            vector<string> filePath = splitString(rootDirectory.directoryList[i].fileList[j].filePath, '/');
            string existingFileName = filePath[filePath.size() - 1];
            
            if (existingFileName == fileName) {
                directoryIndex = i;
                fileIndex = j;
                break;
            }
        }
    }
    
    if (fileIndex == -1 || directoryIndex == -1)
        cout<<"File "<<fileName<<" not found\n";
    else {
        rootDirectory.directoryList[directoryIndex].fileList.erase(rootDirectory.directoryList[directoryIndex].fileList.begin() + fileIndex);
        cout<<"File "<<fileName<<" deleted\n";
    }
}

string seekFile(string fileName, int offset) {
    // returns the contents of the file from offset to the end of the file.
    string fileContent = readFile(fileName);
    if (!fileContent.size()) {
        cout<<"File "<<fileName<<" does not exits\n";
        return "";
    }
    else {
        auto bytesInFile = fileContent.size();
        if (offset >= bytesInFile) {
            cout<<"Offset is too large, end of file reached\n";
            return "";
        }
        return fileContent.substr(offset + 1, fileContent.size() - 1);
    }
}

void createNewFileInDirectory(string fileName, string directoryName, string userData) {
    
    // steps to creating a file
    // first check if directory "directoryName" exists
    // if it exists, create file "fileName" in directory "directoryName"
    string pathOfCurrentDirectory = "root/" + directoryName;
    string pathOfCurrentFile = pathOfCurrentDirectory + "/" + fileName;
    
    if (checkForDirectory(directoryName))
        createFileInDirectory(fileName, directoryName, userData, pathOfCurrentDirectory, pathOfCurrentFile);
    // else create new directory "directoryName" and create file "fileName" in directory just created
    else
        createFileWithDirectory(fileName, directoryName, userData, pathOfCurrentDirectory, pathOfCurrentFile);
}

int main () {
    vector<vector<string>> commands {{"file1", "directory1", "AAAA"},
                                     {"file2", "directory1", "BBBB"},
                                     {"file3", "directory1", "IIII"},
                                     {"file4", "directory2", "CCCC"},
                                     {"file5", "directory2", "DDDD"},
                                     {"file6", "directory2", "GGGG"},
                                     {"file7", "directory2", "HHHH"},
                                     {"file8", "directory3", "EEEE"},
                                     {"file9", "directory3", "FFFF"},
                                     };
    
    for(int i = 0; i < commands.size(); i++)
        createNewFileInDirectory(commands[i][0], commands[i][1], commands[i][2]);
    
    return 0;
}



