/* 
 * Emilio Cazares Borbon 04/19/2026
 * 
 * Clustering Processor Implementation File.
 * 
 * Created for Georgetown University Data Mining COSC 3590, Spring 2026.
 * 
 * For details, please see `/include/clustering_processor.h`.
 */

#include "../include/clustering_processor.h"

namespace ClusteringProcessorNS
{


/****************************************************************
 ****************************************************************
 **                                                            **
 **          ClusteringProcessor Class Implementation          **
 **                                                            **
 ****************************************************************
 ****************************************************************/

/****************************************************************
 *                     Default Constructor                      *
 ****************************************************************/
ClusteringProcessor::ClusteringProcessor ()
{
    return;
}

/****************************************************************
 *                  Parameterized Constructor                   *
 ****************************************************************/
ClusteringProcessor::ClusteringProcessor (const std::string &filename)
{
    readDataSet(filename);

    return;
}

/****************************************************************
 *                      Copy Constructor                        *
 ****************************************************************/
ClusteringProcessor::ClusteringProcessor (const ClusteringProcessor &other)
{
    copyProcessor (other);

    return;
}

/****************************************************************
 *                         Destructor                           *
 ****************************************************************/
ClusteringProcessor::~ClusteringProcessor ()
{
    clearProcessor ();

    return;
}

/****************************************************************
 *                     Assignment Operator                      *
 ****************************************************************/
ClusteringProcessor &ClusteringProcessor::operator=(const ClusteringProcessor &other)
{
    if (this != &other)
    {
        copyProcessor (other);
    }

    return *this;
}

/****************************************************************
 *                     Copy Processor Function                  *
 ****************************************************************/
void ClusteringProcessor::copyProcessor (const ClusteringProcessor &other)
{
    if (this == &other)
        return;

    try
    {
        this->dataset = other.dataset;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error copying ClusteringProcessor: " << e.what() << std::endl;
        this->clearProcessor();
    }
    catch (...)
    {
        std::cerr << "Unknown error copying ClusteringProcessor." << std::endl;
        this->clearProcessor();
    }

    return;
}

/****************************************************************
 *                        clearProcessor                        *
 ****************************************************************/
void ClusteringProcessor::clearProcessor ()
{
    dataset.clearARFF();

    return;
}

/****************************************************************
 *                         readDataSet                          *
 ****************************************************************/
void ClusteringProcessor::readDataSet (const std::string &filename)
{
    dataset.readARFF(filename);

    return;
}

/****************************************************************
 *                      setClassAttribute                       *
 ****************************************************************/
void ClusteringProcessor::setClassAttribute (std::string classAttributeName)
{
    for (size_t i = 0; i < dataset.attributes.size (); i++)
    {
        if (dataset.attributes[i].name == classAttributeName)
        {
            classAttribute = dataset.attributes[i];
            classAttributeIndex = i;
            getNumFeatures();

            return;
        }
    }

    std::cerr << "Error: Class attribute '" << classAttributeName << "' not found in dataset." << std::endl;
    throw std::invalid_argument("Class attribute not found in dataset.");
}

/****************************************************************
 *                         getClassIndex                        *
 ****************************************************************/
size_t ClusteringProcessor::getClassIndex (const std::string &classLabel) const
{
    for (size_t i = 0; i < classAttribute.values.size (); i++)
    {
        if (classAttribute.values[i] == classLabel)
            return i;
    }

    throw std::invalid_argument("Class label not found in class attribute values.");
}

/****************************************************************
 *                     replaceMissingValues                     *
 ****************************************************************/
void ClusteringProcessor::replaceMissingValues (replaceMissingStrategy strategy)
{
    switch (strategy)
    {
        case MEAN_MODE:
            MEAN_MODE_ReplaceMissingValues();
            break;
        
        default:
            std::cerr << "Error: Invalid replace missing values strategy." << std::endl;
            throw std::invalid_argument("Invalid replace missing values strategy.");
    }

    return;
}

/****************************************************************
 *                MEAN_MODE_ReplaceMissingValues                *
 ****************************************************************/
void ClusteringProcessor::MEAN_MODE_ReplaceMissingValues ()
{
    std::map<std::string, std::vector<double>> counts;

    MEAN_MODE_SetReplacementMap (counts);

    MEAN_MODE_UpdateMissingValueInfo (counts);

    MEAN_MODE_ComputeFinalReplacements (counts);

    return;
}

/****************************************************************
 *                  MEAN_MODE_SetReplacementMap                 *
 ****************************************************************/
void ClusteringProcessor::MEAN_MODE_SetReplacementMap (std::map<std::string, std::vector<double>> &counts)
{
    for (const auto &attribute : dataset.attributes)
    {
        switch (attribute.type)
        {
            case NUMERIC:
                counts[attribute.name] = std::vector<double>{0.0, 0.0}; // sum, count
                break;
            
            case NOMINAL:
                counts[attribute.name] = std::vector<double>(attribute.values.size(), 0.0); // count for each nominal value
                break;

            default:
                std::cerr << "Error: Unsupported attribute type for mean/mode replacement." << std::endl;
                throw std::invalid_argument("Unsupported attribute type for mean/mode replacement.");
        }
    }

    return;
}

/****************************************************************
 *                MEAN_MODE_UpdateMissingValueInfo              *
 ****************************************************************/
void ClusteringProcessor::MEAN_MODE_UpdateMissingValueInfo (std::map<std::string, std::vector<double>> &counts)
{
    for (const auto &instance : dataset.data)
    {
        const DataInstance &dataInstance = instance;
        
        for (size_t attributeIndex = 0; attributeIndex < dataset.attributes.size (); attributeIndex++)
        {
            const Attribute &attribute = dataset.attributes[attributeIndex];
            const std::string &value = dataInstance.values[attributeIndex];

            if (value != "?")
            {
                switch (attribute.type)
                {
                    case NUMERIC:
                        counts[attribute.name][0] += std::stod(value); // sum
                        counts[attribute.name][1] += 1.0; // count
                        break;
                    
                    case NOMINAL:
                    {
                        for (size_t valueIndex = 0; valueIndex < attribute.values.size (); valueIndex++)
                        {
                            if (attribute.values[valueIndex] == value)
                                counts[attribute.name][valueIndex] += 1.0;
                        }
                        break;
                    }
                    default:
                        std::cerr << "Error: Unsupported attribute type for mean/mode replacement." << std::endl;
                        throw std::invalid_argument("Unsupported attribute type for mean/mode replacement.");
                }
            }
        }
    }

    return;
}

/****************************************************************
 *              MEAN_MODE_ComputeFinalReplacements              *
 ****************************************************************/
void ClusteringProcessor::MEAN_MODE_ComputeFinalReplacements (std::map<std::string, std::vector<double>> &counts)
{
    missingValueInfo.values.clear ();

    for (size_t i = 0; i < dataset.attributes.size (); i++)
    {
        const Attribute &attribute = dataset.attributes[i];

        if (attribute.type == NUMERIC)
        {
            double sum = counts[attribute.name][0];
            double count = counts[attribute.name][1];

            std::string replacementValue = (count > 0) ? std::to_string(sum / count) : "0.0";
            missingValueInfo.values.push_back(replacementValue);
        }
        else if (attribute.type == NOMINAL)
        {
            const std::vector<double> &valueCounts = counts[attribute.name];
            size_t maxIndex = 0;

            for (size_t valueIndex = 1; valueIndex < valueCounts.size (); valueIndex++)
            {
                if (valueCounts[valueIndex] > valueCounts[maxIndex])
                    maxIndex = valueIndex;
            }
            std::string replacementValue = attribute.values[maxIndex];
            missingValueInfo.values.push_back(replacementValue);
        }
        else
        {
            missingValueInfo.values.push_back("?");
            std::cerr << "Warning: Unsupported attribute type for mean/mode replacement. Missing value will remain '?'" << std::endl;
        }
    }

    return;
}

/****************************************************************
 *                      getReplacementValue                     *
 ****************************************************************/
std::string ClusteringProcessor::getReplacementValue (const size_t attributeIndex) const
{
    std::string replacementValue = "?";

    if (attributeIndex < missingValueInfo.values.size ())
        replacementValue = missingValueInfo.values[attributeIndex];
    else
        std::cerr << "Error: Attribute index out of bounds for missing value replacement." << std::endl;
    
    return replacementValue;
}

/****************************************************************
 *                    normalizeNumericValues                    *
 ****************************************************************/
void ClusteringProcessor::normalizeNumericValues ()
{
    normalizationInfo.clear ();

    ZSCORE_ComputeNormalization ();

    return;
}

/****************************************************************
 *                  printNormalizationInfo                     *
 ****************************************************************/
void ClusteringProcessor::printNormalizationInfo () const
{
    std::ios oldState (nullptr);
    oldState.copyfmt (std::cout);
    std::cout << std::fixed << std::setprecision (6);

    {
        const std::string heading = " Normalization Information ";
        const int fill = 71 - static_cast<int> (heading.size ());
        const int left = fill / 2;
        const int right = fill - left;
        std::cout << std::string (left, '=') << heading << std::string (right, '=') << std::endl;
    }

    std::cout << std::left << std::setw (24) << "Attribute";
    std::cout << std::setw (18) << "Mean";
    std::cout << std::setw (18) << "Std Dev";
    std::cout << std::right << std::endl;

    for (const auto &attribute : dataset.attributes)
    {
        if (attribute.type != NUMERIC)
            continue;

        std::cout << std::left << std::setw (24) << attribute.name;

        auto statsIt = normalizationInfo.find (attribute.name);
        if (statsIt != normalizationInfo.end ())
        {
            std::cout << std::setw (18) << statsIt->second.mean;
            std::cout << std::setw (18) << statsIt->second.stddev;
        }
        else
        {
            std::cout << std::setw (18) << "(none)";
            std::cout << std::setw (18) << "(none)";
        }

        std::cout << std::right << std::endl;
    }

    std::cout << std::string (71, '=') << std::endl;
    std::cout << std::endl;

    std::cout.copyfmt (oldState);

    return;
}

/****************************************************************
 *                   ZSCORE_ComputeNormalization                *
 ****************************************************************/
void ClusteringProcessor::ZSCORE_ComputeNormalization ()
{
    std::map<std::string, std::vector<double>> accum;

    ZSCORE_SetAccumulationMap (accum);

    ZSCORE_UpdateAccumulationMap (accum);

    ZSCORE_ComputeFinalStats (accum);

    return;
}

/****************************************************************
 *                   ZSCORE_SetAccumulationMap                  *
 ****************************************************************/
void ClusteringProcessor::ZSCORE_SetAccumulationMap (std::map<std::string, std::vector<double>> &accum)
{
    for (const auto &attribute : dataset.attributes)
    {
        if (attribute.type == NUMERIC)
            accum[attribute.name] = std::vector<double> {0.0, 0.0, 0.0};
    }

    return;
}

/****************************************************************
 *                  ZSCORE_UpdateAccumulationMap                *
 ****************************************************************/
void ClusteringProcessor::ZSCORE_UpdateAccumulationMap (std::map<std::string, std::vector<double>> &accum)
{
    for (const auto &instance : dataset.data)
    {
        const DataInstance &dataInstance = instance;

        for (size_t attributeIndex = 0; attributeIndex < dataset.attributes.size (); attributeIndex++)
        {
            const Attribute &attribute = dataset.attributes[attributeIndex];
            const std::string &value = dataInstance.values[attributeIndex];

            if (attribute.type == NUMERIC && value != "?")
            {
                double numericValue = std::stod(value);
                accum[attribute.name][0] += numericValue;
                accum[attribute.name][1] += numericValue * numericValue;
                accum[attribute.name][2] += 1.0;
            }
        }
    }
}

/****************************************************************
 *                    ZSCORE_ComputeFinalStats                  *
 ****************************************************************/
void ClusteringProcessor::ZSCORE_ComputeFinalStats (const std::map<std::string, std::vector<double>> &accum)
{
    for (const auto &attribute : dataset.attributes)
    {
        if (attribute.type == NUMERIC)
        {
            const std::vector<double> &stats = accum.at (attribute.name);
            double sum = stats[0];
            double sumSquares = stats[1];
            double count = stats[2];

            double mean = 0.0;
            double stddev = 0.0;

            if (count > 0)
            {
                mean = sum / count;
                double variance = (sumSquares / count) - (mean * mean);

                if (variance < 0.0)
                    variance = 0.0;
                
                stddev = std::sqrt(variance);
                if (stddev == 0.0)
                    stddev = 1.0;
            }

            normalizationInfo[attribute.name] = {mean, stddev};
        }
    }

    return;
}

/****************************************************************
 *                      getNormalizedValue                      *
 ****************************************************************/
std::string ClusteringProcessor::getNormalizedValue (const size_t attributeIndex, const std::string &rawValue) const
{
    if (rawValue == "?")
        return rawValue;

    if (attributeIndex >= dataset.attributes.size ())
        return rawValue;

    const Attribute &attribute = dataset.attributes[attributeIndex];
    if (attribute.type != NUMERIC)
        return rawValue;

    auto it = normalizationInfo.find (attribute.name);
    if (it == normalizationInfo.end ())
    {
        std::cerr << "Error: Normalization stats not found for attribute '" << attribute.name << "'." << std::endl;
        return rawValue;
    }

    double numericValue = std::stod(rawValue);
    double mean = it->second.mean;
    double stddev = it->second.stddev;
    double zscore = (numericValue - mean) / stddev;

    return std::to_string(zscore);
}

/****************************************************************
 *                    vectorizeDataInstances                    *
 ****************************************************************/
void ClusteringProcessor::vectorizeDataInstances ()
{
    vectorizedData.clear ();

    for (const auto &instance : dataset.data)
    {
        vectorizedData.push_back (getVectorizedInstance (instance));
    }
    return;
}

/****************************************************************
 *                     getVectorizedInstance                    *
 ****************************************************************/
vectorizationInfo ClusteringProcessor::getVectorizedInstance (const DataInstance &instance) const
{
    vectorizationInfo vectorizedInstance;
    vectorizedInstance.classLabel = instance.values[classAttributeIndex];

    for (size_t attributeIndex = 0; attributeIndex < dataset.attributes.size (); attributeIndex++)
    {
        const Attribute &attribute = dataset.attributes[attributeIndex];
        std::string value = instance.values[attributeIndex];

        if (attribute.name != classAttribute.name)
        {
            if (value == "?")
                value = getReplacementValue (attributeIndex);

            if (attribute.type == NUMERIC)
            {
                std::string normalizedValue = getNormalizedValue (attributeIndex, value);
                vectorizedInstance.vectorizedInstance.push_back(std::stod(normalizedValue));
            }
            else if (attribute.type == NOMINAL)
            {
                for (const auto &nominalValue : attribute.values)
                    vectorizedInstance.vectorizedInstance.push_back(value == nominalValue ? 1.0 : 0.0);
            }
        }
    }

    return vectorizedInstance;
}

/****************************************************************
 *                         getNumFeatures                       *
 ****************************************************************/
void ClusteringProcessor::getNumFeatures ()
{
    numFeatures = 0;

    for (const auto &attribute : dataset.attributes)
    {
        if (attribute.name != this->classAttribute.name)
        {
            if (attribute.type == NUMERIC)
                numFeatures += 1;
            else if (attribute.type == NOMINAL)
                numFeatures += attribute.values.size ();
        }
    }

    return;
}

/****************************************************************
 *                     getEuclideanDistance                     *
 ****************************************************************/
double ClusteringProcessor::getEuclideanDistance (const vectorizationInfo &instance1, const vectorizationInfo &instance2) const
{
    if (instance1.vectorizedInstance.size () != instance2.vectorizedInstance.size ())
    {
        std::cerr << "Error: Cannot compute Euclidean distance between instances with different feature counts." << std::endl;
        throw std::invalid_argument("Instances have different feature counts.");
    }

    double sumSquares = 0.0;
    for (size_t i = 0; i < instance1.vectorizedInstance.size (); i++)
    {
        double diff = instance1.vectorizedInstance[i] - instance2.vectorizedInstance[i];
        sumSquares += diff * diff;
    }

    return std::sqrt(sumSquares);
}

/****************************************************************
 *                    run Clustering Algorithm                  *
 ****************************************************************/
void ClusteringProcessor::run (size_t k, terminationStrategy termStrategy)
{
    // choosing sqrt(n) for clustering
    shuffleDataInstances (vectorizedData, SEED);

    size_t randInstanceCount = floor(std::sqrt(vectorizedData.size ()));

    if (randInstanceCount < k)
        randInstanceCount = k;

    // HAC 
    
}






/****************************************************************
 ****************************************************************
 **                                                            **
 **            Additional Functions Implementation             **
 **                                                            **
 ****************************************************************
 ****************************************************************/

/****************************************************************
 *               shuffleDataInstances Function                  *
 ****************************************************************/
void shuffleDataInstances (std::vector<vectorizationInfo> &dataInstances, size_t seed)
{
    static std::mt19937 generator (seed);
    std::shuffle (dataInstances.begin (), dataInstances.end (), generator);

    return;
}



} // namespace ClusteringProcessorNS