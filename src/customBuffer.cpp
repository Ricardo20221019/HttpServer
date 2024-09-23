#include "customBuffer.h"

#include <stdlib.h>
#include <sys/uio.h>
#include <string.h>
#include <iostream>


CustomBuffer::CustomBuffer()
    : readIndex(0),
      writeIndex(0)
{
}

CustomBuffer::~CustomBuffer()
{
}

int CustomBuffer::appendString(std::string s)
{
    data.append(s);
    return 0;
}

int CustomBuffer::appendChar(char c)
{
    data+=c;
    return 0;
}

