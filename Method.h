#pragma once

#include <vector>

#include "Utils.h"
#include "Encapsulation.h"
#include "Attribute.h"

struct Method
{
    Encapsulation encapsulation;
    bool isVirtual;
    std::string returnType;
    std::string name;
    std::vector<Attribute> arguments;

    bool isConstructor() const {
        return returnType.empty();
    }

    bool isDestructor() const {
        return returnType.empty()
        && name[0] == '~';
    }
};

Method parseMethod(string line)
{
    Method method;

    if(line[0]=='/')
    {
        if(line.back() != '/')
            throw runtime_error("Error!"+line+"is not well formated (missing / at the end)");

        line = line.substr(1, line.length()-2);
        method.isVirtual=true;
    }

    method.encapsulation = getEncapsulation(line[0]);

    string str;
    for(size_t k=1;k<line.length();k++)
        if(line[k] !=' ')
            str += line[k];
    line = str;

    size_t beginPar = line.find('(');
    size_t endPar = line.find(')');

    method.name = line.substr(0, beginPar);

    if(beginPar+1<endPar)
    {
        auto arguments = splitArguments(line.substr(beginPar+1, endPar-beginPar-1));

        for(auto& argument : arguments)
        {
            auto argumentInfos = split(argument,":");
            method.arguments.push_back({
                PRIVATE,
                argumentInfos.size()==2 ? checkStdNamespace(argumentInfos[1]) : "",
                argumentInfos[0]
            });
        }
    }

    auto methodInfo=split(line.substr(endPar+1),":");

    if(methodInfo.size()==2)
        method.returnType=checkStdNamespace(methodInfo[1]);

    return method;  
}