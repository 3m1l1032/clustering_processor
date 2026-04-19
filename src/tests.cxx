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
} // namespace

int main ()
{
    const auto masterStart = std::chrono::high_resolution_clock::now ();
    using Processor = ClusteringProcessorNS::ClusteringProcessor;

    try
    {
        std::cout << "Clustering Processor Test File" << std::endl;
        std::cout << std::endl;

        std::string filename = "data/adult-small.arff";
        ARFF rawDataset (filename);

        std::vector<std::pair<std::string, long long>> timings;

        CLUSTERING_PROCESSOR_SPLIT_DEBUGGER ("Creating ClusteringProcessor instance with dataset: " + filename);
        auto start = std::chrono::high_resolution_clock::now ();
        Processor clusterProcessor (filename);
        auto end = std::chrono::high_resolution_clock::now ();
        long long duration = std::chrono::duration_cast<std::chrono::microseconds> (end - start).count ();
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
        std::cout << "Replacement values by attribute" << std::endl;
        for (size_t attributeIndex = 0; attributeIndex < rawDataset.attributes.size (); attributeIndex++)
        {
            const Attribute &attribute = rawDataset.attributes[attributeIndex];
            std::cout << "  [" << attributeIndex << "] " << attribute.name << " -> "
                      << clusterProcessor.getReplacementValue (attributeIndex) << std::endl;
        }

        std::cout << std::endl;
        std::cout << "Normalization values" << std::endl;
        clusterProcessor.printNormalizationInfo ();

        std::cout << std::endl;
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