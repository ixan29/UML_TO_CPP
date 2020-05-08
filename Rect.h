#pragma once

#include "Point.h"
#include "tinyxml2.h"
#include <iostream>

struct Rect
{
    Point point;
    int w,h;

    #define ERR 5

    bool contains(const Point& p) const {
        return
        point.x <= p.x + ERR && p.x - ERR <= point.x + w &&
        point.y <= p.y + ERR && p.y - ERR <= point.y + h;
    }

    bool contains(const Rect& rect) const {
        return contains(rect.point)
        && contains(Point{rect.point.x+rect.w, rect.point.y+rect.h});
    }

    Rect()
    :point()
    ,w(0)
    ,h(0)
    {}

    Rect(Point point_, int w_, int h_)
    :point(point_)
    ,w(w_)
    ,h(h_)
    {}
};

const tinyxml2::XMLElement* operator>>(const tinyxml2::XMLElement* element, Rect& rect) {

    rect.point.x = element->FirstChildElement("x")->IntText();
    rect.point.y = element->FirstChildElement("y")->IntText();
    rect.w = element->FirstChildElement("w")->IntText();
    rect.h = element->FirstChildElement("h")->IntText();

    return element;
}

std::ostream& operator<<(std::ostream& os, const Rect& rect)
{
    return os << "{ "
    << "x: " << rect.point.x << ", "
    << "y: " << rect.point.y << ", "
    << "w: " << rect.w << ", "
    << "h: " << rect.h << "}";
}