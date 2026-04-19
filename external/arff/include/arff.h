/*
 * Emilio Cazares Borbon 01/28/2026
 * 
 * Read ARFF files (not Sparse ARFF).
 * 
 * Created for Georgetown University Data Mining COSC 3590, Spring 2026.
 * 
 * ARFF reference: https://ml.cms.waikato.ac.nz/weka/arff.html
 * 
 * Hours spent on this library: 10 hours. (Hours spent are for project 01)
 */


#ifndef READ_ARFF_H_
#define READ_ARFF_H_


/****************************************
 *                                      *
 *               Includes               *
 *                                      *
 ****************************************/
#include <iostream>
#include <fstream>
#include <sstream>
//#include <iomanip>
#include <string>
#include <vector>
#include <algorithm>
//#include <stdexcept>
#include <exception>


namespace arff
{

/****************************************
 *                                      *
 *              Constants               *
 *                                      *
 ****************************************/
const char KEY_WORD = '@';
const char COMMENT_CHAR = '%';
const std::string RELATION_KEYWORD = "@relation";
const std::string ATTRIBUTE_KEYWORD = "@attribute";
const std::string DATA_KEYWORD = "@data";
const char VALUE_LIST_START = '{';
const char VALUE_LIST_END = '}';
const char VALUE_LIST_DELIMITER = ',';
const char STRING_DELIMITER = '\'';
const std::string MISSING_VALUE = "?";


/****************************************
 *                                      *
 *             Namespaces               *
 *                                      *
 ****************************************/ 
//using std::cerr;
//using std::cin;
//using std::endl;
//using std::string;
//using std::vector;
//using std::ifstream;
//using std::stringstream;

//using std::exception;


/****************************************
 *                                      *
 *        Structs, Enums, Typedefs      *
 *                                      *
 ****************************************/ 
enum AttributeType
{
    NUMERIC,
    NOMINAL,
    STRING, // Not implemented, yet
    DATE // Not implemented, yet
};

struct Attribute
{
    std::string name;
    AttributeType type;
    std::vector<std::string> values;
};

struct DataInstance
{
    std::vector<std::string> values;
};

struct Comment
{
    int lineNumber;
    std::string text;
};

struct ParseError
{
    int lineNumber;
    std::string errorMessage;
    std::string lineContent;
};


/****************************************
 *                                      *
 *               Classes                *
 *                                      *
 ****************************************/
class ARFF
{
    friend std::ostream & operator<< (std::ostream &os, const ARFF &arff);
    
public:
    /*  Constructors and Destructor  */
    ARFF ();
    ARFF (const std::string &filename);
    ARFF (const ARFF &other);
    ~ARFF ();
    
    ARFF & operator= (const ARFF &other);
    void copyARFF (const ARFF &other);
    
    /*  Member Functions  */
    void readARFF (const std::string &filename);
    void printParseErrors () const;
    //void printARFF () const;
    // Should probably have the replacement functions somewhere?

//private:
    /*  Data Members  */
    // Metadata
    std::string filename;
    //int nAttributes;
    
    // Header
    std::string relation;
    std::vector<Attribute> attributes;
    
    // Data
    std::vector<DataInstance> data;
    std::vector<Comment> comments;
    std::vector<ParseError> parseErrors;
    
    /*  Helper Functions  */
    void clearARFF ();
    void parseAttributes (std::stringstream &ss, int lineNumber, const std::string &line);
    void parseData (std::ifstream &inFile, int &lineNumber);
    void parseComments (std::stringstream &ss, int lineNumber);
    void validateDataInstance (const DataInstance &dataInstance);

    std::string getName (std::stringstream &ss);
};

// functions
/****************************************
 *                                      *
 *             Functions                *
 *                                      *
 ****************************************/
std::string & tolower (std::string &str);

std::ostream & operator<< (std::ostream &os, const Attribute &attribute);
std::ostream & operator<< (std::ostream &os, const DataInstance &dataInstance);
std::ostream & operator<< (std::ostream &os, const Comment &comment);
std::ostream & operator<< (std::ostream &os, const ParseError &error);

} // namespace arff

#endif /*  READ_ARFF_H_  */


/****************************************
 *                                      *
 *          Debugging macro             *
 *                                      *
 ****************************************/
#ifdef ARFF_MAIN_DEBUGGER
#define ARFF_DEBUGGER
#define ARFF_CONSTRUCTOR_DEBUGGER
#define ARFF_DESTRUCTOR_DEBUGGER
#define ARFF_COPY_DEBUGGER
#define ARFF_READ_DEBUGGER
#endif /*  ARFF_MAIN_DEBUGGER  */

#ifdef ARFF_DEBUGGER
#undef ARFF_DEBUGGER
#define ARFF_DEBUGGER(msg) std::cerr << "[ARFF_DEBUGGER] " << msg << std::endl;
#else
#define ARFF_DEBUGGER(msg)
#endif /*  ARFF_DEBUGGER  */

#ifdef ARFF_CONSTRUCTOR_DEBUGGER
#undef ARFF_CONSTRUCTOR_DEBUGGER
#define ARFF_CONSTRUCTOR_DEBUGGER(msg) std::cerr << "[ARFF_CONSTRUCTOR_DEBUGGER] " << msg << std::endl;
#else
#define ARFF_CONSTRUCTOR_DEBUGGER(msg)
#endif /*  ARFF_CONSTRUCTOR_DEBUGGER  */

#ifdef ARFF_DESTRUCTOR_DEBUGGER
#undef ARFF_DESTRUCTOR_DEBUGGER
#define ARFF_DESTRUCTOR_DEBUGGER(msg) std::cerr << "[ARFF_DESTRUCTOR_DEBUGGER] " << msg << std::endl;
#else
#define ARFF_DESTRUCTOR_DEBUGGER(msg)
#endif /*  ARFF_DESTRUCTOR_DEBUGGER  */

#ifdef ARFF_COPY_DEBUGGER
#undef ARFF_COPY_DEBUGGER
#define ARFF_COPY_DEBUGGER(msg) std::cerr << "[ARFF_COPY_DEBUGGER] " << msg << std::endl;
#else
#define ARFF_COPY_DEBUGGER(msg)
#endif /*  ARFF_COPY_DEBUGGER  */

#ifdef ARFF_READ_DEBUGGER
#undef ARFF_READ_DEBUGGER
#define ARFF_READ_DEBUGGER(msg) std::cerr << "[ARFF_READ_DEBUGGER] " << msg << std::endl;
#else
#define ARFF_READ_DEBUGGER(msg)
#endif /*  ARFF_READ_DEBUGGER  */