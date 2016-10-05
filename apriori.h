/**
    Copyright (c) 2016 <PeiXu xuxx0884@umn.edu>

    Permission is hereby granted, free of charge, to any person obtaining a copy
       of this software and associated documentation files (the "Software"), to
       deal in the Software without restriction, including without limitation
       the rights to use, copy, modify, merge, publish, distribute, sublicense,
       and/or sell copies of the Software, and to permit persons to whom the
       Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
       all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
       IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
       FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
       THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
       OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
       ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
       OTHER DEALINGS IN THE SOFTWARE.
 */

/**
    File: apriori.h
    Purpose: Header files for the implementation of Apriori algorithm using Hash
       Tree
    @author Pei Xu
    @version 1.0 10/3/2016
 */

#ifndef APRIORI_H
#define APRIORI_H
#include <map>
#include <list>
#include <forward_list>
#include <algorithm>
#include <thread>
#include <string>  // For generating hash key used to store the frequency or
                   // support of an itemset
#include <sstream> // same to the above
#include <mutex>

#include "hashTree.h"

typedef Data Itemset;

struct Rule
{
    Rule() {}

    Rule(Itemset lhs, Itemset rhs, int support, float confidence) : lhs(lhs), rhs(
            rhs), support(support), confidence(confidence) {}

    Itemset lhs; // antecedent, left-hand side
    Itemset rhs; // consequent, right-hand side
    int     support;
    float   confidence;
};

struct Transcation {
    Itemset collection;
    Transcation(const Itemset& collection);
};

class Apriori {
public:
    ~Apriori() {for(auto t : this->transcations) delete t.second;}
    std::list<Rule>run(const int unsigned& min_support,
                       const float       & min_conf = 0,
                       const int unsigned& hash_range = 2,
                       const int unsigned& max_leafsize = 5);
    void               addTranscation(const int & id, const Itemset& collection);
    static inline std::string genFreqTableKey(const Itemset& item_set);
    std::mutex  lock;

protected:

    std::map<std::string, float> freq_table;
    std::list<Rule> rules;

private:

    std::map<int, Transcation*> transcations;
    std::list<Dataset>     candidates;

    int genRules(const Itemset   & item_set,
                  std::list<Itemset>basis,
                  const float     & min_conf,
                  const int       & current_support);
    static std::list<Itemset>genCandidates(std::list<Itemset>& basis);

    // static void              countTransSupport(
    //     const std::forward_list<Transcation>::iterator trans_before_begin,
    //     const std::forward_list<Transcation>::iterator trans_end,
    //     HashTree                        * hashtree
    //     int & );
    static void countTransSupport(const std::map<int, Transcation *>::iterator trans_begin, const std::map<int, Transcation *>::iterator trans_end, HashTree *hashtree, std::map<int, Transcation *> * trans_handle, bool * boom, std::mutex * lock, std::list<int> * obsoleted);

};

#endif // ifndef APRIORI_H
