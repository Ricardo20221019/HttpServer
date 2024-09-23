#ifndef BUFFER_H
#define BUFFER_H

#include <iostream>
#include <string>


class CustomBuffer
{
public:
    std::string data;
    int readIndex;
    int writeIndex;
public:
    CustomBuffer();
    ~CustomBuffer();
    std::string dataBegin() {return data.substr(readIndex,data.size()-readIndex);};
    int writeableSize() 
    {
        return data.size() - writeIndex;
    };
    int readableSize() {return writeIndex - readIndex;};
    int frontSpareSize() {return readIndex;};
    int appendChar(char c);
    int appendString(std::string s);
};
#endif