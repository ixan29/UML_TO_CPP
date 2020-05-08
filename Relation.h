#pragma once

#include "tinyxml2.h"

#include "utils.h"
#include "Point.h"
#include "Rect.h"

enum RelationType {AGGREGATION, COMPOSITION, INHERITANCE, IGNORE};

struct Relation
{
    RelationType type;
    std::string message;
    Point begin, end;
};

Relation parseRelation(const tinyxml2::XMLElement* xmlElement)
{
    Relation relation;

    Rect rect;
    xmlElement->FirstChildElement("coordinates") >> rect;

    auto arrowCoords = split(xmlElement->FirstChildElement("additional_attributes")->GetText(),";");

    relation.begin = Point {
        rect.point.x+stoi(arrowCoords[0]),
        rect.point.y+stoi(arrowCoords[1])
    };
    relation.end = Point {
        rect.point.x+stoi(*(arrowCoords.end()-2)),
        rect.point.y+stoi(arrowCoords.back())
    };


    std::string content = xmlElement->FirstChildElement("panel_attributes")->GetText();
    
    std::string arrowInfo = content.substr(0, content.find('\n'));

    if(arrowInfo.length() < content.length())
    {
        for(char c : content.substr(content.find('\n')+1))
            if(c != ' ' && c != '\n')
                relation.message += c;
    }
    
    char arrowDirection;

    if(arrowInfo.find('<')<arrowInfo.length())
    {
        arrowDirection = '<';
        swap(relation.begin, relation.end);
    }
    else
    if(content.find('>')<content.length())
        arrowDirection = '>';
    else
    {
        relation.type = IGNORE;
        return relation;
    }

    int count=-1;

    for(size_t idx=0 ; idx<arrowInfo.length(); count++)
        idx = arrowInfo.find(arrowDirection, idx+1);

    switch(count)
    {
        case 0:
        case 1: relation.type = IGNORE; break;
        case 2: relation.type = INHERITANCE; break;
        case 3: relation.type = IGNORE; break;
        case 4:
            relation.type = AGGREGATION;
            swap(relation.begin, relation.end);
            break;
        case 5:
            relation.type = COMPOSITION;
            swap(relation.begin, relation.end);
            break;
        default: throw std::runtime_error("Error! Too much arrows!");
    }

    return relation;
}