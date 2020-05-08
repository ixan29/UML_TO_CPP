#pragma once

#include <string>

std::string normalizeName(const std::string& name)
{
    std::string normal;

    for(char c : name)
        if(isupper(c))
            (normal += ' ') += tolower(c);
        else
            normal += c;

    return normal;
}

std::string capitalize(std::string str)
{
    str[0] = toupper(str[0]);
    return str;
}

std::vector<std::string> split(const std::string& s, const std::string& delim)
{
    std::vector<std::string> split;

    auto start = 0U;
    auto end = s.find(delim);
    while (end != std::string::npos)
    {
        split.push_back(s.substr(start, end - start));
        start = end + delim.length();
        end = s.find(delim, start);
    }

    split.push_back(s.substr(start, end));

    return split;
}

vector<string> splitArguments(const string& line)
{
    vector<string> arguments;
    int level = 0;

    size_t previousIdx=0;

    for(size_t k=0; k<line.length();k++)
        switch(line[k])
        {
            case '<' : level++; break;
            case '>' : level--; break;

            case ',':
                if(level==0)
                {
                    arguments.push_back(line.substr(previousIdx, k-previousIdx));
                    previousIdx = k + 1;
                }
            break;
        }

    arguments.push_back(line.substr(previousIdx));

    return arguments;
}