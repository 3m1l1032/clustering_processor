/* 
 * Emilio Cazares Borbon 04/19/2024
 * 
 * Clustering Processor Header File.
 * 
 * Created for Georgetown University Data Mining COSC 3590, Spring 2026.
 * 
 * Hours spent on importing and editing previous code: 1 hour.
 * Hours spent on adding pre-processing from previous proeject: 1.5 hours.
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
#include <map>
#include <mutex>
#include <atomic>
#include <limits>

#include "../external/arff/include/arff.h"


namespace ClusteringProcessorNS
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
enum replaceMissingStrategy
{
    MEAN_MODE
};

struct normalizationStats
{
    double mean;
    double stddev;
};



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
    void setClassAttribute (std::string classAttributeName);
    void replaceMissingValues (replaceMissingStrategy strategy = MEAN_MODE);
    void normalizeNumericValues ();
    void printNormalizationInfo () const;

    std::string getReplacementValue (const size_t attributeIndex) const;
    std::string getNormalizedValue (const size_t attributeIndex, const std::string &rawValue) const;
    
    /*  Cleanup Functions  */
    void clearProcessor ();

private:
    /*  Data Members  */
    ARFF dataset;

    Attribute classAttribute;
    size_t classAttributeIndex;
    DataInstance missingValueInfo;
    std::map<std::string, normalizationStats> normalizationInfo; // attributeName -> stats

    /*  Helper Functions  */
    void MEAN_MODE_ReplaceMissingValues ();
    void MEAN_MODE_SetReplacementMap (std::map<std::string, std::vector<double>> &counts);
    void MEAN_MODE_UpdateMissingValueInfo (std::map<std::string, std::vector<double>> &counts);
    void MEAN_MODE_ComputeFinalReplacements (std::map<std::string, std::vector<double>> &counts);


    void ZSCORE_ComputeNormalization ();
    void ZSCORE_SetAccumulationMap (std::map<std::string, std::vector<double>> &accum);
    void ZSCORE_UpdateAccumulationMap (std::map<std::string, std::vector<double>> &accum);
    void ZSCORE_ComputeFinalStats (const std::map<std::string, std::vector<double>> &accum);

    size_t getClassIndex (const std::string &classLabel) const;
};


/****************************************
 *                                      *
 *            Functions                 *
 *                                      *
 ****************************************/


} // namespace ClusteringProcessorNS

#endif /*  CLUSTERING_PROCESSOR_H  */


/****************************************
 *                                      *
 *          Debugging Macros            *
 *                                      *
 ****************************************/