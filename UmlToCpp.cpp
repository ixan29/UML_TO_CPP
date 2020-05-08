#include "tinyxml2.cpp"
#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <fstream>
#include <algorithm>
#include <map>
#include <io.h>
#include <limits>
#include "Gui.cpp"

#include "Point.h"
#include "Rect.h"
#include "Encapsulation.h"
#include "Attribute.h"
#include "Method.h"
#include "ClassData.h"
#include "Package.h"
#include "Relation.h"

using namespace std;
using namespace tinyxml2;

vector<ClassData> classesData;

ClassData& getClassDataByName(const string& name)
{
    for(auto& classData : classesData)
        if(classData.name == name)
            return classData;

    throw runtime_error("Error! classData having "+name+" could not be found!");
}

const bool hasAttributeWithName(const vector<Attribute>& attributes, const string& name)
{
    for(auto& attribute : attributes)
        if(attribute.name == name)
            return true;

    return false;
}

Package globalPackage;

Info info;

void writeConstructorDocumentation(ostream& os, const Method& method, const std::string& name)
{
    os
    << "\t/**" << endl
    << "\t *"<< normalizeName(name) << " constructor" << endl;

    for(auto& argument : method.arguments)
        os << "\t * @param " << argument.name << " initial value of " << argument.normalName() << endl;

    os << "\t**/" << endl;
}

void writeDestructorDocumentation(ostream& os, const std::string& name, const bool isVirtual)
{
    os
    << "\t/**" << endl
    << "\t * "<< normalizeName(name);

//    if(isVirtual)
//        os << " virtual";

    os
    << " destructor" << endl
    << "\t**/" << endl;
}

void writeMethodDocumention(ostream& os, const Method& method)
{
    os
    << "\t/**" << endl
    << "\t * TODO complete method documentation" << endl;

    for(auto& attribute : method.arguments)
        os << "\t * @param " << attribute.name << endl;

    if(method.returnType != "void")
        os << "\t * @return" << endl;

    os << "\t**/" << endl;
}

void writeMethodsInHeader(ostream& os, const ClassData& classData, Encapsulation previousEncapsulation)
{
    bool writingConstructorsFinished = false;

    for(const Method& method : classData.methods)
    {
        if(method.encapsulation != previousEncapsulation)
        {
            os << method.encapsulation << endl;
            previousEncapsulation = method.encapsulation;
        }

        if(method.isDestructor())
        {
            writingConstructorsFinished = true;
        }

        if(!method.isConstructor() && !writingConstructorsFinished)
        {
            writingConstructorsFinished = true;

            writeDestructorDocumentation(os, classData.name, classData.hasVirtualMethod());

            //better be safe and create a virtual destructor
            if(classData.hasVirtualMethod())
                os << "\tvirtual ~"+classData.name+"() {}" << endl << endl;
            else
                os << "\t~"+classData.name+"() {}" << endl << endl;
        }

        if(method.name=="operator<=>")
        {
            string type = method.arguments[0].type;
            type = type.substr(0, type.length()-1);
            type = normalizeName(type);

            if(type[0]==' ')
                type = type.substr(1);

            os << endl;
            for(string ops : {"<",">","==","!="})
            {   
                os <<
                "\t/**" << endl <<
                "\t * Compares with an other " << type << " using the \"" << ops << "\" operator" << endl <<
                "\t * @param other " << type << " to be compared with" << endl <<
                "\t * @return the result of the \"" << ops << "\" comparison" << endl <<
                "\t**/" << endl <<
                "\tconst bool operator" << ops <<
                "(" << method.arguments[0].type << " " << method.arguments[0].name << ") const";

                if(classData.isInterface)
                    os << " = 0";

                os << ";" << endl;
            }
        }
        else
        if(method.isConstructor())
        {
            writeConstructorDocumentation(os, method, classData.name);
            os << "\t" << classData.name << "(";

            if(method.arguments[0].type.empty())
            {
                const Attribute* argument = classData.getAttributeByName(method.arguments[0].name);

                if(argument == nullptr)
                    throw runtime_error("Error! "+classData.name+"constructor argument "+
                    method.arguments[0].name+" type could not be deduced!");

                string type = argument->type;
                    
                if(type.find("std::unique_ptr")==0)
                    os << type << method.arguments[0].name << "_";
                else
                if(type.find("std::shared_ptr")==0
                || type.find("std::weak_ptr")==0)
                    os << "const " << type << " " << method.arguments[0].name << "_";
                else
                    os << "const " << type << "& " << method.arguments[0].name << "_";
            }
            else
                os << method.arguments[0].type << " " << method.arguments[0].name;

            for(size_t k=1 ; k<method.arguments.size() ; k++)
            {
                os << ", ";

                if(method.arguments[k].type.empty())
                {
                    const Attribute* argument = classData.getAttributeByName(method.arguments[k].name);

                    if(argument == nullptr)
                        throw runtime_error("Error! "+classData.name+"constructor argument "+
                        method.arguments[k].name+" type could not be deduced!");

                    string type = argument->type;

                    if(type.find("std::unique_ptr")==0)
                        os << type << method.arguments[k].name << "_";
                    else
                    if(type.find("std::shared_ptr")==0
                    || type.find("std::weak_ptr")==0)
                        os << "const " << type << " " << method.arguments[k].name << "_";
                    else
                        os << "const " << type << "& " << method.arguments[k].name << "_";
                }
                else
                    os << method.arguments[k].type << " " << method.arguments[k].name;
            }

            os << ");" << endl;
        }
        else
        {
            writeMethodDocumention(os, method);
            os << "\t";

            if(method.isVirtual)
                os << "virtual ";

            os << method.returnType << " " << method.name << "(";

            if(method.arguments.size()>0)
            {
                os << method.arguments[0].type << " " << method.arguments[0].name;

                for(size_t k=1; k<method.arguments.size() ; k++)
                    os << ", " << method.arguments[k].type << " " << method.arguments[k].name;
            }

            os << ")";

            if(classData.isInterface)
                os << " = 0";

            os << ";" << endl;
        }
    }
}

string toUpper(const string& str)
{
    string upper;
    upper.reserve(str.length());

    if(isupper(str[0]))
        upper += "_";
    else 
        upper += "__";

    for(char c : str)
        if(isupper(c))
            (upper += '_') += c;
        else
            upper += toupper(c);

    upper += "_H__";

    return upper;
}

bool useStdObject(const ClassData& classData, const string& str)
{
    for(auto& attribute : classData.attributes)
        if(attribute.type.find(str) < attribute.type.length())
            return true;

    for(auto& method : classData.methods)
    {   
        if(method.returnType.find(str) < method.returnType.length())
            return true;

        for(auto& argument : method.arguments)
            if(argument.type.find(str) < argument.type.length())
                return true;
    }

    return false;
}

const pair<string, string> stdLibs[] = {
    {"std::string", "<string>"},
    {"std::vector", "<vector>"},
    {"std::map", "<map>"},
    {"std::mutex", "<mutex>"},
    {"_ptr", "<memory>"}
};

void includeStdLibs(ostream& os, const ClassData& classData)
{
    bool needsEndl=false;

    for(auto& pair : stdLibs)
        if(useStdObject(classData, pair.first))
        {
            os << "#include " << pair.second << endl;
            needsEndl = true;
        }

    if(needsEndl)
        os << endl;
}

void includeDependantClasses(ostream& os, const ClassData& classData)
{
    // TODO package include 

    for(auto& dependantClass : classData.dependantClasses)
        os << "#include \"" << globalPackage.getPath(classData, getClassDataByName(dependantClass)) << dependantClass << ".h\"" << endl;

    if(classData.dependantClasses.size()>0)
        os << endl;
}

void writeGetCommentary(ostream& os, const Attribute& attribute)
{
    os
    << "\t/**" << endl
    << "\t * Gets the " << attribute.normalName() << "." << endl;

    if(attribute.type.find("std::unique_ptr") < attribute.type.length()
    && attribute.type.find("std::unique_ptr") > 2)
        os
        << "\t * the access to the container's unique pointer elements is made trough a container of unsafe raw pointers" << endl
        << "\t * Under no cirucmstances the raw pointers should be used for destroying objects or segmentation faults will occur" << endl;

    if(attribute.hasProperty("mutex"))
        os << "\t * The access is thread safe." << endl;

    os
    << "\t * @return the " << attribute.normalName() << endl
    << "\t**/"<<endl;
}

void writeSetCommentary(ostream& os, const Attribute& attribute)
{
    os
    << "\t/**" << endl
    << "\t * Sets a new " << attribute.normalName() << "." << endl;

    if(attribute.hasProperty("mutex"))
        os << "\t * The modification is thread safe." << endl;

    os
    << "\t * @param "<< attribute.name <<" the new " << attribute.normalName() << " to set" << endl
    << "\t**/"<<endl;
}

void writeHasCommentary(ostream& os, const Attribute& attribute)
{
    os
    << "\t/**" << endl
    << "\t * Checks if the " << attribute.normalName() << " contain the specified " << attribute.normalSingularName() << "." << endl;

    if(attribute.hasProperty("mutex"))
        os << "\t * The verification is thread safe." << endl;

    os
    << "\t * @param "<< attribute.getSingularName() <<" the " << attribute.normalSingularName() << " to check" << endl
    << "\t * @return " << "true if the " << attribute.normalName() << " contain the specified " << attribute.normalSingularName() << ", false otherwise" << endl 
    << "\t**/"<<endl;
}

void writeAddCommentary(ostream& os, const Attribute& attribute)
{
    os
    << "\t/**" << endl
    << "\t * Adds the " << attribute.normalSingularName() << "." << endl;

    if(attribute.hasProperty("mutex"))
        os << "\t * The inclusion is thread safe." << endl;

    os << "\t * @param "<< attribute.getSingularName() <<" the " << attribute.normalSingularName() << " to add" << endl;
    
    if(attribute.hasProperty("has")
    || attribute.hasProperty("remove"))
        os << "\t * @return " << "true if the inclusion succeded, false if the " << attribute.normalSingularName() << " is already included and thus no action is needed." << endl;
    
    os << "\t**/"<<endl;
}

void writeRemoveCommentary(ostream& os, const Attribute& attribute)
{
    os
    << "\t/**" << endl
    << "\t * Removes the " << attribute.normalSingularName() << "." << endl;

    if(attribute.hasProperty("mutex"))
        os << "\t * The removal is thread safe." << endl;

    os
    << "\t * @param "<< attribute.getSingularName() <<" the " << attribute.normalSingularName() << " to remove" << endl
    << "\t * @return " << "true if the removal succeded, false if the " << attribute.normalSingularName() << " isn't included and thus no action is needed." << endl
    << "\t**/"<<endl;
}

void writeClearCommentary(ostream& os, const Attribute& attribute)
{
    os
    << "\t/**" << endl
    << "\t * Clears the " << attribute.normalName() << "." << endl;

    if(attribute.hasProperty("mutex"))
        os << "\t * The clearance is thread safe." << endl;

    os << "\t**/"<<endl;
}

Encapsulation writeAttributesInHeader(ostream& os, const ClassData& classData)
{
    Encapsulation previousEncapsulation = PRIVATE;

    for(const Attribute& attribute : classData.attributes)
    {
        if(attribute.encapsulation != previousEncapsulation)
        {
            os << attribute.encapsulation << endl;
            previousEncapsulation = attribute.encapsulation;
        }

        os << "\t" << attribute.type << " " << attribute.name << ";" << endl;
    }

    for(const Attribute& attribute : classData.attributes)
    {
        if(attribute.properties.size() > 0)
            if(previousEncapsulation != PUBLIC)
            {
                os << endl << "public: " << endl;
                previousEncapsulation = PUBLIC;
            }

        if(attribute.hasProperty("get"))
        {
            writeGetCommentary(os, attribute);

            if(attribute.type.find("std::unique_ptr")==0)
            {
                auto noUnique = attribute.type.substr(
                    strlen("std::unique_ptr<"),
                    attribute.type.length()-strlen("std::unique_ptr<")-1
                );

                os << "\tconst " << noUnique << "& get" << capitalize(attribute.name) << "()";
            }
            else
            //if it's a container of unique_ptr
            if(attribute.type.find("std::unique_ptr") < attribute.type.length()
            && attribute.type.find("std::unique_ptr") > 2)
            {
                auto container = attribute.type.substr(0,attribute.type.find("<std::unique_ptr"));

                auto sub = attribute.type.substr(attribute.type.find("std::unique_ptr"));

                auto noUnique = sub.substr(
                    strlen("std::unique_ptr<"),
                    sub.length()-strlen("std::unique_ptr<")-2
                );

                os << "\t"<<container<<"<"<<noUnique<<" * const> get" << capitalize(attribute.name) << "()";
            }
            else
            {
                if(attribute.type.find("std::shared_ptr")==0
                || attribute.type.find("std::weak_ptr")==0)
                    os << "\tconst " << attribute.type << " get" << capitalize(attribute.name) << "()";
                else
                if(attribute.type.find('&')<attribute.type.length())
                    os << "\tconst " << attribute.type << " get" << capitalize(attribute.name) << "()";
                else
                if(attribute.isContainer())
                    os << "\tconst " << attribute.type << " get" << capitalize(attribute.name) << "()";
                else
                    os << "\tconst " << attribute.type << "& get" << capitalize(attribute.name) << "()";
            }

            if(!attribute.hasProperty("mutex"))
                os << " const";

            os << ";" << endl;
        }

        if(attribute.hasProperty("set"))
        {
            writeSetCommentary(os, attribute);

            if(attribute.type.find("std::unique_ptr")==0)
                os << "\tvoid set"<<capitalize(attribute.name)<<"("+attribute.type<<" "+attribute.name+"_);" << endl;
            else
            if(attribute.type.find("std::shared_ptr")==0
            || attribute.type.find("std::weak_ptr")==0)
                os << "\tvoid set"<<capitalize(attribute.name)<<"(const "<<attribute.type<<" "+attribute.name+"_);" << endl;
            else
            if(attribute.type.find('&')<attribute.type.length())
                os << "\tvoid set"<<capitalize(attribute.name)<<"(const "<<attribute.type<<" "+attribute.name+"_);" << endl;
            else
                os << "\tvoid set"<<capitalize(attribute.name)<<"(const "<<attribute.type<<"& "+attribute.name+"_);" << endl;
        
        }

        if(attribute.hasProperty("has")
        || attribute.hasProperty("remove"))
        {
            writeHasCommentary(os, attribute);

            if(!attribute.hasProperty("has"))
                os << "private:" << endl;

            size_t begin = attribute.type.find('<');
            size_t end = attribute.type.find_last_of('>');

            string elemType = attribute.type.substr(begin+1, end-begin-1);

            os << "\tbool has"<<capitalize(attribute.getSingularName())<<"(const ";

            if(elemType.find("std::unique_ptr")==0)
            {
                auto sub = attribute.type.substr(attribute.type.find("std::unique_ptr"));

                auto noUnique = sub.substr(
                    strlen("std::unique_ptr<"),
                    sub.length()-strlen("std::unique_ptr<")-2
                );

                os << noUnique << " * const";
            }
            else
            if(elemType.find("std::shared_ptr")!=0
            && elemType.find("std::weak_ptr")!=0)
                os << elemType << "&";
            else
                os << elemType;

            os << " " << attribute.getSingularName() <<")";
            
            if(!attribute.hasProperty("mutex"))
                os << " const";

            os << ";" << endl;

            if(!attribute.hasProperty("has"))
                os << "public:" << endl;
        }

        if(attribute.hasProperty("add"))
        {
            writeAddCommentary(os, attribute);

            size_t begin = attribute.type.find('<');
            size_t end = attribute.type.find_last_of('>');

            string elemType = attribute.type.substr(begin+1, end-begin-1);

            os << '\t';

            if(attribute.hasProperty("has")
            || attribute.hasProperty("remove"))
                os << "bool ";
            else
                os << "void ";

            os << "add"<<capitalize(attribute.getSingularName())<<"(";

            if(elemType.find("std::unique_ptr")==0)
                os << elemType;
            else
            if(elemType.find("std::shared_ptr")==0
            || elemType.find("std::weak_ptr")==0)
                os << "const "<<elemType;
            else
                os << "const "<<elemType <<"&";

            os << " " << attribute.getSingularName() << ");" << endl;
        }

        if(attribute.hasProperty("remove"))
        {
            writeRemoveCommentary(os, attribute);

            size_t begin = attribute.type.find('<');
            size_t end = attribute.type.find_last_of('>');

            string elemType = attribute.type.substr(begin+1, end-begin-1);

            os << "\tbool remove"<<capitalize(attribute.getSingularName())<<"(const ";

            if(elemType.find("std::unique_ptr")==0)
            {
                auto sub = attribute.type.substr(attribute.type.find("std::unique_ptr"));

                auto noUnique = sub.substr(
                    strlen("std::unique_ptr<"),
                    sub.length()-strlen("std::unique_ptr<")-2
                );

                os << noUnique << " * const";
            }
            else
            if(elemType.find("std::shared_ptr")!=0
            && elemType.find("std::weak_ptr")!=0)
                os << elemType << "&";
            else
                os << elemType;            

            os << " " << attribute.getSingularName() << ");"<< endl;
        }

        if(attribute.hasProperty("clear"))
        {
            writeClearCommentary(os, attribute);
            os << "\tvoid clear"<<capitalize(attribute.name)<<"();" << endl;
        }

        if(attribute.properties.size()>0)
            os << endl;
    }

    return previousEncapsulation;
}

void writeInHeader(ostream& os, const ClassData& classData)
{
    os << "#ifndef " << toUpper(classData.name) << endl;
    os << "#define " << toUpper(classData.name) << endl;
    os << endl;

    includeStdLibs(os, classData);
    includeDependantClasses(os, classData);

    os
    << "/**" << endl
    << " * "<< classData.name;

    if(classData.isInterface)
        os << " interface";
    else
        os << " class";

    os << endl
    << " * " << endl
    << " * TODO add description" << endl
    << "**/" << endl;


    os << "class " << classData.name;

    for(size_t k=0; k<classData.inheritances.size(); k++)
    {
        os << (k==0 ? ": " : ", ");

        if(classData.inheritances[k].isVirtual)
            os << "virtual ";

        os << "public " << classData.inheritances[k].baseClass.name;
    }
    
    os << endl << "{" << endl;

    auto encapsulation = writeAttributesInHeader(os, classData);
    os << endl;
    writeMethodsInHeader(os, classData, encapsulation);
    
    os << "};";
    os << endl << endl;
    os << "#endif /* " << toUpper(classData.name) << " */";
    os << endl << endl;

}

void writeLockGuard(ostream& os, const Attribute& attribute)
{
    if(attribute.hasProperty("mutex"))
        os << "\tstd::lock_guard<std::mutex> lock("<<attribute.name<<"Mutex);" << endl;
}

void writeAttributesInImpl(ostream& os, const vector<Attribute>& attributes, const string& className)
{
    for(const Attribute& attribute : attributes)
    {
        if(attribute.hasProperty("get"))
        {
            if(attribute.type.find("std::unique_ptr")==0)
            {
                auto noUnique = attribute.type.substr(
                    strlen("std::unique_ptr<"),
                    attribute.type.length()-strlen("std::unique_ptr<")-1
                );

                os << "const " << noUnique << "& "<< className <<"::get"<< capitalize(attribute.name) << "() ";
                
                if(!attribute.hasProperty("mutex"))
                    os << "const ";
                
                os << " {" << endl;
                writeLockGuard(os, attribute);
                os << "\treturn *" << attribute.name << ";" << endl
                << "}" << endl
                << endl;
            }
            else
            if(attribute.type.find("std::unique_ptr") < attribute.type.length()
            && attribute.type.find("std::unique_ptr") > 2)
            {
                auto container = attribute.type.substr(0,attribute.type.find("<std::unique_ptr"));

                auto sub = attribute.type.substr(attribute.type.find("std::unique_ptr"));

                auto noUnique = sub.substr(
                    strlen("std::unique_ptr<"),
                    sub.length()-strlen("std::unique_ptr<")-2
                );

                string type = container+"<"+noUnique+" * const>";

                os<<type<<" " << className <<"::get" << capitalize(attribute.name) << "()";
                
                if(!attribute.hasProperty("mutex"))
                    os << " const";

                os << " {" << endl;

                writeLockGuard(os, attribute);
                
                os
                << endl
                << "\t" << type << " v;" << endl
                << "\tv.reserve("<<attribute.name<<".size());" << endl
                << endl
                << "\tfor(auto& e : " << attribute.name << ") {" << endl
                << "\t\tv.push_back(e.get());" << endl
                << "\t}" << endl
                << endl
                << "\treturn v;" << endl
                << "}" << endl << endl;

                /*
                auto container = attribute.type.substr(0,attribute.type.find("<std::unique_ptr"));

                auto sub = attribute.type.substr(attribute.type.find("std::unique_ptr"));

                auto noUnique = sub.substr(
                    strlen("std::unique_ptr<"),
                    sub.length()-strlen("std::unique_ptr<")-2
                );

                os <<noUnique<<"& "<<className<<"::get" << capitalize(attribute.getSingularName()) << "(size_t idx) ";
                
                if(!attribute.hasProperty("mutex"))
                    os << "const ";

                os << "{" << endl;
                writeLockGuard(os, attribute);

                os
                << "\treturn *"<<attribute.name<<"[idx];" << endl
                << "}" << endl<<endl;

                os<<"size_t "<<className<<"::get"<<capitalize(attribute.name)<<"Size() {"<<endl;
                writeLockGuard(os, attribute);
                os
                <<"\treturn "<<attribute.name<<".size();"<<endl
                <<"}" << endl << endl;
                */
            }
            else
            if(attribute.type.find("std::shared_ptr")==0
            || attribute.type.find("std::weak_ptr")==0)
            {   
                os << "const "<< attribute.type << " " << className << "::get"<<capitalize(attribute.name)<<"() ";
                
                if(!attribute.hasProperty("mutex"))
                    os << "const ";
                
                os << "{" << endl;
                writeLockGuard(os, attribute);
                os << "\treturn " << attribute.name << ";" << endl
                << "}" << endl
                << endl;
            }
            else
            if(attribute.type.find('&')<attribute.type.length())
            {   
                os << "const "<< attribute.type << " " << className << "::get"<<capitalize(attribute.name)<<"() ";

                if(!attribute.hasProperty("mutex"))
                    os << "const ";
                
                os << "{" << endl;
                writeLockGuard(os, attribute);
                os << "\treturn " << attribute.name << ";" << endl
                << "}" << endl
                << endl;
            }
            else
            if(attribute.isContainer())
            {
                os << "const "<< attribute.type << " " << className << "::get"<<capitalize(attribute.name)<<"() ";
                
                if(!attribute.hasProperty("mutex"))
                    os << "const ";

                os << "{" << endl;
                writeLockGuard(os, attribute);
                os << "\treturn " << attribute.name << ";" << endl
                << "}" << endl
                << endl;
            }
            else
            {
                os << "const "<< attribute.type << "& " << className << "::get"<<capitalize(attribute.name)<<"() ";
                
                if(!attribute.hasProperty("mutex"))
                    os << "const ";

                os << "{" << endl;
                writeLockGuard(os, attribute);
                os << "\treturn " << attribute.name << ";" << endl
                << "}" << endl
                << endl;
            }
        }

        if(attribute.hasProperty("set"))
        {
            if(attribute.type.find("std::unique_ptr")==0)
            {
                os << "void "<<className+"::set"<<capitalize(attribute.name)<<"("+attribute.type<<" "+attribute.name+"_) {" << endl;
                writeLockGuard(os, attribute);
                os << "\t" << attribute.name << " = std::move(" << attribute.name << "_);" << endl
                << "}" << endl
                << endl;
            }
            else
            if(attribute.type.find("std::shared_ptr")==0
            || attribute.type.find("std::weak_ptr")==0)
            {
                os << "void "<<className+"::set"<<capitalize(attribute.name)<<"(const "<<attribute.type<<" "+attribute.name+"_) {" << endl;
                writeLockGuard(os, attribute);
                os << "\t" << attribute.name << " = " << attribute.name << "_;" << endl
                << "}" << endl
                << endl;
            }
            else
            if(attribute.type.find('&') < attribute.type.length())
            {
                os << "void "<<className+"::set"<<capitalize(attribute.name)<<"(const "<<attribute.type<<" "+attribute.name+"_) {" << endl;
                writeLockGuard(os, attribute);
                os << "\t" << attribute.name << " = " << attribute.name << "_;" << endl
                << "}" << endl
                << endl;
            }
            else
            {
                os << "void "<<className+"::set"<<capitalize(attribute.name)<<"(const "<<attribute.type<<"& "+attribute.name+"_) {" << endl;
                writeLockGuard(os, attribute);
                os << "\t" << attribute.name << " = " << attribute.name << "_;" << endl
                << "}" << endl
                << endl;
            }
        }

        if(attribute.hasProperty("has")
        || attribute.hasProperty("remove"))
        {
            size_t begin = attribute.type.find('<');
            size_t end = attribute.type.find_last_of('>');

            string elemType = attribute.type.substr(begin+1, end-begin-1);

            os << "bool " << className << "::has"<<capitalize(attribute.getSingularName())<<"(const "<<elemType;

            if(elemType.find("std::shared_ptr")!=0
            && elemType.find("std::weak_ptr")!=0)
                os << "&";

            os << " "<<attribute.getSingularName()<<")";
            
            if(!attribute.hasProperty("mutex"))
                os << " const";

            os << " {" << endl;

            writeLockGuard(os, attribute);

            os << "\treturn std::find("<<attribute.name<<".begin(), "<<attribute.name<<".end(), "<<
            attribute.getSingularName() << ") != " << attribute.name << ".end();" << endl
            << "}" << endl << endl;

        }

        if(attribute.hasProperty("add"))
        {
            size_t begin = attribute.type.find('<');
            size_t end = attribute.type.find_last_of('>');

            string elemType = attribute.type.substr(begin+1, end-begin-1);

            if(attribute.hasProperty("has")
            || attribute.hasProperty("remove"))
                os << "bool ";
            else
                os << "void ";

            os << className<<"::add"<<capitalize(attribute.getSingularName())<<"(";

            if(elemType.find("std::unique_ptr")==0)
                os << elemType;
            else
            if(elemType.find("std::shared_ptr")==0
            || elemType.find("std::weak_ptr")==0)
                os << "const " << elemType;
            else
                os << "const " << elemType<< "&";

            os << " " << attribute.getSingularName() << ") {" << endl;

            writeLockGuard(os, attribute);

            if(attribute.hasProperty("has")
            || attribute.hasProperty("remove"))
                os << "\tauto it = std::find("<<attribute.name<<".begin(), "<<attribute.name<<".end(), "<<attribute.getSingularName() << ");" << endl
                << "\tif(it == " << attribute.name << ".end()) {" << endl
                << "\t\t" << attribute.name << ".push_back("<<attribute.getSingularName()<<");" << endl
                << "\t\treturn true;" << endl
                << "\t}" << endl
                << "\treturn false;"<<endl;
            else
                os << "\t" << attribute.name << ".push_back("<<attribute.getSingularName()<<");" << endl;

            os << "}" << endl << endl;
        }

        if(attribute.hasProperty("remove"))
        {
            size_t begin = attribute.type.find('<');
            size_t end = attribute.type.find_last_of('>');

            string elemType = attribute.type.substr(begin+1, end-begin-1);

            os << "bool "<<className<<"::remove"<<capitalize(attribute.getSingularName())<<"(const "<<elemType;

            if(elemType.find("std::shared_ptr")!=0
            && elemType.find("std::weak_ptr")!=0)
                os << "&";

            os << " "<<attribute.getSingularName()<<") {" << endl;

            writeLockGuard(os, attribute);

            os << "\tauto it = std::find("<<attribute.name<<".begin(), "<<attribute.name<<".end(), "<<attribute.getSingularName() << ");" << endl
            << "\tif(it != " << attribute.name << ".end()) {" << endl
            << "\t\t" << attribute.name << ".erase(it);" << endl
            << "\t\treturn true;" << endl
            << "\t}" << endl
            << "\treturn false;"<<endl
            << "}" << endl << endl;
        }

        if(attribute.hasProperty("clear"))
        {   
            os << "void "<<className<<"::clear"<<capitalize(attribute.name)<<"() {" << endl;
            writeLockGuard(os, attribute);
            os << "\t"<<attribute.name<<".clear();" << endl
            << "}" << endl << endl;
        }
    }
}

void writeMethodsInImpl(ostream& os, const ClassData& classData)
{
    for(const Method& method : classData.methods)
    {
        if(method.name=="operator<=>")
        {
            os << endl;
            for(string ops : {"<",">","=="})
                os << "const bool "<< classData.name <<"::operator" << ops <<
                "(" << method.arguments[0].type << " " << method.arguments[0].name << ") const {" << endl <<
                "\treturn false;" << endl <<
                "}" << endl <<
                endl;

            os << "const bool "<< classData.name <<"::operator!=" <<
            "(" << method.arguments[0].type << " " << method.arguments[0].name << ") const {" << endl <<
            "\treturn !( *this == " << method.arguments[0].name << " );" << endl <<
            "}" << endl <<
            endl;
        }
        else
        if(method.isConstructor())
        {
            vector<string> baseClassesArguments;

            os << classData.name << "::" << classData.name << "(";

            if(method.arguments.size()>0)
            {
                if(method.arguments[0].type.empty())
                {
                    const Attribute* argument = classData.getAttributeByName(method.arguments[0].name);

                    if(argument == nullptr)
                        throw runtime_error("Error! "+classData.name+"constructor argument "+
                        method.arguments[0].name+" type could not be deduced!");

                    string type = argument->type;

                    if(type.find("std::unique_ptr")==0)
                        os << type << method.arguments[0].name << "_";
                    else
                    if(type.find("std::shared_ptr")==0
                    || type.find("std::weak_ptr")==0)
                        os << "const " << type << " " << method.arguments[0].name << "_";
                    else
                        os << "const " << type << "& " << method.arguments[0].name << "_";

                    if(!hasAttributeWithName(classData.attributes, argument->type))
                        baseClassesArguments.push_back(argument->name);
                }
                else
                    os << method.arguments[0].type <<
                    " " << method.arguments[0].name;

                for(size_t k=1 ; k<method.arguments.size() ; k++)
                {
                    os << ", ";

                    if(method.arguments[k].type.empty())
                    {
                        const Attribute* argument = classData.getAttributeByName(method.arguments[k].name);

                        if(argument == nullptr)
                            throw runtime_error("Error! "+classData.name+"constructor argument "+
                            method.arguments[k].name+" type could not be deduced!");

                        string type = argument->type;

                        if(type.find("std::unique_ptr")==0)
                            os << type << method.arguments[k].name << "_";
                        else
                        if(type.find("std::shared_ptr")==0
                        || type.find("std::weak_ptr")==0)
                            os << "const " << type << " " << method.arguments[k].name << "_";
                        else
                            os << "const " << type << "& " << method.arguments[k].name << "_";

                        if(!hasAttributeWithName(classData.attributes, argument->name))
                            baseClassesArguments.push_back(argument->name);
                    }
                    else
                        os << method.arguments[k].type <<
                        " " << method.arguments[k].name;
                }
            }

            os << ")" << endl;

            bool dots = false;

            if(classData.inheritances.size()>0)
            {
                for(auto& inheritance : classData.inheritances)
                {
                    const Method* constructor = inheritance.baseClass.getMatchingConstructor(
                        baseClassesArguments
                    );

                    if(constructor != nullptr)
                    {
                        if(!dots)
                        {
                            os << ":";
                            dots = true; 
                        }
                        else
                        {
                            os << ",";
                        }
                        
                        os << inheritance.baseClass.name << "(";

                        if( 
                            classData.getAttributeByName(
                                constructor->arguments[0].name
                            )
                            ->type.find("std::unique_ptr")==0
                        )
                            os << "std::move(" << constructor->arguments[0].name<<"_)";
                        else
                            os << "" << constructor->arguments[0].name<<"_";

                        for(size_t k=1;k<constructor->arguments.size();k++)
                            if(
                                classData.getAttributeByName(
                                    constructor->arguments[k].name
                                )
                                ->type.find("std::unique_ptr")==0
                            )
                                os << ", std::move(" << constructor->arguments[k].name<<"_)";
                            else
                                os << ", " << constructor->arguments[k].name<<"_";

                        os << ")" << endl;
                    }
                    else
                    if(!inheritance.baseClass.hasDefaultConstructor())
                    {
                        if(!dots)
                        {
                            os << ":";
                            dots = true; 
                        }
                        else
                        {
                            os << ",";
                        }
                        
                        os << inheritance.baseClass.name << "(/** TODO: could not auto-detect arguments for base class **/)" << endl;
                    }
                }
            }

            for(size_t k=0 ; k<method.arguments.size(); k++)
                if(method.arguments[k].type.empty()
                && hasAttributeWithName(classData.attributes,method.arguments[k].name))
                {
                    if(!dots)
                    {
                        os << ":";
                        dots = true; 
                    }
                    else
                    {
                        os << ",";
                    }
                    
                    string name = method.arguments[k].name;

                    if(classData.getAttributeByName(method.arguments[k].name)->type.find("std::unique_ptr")==0)
                        os << name << "(std::move(" << name << "_))" << endl;
                    else
                        os << name << "(" << name << "_)" << endl;
                }

            for(auto& attribute : classData.attributes)
                try
                {
                    if( !hasAttributeWithName(method.arguments, attribute.name)
                    &&  !getClassDataByName(attribute.type).hasDefaultConstructor())
                    {
                        if(!dots)
                        {
                            os << ":";
                            dots = true; 
                        }
                        else
                        {
                            os << ",";
                        }

                        os << attribute.name << "(/** TODO: need manual completion here **/)" << endl;
                    }
                }
                catch(const std::exception& e) {}

            os << "{}" << endl << endl;
        }
        else
        {
            if(!method.returnType.empty())
                os << method.returnType << " ";

            os << classData.name << "::" << method.name << "(";

            if(method.arguments.size()>0)
            {
                os << method.arguments[0].type << " " << method.arguments[0].name;

                for(size_t k=1; k<method.arguments.size() ; k++)
                    os << ", " << method.arguments[k].type << " " << method.arguments[k].name;
            }

            os << ")" << endl;

            if (method.returnType == "" || method.returnType == "void")
                os
                << "{" << endl
                << endl
                << "}" << endl
                << endl;
            else
                os
                << "{" << endl
                << "\t" << method.returnType << " autoGen;" << endl
                << endl
                << "\treturn autoGen;" << endl
                << "}" << endl
                << endl;
        }
    }
}

bool needsAlgorithmInclusion(const ClassData& classData)
{
    for(auto& attribute : classData.attributes)
        if(attribute.hasProperty("has")
        || attribute.hasProperty("remove"))
            return true;

    return false;
}

void writeInImpl(ostream& os, const ClassData& classData)
{
    os << "#include \""+classData.name+".h\"" << endl;

    if(needsAlgorithmInclusion(classData))
        os << "#include <algorithm>" << endl;

    os << endl;

    writeAttributesInImpl(os, classData.attributes, classData.name);
    os << endl;
    writeMethodsInImpl(os, classData);
}

bool hasClassContainingPoint(vector<ClassData>& classesData, const Point& point)
{
    for(auto& classData : classesData)
        if(classData.coords.contains(point))
            return true;

    return false;
}

ClassData& findClassContainingPoint(vector<ClassData>& classesData, const Point& point)
{
    for(auto& classData : classesData)
        if(classData.coords.contains(point))
            return classData;

    throw runtime_error("Error! classData containing point could not be found!");
}

void createRelation(const Relation& relation)
{
    auto& begin = findClassContainingPoint(classesData, relation.begin);
    auto& end   = findClassContainingPoint(classesData, relation.end);

    if(count(begin.dependantClasses.begin(), begin.dependantClasses.end(), end.name) == 0)
        begin.dependantClasses.push_back(end.name);

    string message = relation.message;
    string attribute;
    string attrType = end.name;

    switch(relation.type)
    {
        case COMPOSITION:

            if(message.find("shared") < message.length())
            {
                attrType = "shared_ptr<"+attrType+">";
                message = message.substr(0, message.find("shared")) + message.substr(message.find("shared")+strlen("shared"));
            }
            else
            if(getClassDataByName(end.name).isVirtual())
                attrType = "unique_ptr<"+attrType+">";

            if(message.find('*') < message.length())
            {
                attrType = "vector<"+attrType+">";
                message = message.substr(message.find('*')+1);
            }   

            if(message.find('[') < message.length())
            {
                auto s = split(message,"[");
                attribute = s[0]+":"+attrType+"["+s[1];
            }
            else
            {
                attribute = message+":"+attrType;
            }

            begin.addAttribute(parseAttribute(attribute));
            break;

        case AGGREGATION:
            if(message.find('*') < message.length())
            {
                if(message.find('&') < message.length())
                    throw runtime_error("Error! An attempt of creating a container of references was detected: "+message);

                attrType = "vector<weak_ptr<"+attrType+">>";
                message = message.substr(message.find('*')+1);
            }  
            else if(message.find('&') < message.length())
            {
                attrType = attrType+"&";
                message = message.substr(0, message.find('&')) + message.substr(message.find('&')+1);
            }
            else
                attrType = "weak_ptr<"+attrType+">";            

            if(message.find('[') < message.length())
            {
                auto s = split(message,"[");
                attribute = s[0]+":"+attrType+"["+s[1];
            }
            else
            {
                attribute = message+":"+attrType;
            }

            begin.addAttribute(parseAttribute(attribute));
            break;

        case INHERITANCE:
            begin.inheritances.push_back({
                end.isInterface,
                getClassDataByName(end.name)
            });
            break;
    }
}

void writeFiles(const ClassData& classData, const std::string& fileDestination)
{
    ofstream ofsh(fileDestination+classData.name+".h");
    writeInHeader(ofsh, classData);

    if(!classData.isInterface)
    {   
        ofstream ofsi(fileDestination+classData.name+".cpp");
        writeInImpl(ofsi, classData);
    }
}

int main(int argc, char* argv[])
{
    /*
    if(argc==1) {
        cout << "Error! missing arguments!" << endl;
        return 1;
    }
    */

    info = consoleGui();

    XMLDocument doc;

    // doc.LoadFile(argv[1]);
    doc.LoadFile(info.uxfPath.c_str());
    auto elemsList = doc.FirstChildElement("diagram");

    globalPackage.coords = Rect (
        Point (
            numeric_limits<int>::min() / 4,
            numeric_limits<int>::min() / 4
        ),
        numeric_limits<int>::max() / 2,
        numeric_limits<int>::max() / 2
    );
    globalPackage.name = info.srcFolderPath;

    vector<Package> packages;

    for (
        XMLElement* elem;
        (elem=elemsList->FirstChildElement("element")) != nullptr;
        elemsList->DeleteChild(elem)
    )
        if(string(elem->FirstChildElement("id")->GetText())=="UMLPackage")
            packages.push_back(parsePackage(elem));

    sort(packages.begin(), packages.end(),
        [] (Package& a, Package& b) {
            return a.coords.w * a.coords.h > b.coords.w * b.coords.h;
        }
    );

    for(auto & pack : packages)
        globalPackage.insert(pack);

    globalPackage.makeDirectories("");

    //doc.LoadFile(argv[1]);
    doc.LoadFile(info.uxfPath.c_str());
    elemsList = doc.FirstChildElement("diagram");

    for (
        XMLElement* elem;
        (elem=elemsList->FirstChildElement("element")) != nullptr;
        elemsList->DeleteChild(elem)
    )
        if(string(elem->FirstChildElement("id")->GetText())=="UMLClass")
            classesData.push_back(parseClassData(elem));

    //doc.LoadFile(argv[1]);
    doc.LoadFile(info.uxfPath.c_str());
    elemsList = doc.FirstChildElement("diagram");

    for (
        XMLElement* elem;
        (elem=elemsList->FirstChildElement("element")) != nullptr;
        elemsList->DeleteChild(elem)
    )
        if(string(elem->FirstChildElement("id")->GetText())=="Relation")
            createRelation(parseRelation(elem));

    for (auto& classData : classesData)
        writeFiles(classData, globalPackage.getDestination(classData));

    std::cout << "The files have been generated successfully" << std::endl;
    return 0;
}
