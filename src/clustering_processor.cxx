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

namespace ClusteringProcessor
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




} // namespace ClusteringProcessor