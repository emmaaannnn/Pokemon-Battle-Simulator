#include <include/rapidjson/document.h>
#include <include/rapidjson/prettywriter.h>
#include <include/rapidjson/stringbuffer.h>
#include "include/rapidjson/filereadstream.h"

#include <cstdio>
#include <fstream> 
#include <iostream> 
#include "Pokemon.cpp"

using namespace std; 
using namespace rapidjson; 
int main(){
    FILE* file = fopen("1.json", "rb");
    char readBuffer[65536];
    rapidjson::FileReadStream data(file, readBuffer, 
                                 sizeof(readBuffer)); 

    rapidjson::Document doc;
    doc.ParseStream(data);
    fclose(file);
    std::cout << doc["name"].GetString() << std::endl;
    return -1;
};