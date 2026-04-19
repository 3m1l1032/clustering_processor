/* 
 * Emilio Cazares Borbon 04/19/2024
 * 
 * Clustering Processor Header File.
 * 
 * Created for Georgetown University Data Mining COSC 3590, Spring 2026.
 * 
 * Hours spent on importing and editing previous code: 1 hour.
 */

#ifndef CLUSTERING_PROCESSOR_H
#define CLUSTERING_PROCESSOR_H

/****************************************
 *                                      *
 *              Includes                *
 *                                      *
 ****************************************/
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>
#include <exception>
#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <thread>
#include <filesystem>
#include <chrono>
#include <sstream>

#include "../external/arff/include/arff.h"


namespace ClusteringProcessor
{

/****************************************
 *                                      *
 *             Namespaces               *
 *                                      *
 ****************************************/
using arff::ARFF;
using arff::DataInstance;
using arff::Attribute;
using arff::AttributeType;
using arff::NUMERIC;
using arff::NOMINAL;


/****************************************
 *                                      *
 *      Strucs, Enums, and Typedefs     *
 *                                      *
 ****************************************/



/****************************************
 *                                      *
 *              Classes                 *
 *                                      *
 ****************************************/
class ClusteringProcessor
{
public:
    /*  Constructors and Destructor  */
    ClusteringProcessor ();
    ClusteringProcessor (const std::string &filename);
    ClusteringProcessor (const ClusteringProcessor &other);
    ~ClusteringProcessor ();
    
    ClusteringProcessor & operator= (const ClusteringProcessor &other);
    void copyProcessor (const ClusteringProcessor &other);
    
    /*  Member Functions  */
    void readDataSet (const std::string &filename);
    
    /*  Cleanup Functions  */
    void clearProcessor ();

private:
    /*  Data Members  */
    ARFF dataset;

    /*  Helper Functions  */
};


/****************************************
 *                                      *
 *            Functions                 *
 *                                      *
 ****************************************/


} // namespace ClusteringProcessor

#endif /*  CLUSTERING_PROCESSOR_H  */


/****************************************
 *                                      *
 *          Debugging Macros            *
 *                                      *
 ****************************************/