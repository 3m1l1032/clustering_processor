/* 
 * Emilio Cazares Borbon 04/19/2026
 * 
 * Clustering Project Main Implementation File.
 * 
 * Created for Georgetown University Data Mining COSC 3590, Spring 2026.
 * 
 * For details, please see `/include/main.h`.
 */

#include "../include/main.h"

struct ExperimentResult
{
    terminationStrategy strategy = SSE;
    size_t k = 0;
    long long runtimeMicroseconds = 0;
    double finalInterDistance = 0.0;
    double finalIntraDistance = 0.0;
    size_t minClusterSize = 0;
    size_t maxClusterSize = 0;
    double avgClusterSize = 0.0;
    std::vector<std::map<std::string, size_t>> classCounts;
    std::vector<double> interByIteration;
    std::vector<double> intraByIteration;
    std::vector<double> sseByIteration;
    std::string savedFile;
};

size_t parseThreadCountArg (const std::string &arg)
{
    const std::string prefix = "--threads=";
    if (arg.rfind (prefix, 0) != 0)
        return 0;

    const std::string value = arg.substr (prefix.size ());
    if (value.empty ())
        return 0;

    return static_cast<size_t> (std::stoul (value));
}

std::string formatDuration (long long microseconds)
{
    const long long totalMilliseconds = microseconds / 1000;
    const long long minutes = totalMilliseconds / 60000;
    const long long secondsPart = (totalMilliseconds / 1000) % 60;
    const long long millisecondsPart = totalMilliseconds % 1000;
    const double seconds = static_cast<double> (microseconds) / 1000000.0;

    std::ostringstream out;
    out << std::fixed << std::setprecision (3) << seconds << " s";
    out << " (" << minutes << ":";
    out << std::setw (2) << std::setfill ('0') << secondsPart << ".";
    out << std::setw (3) << std::setfill ('0') << millisecondsPart << ")";

    return out.str ();
}

ExperimentResult runOneExperiment (const ClusteringProcessor &preprocessedProcessor,
                                   size_t k,
                                   terminationStrategy strategy)
{
    ExperimentResult result;
    result.strategy = strategy;
    result.k = k;

    ClusteringProcessor processorCopy (preprocessedProcessor);

    auto start = std::chrono::high_resolution_clock::now ();
    dendrogramLevel level = processorCopy.run (k, strategy);
    auto end = std::chrono::high_resolution_clock::now ();

    result.runtimeMicroseconds =
        std::chrono::duration_cast<std::chrono::microseconds> (end - start).count ();

    result.interByIteration = level.interClusterDistances;
    result.intraByIteration = level.intraClusterDistances;
    result.sseByIteration = level.SSEs;

    if (!level.interClusterDistances.empty ())
        result.finalInterDistance = level.interClusterDistances.back ();

    if (!level.intraClusterDistances.empty ())
        result.finalIntraDistance = level.intraClusterDistances.back ();

    if (!level.clusters.empty ())
    {
        result.minClusterSize = std::numeric_limits<size_t>::max ();
        size_t totalSize = 0;

        for (const auto &cluster : level.clusters)
        {
            const size_t clusterSize = cluster.instances.size ();
            totalSize += clusterSize;

            if (clusterSize < result.minClusterSize)
                result.minClusterSize = clusterSize;
            if (clusterSize > result.maxClusterSize)
                result.maxClusterSize = clusterSize;

            std::map<std::string, size_t> counts;
            for (const auto &instance : cluster.instances)
                counts[instance.classLabel] += 1;

            result.classCounts.push_back (counts);
        }

        result.avgClusterSize = static_cast<double> (totalSize) /
                                static_cast<double> (level.clusters.size ());
    }

    result.savedFile = processorCopy.saveDendrogramLevel (level, strategy, k, "results");

    return result;
}

void printOneResult (const ExperimentResult &result)
{
    std::cout << "============================================================" << std::endl;
    std::cout << "Strategy = " << (result.strategy == SSE ? "SSE" : "ONE") << std::endl;
    std::cout << "k = " << result.k << std::endl;
    std::cout << "Time (clustering only, no pre-processing): "
              << formatDuration (result.runtimeMicroseconds) << std::endl;
    std::cout << "Final inter-cluster distance: " << result.finalInterDistance << std::endl;
    std::cout << "Final intra-cluster distance: " << result.finalIntraDistance << std::endl;
    std::cout << "Cluster size stats -> min: " << result.minClusterSize
              << ", max: " << result.maxClusterSize
              << ", avg: " << result.avgClusterSize << std::endl;

    std::cout << "Class label distribution:" << std::endl;
    for (size_t i = 0; i < result.classCounts.size (); i++)
    {
        std::cout << "  Cluster " << i + 1 << ": ";
        bool first = true;
        for (const auto &entry : result.classCounts[i])
        {
            if (!first)
                std::cout << ", ";
            std::cout << entry.first << "=" << entry.second;
            first = false;
        }
        if (first)
            std::cout << "(none)";
        std::cout << std::endl;
    }

    std::cout << "Iteration metrics:" << std::endl;
    const size_t rows = std::max (result.interByIteration.size (), result.intraByIteration.size ());
    for (size_t i = 0; i < rows; i++)
    {
        std::cout << "  Iteration " << i + 1 << ": ";

        if (i < result.interByIteration.size ())
            std::cout << "inter=" << result.interByIteration[i] << ", ";
        else
            std::cout << "inter=n/a, ";

        if (i < result.intraByIteration.size ())
            std::cout << "intra=" << result.intraByIteration[i] << ", ";
        else
            std::cout << "intra=n/a, ";

        if (i < result.sseByIteration.size ())
            std::cout << "SSE=" << result.sseByIteration[i];
        else
            std::cout << "SSE=n/a";

        std::cout << std::endl;
    }

    std::cout << "Saved to: " << result.savedFile << std::endl;
}

int main (int argc, char **argv)
{
    const std::string defaultFile = "data/adult-small.arff";
    const std::string defaultClass = "class";
    std::string filename = defaultFile;
    std::string classAttribute = defaultClass;
    std::vector<size_t> kValues = {2, 3, 4, 5, 6, 7, 8};
    std::vector<terminationStrategy> strategies = {ONE, SSE};

    bool useThreads = true;
    size_t requestedThreads = 0;

    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];

        if (arg == "--sequential")
            useThreads = false;
        else if (arg.rfind ("--threads=", 0) == 0)
            requestedThreads = parseThreadCountArg (arg);
        else if (filename == defaultFile)
            filename = arg;
        else if (classAttribute == defaultClass)
            classAttribute = arg;
    }

    try
    {
        ClusteringProcessor baseProcessor (filename);
        baseProcessor.setClassAttribute (classAttribute);
        baseProcessor.replaceMissingValues (MEAN_MODE);
        baseProcessor.normalizeNumericValues ();
        baseProcessor.vectorizeDataInstances ();

        std::vector<std::pair<terminationStrategy, size_t>> jobs;
        for (const auto &currStrategy : strategies)
        {
            for (const auto &k : kValues)
                jobs.push_back ({currStrategy, k});
        }

        std::vector<ExperimentResult> results (jobs.size ());

        if (!useThreads)
        {
            for (size_t i = 0; i < jobs.size (); i++)
                results[i] = runOneExperiment (baseProcessor, jobs[i].second, jobs[i].first);
        }
        else
        {
            size_t workerCount = static_cast<size_t> (std::thread::hardware_concurrency ());
            if (workerCount == 0)
                workerCount = 1;

            if (requestedThreads > 0)
                workerCount = requestedThreads;

            if (workerCount > jobs.size ())
                workerCount = jobs.size ();

            std::atomic<size_t> nextIndex (0);
            std::atomic<bool> hasError (false);
            std::mutex errorMutex;
            std::string errorMessage;
            std::vector<std::thread> workers;

            for (size_t worker = 0; worker < workerCount; worker++)
            {
                workers.emplace_back ([&]()
                {
                    while (true)
                    {
                        const size_t index = nextIndex.fetch_add (1);
                        if (index >= jobs.size ())
                            break;

                        if (hasError)
                            break;

                        try
                        {
                            results[index] = runOneExperiment (baseProcessor, jobs[index].second, jobs[index].first);
                        }
                        catch (const std::exception &e)
                        {
                            std::lock_guard<std::mutex> lock (errorMutex);
                            if (!hasError)
                            {
                                hasError = true;
                                errorMessage = e.what ();
                            }
                        }
                        catch (...)
                        {
                            std::lock_guard<std::mutex> lock (errorMutex);
                            if (!hasError)
                            {
                                hasError = true;
                                errorMessage = "Unknown error in threaded experiment.";
                            }
                        }
                    }
                });
            }

            for (auto &thread : workers)
                thread.join ();

            if (hasError)
                throw std::runtime_error (errorMessage);
        }

        std::cout << "Clustering Experiment Results" << std::endl;
        std::cout << "Dataset: " << filename << std::endl;
        std::cout << "Class attribute: " << classAttribute << std::endl;
        std::cout << "Termination strategies: ONE and SSE" << std::endl;
        std::cout << std::endl;

        for (const auto &result : results)
            printOneResult (result);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error running experiment: " << e.what () << std::endl;
        return 1;
    }

    return 0;
}