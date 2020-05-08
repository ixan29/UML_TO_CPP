#pragma once

#include <iostream>

enum Encapsulation { PRIVATE, PROTECTED, PUBLIC};

Encapsulation getEncapsulation(char c)
{
    switch(c)
    {
        case '-' : return PRIVATE;
        case '#' : return PROTECTED;
        case '+' : return PUBLIC;
        default : throw std::runtime_error("Error! Could not deduce encapsulation!");
    }
}

std::ostream& operator<<(std::ostream& os, Encapsulation encapsulation)
{
    switch(encapsulation)
    {
        case PRIVATE: return os << "private:";
        case PROTECTED: return os << "protected:";
        case PUBLIC: return os << "public:";
        default : throw std::runtime_error("Error! Could not deduce encapsulation!");
    }
}