#include <iostream>
#include <fstream>
#include <string>

using namespace std;


enum DefaultReturnType {
    EMPTY,
    DEFAULT_VALUE,
    THROW_EXCEPTION
};


struct Info
{
    string uxfPath = "";
    string srcFolderPath = "";

    bool generateDoc = true;
    bool generateTODOForClassDescription = true;
    bool generateDocAttributeProperties = true;
    bool generateDocConstructors = true;
    bool generateDocTemplates = true;
    string legalNoticeText = "";

    bool generateImplAttributeProperties = true;
    bool generateImplConstructors = true;
    bool generateTODOToImplEmptyMethods = true;
    DefaultReturnType defaultReturnType = THROW_EXCEPTION;
};

bool hasEnding (std::string const &fullString, std::string const &ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

string selectUxfFile()
{
    cout << "Write the destination of the uxf file:" << endl;

    string filePath;
    cin >> filePath;

    filePath.substr(0, filePath.find('\n'));

    ifstream f(filePath.c_str());
    
    while(!f.good() || !hasEnding(filePath, ".uxf"))
    {
        if(!f.good())
            cout << "Error! The file could not be found!" << endl;
        else
            cout << "The file not an uxf file" << endl;

        cout << "Please try again" << endl;
        cin >> filePath;
    }
    
    return filePath;
}

string selectFolderPath()
{
    cout << "Write the folder in which the source code will be generated:" << endl;

    string folderPath;
    cin >> folderPath;

    if(folderPath.back() != '/')
        folderPath += '/';

    return folderPath;
}

const char* onOrOff(bool b) {
    return b ? "[ON]" : "[OFF]";
}

void documentationOptions(Info& info)
{
    while(true)
    {
        cout << endl;
        cout << endl;
        cout << endl;
        cout << "The current documentation options:" << endl;
        if(info.generateDoc)
        {
            cout << "\t-documentation generated for accessors and mutators: " << onOrOff(info.generateDocAttributeProperties) << endl;
            cout << "\t-documentation generated for constructors: " << onOrOff(info.generateDocConstructors) << endl;
            cout << "\t-documentation generated for other methods: " << onOrOff(info.generateDocTemplates) << endl;
        }
        else
        {
            cout << "\t-no documentation generated" << endl;
        }

        cout << endl;
        cout << "Select your choice:" << endl;
        cout << endl;

        if(info.generateDoc)
        {
            cout << "-d: don't generate documentation (will deactivate all the other options)" << endl;

            if (info.generateDocAttributeProperties)
                cout << "-a: don't generate documentation for accessors and mutators (get, set, add, contains, remove, clear)" << endl;
            else
                cout << "-a: generate documentation for accessors and mutators (get, set, add, contains, remove, clear)" << endl;

            if (info.generateDocConstructors)
                cout << "-c: don't generate documentation for constructors" << endl;
            else
                cout << "-c: generate documentation for constructors" << endl;

            if (info.generateDocTemplates)
                cout << "-o: don't generate documentation templates for other methods" << endl;
            else
                cout << "-o: generate documentation templates for other methods" << endl;
        }
        else
        {
            cout << "-d: generate documentation (the other options would be available after activation)" << endl;
        }
        
        cout << "-r: return" << endl;

        string s;
        cin >> s;

        if(s.length() <= 2)
        switch(s[0])
        {
            case 'd':
                info.generateDoc = !info.generateDoc;
                continue;

            case 'a':
                info.generateDocAttributeProperties = !info.generateDocAttributeProperties;
                continue;

            case 'c':
                info.generateDocConstructors = !info.generateDocConstructors;
                continue;

            case 'o':
                info.generateDocTemplates = !info.generateDocTemplates;
                continue;

            case 'r':
                return;
        }

        cout << endl;
        cout << "Error! the entry is invalid!" << endl;
        cout << endl;
    }
}

DefaultReturnType selectDefaultReturnType()
{
    while(true)
    {
        cout << endl;
        cout << endl;
        cout << endl;
        cout << "Choose a default return type for empty functions:" << endl;
        cout << endl;
        cout << "-e: empty (will create compilation errors)" << endl;
        cout << "-d: default return value (0 for numbers, nullptr for pointers, default constructors for objects, nothing for void)" << endl;
        cout << "-x: throw a runtime error which signals the method has not been implemented yet" << endl;
        cout << endl;

        string s;
        cin >> s;

        if(s.length() <= 2)
        switch(s[0])
        {
            case 'e': return EMPTY;
            case 'd': return DEFAULT_VALUE;
            case 'x': return THROW_EXCEPTION;
        }

        cout << endl;
        cout << "Error! the entry is invalid!" << endl;
        cout << endl;
    }
}

void implementationOptions(Info& info)
{
    while(true)
    {
        cout << endl;
        cout << endl;
        cout << endl;
        cout << "The current implementation options:" << endl;
        cout << "\timplementation generated for accessors and mutators: " << onOrOff(info.generateImplAttributeProperties) << endl;
        cout << "\timplementation generated for constructors: " << onOrOff(info.generateDocConstructors) << endl;
        cout << "\tTODO generated to reming implementing empty methods: " << onOrOff(info.generateTODOToImplEmptyMethods) << endl;

        cout << "\tDefault return type for empty methods: ";
        switch(info.defaultReturnType) {
            case EMPTY: cout << "empty"; break;
            case DEFAULT_VALUE: cout << "return default value"; break;
            case THROW_EXCEPTION: cout << "throw exception"; break;
        }

        cout << endl;
        cout << "Select your choice:" << endl;
        cout << endl;

        if(info.generateImplAttributeProperties)
            cout << "-a: don't generate implementation for method accessors and mutators (get, set, add, contains, remove, clear)" << endl;
        else
            cout << "-a: generate implementation for method accessors and mutators (get, set, add, contains, remove, clear)" << endl;
    
        if(info.generateImplConstructors)
            cout << "-c: don't generate implementation for constructors" << endl;
        else
            cout << "-c: generate implementation for constructors" << endl;

        if(info.generateTODOToImplEmptyMethods)
            cout << "-t: don't generate a TODO for empty methods" << endl;
        else
            cout << "-t: generate TODO for empty methods" << endl;

        cout << endl;
        cout << "-d: select the default return type for empty methods" << endl;
        cout << "-r: return" << endl;

        string s;
        cin >> s;

        if(s.length() <= 2)
        switch(s[0])
        {
            case 'a':
                info.generateImplAttributeProperties = !info.generateImplAttributeProperties;
                continue;

            case 'c':
                info.generateImplConstructors = !info.generateImplConstructors;
                continue;

            case 't':
                info.generateTODOToImplEmptyMethods = !info.generateTODOToImplEmptyMethods;
                continue;

            case 'd':
                info.defaultReturnType = selectDefaultReturnType();
                continue;

            case 'r':
                return;
        }

        cout << endl;
        cout << "Error! the entry is invalid!" << endl;
        cout << endl;
    }
}

void additionnalOptions(Info& info)
{
    while(true)
    {
        cout << endl;
        cout << endl;
        cout << endl;
        cout << "Select the options category you wish to change:" << endl;
        cout << endl;
        cout << "-d: change the documentation options" << endl;
        cout << "-i: change the implementation options" << endl;
        cout << "-q: quit the options menu and generate the source code " << endl;
        cout << endl;

        string s;
        cin >> s;
        
        if(s.length() <= 2)
        switch(s[0])
        {
            case 'd':
                documentationOptions(info);
                continue;

            case 'i':
                implementationOptions(info);
                continue;

            case 'q':
                return;
        }

        cout << endl;
        cout << "Error! the entry is invalid!" << endl;
        cout << endl;
    }
}

Info consoleGui()
{
    Info info;

    info.uxfPath = selectUxfFile();
    info.srcFolderPath = selectFolderPath();

    // additionnalOptions(info);

    return info;
}