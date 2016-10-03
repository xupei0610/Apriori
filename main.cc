/**
    File: main.cc
    Purpose:
        This file is the main program for Project 1 of CSci 5523 Data Mining.
        It uses the Apriori algorithm with Hash Tree to analyze the given data
           in *small* or *large* files or other data having similar structures
        The algorithm program in apriori and hashTree was coded by PeiXu, and
           follows the MIT license.

    @author Pei Xu
    @version 0.2 10/3/2016
 */

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <list>

#include <ctime>

#include "apriori.h"

std::map<int, std::vector<int> >readData(const char *filename)
{
    std::fstream data_file;                // Data File read from
    std::map<int, std::vector<int> > data; // Transcations
    const char *delim = "\t 　";            // Delimiters in a line to split
                                           // transcation
                                           // and item ids;

    data_file.open(filename, std::ios::in);

    if (data_file.fail())
    {
        std::cerr << "Progam Stopped." << std::endl;
        std::cerr << "Unable to Open the Input File." << std::endl;
        exit(1);
    }

    std::string line; // Current Line being read

    while (std::getline(data_file, line))
    {
        if (line.empty()) continue;

        char *pch;
        char *dup_line = strdup(line.c_str());
        pch = strtok(dup_line, delim);
        free(dup_line);

        bool index = false;
        int  trans_id;

        while (pch != NULL)
        {
            if (index == false) {
                trans_id = std::stoi(pch);
                index    = true;
            }
            else
            {
                data[trans_id].push_back(std::stoi(pch));
            }
            pch = strtok(NULL, delim);
        }
    }

    data_file.close();

    return data;
}

int outputResults(std::list<Rule>& results, std::ostream& output_stream)
{
    int entries = 0;

    for (std::list<Rule>::iterator it = results.begin(); it != results.end();
         it++)
    {
        Rule item = *it;

        std::ostringstream lhs;
        std::ostringstream rhs;

        std::copy(item.lhs.begin(), item.lhs.end(),
                  std::ostream_iterator<int>(lhs, ","));
        std::copy(item.rhs.begin(), item.rhs.end(),
                  std::ostream_iterator<int>(rhs, ","));

        output_stream << "{" << lhs.str().substr(0, lhs.str().size() - 1) << "}|";
        output_stream << "{" << rhs.str().substr(0, rhs.str().size() - 1) << "}|";
        output_stream << item.support << "|" << item.confidence << std::endl;

        entries++;
    }

    return entries;
}

int main(int argc, char *argv[])
{
    int unsigned min_sup; // Minimum Support
    float min_conf;       // Minimum Confidence
    char *input_file;     // Name of Data File read from
    char *output_file;    // Name of Results File written in
    int unsigned hfrange;
    int unsigned maxleafsize;

    if (argc == 7)
    {
        min_sup     = std::atoi(argv[1]);
        min_conf    = std::atof(argv[2]);
        input_file  = argv[3];
        output_file = argv[4];
        hfrange     = std::atoi(argv[5]);
        maxleafsize = std::atoi(argv[6]);
    }
    else
    {
        // TODO: Exception
        std::cerr << "Program Stopped." << std::endl;
        std::cerr << "Wrong Number of Parameters were given." << std::endl;
        exit(1);
    }

    if (min_sup < 1)
    {
        std::cerr << "Program Stopped." << std::endl;
        std::cerr << "Invalid MinSupport was given." << std::endl;
        exit(1);
    }

    Apriori apriori;

    std::cout << "Paremeters Load:" << std::endl;
    std::cout << "\t MinSupport: " << min_sup << std::endl;
    std::cout << "\t MinConfidence: " << min_conf << std::endl;
    std::cout << "\t HashRange: " << hfrange << std::endl;
    std::cout << "\t LeafSize: " << maxleafsize << std::endl;
    std::cout << "\t Data File: " << input_file << std::endl;
    std::cout << "\t Output File: " << output_file << std::endl;

    std::cout << "Loading Data File ...";
    std::map<int, std::vector<int> > data = readData(input_file);

    for (std::map<int, std::vector<int> >::iterator data_item = data.begin();
         data_item != data.end(); ++data_item)
    {
        apriori.addTranscation(data_item->second);
    }
    std::cout << " Completed" << std::endl;

    std::cout << "Analyzing Data ..." << std::endl;
    clock_t t               = clock();
    std::list<Rule> results =
        apriori.run(min_sup, min_conf, hfrange, maxleafsize);
    t = clock() - t;
    std::cout << "Analysis Completed" << std::endl;

    std::cout << "Outputing Results ...";

    int num_of_rules = 0;
    std::fstream opstream;
    opstream.open(output_file, std::ios::out | std::ios::trunc);

    if (opstream.fail())
    {
        std::cerr << "Unable to Open the Output File. Output to screen." <<
            std::endl;
        num_of_rules = outputResults(results, std::cout);
    }
    else
    {
        time_t rawtime;
        time(&rawtime);
        opstream << "Date: " << ctime(&rawtime);
        opstream << "Time Used: " << (float)t / CLOCKS_PER_SEC << std::endl;
        opstream << "Parameters: " << "MinSupport: " << min_sup <<
        "|MinConfidence: " << "|HashRange: " << hfrange << "|MaxLeafSize: " <<
        maxleafsize << std::endl;
        num_of_rules = outputResults(results, opstream);
        opstream.close();
    }
    std::cout << "Completed" << std::endl;

    std::cout << "\nData Mining Completed." << std::endl;
    std::cout << num_of_rules << " of rules were generated." << std::endl;
    std::cout << "Results were written in the file \033[1m\033[4m" <<
        output_file << "\033[0m\033[00m." << std::endl;

    std::cout << "Time used: " << (float)t / CLOCKS_PER_SEC << "s" << std::endl;
}
