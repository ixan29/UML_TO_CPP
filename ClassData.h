#pragma once

#include <algorithm>
#include <vector>

#include "tinyxml2.h"
#include "Rect.h"
#include "Attribute.h"
#include "Method.h"

struct ClassData;

struct Inheritance {
    bool isVirtual;
    ClassData& baseClass;
};

struct ClassData
{
    std::string name;
    Rect coords;
    std::vector<Attribute> attributes;
    std::vector<Method> methods;
    std::vector<std::string> dependantClasses;

    bool isInterface;
    std::vector<Inheritance> inheritances;

    void addDependantClass(const std::string& dependency) {
        if(std::find(dependantClasses.begin(), dependantClasses.end(), dependency)==dependantClasses.end())
            dependantClasses.push_back(dependency);
    }

    void addAttribute(const Attribute& attribute) {
        attributes.push_back(attribute);

        if(attribute.hasProperty("mutex"))
            attributes.push_back({
                PRIVATE,
                "std::mutex",
                attribute.name+"Mutex",
                {}
            });
    }

    bool hasVirtualMethod() const {
        for(auto& method : methods)
            if(method.isVirtual)
                return true;

        return false;
    }

    bool isVirtual() const {
        return  isInterface
        ||      hasVirtualMethod();
    }

    bool hasConstructor() const {
        for(auto& method : methods) {
            if(method.isConstructor()) {
                return true;
            }
        }

        return false;
    }

    bool hasDefaultConstructor() const
    {
        for(auto& method : methods) {
            if( method.isConstructor()
            ||  method.arguments.size()==0) {
                return true;
            }
        }

        return !hasConstructor();
    }

    const Attribute* getAttributeByName(const std::string& name) const
    {
        for(auto& attribute : attributes) {
            if(attribute.name == name) {
                return &attribute;
            }
        }

        for(auto& inheritance : inheritances)
        {
            auto attribute = inheritance.baseClass.getAttributeByName(name);

            if(attribute!=nullptr) {
                return attribute;
            }
        }

        return nullptr;
    }

    const Method* getMatchingConstructor(const std::vector<std::string> arguments) const
    {
        std::vector<const Method*> constructors;

        for(auto& method : methods) {
            if(method.isConstructor()) {
                constructors.push_back(&method);
            }
        }

        std::sort(constructors.begin(), constructors.end(),
        [] (const Method* a, const Method* b) {
            return a->arguments.size() >= b->arguments.size();
        });

        for(auto& constructor : constructors)
        {
            bool valid = true;

            for(auto& argument : constructor->arguments)
            {
                if(std::find(arguments.begin(), arguments.end(), argument.name) == arguments.end()) {
                    valid = false;
                }
            }

            if(valid) {
                return constructor;
            }
        }

        return nullptr;
    }
};

ClassData parseClassData(const tinyxml2::XMLElement* xmlElement)
{
    ClassData classData;

    xmlElement->FirstChildElement("coordinates") >> classData.coords;

    string content = xmlElement->FirstChildElement("panel_attributes")->GetText();

    auto lines = split(content, "\n");

    size_t n=0;

    classData.isInterface = false;

    for (; n < lines.size() && lines[n] != "--" ; n++)
        if(lines[n].empty())
            continue;
        else
        if(lines[n].find("interface")<lines[n].length())
            classData.isInterface = true;
        else
        if(lines[n].find("layer")==0
        || lines[n].find("bg")==0
        || lines[n].find('{')==0)
            continue;
        else
        if(classData.name.empty())
            classData.name = lines[n];
        else
            throw runtime_error("Error parsing class title!");
   
    n++;

    for (; n < lines.size() && lines[n] != "--" ; n++)
        if(lines[n].empty())
            continue;
        else
            classData.addAttribute(parseAttribute(lines[n]));

    n++;

    for( ; n < lines.size() && lines[n] != "--"; n++)
        if(lines[n].empty())
            continue;
        else
            classData.methods.push_back(parseMethod(lines[n]));

    return classData;
}
