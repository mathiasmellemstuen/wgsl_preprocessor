#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <map>
#include <stack>

bool strContains(const std::string& a_str, const std::string& a_contains)
{
    return a_str.find(a_contains) != std::string::npos; 
}

std::string extractBetweenQuotes(const std::string& a_input) {
    size_t start = a_input.find('"');
    size_t end = a_input.find('"', start + 1);

    if (start != std::string::npos && end != std::string::npos && end > start) {
        return a_input.substr(start + 1, end - start - 1);
    }

    return "";
}

std::vector<std::string> readFileLines(const std::string& a_filepath)
{
    std::ifstream file(a_filepath);
    std::vector<std::string> lines; 

    if(!file)
    {
        std::cout << "Could not open file" << a_filepath << std::endl; 
        exit(1); 
    }

    std::string line; 
    while(std::getline(file, line))
    {
        lines.push_back(line); 
    }

    return lines; 
}


std::vector<std::string> readIncludeFiles(const std::string& a_filepath, std::vector<std::string>& a_processedLines)
{
    std::vector<std::string> processedLines; 

    std::vector<std::string> currentFileLinesUnprocessed = readFileLines(a_filepath);

    for(const std::string& line : currentFileLinesUnprocessed)
    {
        // We filter out the comments
        if(strContains(line, "//"))
        {
            continue; 
        }

        if(strContains(line, "#include"))
        {
            std::string relativeFilePath = extractBetweenQuotes(line); 

            // Did not contain anything between quotes
            if(relativeFilePath == "")
            {
                continue; 
            }
        }
    }

    return processedLines; 
}

std::string excludeFileFromPath(const std::string& a_filePath)
{
    size_t pos = a_filePath.find_last_of("/"); 


    if(pos != std::string::npos)
    {
        std::string dir = a_filePath.substr(0, pos + 1);
        return dir;
    }

    // No directory part found!
    return ""; 
}

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;

    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}

/*
    Arguments:
    wgl file path
    wgl out file path
*/
int main(int argc, char* argv[])
{

    if(argc <= 2)
    {
        std::cout << "Wrong argument count!" << std::endl; 
        return 1; 
    }

    std::string inPath = std::string(argv[1]);
    std::string outPath = std::string(argv[2]);

    std::cout << "Processing input file: " << inPath << std::endl; 

    std::map<std::string, std::string> defines;

    std::vector<std::string> content = readFileLines(inPath); 
    std::string currentFilePath = inPath;

    enum class IfType
    {
        IFDEF,
        IFNDEF
    };

    struct IfData
    {
        size_t line; 
        IfType type; 
    };

    std::stack<IfData> ifStack; 

    // Handling #include statements
    for(size_t i = 0; i < content.size();)
    {
        // We filter out the comments
        // This is for the case where we comment out an include
        if(strContains(content[i], "//"))
        {
            i++;
            continue; 
        }

        if(strContains(content[i], "#include"))
        {
            std::string relativeFilePath = extractBetweenQuotes(content[i]); 

            // Did not contain anything between quotes
            if(relativeFilePath == "")
            {
                continue; 
            }

            std::string newFilePath = excludeFileFromPath(currentFilePath) + relativeFilePath; 
            std::vector<std::string> newFileContent = readFileLines(newFilePath);
            
            // Erasing the line where the #include statement is on
            content.erase(content.begin() + i); 

            // Adding the file content to that line
            content.insert(content.begin() + i, newFileContent.begin(), newFileContent.end()); 

            // We need to start this loop over again
            i = 0; 
            continue; 
        }
        i++; 
    }

    // Handling #ifdef, #ifndef and #define statements
    for(size_t i = 0; i < content.size();)
    {
        // Handling #endif
        if(strContains(content[i], "#endif"))
        {
            if(ifStack.size() >= 1)
            {
                // We pop the last value in the ifStack
                ifStack.pop(); 
            }
            i++;
            continue; 
        }

        // Handling #ifdef
        if(strContains(content[i], "#ifdef") || strContains(content[i], "#ifndef"))
        {
            std::vector<std::string> strSplitted = split(content[i], ' ');
            
            bool valueFalse = (defines.find(strSplitted[1]) == defines.end() || defines[strSplitted[1]] == "0" || defines[strSplitted[1]] == "false"); 

            if((strContains(content[i], "#ifdef") &&  valueFalse) || (strContains(content[i], "#ifndef") && !valueFalse))
            {
                ifStack.push(IfData{i, IfType::IFDEF});
            }
            i++;
            continue; 
        }

        // Deleting the lines if we are inside a ifndef or ifdef block that
        if(ifStack.size() != 0)
        {
            content.erase(content.begin() + i); 

            // Not i++ !; 
            continue; 
        }

        // Handling #defines
        if(strContains(content[i], "#define"))
        {
            std::vector<std::string> strSplitted = split(content[i], ' ');

            if(strSplitted.size() >= 3)
            {
                defines[strSplitted[1]] = strSplitted[2];  
            }
            else
            {
                defines[strSplitted[1]] = "1";
            }
            i++; 
            continue; 
        }

        i++;
    }

    // Cleaning up and removing all preprocessor lines
    for(size_t i = 0; i < content.size();)
    {

        if(strContains(content[i], "#define") || strContains(content[i], "#ifdef") || strContains(content[i], "#ifndef") || strContains(content[i], "#endif"))
        {
            content.erase(content.begin() + i); 
            continue; 
        }
        i++; 
    }

    std::ofstream outFile(outPath); 

    if(outFile.is_open() == false)
    {
        std::cout << "Error: Could not write to " << outPath << std::endl; 
        return 1; 
    }

    std::cout << "Writing " << content.size() << " lines to " << outPath << std::endl; 

    for(size_t i = 0; i < content.size(); i++)
    {
        outFile << content[i] << "\n"; 
    }

    outFile.close(); 

    return 0; 
}