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
 @version 0.9 10/7/2016
 */

#ifndef APRIORI_H
#define APRIORI_H
#include <map>
#include <list>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono> // For record running time
#include <iostream>
#include <iterator>

#include "manager.h"

namespace apriori {
    
    typedef std::list<item::ItemID>Itemset;
    
    struct Rule
    {
        Rule(const Itemset & lhs, const Itemset & rhs, const int & support, const float & confidence) : lhs(lhs), rhs(
                                                                                                                      rhs), support(support), confidence(confidence) {}
        
        Itemset lhs; // antecedent, left-hand side
        Itemset rhs; // consequent, right-hand side
        int     support;
        float   confidence;
    };
    
    class Apriori {
    public:
        
        Apriori();
        
        ~Apriori();
        int run(const int unsigned& min_support,
                 const float       & min_conf = -1);
        void    setOutputStream(std::ostream * output_stream);
        void           addTransaction(const int    & id,
                                      const Itemset& collection);
        
        int            getFreq(const std::list<int>& itemset);
        void           useHashTree(const int unsigned& hash_range = 100000,
                                   const int unsigned& max_leafsize = 5);
        std::list<Rule>getRules();
        double         getRuleGenTime();
        double         getFreqGenTime();
        std::list<Rule> rules;
 
    private:
        
        int unsigned _hash_range;
        int unsigned _max_leafsize;
        int unsigned _min_sup;
        int _rule_num;
        std::map<std::list<int>, int> _freq_table;
        float _min_conf;
        bool _hash_tree;
        std::ostream * _opstream;
        
        
        manager::Manager *_trans_manager;
        
        
        std::chrono::duration<double, std::ratio<1> > _time_freq_gen;
        std::chrono::duration<double, std::ratio<1> > _time_rule_gen;
        int               _collect();

        void              _setFreq(const std::list<int>& itemset,
                                   const int    & sup);
        int               _genRules(const std::list<int>& itemset, const int & current_sup);
        std::list<Itemset>_genCands(const std::list<std::list<int> > & basis);
        void _addRule(const std::list<int> & lhs, const std::list<int> & rhs, const int & support, const float & confidence);
        
    };
}


#endif // ifndef APRIORI_H
