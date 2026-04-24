/* 
 * Emilio Cazares Borbon 04/19/2024
 * 
 * Clustering Processor Tests File.
 * 
 * Created for Georgetown University Data Mining COSC 3590, Spring 2026.
 * 
 * For details, please see `/include/clustering_processor.h`.
 */

#include "../include/main.h"

/****************************************
 *                                      *
 *         Debugging Macros             *
 *                                      *
 ****************************************/
#define CLUSTERING_PROCESSOR_SPLIT_DEBUGGER(message) \
    std::cout << "[DEBUG] " << message << std::endl

namespace
{
void requireCondition (bool condition, const std::string &message)
{
    if (!condition)
        throw std::runtime_error (message);
}

void requireNear (double actual, double expected, double epsilon, const std::string &message)
{
    if (std::fabs (actual - expected) > epsilon)
        throw std::runtime_error (message);
}

size_t findFirstAttributeOfType (const ARFF &dataset, AttributeType type, const std::string &excludedName = "")
{
    for (size_t attributeIndex = 0; attributeIndex < dataset.attributes.size (); attributeIndex++)
    {
        const Attribute &attribute = dataset.attributes[attributeIndex];
        if (attribute.type == type && attribute.name != excludedName)
            return attributeIndex;
    }

    throw std::runtime_error ("Requested attribute type not found in dataset.");
}

std::string computeExpectedReplacementValue (const ARFF &dataset, size_t attributeIndex)
{
    const Attribute &attribute = dataset.attributes[attributeIndex];

    if (attribute.type == NUMERIC)
    {
        double sum = 0.0;
        double count = 0.0;
        for (const auto &instance : dataset.data)
        {
            const std::string &value = instance.values[attributeIndex];
            if (value == "?")
                continue;

            sum += std::stod (value);
            count += 1.0;
        }

        return (count > 0.0) ? std::to_string (sum / count) : "0.0";
    }

    if (attribute.type == NOMINAL)
    {
        std::vector<double> counts (attribute.values.size (), 0.0);
        for (const auto &instance : dataset.data)
        {
            const std::string &value = instance.values[attributeIndex];
            if (value == "?")
                continue;

            for (size_t valueIndex = 0; valueIndex < attribute.values.size (); valueIndex++)
            {
                if (attribute.values[valueIndex] == value)
                    counts[valueIndex] += 1.0;
            }
        }

        size_t maxIndex = 0;
        for (size_t valueIndex = 1; valueIndex < counts.size (); valueIndex++)
        {
            if (counts[valueIndex] > counts[maxIndex])
                maxIndex = valueIndex;
        }

        return attribute.values[maxIndex];
    }

    throw std::runtime_error ("Unsupported attribute type for expected replacement computation.");
}

std::string formatFixed6 (double value)
{
    std::ostringstream stream;
    stream << std::fixed << std::setprecision (6) << value;
    return stream.str ();
}

struct runCaseResult
{
    std::string name;
    size_t k;
    terminationStrategy strategy;
    dendrogramLevel level;
    long long durationMicros;
};

void requireRunLevelIntegrity (const runCaseResult &result, size_t expectedDatasetSize)
{
    requireCondition (result.level.clusters.size () == result.k,
                      "Run method produced an unexpected cluster count for " + result.name + ".");

    size_t totalAssignedInstances = 0;
    for (const auto &cluster : result.level.clusters)
    {
        totalAssignedInstances += cluster.instances.size ();

        if (!cluster.instances.empty ())
        {
            requireCondition (
                cluster.centroid.vectorizedInstance.size () == cluster.instances[0].vectorizedInstance.size (),
                "Centroid dimensionality mismatch detected for " + result.name + ".");
        }
    }

    requireCondition (totalAssignedInstances == expectedDatasetSize,
                      "Run method did not assign every instance exactly once for " + result.name + ".");

    requireCondition (!result.level.interClusterDistances.empty (),
                      "Run method should populate inter-cluster distances for " + result.name + ".");
    requireCondition (!result.level.intraClusterDistances.empty (),
                      "Run method should populate intra-cluster distances for " + result.name + ".");

    if (result.strategy == SSE)
    {
        requireCondition (!result.level.SSEs.empty (),
                          "SSE strategy should record SSE history for " + result.name + ".");
    }
}

} // namespace

int main ()
{
    const auto masterStart = std::chrono::high_resolution_clock::now ();
    using Processor = ClusteringProcessorNS::ClusteringProcessor;

    try
    {
        std::cout << "Clustering Processor Test File" << std::endl;
        std::cout << std::endl;

        std::string filename = "data/adult-big.arff";
        ARFF rawDataset (filename);

        std::vector<std::pair<std::string, long long>> timings;
        std::chrono::time_point<std::chrono::high_resolution_clock> start;
        std::chrono::time_point<std::chrono::high_resolution_clock> end;
        long long duration = 0;

        std::cout << "Testing normalization output before statistics are computed" << std::endl;
        start = std::chrono::high_resolution_clock::now ();
        Processor preNormProcessor (filename);
        preNormProcessor.setClassAttribute ("class");
        std::ostringstream preNormalizationCapture;
        std::streambuf *preNormOldCoutBuffer = std::cout.rdbuf (preNormalizationCapture.rdbuf ());
        preNormProcessor.printNormalizationInfo ();
        std::cout.rdbuf (preNormOldCoutBuffer);
        end = std::chrono::high_resolution_clock::now ();
        duration = std::chrono::duration_cast<std::chrono::microseconds> (end - start).count ();
        timings.push_back ({"preNormalizationPrintCheck", duration});

        const std::string preNormalizationOutput = preNormalizationCapture.str ();
        requireCondition (preNormalizationOutput.find ("Normalization Information") != std::string::npos,
                  "Pre-normalization output heading was not printed.");
        requireCondition (preNormalizationOutput.find ("(none)") != std::string::npos,
                  "Pre-normalization output should show '(none)' stats before normalization.");

        CLUSTERING_PROCESSOR_SPLIT_DEBUGGER ("Creating ClusteringProcessor instance with dataset: " + filename);
        start = std::chrono::high_resolution_clock::now ();
        Processor clusterProcessor (filename);
        end = std::chrono::high_resolution_clock::now ();
        duration = std::chrono::duration_cast<std::chrono::microseconds> (end - start).count ();
        timings.push_back ({"ClusteringProcessor (constructor)", duration});

        CLUSTERING_PROCESSOR_SPLIT_DEBUGGER ("Setting class attribute to 'class'");
        start = std::chrono::high_resolution_clock::now ();
        clusterProcessor.setClassAttribute ("class");
        end = std::chrono::high_resolution_clock::now ();
        duration = std::chrono::duration_cast<std::chrono::microseconds> (end - start).count ();
        timings.push_back ({"setClassAttribute", duration});

        CLUSTERING_PROCESSOR_SPLIT_DEBUGGER ("Replacing missing values using MEAN_MODE strategy");
        start = std::chrono::high_resolution_clock::now ();
        clusterProcessor.replaceMissingValues (MEAN_MODE);
        end = std::chrono::high_resolution_clock::now ();
        duration = std::chrono::duration_cast<std::chrono::microseconds> (end - start).count ();
        timings.push_back ({"replaceMissingValues", duration});

        CLUSTERING_PROCESSOR_SPLIT_DEBUGGER ("Computing z-score normalization statistics");
        start = std::chrono::high_resolution_clock::now ();
        clusterProcessor.normalizeNumericValues ();
        end = std::chrono::high_resolution_clock::now ();
        duration = std::chrono::duration_cast<std::chrono::microseconds> (end - start).count ();
        timings.push_back ({"normalizeNumericValues", duration});

        std::cout << std::endl;
        std::cout << "Testing replacement values" << std::endl;
        size_t numericAttributeIndex = findFirstAttributeOfType (rawDataset, NUMERIC, "class");
        size_t nominalAttributeIndex = findFirstAttributeOfType (rawDataset, NOMINAL, "class");
        size_t secondAttributeIndex = rawDataset.attributes.size ();
        for (size_t attributeIndex = nominalAttributeIndex + 1; attributeIndex < rawDataset.attributes.size (); attributeIndex++)
        {
            if (rawDataset.attributes[attributeIndex].type == NOMINAL && rawDataset.attributes[attributeIndex].name != "class")
            {
                secondAttributeIndex = attributeIndex;
                break;
            }
        }

        requireCondition (clusterProcessor.getReplacementValue (numericAttributeIndex) == computeExpectedReplacementValue (rawDataset, numericAttributeIndex),
                          "Unexpected replacement value for first numeric attribute.");
        requireCondition (clusterProcessor.getReplacementValue (nominalAttributeIndex) == computeExpectedReplacementValue (rawDataset, nominalAttributeIndex),
                          "Unexpected replacement value for first nominal attribute.");
        if (secondAttributeIndex < rawDataset.attributes.size ())
        {
            requireCondition (clusterProcessor.getReplacementValue (secondAttributeIndex) == computeExpectedReplacementValue (rawDataset, secondAttributeIndex),
                              "Unexpected replacement value for second nominal attribute.");
        }

        std::cout << std::endl;
        std::cout << "Testing getNormalizedValue guard behavior" << std::endl;
        start = std::chrono::high_resolution_clock::now ();
        requireCondition (clusterProcessor.getNormalizedValue (0, "?") == "?",
                          "Missing numeric value should remain '?'.");
        requireCondition (clusterProcessor.getNormalizedValue (rawDataset.attributes.size () + 3, "123.0") == "123.0",
                          "Out-of-bounds normalization query should return raw value.");

        size_t firstNominalIndex = rawDataset.attributes.size ();
        for (size_t attributeIndex = 0; attributeIndex < rawDataset.attributes.size (); attributeIndex++)
        {
            const Attribute &attribute = rawDataset.attributes[attributeIndex];
            if (attribute.type == NOMINAL && attribute.name != "class")
            {
                firstNominalIndex = attributeIndex;
                break;
            }
        }
        requireCondition (firstNominalIndex < rawDataset.attributes.size (),
                          "Could not find a non-class nominal attribute to test normalization guards.");

        const std::string nominalRaw = rawDataset.data.front ().values[firstNominalIndex];
        requireCondition (clusterProcessor.getNormalizedValue (firstNominalIndex, nominalRaw) == nominalRaw,
                          "Nominal values should be returned unchanged by getNormalizedValue.");
        end = std::chrono::high_resolution_clock::now ();
        duration = std::chrono::duration_cast<std::chrono::microseconds> (end - start).count ();
        timings.push_back ({"getNormalizedValueGuards", duration});

        std::cout << std::endl;
        std::cout << "Checking normalization info output" << std::endl;
        start = std::chrono::high_resolution_clock::now ();
        std::ostringstream normalizationCapture;
        std::streambuf *oldCoutBuffer = std::cout.rdbuf (normalizationCapture.rdbuf ());
        clusterProcessor.printNormalizationInfo ();
        std::cout.rdbuf (oldCoutBuffer);

        const std::string normalizationOutput = normalizationCapture.str ();
        requireCondition (normalizationOutput.find ("Normalization Information") != std::string::npos,
                          "Normalization output heading was not printed.");
        requireCondition (normalizationOutput.find ("Attribute") != std::string::npos,
                          "Normalization output table header was not printed.");
        const std::string numericAttributeName = rawDataset.attributes[numericAttributeIndex].name;
        requireCondition (normalizationOutput.find (numericAttributeName) != std::string::npos,
                          "Normalization output did not include the first numeric attribute.");

        double numericSum = 0.0;
        double numericCount = 0.0;
        for (const auto &instance : rawDataset.data)
        {
            const std::string &value = instance.values[numericAttributeIndex];
            if (value == "?")
                continue;

            numericSum += std::stod (value);
            numericCount += 1.0;
        }

        const double expectedNumericMean = numericCount > 0.0 ? numericSum / numericCount : 0.0;
        requireCondition (normalizationOutput.find (formatFixed6 (expectedNumericMean)) != std::string::npos,
                          "Normalization output did not include the expected first numeric attribute mean.");
        end = std::chrono::high_resolution_clock::now ();
        duration = std::chrono::duration_cast<std::chrono::microseconds> (end - start).count ();
        timings.push_back ({"normalizationOutputCheck", duration});

        std::cout << std::endl;
        std::cout << "Testing vectorization" << std::endl;
        start = std::chrono::high_resolution_clock::now ();
        clusterProcessor.vectorizeDataInstances ();
        end = std::chrono::high_resolution_clock::now ();
        duration = std::chrono::duration_cast<std::chrono::microseconds> (end - start).count ();
        timings.push_back ({"vectorizeDataInstances", duration});

        const DataInstance &sampleInstance = rawDataset.data.front ();
        size_t classIndex = 0;
        bool classFound = false;
        for (size_t attributeIndex = 0; attributeIndex < rawDataset.attributes.size (); attributeIndex++)
        {
            if (rawDataset.attributes[attributeIndex].name == "class")
            {
                classIndex = attributeIndex;
                classFound = true;
                break;
            }
        }
        requireCondition (classFound, "Class attribute index not found in dataset attributes.");

        vectorizationInfo actualVector = clusterProcessor.getVectorizedInstance (sampleInstance);
        vectorizationInfo expectedVector;
        expectedVector.classLabel = sampleInstance.values[classIndex];

        for (size_t attributeIndex = 0; attributeIndex < rawDataset.attributes.size (); attributeIndex++)
        {
            const Attribute &attribute = rawDataset.attributes[attributeIndex];
            if (attribute.name == "class")
                continue;

            std::string value = sampleInstance.values[attributeIndex];
            if (value == "?")
                value = clusterProcessor.getReplacementValue (attributeIndex);

            if (attribute.type == NUMERIC)
            {
                expectedVector.vectorizedInstance.push_back (
                    std::stod (clusterProcessor.getNormalizedValue (attributeIndex, value)));
            }
            else if (attribute.type == NOMINAL)
            {
                for (const auto &nominalValue : attribute.values)
                    expectedVector.vectorizedInstance.push_back (value == nominalValue ? 1.0 : 0.0);
            }
        }

        requireCondition (actualVector.classLabel == expectedVector.classLabel,
                          "Vectorized instance class label does not match the source data.");
        requireCondition (actualVector.vectorizedInstance.size () == expectedVector.vectorizedInstance.size (),
                          "Vectorized instance size does not match the expected feature count.");

        for (size_t index = 0; index < expectedVector.vectorizedInstance.size (); index++)
        {
            requireNear (actualVector.vectorizedInstance[index], expectedVector.vectorizedInstance[index], 1e-9,
                         "Vectorized instance value does not match the expected encoding.");
        }

        const DataInstance *missingInstance = nullptr;
        for (const auto &instance : rawDataset.data)
        {
            for (const auto &value : instance.values)
            {
                if (value == "?")
                {
                    missingInstance = &instance;
                    break;
                }
            }

            if (missingInstance != nullptr)
                break;
        }

        if (missingInstance != nullptr)
        {
            std::cout << "Checking vectorization on an instance with missing values" << std::endl;
            vectorizationInfo missingVector = clusterProcessor.getVectorizedInstance (*missingInstance);

            requireCondition (missingVector.classLabel == missingInstance->values[classIndex],
                              "Vectorized missing-instance class label is incorrect.");
            requireCondition (missingVector.vectorizedInstance.size () == expectedVector.vectorizedInstance.size (),
                              "Vectorized missing-instance size does not match expected feature count.");

            for (const auto &featureValue : missingVector.vectorizedInstance)
            {
                requireCondition (std::isfinite (featureValue),
                                  "Vectorized missing-instance contains a non-finite value.");
            }
        }

        std::cout << std::endl;
        std::cout << "Testing getEuclideanDistance" << std::endl;
        start = std::chrono::high_resolution_clock::now ();
        vectorizationInfo distanceA;
        distanceA.classLabel = "A";
        distanceA.vectorizedInstance = {0.0, 0.0};

        vectorizationInfo distanceB;
        distanceB.classLabel = "B";
        distanceB.vectorizedInstance = {3.0, 4.0};

        vectorizationInfo distanceC;
        distanceC.classLabel = "C";
        distanceC.vectorizedInstance = {0.0, 4.0};

        double euclideanAB = clusterProcessor.getEuclideanDistance (distanceA, distanceB);
        requireNear (euclideanAB, 5.0, 1e-9,
                     "Euclidean distance should match the 3-4-5 triangle expected value.");

        double euclideanAA = clusterProcessor.getEuclideanDistance (distanceA, distanceA);
        requireNear (euclideanAA, 0.0, 1e-12,
                     "Euclidean distance from a point to itself should be zero.");

        bool distanceThrowDetected = false;
        std::ostringstream distanceErrorCapture;
        std::streambuf *oldCerrBuffer = std::cerr.rdbuf (distanceErrorCapture.rdbuf ());
        try
        {
            vectorizationInfo badDimensionInstance;
            badDimensionInstance.classLabel = "D";
            badDimensionInstance.vectorizedInstance = {1.0};
            (void)clusterProcessor.getEuclideanDistance (distanceA, badDimensionInstance);
        }
        catch (const std::invalid_argument &)
        {
            distanceThrowDetected = true;
        }
        std::cerr.rdbuf (oldCerrBuffer);
        requireCondition (distanceThrowDetected,
                          "getEuclideanDistance should throw on mismatched feature counts.");
        end = std::chrono::high_resolution_clock::now ();
        duration = std::chrono::duration_cast<std::chrono::microseconds> (end - start).count ();
        timings.push_back ({"getEuclideanDistanceTests", duration});

        std::cout << std::endl;
        std::cout << "Testing getSSE" << std::endl;
        start = std::chrono::high_resolution_clock::now ();
        clusterInfo firstCluster;
        firstCluster.centroid = distanceA;
        firstCluster.instances = {distanceA, distanceB, distanceC};

        std::vector<clusterInfo> syntheticClusters = {firstCluster};
        double syntheticSSE = clusterProcessor.getSSE (syntheticClusters);
        requireNear (syntheticSSE, 41.0, 1e-9,
                     "SSE should equal squared-distance sum for synthetic cluster points.");

        std::vector<clusterInfo> emptyClusters;
        double emptySSE = clusterProcessor.getSSE (emptyClusters);
        requireNear (emptySSE, 0.0, 1e-12,
                     "SSE should be zero for an empty cluster list.");
        end = std::chrono::high_resolution_clock::now ();
        duration = std::chrono::duration_cast<std::chrono::microseconds> (end - start).count ();
        timings.push_back ({"getSSETests", duration});

        std::cout << std::endl;
        std::cout << "Testing run method with ONE termination strategy" << std::endl;
        start = std::chrono::high_resolution_clock::now ();
        runCaseResult runOneResult;
        runOneResult.name = "runMethodONE_k2";
        runOneResult.k = 2;
        runOneResult.strategy = ONE;
        runOneResult.level = clusterProcessor.run (2, ONE);
        end = std::chrono::high_resolution_clock::now ();
        runOneResult.durationMicros = std::chrono::duration_cast<std::chrono::microseconds> (end - start).count ();
        duration = runOneResult.durationMicros;
        timings.push_back ({"runMethodONE", duration});
        requireRunLevelIntegrity (runOneResult, rawDataset.data.size ());

        runCaseResult runK1Result;
        std::cout << std::endl;
        std::cout << "Testing run method with k=1 edge case" << std::endl;
        start = std::chrono::high_resolution_clock::now ();
        runK1Result.name = "runMethodONE_k1";
        runK1Result.k = 1;
        runK1Result.strategy = ONE;
        runK1Result.level = clusterProcessor.run (1, ONE);
        end = std::chrono::high_resolution_clock::now ();
        runK1Result.durationMicros = std::chrono::duration_cast<std::chrono::microseconds> (end - start).count ();
        duration = runK1Result.durationMicros;
        timings.push_back ({"runMethodK1", duration});
        requireRunLevelIntegrity (runK1Result, rawDataset.data.size ());

        std::cout << std::endl;
        std::cout << "Testing run method with SSE termination strategy" << std::endl;
        start = std::chrono::high_resolution_clock::now ();
        runCaseResult runSSEResult;
        runSSEResult.name = "runMethodSSE_k3";
        runSSEResult.k = 3;
        runSSEResult.strategy = SSE;
        runSSEResult.level = clusterProcessor.run (3, SSE);
        end = std::chrono::high_resolution_clock::now ();
        runSSEResult.durationMicros = std::chrono::duration_cast<std::chrono::microseconds> (end - start).count ();
        duration = runSSEResult.durationMicros;
        timings.push_back ({"runMethodSSE", duration});
        requireRunLevelIntegrity (runSSEResult, rawDataset.data.size ());

        runCaseResult runSSEK4Result;
        std::cout << std::endl;
        std::cout << "Testing run method with SSE and different k value" << std::endl;
        start = std::chrono::high_resolution_clock::now ();
        runSSEK4Result.name = "runMethodSSE_k4";
        runSSEK4Result.k = 4;
        runSSEK4Result.strategy = SSE;
        runSSEK4Result.level = clusterProcessor.run (4, SSE);
        end = std::chrono::high_resolution_clock::now ();
        runSSEK4Result.durationMicros = std::chrono::duration_cast<std::chrono::microseconds> (end - start).count ();
        duration = runSSEK4Result.durationMicros;
        timings.push_back ({"runMethodSSEK4", duration});
        requireRunLevelIntegrity (runSSEK4Result, rawDataset.data.size ());

        std::cout << std::endl;
        std::cout << "Testing run method with different k value" << std::endl;
        start = std::chrono::high_resolution_clock::now ();
        runCaseResult runK5Result;
        runK5Result.name = "runMethodONE_k5";
        runK5Result.k = 5;
        runK5Result.strategy = ONE;
        runK5Result.level = clusterProcessor.run (5, ONE);
        end = std::chrono::high_resolution_clock::now ();
        runK5Result.durationMicros = std::chrono::duration_cast<std::chrono::microseconds> (end - start).count ();
        duration = runK5Result.durationMicros;
        timings.push_back ({"runMethodK5", duration});
        requireRunLevelIntegrity (runK5Result, rawDataset.data.size ());

        std::cout << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "       RUN LEVEL PRINTER OUTPUT         " << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "Run: " << runOneResult.name << " | k=" << runOneResult.k << " | time(us)=" << runOneResult.durationMicros << std::endl;
        clusterProcessor.printDendrogramLevel (runOneResult.level);
        std::cout << std::endl;

        std::cout << "Run: " << runK1Result.name << " | k=" << runK1Result.k << " | time(us)=" << runK1Result.durationMicros << std::endl;
        clusterProcessor.printDendrogramLevel (runK1Result.level);
        std::cout << std::endl;

        std::cout << "Run: " << runSSEResult.name << " | k=" << runSSEResult.k << " | time(us)=" << runSSEResult.durationMicros << std::endl;
        clusterProcessor.printDendrogramLevel (runSSEResult.level);
        std::cout << std::endl;

        std::cout << "Run: " << runSSEK4Result.name << " | k=" << runSSEK4Result.k << " | time(us)=" << runSSEK4Result.durationMicros << std::endl;
        clusterProcessor.printDendrogramLevel (runSSEK4Result.level);
        std::cout << std::endl;

        std::cout << "Run: " << runK5Result.name << " | k=" << runK5Result.k << " | time(us)=" << runK5Result.durationMicros << std::endl;
        clusterProcessor.printDendrogramLevel (runK5Result.level);
        std::cout << "========================================" << std::endl;

        std::cout << "Data preprocessing completed successfully." << std::endl;

        std::cout << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "           TIMING REPORT                " << std::endl;
        std::cout << "========================================" << std::endl;

        long long totalTime = 0;
        for (const auto &timing : timings)
        {
            totalTime += timing.second;

            long long micros = timing.second;
            long long seconds = (micros / 1000000) % 60;
            long long minutes = (micros / 60000000) % 60;
            long long microRemainder = micros % 1000000;
            long long milliRemainder = microRemainder / 1000;
            long long microTail = microRemainder % 1000;

            std::cout << timing.first << ": ";
            std::cout << minutes << ":"
                      << (seconds < 10 ? "0" : "") << seconds << "."
                      << (milliRemainder < 10 ? "00" : milliRemainder < 100 ? "0" : "") << milliRemainder
                      << (microTail < 10 ? "00" : microTail < 100 ? "0" : "") << microTail
                      << std::endl;
        }

        std::cout << "----------------------------------------" << std::endl;
        long long totalMicros = totalTime;
        long long totalSec = (totalMicros / 1000000) % 60;
        long long totalMin = (totalMicros / 60000000) % 60;
        long long totalMicroRemainder = totalMicros % 1000000;
        long long totalMilliRemainder = totalMicroRemainder / 1000;
        long long totalMicroTail = totalMicroRemainder % 1000;

        std::cout << "TOTAL TIME: ";
        std::cout << totalMin << ":"
                  << (totalSec < 10 ? "0" : "") << totalSec << "."
                  << (totalMilliRemainder < 10 ? "00" : totalMilliRemainder < 100 ? "0" : "") << totalMilliRemainder
                  << (totalMicroTail < 10 ? "00" : totalMicroTail < 100 ? "0" : "") << totalMicroTail
                  << std::endl;

        std::cout << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "         MASTER TIMER (actual total)    " << std::endl;
        std::cout << "========================================" << std::endl;

        auto masterEnd = std::chrono::high_resolution_clock::now ();
        long long masterDuration = std::chrono::duration_cast<std::chrono::microseconds> (masterEnd - masterStart).count ();

        long long masterMicros = masterDuration;
        long long masterSec = (masterMicros / 1000000) % 60;
        long long masterMin = (masterMicros / 60000000) % 60;
        long long masterMicroRemainder = masterMicros % 1000000;
        long long masterMilliRemainder = masterMicroRemainder / 1000;
        long long masterMicroTail = masterMicroRemainder % 1000;

        std::cout << "Elapsed time: ";
        std::cout << masterMin << ":"
                  << (masterSec < 10 ? "0" : "") << masterSec << "."
                  << (masterMilliRemainder < 10 ? "00" : masterMilliRemainder < 100 ? "0" : "") << masterMilliRemainder
                  << (masterMicroTail < 10 ? "00" : masterMicroTail < 100 ? "0" : "") << masterMicroTail
                  << std::endl;
        std::cout << "========================================" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception caught during testing: " << e.what () << std::endl;
    }
    catch (...)
    {
        std::cerr << "Some fatal error during testing!" << std::endl;
    }

    return 0;
}