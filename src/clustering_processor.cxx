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
        this->classAttribute = other.classAttribute;
        this->classAttributeIndex = other.classAttributeIndex;
        this->missingValueInfo = other.missingValueInfo;
        this->normalizationInfo = other.normalizationInfo;
        this->vectorizedData = other.vectorizedData;
        this->numFeatures = other.numFeatures;
        this->clusterDendrogram = other.clusterDendrogram;
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
 *                           getSSE                             *
 ****************************************************************/
double ClusteringProcessor::getSSE (const std::vector<clusterInfo> &clusters) const
{
    double sse = 0.0;
    for (const auto &cluster : clusters)
    {
        for (const auto &instance : cluster.instances)
        {
            double distance = getEuclideanDistance (instance, cluster.centroid);
            sse += distance * distance;
        }
    }
    return sse;
}

/****************************************************************
 *                    run Clustering Algorithm                  *
 ****************************************************************/
dendrogramLevel ClusteringProcessor::run (size_t k, terminationStrategy termStrategy)
{
    // choosing sqrt(n) for clustering
    shuffleDataInstances (vectorizedData, SEED);

    if (vectorizedData.empty ())
        throw std::invalid_argument("Cannot cluster an empty dataset.");

    if (k == 0 || k > vectorizedData.size ())
        throw std::invalid_argument("Invalid k value for clustering.");

    size_t randInstanceCount = floor(std::sqrt(vectorizedData.size ()));

    if (randInstanceCount < k)
        randInstanceCount = k;

    // HAC 
    clusterDendrogram.levels.clear ();
    std::vector<clusterInfo> clusters;

    for (size_t i = 0; i < randInstanceCount; i++)
    {
        clusterInfo newCluster;
        newCluster.instances.push_back(vectorizedData[i]);
        newCluster.centroid = vectorizedData[i];
        clusters.push_back(newCluster);
    }

    while (clusters.size () > k)
    {
        double minDistance = std::numeric_limits<double>::max ();
        size_t mergeIndex1 = 0;
        size_t mergeIndex2 = 0;

        for (size_t i = 0; i < clusters.size (); i++)
        {
            for (size_t j = i + 1; j < clusters.size (); j++)
            {
                double distance = getEuclideanDistance (clusters[i].centroid, clusters[j].centroid);
                if (distance < minDistance)
                {
                    minDistance = distance;
                    mergeIndex1 = i;
                    mergeIndex2 = j;
                }
            }
        }

        // Merge the closest clusters
        clusterInfo &cluster1 = clusters[mergeIndex1];
        clusterInfo &cluster2 = clusters[mergeIndex2];

        cluster1.instances.insert(cluster1.instances.end (), cluster2.instances.begin (), cluster2.instances.end ());
        
        // Recompute centroid
        vectorizationInfo newCentroid;
        std::map<std::string, double> classCounts;
        for (const auto &instance : cluster1.instances)
        {
            classCounts[instance.classLabel] += 1.0;
        }
        double maxCount = 0.0;
        for (const auto &entry : classCounts)
        {
            if (entry.second > maxCount)
            {
                maxCount = entry.second;
                newCentroid.classLabel = entry.first;
            }
        }

        for (size_t featureIndex = 0; featureIndex < cluster1.centroid.vectorizedInstance.size (); featureIndex++)
        {
            double sumFeatureValue = 0.0;
            for (const auto &instance : cluster1.instances)
            {
                sumFeatureValue += instance.vectorizedInstance[featureIndex];
            }
            newCentroid.vectorizedInstance.push_back(sumFeatureValue / cluster1.instances.size ());
        }
        cluster1.centroid = newCentroid;

        // Remove the merged cluster
        clusters.erase(clusters.begin () + mergeIndex2);

        // Store the current level of the dendrogram
        dendrogramLevel level;
        level.clusters = clusters;
        calculateDistances (level);
        clusterDendrogram.levels.push_back(level);
    }

    // k-means clustering
    // Approach A (and step one of Approach B)
    dendrogramLevel kMeansLevel;
    if (!clusterDendrogram.levels.empty ())
        kMeansLevel = clusterDendrogram.levels.back ();
    else
        kMeansLevel.clusters = clusters;

    for (size_t i = randInstanceCount; i < vectorizedData.size (); i++)
    {
        const vectorizationInfo &instance = vectorizedData[i];
        double minDistance = std::numeric_limits<double>::max ();
        size_t closestClusterIndex = 0;

        for (size_t clusterIndex = 0; clusterIndex < kMeansLevel.clusters.size (); clusterIndex++)
        {
            double distance = getEuclideanDistance (instance, kMeansLevel.clusters[clusterIndex].centroid);
            if (distance < minDistance)
            {
                minDistance = distance;
                closestClusterIndex = clusterIndex;
            }
        }

        kMeansLevel.clusters[closestClusterIndex].instances.push_back(instance);
    }

    calculateDistances (kMeansLevel);

    if (termStrategy == ONE)
        return kMeansLevel;

    if (termStrategy != SSE)
        throw std::invalid_argument("Invalid termination strategy for clustering algorithm.");

    // Approach B
    kMeansLevel.SSEs.push_back (getSSE (kMeansLevel.clusters));
    double SSEChange = std::numeric_limits<double>::max ();
    while (SSEChange > SSE_TERMINATION_THRESHOLD)
    {
        std::vector<vectorizationInfo> previousCentroids;
        for (const auto &cluster : kMeansLevel.clusters)
            previousCentroids.push_back(cluster.centroid);

        for (auto &cluster : kMeansLevel.clusters)
            cluster.instances.clear();

        for (size_t i = 0; i < vectorizedData.size (); i++)
        {
            const vectorizationInfo &instance = vectorizedData[i];
            double minDistance = std::numeric_limits<double>::max ();
            size_t closestClusterIndex = 0;

            for (size_t clusterIndex = 0; clusterIndex < kMeansLevel.clusters.size (); clusterIndex++)
            {
                double distance = getEuclideanDistance (instance, previousCentroids[clusterIndex]);
                if (distance < minDistance)
                {
                    minDistance = distance;
                    closestClusterIndex = clusterIndex;
                }
            }

            kMeansLevel.clusters[closestClusterIndex].instances.push_back(instance);
        }

        for (size_t clusterIndex = 0; clusterIndex < kMeansLevel.clusters.size (); clusterIndex++)
        {
            auto &cluster = kMeansLevel.clusters[clusterIndex];

            if (cluster.instances.empty ())
            {
                cluster.centroid = previousCentroids[clusterIndex];
                continue;
            }

            vectorizationInfo newCentroid;
            std::map<std::string, double> classCounts;
            for (const auto &instance : cluster.instances)
                classCounts[instance.classLabel] += 1.0;

            double maxCount = 0.0;
            for (const auto &entry : classCounts)
            {
                if (entry.second > maxCount)
                {
                    maxCount = entry.second;
                    newCentroid.classLabel = entry.first;
                }
            }

            for (size_t featureIndex = 0; featureIndex < cluster.instances[0].vectorizedInstance.size (); featureIndex++)
            {
                double sumFeatureValue = 0.0;
                for (const auto &instance : cluster.instances)
                    sumFeatureValue += instance.vectorizedInstance[featureIndex];
                newCentroid.vectorizedInstance.push_back(sumFeatureValue / cluster.instances.size ());
            }

            cluster.centroid = newCentroid;
        }

        double newSSE = getSSE (kMeansLevel.clusters);
        SSEChange = std::abs(kMeansLevel.SSEs.back () - newSSE);
        kMeansLevel.SSEs.push_back(newSSE);
        calculateDistances (kMeansLevel);
    }

    return kMeansLevel;
}

/****************************************************************
 *                     calculateDistances                       *
 ****************************************************************/
void ClusteringProcessor::calculateDistances (dendrogramLevel &level)
{
    double totalIntraDistance = 0.0;
    double totalInterDistance = 0.0;
    size_t intraCount = 0;
    size_t interCount = 0;
    std::vector<clusterComposition> iterationComposition;

    for (size_t i = 0; i < level.clusters.size (); i++)
    {
        clusterComposition composition;
        composition.clusterSize = level.clusters[i].instances.size ();

        for (const auto &instance : level.clusters[i].instances)
            composition.classCounts[instance.classLabel] += 1;

        iterationComposition.push_back(composition);

        for (size_t j = i + 1; j < level.clusters.size (); j++)
        {
            totalInterDistance += getEuclideanDistance (level.clusters[i].centroid, level.clusters[j].centroid);
            interCount++;
        }

        for (size_t m = 0; m < level.clusters[i].instances.size (); m++)
        {
            for (size_t n = m + 1; n < level.clusters[i].instances.size (); n++)
            {
                totalIntraDistance += getEuclideanDistance (level.clusters[i].instances[m], level.clusters[i].instances[n]);
                intraCount++;
            }
        }
    }

    level.interClusterDistances.push_back(interCount > 0 ? totalInterDistance / interCount : 0.0);
    level.intraClusterDistances.push_back(intraCount > 0 ? totalIntraDistance / intraCount : 0.0);
    level.iterationClusterCompositions.push_back(iterationComposition);

    return;
}

/****************************************************************
 *                     printDendrogramLevel                     *
 ****************************************************************/
void ClusteringProcessor::printDendrogramLevel (const dendrogramLevel &level) const
{
    const auto printClassCounts = [this](const std::map<std::string, size_t> &classCounts)
    {
        bool first = true;

        for (const auto &classLabel : classAttribute.values)
        {
            if (!first)
                std::cout << ", ";

            const size_t count = classCounts.count (classLabel) > 0 ? classCounts.at (classLabel) : 0;
            std::cout << classLabel << "=" << count;
            first = false;
        }

        for (const auto &entry : classCounts)
        {
            if (std::find (classAttribute.values.begin (), classAttribute.values.end (), entry.first) == classAttribute.values.end ())
            {
                if (!first)
                    std::cout << ", ";

                std::cout << entry.first << "=" << entry.second;
                first = false;
            }
        }

        if (first)
            std::cout << "(none)";
    };

    std::cout << "Dendrogram Level with " << level.clusters.size () << " clusters:" << std::endl;
    for (size_t i = 0; i < level.clusters.size (); i++)
    {
        std::map<std::string, size_t> classCounts;

        for (const auto &instance : level.clusters[i].instances)
            classCounts[instance.classLabel] += 1;

        std::cout << "  Cluster " << i + 1 << ": " << level.clusters[i].instances.size () << " instances";
        std::cout << " | class counts: ";
        printClassCounts (classCounts);

        std::cout << std::endl;
    }

    for (size_t i = 0; i < level.interClusterDistances.size (); i++)
    {
        std::cout << "  Iteration " << i + 1 << ":" << std::endl;

        if (i < level.iterationClusterCompositions.size ())
        {
            const auto &composition = level.iterationClusterCompositions[i];
            for (size_t clusterIndex = 0; clusterIndex < composition.size (); clusterIndex++)
            {
                std::cout << "    Cluster " << clusterIndex + 1 << ": size=" << composition[clusterIndex].clusterSize;
                std::cout << " | class counts: ";
                printClassCounts (composition[clusterIndex].classCounts);
                std::cout << std::endl;
            }
        }

        std::cout << "  Intra-cluster distance: " << level.intraClusterDistances[i] << std::endl;
        std::cout << "  Inter-cluster distance: " << level.interClusterDistances[i] << std::endl;
        if (i < level.SSEs.size ())
            std::cout << "  SSE: " << level.SSEs[i] << std::endl;
    }

    return;
}

/****************************************************************
 *                   getTerminationStrategyName                 *
 ****************************************************************/
std::string ClusteringProcessor::getTerminationStrategyName (terminationStrategy strategy) const
{
    if (strategy == ONE)
        return "ONE";

    if (strategy == SSE)
        return "SSE";

    return "UNKNOWN";
}

/****************************************************************
 *                     saveDendrogramLevel                      *
 ****************************************************************/
std::string ClusteringProcessor::saveDendrogramLevel (const dendrogramLevel &level,
                                                      terminationStrategy termStrategy,
                                                      size_t k,
                                                      const std::string &outputDir) const
{
    std::filesystem::create_directories (outputDir);

    const auto now = std::chrono::system_clock::now ();
    const std::time_t nowTime = std::chrono::system_clock::to_time_t (now);

    std::tm localTime {};
#ifdef _WIN32
    localtime_s (&localTime, &nowTime);
#else
    localtime_r (&nowTime, &localTime);
#endif

    std::ostringstream timestamp;
    timestamp << std::put_time (&localTime, "%Y%m%d_%H%M%S");
    const long long micros = std::chrono::duration_cast<std::chrono::microseconds> (now.time_since_epoch ()).count () % 1000000;
    timestamp << "_" << std::setw (6) << std::setfill ('0') << micros;

    const std::string filename = timestamp.str () + "_" + getTerminationStrategyName (termStrategy) + "_k" + std::to_string (k) + ".out";
    const std::filesystem::path outputPath = std::filesystem::path (outputDir) / filename;

    std::ofstream outFile (outputPath.string ());
    if (!outFile.is_open ())
        throw std::runtime_error ("Could not create output file: " + outputPath.string ());

    outFile << "k = " << k << "\n";
    outFile << "terminationStrategy = " << getTerminationStrategyName (termStrategy) << "\n";
    outFile << "clusters = " << level.clusters.size () << "\n\n";

    outFile << "Final cluster class distribution:\n";
    for (size_t i = 0; i < level.clusters.size (); i++)
    {
        std::map<std::string, size_t> classCounts;
        for (const auto &instance : level.clusters[i].instances)
            classCounts[instance.classLabel] += 1;

        outFile << "  Cluster " << i + 1 << " size=" << level.clusters[i].instances.size () << " -> ";

        bool first = true;
        for (const auto &entry : classCounts)
        {
            if (!first)
                outFile << ", ";
            outFile << entry.first << "=" << entry.second;
            first = false;
        }

        if (first)
            outFile << "(none)";

        outFile << "\n";
    }

    outFile << "\nIteration metrics:\n";
    const size_t rows = std::max (level.intraClusterDistances.size (), level.interClusterDistances.size ());
    for (size_t i = 0; i < rows; i++)
    {
        outFile << "  Iteration " << i + 1 << ": ";

        if (i < level.intraClusterDistances.size ())
            outFile << "intra=" << level.intraClusterDistances[i] << ", ";
        else
            outFile << "intra=n/a, ";

        if (i < level.interClusterDistances.size ())
            outFile << "inter=" << level.interClusterDistances[i] << ", ";
        else
            outFile << "inter=n/a, ";

        if (i < level.SSEs.size ())
            outFile << "SSE=" << level.SSEs[i];
        else
            outFile << "SSE=n/a";

        outFile << "\n";
    }

    return outputPath.string ();
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
    thread_local std::mt19937 generator (seed);
    std::shuffle (dataInstances.begin (), dataInstances.end (), generator);

    return;
}



} // namespace ClusteringProcessorNS