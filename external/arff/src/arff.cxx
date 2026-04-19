/*
 * Emilio Cazares Borbon 01/28/2026
 * 
 * Read ARFF files (not Sparse ARFF). 
 * 
 * Created for Georgetown University Data Mining COSC 3590, Spring 2026.
 * 
 * Detail in `arff.h`. 
 */


#include "../include/arff.h"

namespace
{
std::string trimWhitespace(const std::string &input)
{
    const std::string whitespace = " \t\r\n";
    const size_t start = input.find_first_not_of(whitespace);
    if (start == std::string::npos)
    {
        return "";
    }

    const size_t end = input.find_last_not_of(whitespace);
    return input.substr(start, end - start + 1);
}
}

namespace arff
{

/****************************************************************
 ****************************************************************
 **                                                            **
 **                  ARFF Class Implementation                 **
 **                                                            **
 ****************************************************************
 ****************************************************************/


/****************************************************************
 *                      Default Constructor                     *
 ****************************************************************/
ARFF::ARFF ()
{
    ARFF_CONSTRUCTOR_DEBUGGER ("Entered constructor ARFF::ARFF ()");

    this->filename = "";
    this->relation = "";

    ARFF_CONSTRUCTOR_DEBUGGER ("Exiting constructor ARFF::ARFF ()");
    return;
}

/****************************************************************
 *                 Parameterized Constructor                    *
 ****************************************************************/
ARFF::ARFF (const std::string &filename)
{
    ARFF_CONSTRUCTOR_DEBUGGER ("Entered constructor ARFF::ARFF (const std::string &filename)");

    this->filename = filename;
    this->relation = "";

    ARFF_CONSTRUCTOR_DEBUGGER ("Calling ARFF::readARFF (const std::string &filename) from constructor");

    readARFF (filename);

    ARFF_CONSTRUCTOR_DEBUGGER ("Returned from ARFF::readARFF (const std::string &filename) in constructor");
    ARFF_CONSTRUCTOR_DEBUGGER ("Exiting constructor ARFF::ARFF (const std::string &filename)");

    return;
}

/****************************************************************
 *                     Copy Constructor                         *
 ****************************************************************/
ARFF::ARFF (const ARFF &other)
{
    ARFF_CONSTRUCTOR_DEBUGGER ("Entered copy constructor ARFF::ARFF (const ARFF &other)");

    this->filename = "";
    this->relation = "";

    ARFF_COPY_DEBUGGER ("Calling ARFF::copyARFF (const ARFF &other) from copy constructor");
    copyARFF (other);
    ARFF_COPY_DEBUGGER ("Returned from ARFF::copyARFF (const ARFF &other) in copy constructor");

    ARFF_CONSTRUCTOR_DEBUGGER ("Exiting copy constructor ARFF::ARFF (const ARFF &other)");

    return;
}

/****************************************************************
 *                      Destructor                              *
 ****************************************************************/
ARFF::~ARFF ()
{
    ARFF_DESTRUCTOR_DEBUGGER ("Entered destructor ARFF::~ARFF ()");
    ARFF_DESTRUCTOR_DEBUGGER ("Calling ARFF::clearARFF () from destructor");

    clearARFF ();

    ARFF_DESTRUCTOR_DEBUGGER ("Returned from ARFF::clearARFF () in destructor");
    ARFF_DESTRUCTOR_DEBUGGER ("Exiting destructor ARFF::~ARFF ()");

    return;
}

/****************************************************************
 *                  Assignment Operator                        *
 ****************************************************************/
ARFF & ARFF::operator= (const ARFF &other)
{
    ARFF_COPY_DEBUGGER ("Entered assignment operator ARFF::operator= (const ARFF &other)");

    if (this != &other)
    {
        ARFF_COPY_DEBUGGER ("Calling ARFF::copyARFF (const ARFF &other) from assignment operator");
        copyARFF (other);
        ARFF_COPY_DEBUGGER ("Returned from ARFF::copyARFF (const ARFF &other) in assignment operator");
    }

    ARFF_COPY_DEBUGGER ("Exiting assignment operator ARFF::operator= (const ARFF &other)");

    return *this;
}

/****************************************************************
 *                      Copy Function                           *
 ****************************************************************/
void ARFF::copyARFF (const ARFF &other)
{
    ARFF_COPY_DEBUGGER ("Entered copy function ARFF::copyARFF (const ARFF &other)");

    if (this == &other)
    {
        ARFF_COPY_DEBUGGER ("Current object is the same as other object; no copy needed.");
        return;
    }

    try
    {
        ARFF_COPY_DEBUGGER ("Entering try block to copy ARFFReader data members.");

        filename = other.filename;
        relation = other.relation;
        attributes = other.attributes;
        data = other.data;
        comments = other.comments;
        parseErrors = other.parseErrors;

        ARFF_COPY_DEBUGGER ("Successfully copied ARFFReader data members.");
    }
    catch (std::exception &e)
    {
        ARFF_COPY_DEBUGGER ("[EXCEPTION] Exception caught during copy: " + std::string (e.what ()));
        //cerr << "[READ_ARFF__COPY] " << e.what() << endl;
        this->clearARFF ();
        ARFF_COPY_DEBUGGER ("[EXCEPTION] Cleared current ARFFReader object due to error during copy.");
        //cerr << "[READ_ARFF__COPY] Cleared current ARFFReader object due to error during copy." << endl;
    }
    catch (...)
    {
        //cerr << "[READ_ARFF__COPY] Some fatal error during copy!" << endl;
        ARFF_COPY_DEBUGGER ("[EXCEPTION] Some fatal error during copy!");
        this->clearARFF ();
        //cerr << "[READ_ARFF__COPY] Cleared current ARFFReader object due to error during copy." << endl;
        ARFF_COPY_DEBUGGER ("[EXCEPTION] Cleared current ARFFReader object due to error during copy.");
    }

ARFF_COPY_DEBUGGER ("Exiting copy function ARFF::copyARFF (const ARFF &other)");

return;
}

/****************************************************************
 *                     clearARFF Function                       *
 ****************************************************************/
void ARFF::clearARFF ()
{
    ARFF_DESTRUCTOR_DEBUGGER ("Entered helper function ARFF::clearARFF ()");

    filename = "";
    relation = "";
    attributes.clear();
    data.clear();
    comments.clear();
    parseErrors.clear();

    ARFF_DESTRUCTOR_DEBUGGER ("Exiting helper function ARFF::clearARFF ()");

    return;
}

/****************************************************************
 *                     readARFF Function                        *
 ****************************************************************/
void ARFF::readARFF (const std::string &filename)
{
    ARFF_READ_DEBUGGER ("Entered member function ARFF::readARFF (const std::string &filename)");
    
    clearARFF ();
    this->filename = filename;
    std::ifstream inFile;

    // attempting to open file
    try
    {
        ARFF_READ_DEBUGGER ("Attempting to open file: " + filename);
        if (filename.empty ())
            throw std::runtime_error ("Filename is empty.");
        if (filename.substr (filename.find_last_of (".") + 1) != "arff")
            throw std::runtime_error ("File is not an ARFF file: " + filename);

        inFile.open (filename);
        if (!inFile.is_open ())
            throw std::runtime_error ("Could not open ARFF file: " + filename);
        ARFF_READ_DEBUGGER ("Successfully opened file: " + filename);
    }
    catch (std::exception &e)
    {
        ARFF_READ_DEBUGGER ("[EXCEPTION] Exception caught during file open: " + std::string (e.what ()));
        if (inFile.is_open ())
            inFile.close ();
        this->clearARFF ();
        ARFF_READ_DEBUGGER ("[EXCEPTION] Cleared ARFFReader object due to error during file open.");
        ARFF_READ_DEBUGGER ("Exiting member function ARFFReader::readARFF (const std::string &filename)");
        return;
    }
    catch (...)
    {
        ARFF_READ_DEBUGGER ("[EXCEPTION] Some fatal error opening file!");
        if (inFile.is_open ())
            inFile.close ();
        this->clearARFF ();
        ARFF_READ_DEBUGGER ("[EXCEPTION] Cleared ARFFReader object due to fatal error during file open.");
        ARFF_READ_DEBUGGER ("Exiting member function ARFFReader::readARFF (const std::string &filename)");
        return;
    }

    // reading the file
    try
    {
        ARFF_READ_DEBUGGER ("Beginning to parse ARFF file: " + filename);

        int lineNumber = 0;
        bool foundData = false;
        bool foundRelation = false;
        std::string line = "";
        std::string keyWord = "";

        while (getline (inFile, line) && !foundData)
        {
            lineNumber++;
            ARFF_READ_DEBUGGER ("****************************************Parsing line number: " + std::to_string (lineNumber) + "****************************************");
            
            // Skip empty lines
            std::stringstream ss (line);
            if (line.empty () || line.find_first_not_of (" \t") == std::string::npos)
                ARFF_READ_DEBUGGER ("****************************************Skipping empty line at line number: " + std::to_string (lineNumber) + "****************************************");


            char firstChar = ss.peek ();
            
            if (firstChar == COMMENT_CHAR)
            {
                parseComments (ss, lineNumber);
                ARFF_READ_DEBUGGER ("****************************************Complete parsing of line number: " + std::to_string (lineNumber) + "****************************************");
            }
            else if (firstChar == KEY_WORD)
            {
                ss >> keyWord;
                tolower (keyWord);

                if (keyWord == RELATION_KEYWORD && !foundRelation)
                {
                    ARFF_READ_DEBUGGER ("Found relation key word '@relation'. Parsing relation name.");
                    relation = getName (ss);
                    foundRelation = true;
                    ARFF_READ_DEBUGGER ("Successfully parsed relation name: " + relation);
                    ARFF_READ_DEBUGGER ("****************************************Complete parsing of line number: " + std::to_string (lineNumber) + "****************************************");
                }
                else if (keyWord == RELATION_KEYWORD && foundRelation)
                    throw std::runtime_error ("Duplicate @relation declaration found at line " + std::to_string (lineNumber) + "");
                else if (keyWord == ATTRIBUTE_KEYWORD)
                {
                    ARFF_READ_DEBUGGER ("Found attribute key word '@attribute'. Parsing attribute.");
                    parseAttributes (ss, lineNumber, line);
                    ARFF_READ_DEBUGGER ("****************************************Complete parsing of attribute at line number: " + std::to_string (lineNumber) + "****************************************");
                }
                else if (keyWord == DATA_KEYWORD)
                {
                    ARFF_READ_DEBUGGER ("Found data key word '@data'. Parsing data section.");
                    foundData = true;
                    ARFF_READ_DEBUGGER ("****************************************Complete parsing of line number: " + std::to_string (lineNumber) + "****************************************");
                    parseData (inFile, lineNumber);
                }
                else
                {
                    ARFF_READ_DEBUGGER ("Skipping unexpected key word at line number " + std::to_string (lineNumber) + ": " + keyWord);
                    ARFF_READ_DEBUGGER ("****************************************Complete parsing of line number: " + std::to_string (lineNumber) + "****************************************");
                }
            }
            else
            {
                ARFF_READ_DEBUGGER ("Skipping unexpected line at line number " + std::to_string (lineNumber) + ": " + line);
                ARFF_READ_DEBUGGER ("****************************************Complete parsing of line number: " + std::to_string (lineNumber) + "****************************************");
            }
        }
    }
    catch (std::exception &e)
    {
        ARFF_READ_DEBUGGER ("[EXCEPTION] Exception caught during file parsing: " + std::string (e.what ()));
        if (inFile.is_open ())
            inFile.close ();
        this->clearARFF ();
        ARFF_READ_DEBUGGER ("[EXCEPTION] Cleared ARFF object due to error during file parsing.");
        ARFF_READ_DEBUGGER ("Exiting member function ARFF::readARFF (const std::string &filename)");
        return;
    }
    catch (...)
    {
        ARFF_READ_DEBUGGER ("[EXCEPTION] Some fatal error during file parsing!");
        if (inFile.is_open ())
            inFile.close ();
        this->clearARFF ();
        ARFF_READ_DEBUGGER ("[EXCEPTION] Cleared ARFF object due to fatal error during file parsing.");
        ARFF_READ_DEBUGGER ("Exiting member function ARFF::readARFF (const std::string &filename)");
        return;
    }

    ARFF_READ_DEBUGGER ("Finished parsing ARFF file: " + filename + " Closing file.");
    inFile.close ();
    ARFF_READ_DEBUGGER ("Exiting member function ARFF::readARFF (const std::string &filename)");

    return;
}

/****************************************************************
 *                     getName Function                         *
 ****************************************************************/
std::string ARFF::getName (std::stringstream &ss)
{
    ARFF_READ_DEBUGGER ("Entered helper function ARFF::getName (stringstream &ss)");

    std::string name = "";

    try
    {
        // removing leading spaces
        char nextChar;

        ss >> nextChar;

        if (nextChar != STRING_DELIMITER)
        {
            ss.putback (nextChar);
            ss >> name;
        }
        else if (nextChar == STRING_DELIMITER)
            getline (ss, name, STRING_DELIMITER);
        else
            throw std::runtime_error ("Error reading name from ARFF file.");
    }
    catch (std::exception &e)
    {
        ARFF_READ_DEBUGGER ("[EXCEPTION] Exception caught during getName: " + std::string (e.what ()));
        name = "";
        ARFF_READ_DEBUGGER ("[EXCEPTION] Set name to empty string due to error during getName.");
    }
    catch (...)
    {
        ARFF_READ_DEBUGGER ("[EXCEPTION] Some fatal error during getName!");
        name = "";
        ARFF_READ_DEBUGGER ("[EXCEPTION] Set name to empty string due to fatal error during getName.");
    }

    ARFF_READ_DEBUGGER ("Retrieved name: \"" + name + "\"");
    ARFF_READ_DEBUGGER ("Exiting helper function ARFF::getName (stringstream &ss)");

    return name;
}

/****************************************************************
 *                  parseAttributes Function                    *
 ****************************************************************/
void ARFF::parseAttributes (std::stringstream &ss, int lineNumber, const std::string &line)
{
    ARFF_READ_DEBUGGER ("Entered helper function ARFF::parseAttributes (stringstream &ss, int lineNumber, const std::string &line)");

    Attribute newAttribute;
    std::string attributeName;
    std::string attributeTypeStr;
    AttributeType type = AttributeType::NUMERIC;
    std::string trash;
    char nextChar = '?';
    char trashChar = '?';

    // reading attribute
    try
    {
        // name
        attributeName = getName (ss);
        newAttribute.name = attributeName;

        // types
        ss >> nextChar;

        ARFF_READ_DEBUGGER ("nextChar before attribute type parsing: \'" + std::string(1, static_cast<char>(nextChar)) + "\'");
        if (nextChar != VALUE_LIST_START)
        {
            ss.putback (nextChar);
            ARFF_READ_DEBUGGER ("Parsing attribute type for attribute: " + attributeName);
            ss >> attributeTypeStr;
            tolower (attributeTypeStr);
            ARFF_READ_DEBUGGER ("Read attribute type string: " + attributeTypeStr + " for attribute: " + attributeName);

            if (attributeTypeStr == "numeric" || attributeTypeStr == "real" || attributeTypeStr == "integer")
                type = AttributeType::NUMERIC;
            else if (attributeTypeStr == "string")
                type = AttributeType::STRING;
            else if (attributeTypeStr == "date")
                type = AttributeType::DATE;
            else
                throw std::runtime_error ("Unknown attribute type string: " + attributeTypeStr + " for attribute: " + attributeName);

            newAttribute.type = type;
            ARFF_READ_DEBUGGER ("Attribute: " + attributeName + " set to type: " + attributeTypeStr);
        }
        else if (nextChar == VALUE_LIST_START)
        {
            std::vector<std::string> nominalValues;
            std::string nominalValue = "";
            type = AttributeType::NOMINAL;
            newAttribute.type = type;

            ARFF_READ_DEBUGGER ("Parsing nominal values for attribute: " + attributeName);
            ss >> nextChar;
            while (nextChar != VALUE_LIST_END)
            {
                if (nextChar == VALUE_LIST_DELIMITER)
                    ss >> nextChar;

                if (nextChar != STRING_DELIMITER)
                {
                    while (nextChar != VALUE_LIST_DELIMITER && nextChar != VALUE_LIST_END)
                    {
                        nominalValue += nextChar;
                        ss >> nextChar;
                    }
                }
                else if (nextChar == STRING_DELIMITER)
                {
                    ss.get (trashChar); // consume opening quote
                    getline (ss, nominalValue, STRING_DELIMITER);
                    ss >> nextChar; // read next char after closing quote
                }
                nominalValues.push_back (nominalValue);
                nominalValues.back() = trimWhitespace(nominalValues.back());
                ARFF_READ_DEBUGGER ("Added nominal value to attribute: " + attributeName + ": \"" + nominalValue + "\"");
                nominalValue = "";

            }

            newAttribute.values = nominalValues;
            ARFF_READ_DEBUGGER ("Completed parsing nominal values for attribute: " + attributeName);
        }
        else
            throw std::runtime_error ("Error reading attribute type from ARFF file.");

        attributes.push_back (newAttribute);
        ARFF_READ_DEBUGGER ("Successfully parsed attribute: " + newAttribute.name);
    }
    catch (std::exception &e)
    {
        ARFF_READ_DEBUGGER ("[EXCEPTION] Exception caught during parseAttributes: " + std::string (e.what ()));
        ARFF_READ_DEBUGGER ("[EXCEPTION] Skipped attribute due to error during parseAttributes.");
        ParseError error;
        error.lineNumber = lineNumber;
        error.errorMessage = std::string(e.what());
        error.lineContent = line;
        parseErrors.push_back(error);
    }
    catch (...)
    {
        ARFF_READ_DEBUGGER ("[EXCEPTION] Some fatal error during parseAttributes!");
        ARFF_READ_DEBUGGER ("[EXCEPTION] Skipped attribute due to fatal error during parseAttributes.");
        ParseError error;
        error.lineNumber = lineNumber;
        error.errorMessage = "Unknown fatal error during attribute parsing";
        error.lineContent = line;
        parseErrors.push_back(error);
    }

    ARFF_READ_DEBUGGER ("Exiting helper function ARFF::parseAttributes (stringstream &ss, int lineNumber, const std::string &line)");

    return;
}

/****************************************************************
 *                     parseData Function                       *
 ****************************************************************/
void ARFF::parseData (std::ifstream &inFile, int &lineNumber)
{
    ARFF_READ_DEBUGGER ("Entered helper function ARFF::parseData (ifstream &inFile, int &lineNumber)");

    DataInstance newDataInstance;
    std::string line = "";
    std::string value = "";
    char nextChar = '?';

    // reading data instances
    try
    {
        nextChar = inFile.peek ();
        if (nextChar == VALUE_LIST_START)
            ARFF_READ_DEBUGGER ("First character in data section is '{' indicating sparse ARFF format, which is not supported.");
        while (getline (inFile, line))
        {
            lineNumber++;
            ARFF_READ_DEBUGGER ("****************************************Parsing data instance at line number: " + std::to_string (lineNumber) + "****************************************");
            
            // Skip empty lines and comments in data section
            if (line.empty () || line.find_first_not_of (" \t") == std::string::npos)
                ARFF_READ_DEBUGGER ("Skipping empty line at line number: " + std::to_string (lineNumber));

            try
            {
                newDataInstance.values.clear ();
                std::stringstream ss (line);
                
                for (size_t attrIndex = 0; attrIndex < attributes.size (); attrIndex++)
                {
                    ss >> nextChar;
                    value = "";
                    if (nextChar != STRING_DELIMITER)
                    {
                        while (nextChar != VALUE_LIST_DELIMITER && ss.good ())
                        {
                            value += nextChar;
                            ss.get (nextChar);
                        }
                    }
                    else if (nextChar == STRING_DELIMITER)
                    {
                        getline (ss, value, STRING_DELIMITER);
                        ss >> nextChar;
                    }
                    value = trimWhitespace(value);
                    if (value.empty ())
                        throw std::runtime_error ("Missing value for attribute " + attributes[attrIndex].name + " at line " + std::to_string (lineNumber) + ".");
                    newDataInstance.values.push_back (value);
                    ARFF_READ_DEBUGGER ("Parsed value for attribute " + attributes[attrIndex].name + ": " + value);
                }
                if (newDataInstance.values.size () != attributes.size ())
                    throw std::runtime_error ("Incorrect number of attribute values for data instance at line " + std::to_string (lineNumber) + ".");
                else if (ss.good ())
                    throw std::runtime_error ("Too many attribute values for data instance at line " + std::to_string (lineNumber) + ".");

                validateDataInstance (newDataInstance);
                data.push_back (newDataInstance);
                ARFF_READ_DEBUGGER ("Successfully parsed data instance at line number: " + std::to_string (lineNumber));
                ARFF_READ_DEBUGGER ("****************************************Complete parsing of data instance at line number: " + std::to_string (lineNumber) + "****************************************");
            }
            catch (std::exception &e)
            {
                ARFF_READ_DEBUGGER ("[EXCEPTION] Exception caught during data instance parsing: " + std::string (e.what ()));
                ARFF_READ_DEBUGGER ("[EXCEPTION] Skipped data instance due to error during parsing.");
                ParseError error;
                error.lineNumber = lineNumber;
                error.errorMessage = std::string(e.what());
                error.lineContent = line;
                parseErrors.push_back(error);
            }
            catch (...)
            {
                ARFF_READ_DEBUGGER ("[EXCEPTION] Some fatal error during data instance parsing!");
                ARFF_READ_DEBUGGER ("[EXCEPTION] Skipped data instance due to fatal error during parsing.");
                ParseError error;
                error.lineNumber = lineNumber;
                error.errorMessage = "Unknown fatal error during data instance parsing";
                error.lineContent = line;
                parseErrors.push_back(error);
            }
        }
    }
    catch (std::exception &e)
    {
        ARFF_READ_DEBUGGER ("[EXCEPTION] Exception caught during parseData: " + std::string (e.what ()));
        ARFF_READ_DEBUGGER ("[EXCEPTION] Error occurred while parsing data section.");
    }
    catch (...)
    {
        ARFF_READ_DEBUGGER ("[EXCEPTION] Some fatal error during parseData!");
        ARFF_READ_DEBUGGER ("[EXCEPTION] Fatal error occurred while parsing data section.");
    }

    ARFF_READ_DEBUGGER ("Exiting helper function ARFF::parseData (ifstream &inFile, int &lineNumber)");

    return;
}

/****************************************************************
 *                 parseComments Function                       *
 ****************************************************************/
void ARFF::parseComments (std::stringstream &ss, int lineNumber)
{
    ARFF_READ_DEBUGGER ("Entered helper function ARFF::parseComments (std::stringstream &ss, int lineNumber)");

    Comment newComment;
    std::string commentText;

    try
    {
        ss.ignore (); // consume COMMENT_CHAR
        getline (ss, commentText, '\n');

        newComment.lineNumber = lineNumber;
        newComment.text = commentText;
        comments.push_back (newComment);

        ARFF_READ_DEBUGGER ("Successfully parsed comment at line " + std::to_string (lineNumber) + ": " + commentText);
    }
    catch (std::exception &e)
    {
        ARFF_READ_DEBUGGER ("[EXCEPTION] Exception caught during parseComments: " + std::string (e.what ()));
        ARFF_READ_DEBUGGER ("[EXCEPTION] Skipped comment due to error during parseComments.");
    }
    catch (...)
    {
        ARFF_READ_DEBUGGER ("[EXCEPTION] Some fatal error during parseComments!");
        ARFF_READ_DEBUGGER ("[EXCEPTION] Skipped comment due to fatal error during parseComments.");
    }

    ARFF_READ_DEBUGGER ("Exiting helper function ARFF::parseComments (stringstream &ss, int lineNumber)");

    return;
}

/****************************************************************
 *                  operator<< Friend Function                  *
 ****************************************************************/
std::ostream & operator<< (std::ostream &os, const ARFF &arff)
{
    ARFF_DEBUGGER ("Entered friend function operator<< (std::ostream &os, const ARFF &arff)");
    ARFF_DEBUGGER ("Exporting ARFF Data Structure in the ARFF format");

    os << RELATION_KEYWORD << " " << arff.relation << "\n\n";
    for (const auto &attr : arff.attributes)
        os << attr << "\n";
        
    for (const auto &comment : arff.comments)
        os << comment << "\n";

    os << "\n\n" << DATA_KEYWORD << "\n";
    for (const auto &instance : arff.data)
        os << instance << "\n";

    ARFF_DEBUGGER ("Exiting friend function operator<< (std::ostream &os, const ARFF &arff)");

    return os;
}


/****************************************************************
 ****************************************************************
 **                                                            **
 **            Additional Functions Implementation             **
 **                                                            **
 ****************************************************************
 ****************************************************************/


/****************************************************************
 *                     tolower Function                         *
 ****************************************************************/
std::string & tolower (std::string &str)
{
    ARFF_DEBUGGER ("Entered function tolower (std::string &str)");
    for (size_t i = 0; i < str.length (); i++)
    {
        str[i] = std::tolower (str[i]);
    }
    ARFF_DEBUGGER ("Exiting function tolower (std::string &str)");
    return str;
}

/****************************************************************
 *            operator<< for Attribute Struct                   *
 ****************************************************************/
std::ostream & operator<< (std::ostream &os, const Attribute &attribute)
{
    ARFF_DEBUGGER ("Entered function operator<< (std::ostream &os, const Attribute &attribute)");

    os << ATTRIBUTE_KEYWORD << " " << attribute.name << " ";
    if (attribute.type == AttributeType::NUMERIC)
        os << "numeric";
    else if (attribute.type == AttributeType::STRING)
        os << "string";
    else if (attribute.type == AttributeType::DATE)
        os << "date";
    else if (attribute.type == AttributeType::NOMINAL)
    {
        os << "{ ";
        for (size_t i = 0; i < attribute.values.size (); i++)
        {
            os << attribute.values[i];
            if (i != attribute.values.size () - 1)
                os << ", ";
        }
        os << " }";
    }

    ARFF_DEBUGGER ("Exiting function operator<< (std::ostream &os, const Attribute &attribute)");

    return os;
}

/****************************************************************
 *         operator<< for DataInstance Struct                   *
 ****************************************************************/
std::ostream & operator<< (std::ostream &os, const DataInstance &dataInstance)
{
    ARFF_DEBUGGER ("Entered function operator<< (std::ostream &os, const DataInstance &dataInstance)");

    for (size_t i = 0; i < dataInstance.values.size (); i++)
    {
        os << dataInstance.values[i];
        if (i != dataInstance.values.size () - 1)
            os << ", ";
    }

    ARFF_DEBUGGER ("Exiting function operator<< (std::ostream &os, const DataInstance &dataInstance)");

    return os;
}

/****************************************************************
 *            operator<< for Comment Struct                     *
 ****************************************************************/
std::ostream & operator<< (std::ostream &os, const Comment &comment)
{
    ARFF_DEBUGGER ("Entered function operator<< (std::ostream &os, const Comment &comment)");

    // since comments are throughout the file, print the comment but add the line number as a prefix
    os << COMMENT_CHAR << " (line " << comment.lineNumber << ") " << comment.text;

    ARFF_DEBUGGER ("Exiting function operator<< (std::ostream &os, const Comment &comment)");
    
    return os;
}

/****************************************************************
 *           operator<< for ParseError Struct                   *
 ****************************************************************/
std::ostream & operator<< (std::ostream &os, const ParseError &error)
{
    ARFF_DEBUGGER ("Entered function operator<< (std::ostream &os, const ParseError &error)");

    os << "[Line " << error.lineNumber << "] " << error.errorMessage << "\n";
    os << "  Content: " << error.lineContent;

    ARFF_DEBUGGER ("Exiting function operator<< (std::ostream &os, const ParseError &error)");
    
    return os;
}

/****************************************************************
 *                 printParseErrors Function                    *
 ****************************************************************/
void ARFF::printParseErrors () const
{
    ARFF_DEBUGGER ("Entered member function ARFF::printParseErrors ()");

    if (parseErrors.empty())
    {
        std::cout << "No parse errors found." << std::endl;
    }
    else
    {
        std::cout << "\n========================================" << std::endl;
        std::cout << "Parse Errors (" << parseErrors.size() << " total)" << std::endl;
        std::cout << "========================================" << std::endl;
        
        for (const auto &error : parseErrors)
        {
            std::cout << error << std::endl;
        }
        
        std::cout << "========================================\n" << std::endl;
    }

    ARFF_DEBUGGER ("Exiting member function ARFF::printParseErrors ()");
}

/****************************************************************
 *                validateDataInstance Function                 *
 ****************************************************************/
void ARFF::validateDataInstance (const DataInstance &instance)
{
    ARFF_READ_DEBUGGER ("Entered helper function ARFF::validateDataInstance (const DataInstance &instance)");

    for (size_t i = 0; i < instance.values.size (); i++)
    {
        const Attribute &attr = attributes[i];
        const std::string &value = instance.values[i];

        if (value == MISSING_VALUE)
        {
            // missing value, skip validation
            continue;
        }

        try
        {
            if (attr.type == AttributeType::NUMERIC)
            {
                size_t idx;
                std::stod(value, &idx);
                if (value != MISSING_VALUE && idx != value.length())
                    throw std::runtime_error("Non-numeric value for numeric attribute " + attr.name + ": " + value);
                
            }
            else if (attr.type == AttributeType::NOMINAL)
            {
                if (std::find(attr.values.begin(), attr.values.end(), value) == attr.values.end())
                    throw std::runtime_error("Invalid nominal value for attribute " + attr.name + ": " + value);
            }
            else if (attr.type == AttributeType::DATE)
            {
                // nothign yet
            }
            else if (attr.type == AttributeType::STRING)
            {
                // nothing yet
            }
            else
                throw std::runtime_error("Unknown attribute type for attribute " + attr.name);
        }
        catch (std::exception &e)
        {
            ARFF_READ_DEBUGGER ("[EXCEPTION] Exception caught during data instance validation: " + std::string (e.what ()));
            throw;
        }
    }

    ARFF_READ_DEBUGGER ("Exiting helper function ARFF::validateDataInstance (const DataInstance &instance)");

    return;
}

} // namespace arff