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
    File: apriori.cc
    Purpose: Implementation of Apriori algorithm using Hash Tree
    @author Pei Xu
    @version 0.9 10/24/2016
 */

#include <iostream>

#include "apriori.h"

using namespace apriori;

Apriori::Apriori()
{
    this->_trans_tree = nullptr;
    this->_item_tree  = new itemTree::ItemTree();
}

Apriori::~Apriori()
{
    for (auto t : this->_transactions) delete t.second;
    delete this->_trans_tree;
    delete this->_item_tree;
}

void Apriori::addTransaction(const int& id, const Itemset& collection)
{
    this->_transactions[id] = new Transaction(collection);

    for (auto & it : this->_transactions[id]->collection)
    {
        if (this->item_table.find(it) == this->item_table.end())
        {
            this->item_table[it] = std::list<int>({ id });
            this->item_table2[it] = 1;
        }
        else
        {
            this->item_table[it].push_back(id);
            this->item_table2[it]++;
        }
    }
}
void Apriori::setOutputStream(std::ostream * output_stream)
{
    this->_opstream = output_stream;
}
void Apriori::_addRule(const std::list<int> & lhs, const std::list<int> & rhs, const int & support, const float & confidence)
{
    if (this->_opstream == nullptr)
        this->rules.emplace_back(lhs, rhs, support, confidence);
    else
    {
        std::ostringstream lhss;
        std::ostringstream rhss;
        
        std::copy(lhs.begin(), lhs.end(),
                  std::ostream_iterator<int>(lhss, ","));
        std::copy(rhs.begin(), rhs.end(),
                  std::ostream_iterator<int>(rhss, ","));
        
        *(this->_opstream) << "{" << lhss.str().substr(0, lhss.str().size() - 1) << "}|" << "{" << rhss.str().substr(0, rhss.str().size() - 1) << "}|" << support << "|" << confidence << std::endl;
    }
}

void Apriori::_plantTransTree(const int & level)
{
    if (this->_trans_tree != nullptr)
    {
//         delete this->_trans_tree;
//         this->__rebuildTransTree(level);
        return;
    }
    this->_trans_tree = new transTree::TransTree();
    std::map<int, Transaction *>::iterator trans_end = this->_transactions.end();
    Itemset::iterator trans_col_end;
    int sup;
    for (auto trans = this->_transactions.begin(); trans != trans_end;)
    {
        trans_col_end = trans->second->collection.end();
        for (auto it = trans->second->collection.begin();
             it != trans_col_end;)
        {
            sup = this->item_table2[*it];
            if (sup < this->_min_sup)
            {
                it = trans->second->collection.erase(it);
                continue;
            }

            if (sup >= this->_min_sup)
            {
                this->_setFreq(Itemset({ *it }), sup);
            }
            it++;
        }

        if (trans->second->collection.size() > 1)
        {
            this->_trans_tree->addTransaction(trans->second->collection,
                                              trans->first);
                                              trans++;
        }
        else
        {
          trans = this->_transactions.erase(trans);
        }
    }
}

void Apriori::__rebuildTransTree(const int & level)
{
    this->_trans_tree = new transTree::TransTree();
    std::map<int, Transaction *>::iterator trans_end = this->_transactions.end();
    Itemset::iterator trans_col_end;
    this->_trans_tree = new transTree::TransTree();
    for (auto trans = this->_transactions.begin();
         trans != trans_end;)
    {
        trans_col_end = trans->second->collection.end();
        for (auto it = trans->second->collection.begin();
             it != trans_col_end;)
        {
            if (this->item_table2[*it] < this->_min_sup)
            {
                it = trans->second->collection.erase(it);
                continue;
            }
            it++;
        }
        if (trans->second->collection.size() > level)
        {
            this->_trans_tree->addTransaction(trans->second->collection,
                                              trans->first);
                                              trans++;
        }
        else
        {
          trans = this->_transactions.erase(trans);
        }
    }

}

int Apriori::run(const int unsigned& min_support,
                  const float       & min_conf,
                  const int unsigned& hash_range,
                  const int unsigned& max_leafsize)
{
    this->_hash_range   = hash_range;
    this->_max_leafsize = max_leafsize;
    this->_min_sup      = min_support;
    this->_min_conf     = min_conf;

    std::chrono::time_point<std::chrono::high_resolution_clock>
    clock_rule_gen;
    std::chrono::time_point<std::chrono::high_resolution_clock>
    clock_freq_gen;

    std::cout << "Planting item tree..." << std::endl;
    for (auto & it : this->item_table)
    {
        if (it.second.size() >= this->_min_sup)
        {
            if (this->_min_conf < 0)
            this->_addRule(Itemset({it.first}), Itemset(), it.second.size(), -1);
            this->_setFreq(Itemset({it.first}),it.second.size());
        }
    }
    int res = this->_item_tree->plant(this->_min_sup, this->item_table);

    std::cout << "Level: 1; # of Candidates: " << res << std::endl;

    int  level   = 1;
    int freqs = 0;
    std::list<itemTree::Node *> candidates;
    if (this->_min_conf >= 0)
        res = 0;
    do
    {
        std::cout << "  Generating Candidates..." << std::endl;
        clock_freq_gen = std::chrono::high_resolution_clock::now();
        candidates           = this->_item_tree->grow();
        this->_time_freq_gen += std::chrono::high_resolution_clock::now() -
                                       clock_freq_gen;

        if (candidates.empty())
            break;

        level++;
        std::cout << "Level: " <<  level <<   "; # of Candidates: " <<
            candidates.size() << std::endl;


        std::cout << "  Planting hash tree..." << std::endl;
        hashTree::HashTree *hash_tree =
            new hashTree::HashTree(level, this->_hash_range, this->_max_leafsize);

        for (auto & cand: candidates)
        {
            hash_tree->insert(cand);
        }

        std::cout << "  Counting support..." << std::endl;
        clock_freq_gen = std::chrono::high_resolution_clock::now();
        this->_plantTransTree(level-1);
        this->_trans_tree->count(hash_tree);
        this->_time_freq_gen += std::chrono::high_resolution_clock::now() -
                               clock_freq_gen;
        delete hash_tree;

        std::cout << "  Generating rules..." << std::endl;
        clock_rule_gen = std::chrono::high_resolution_clock::now();

        freqs = 0;
        for (auto & cand: candidates)
        {
//            int count = cand->trans.size();
            int count = cand->count;
            if (count < this->_min_sup)
            {
                for (auto & c: cand->dataset)
                {
                    this->item_table2[c] -= count;
                }
            }
            else
            {
                this->_setFreq(cand->dataset, count);

                if (this->_min_conf < 0)
                {
                    this->_addRule(cand->dataset, Itemset(),
                                             count, -1);
                    res++;
                }
                else
                {
                    res+=this->_genRules(cand->dataset, count);
                }
                freqs++;
            }
        }
        this->_time_rule_gen +=
        std::chrono::high_resolution_clock::now() - clock_rule_gen;
        
        std::cout << "Frequent " <<  level <<   "-Itemset: " << freqs << std::endl;
    } while (freqs > 0);
    
    return res;

}

int Apriori::_genRules(const Itemset& itemset, const int & current_sup)
{
    if (itemset.size() < 2) return 0;

    std::list<Itemset> basis;
    Itemset diff;
    int     count       = 0;
    float   conf;
    int sub_sup;
    for (auto & item : itemset)
    {
        basis.push_back(Itemset({ item }));
    }

    while (!basis.empty())
    {
        for (auto it = basis.begin(); it != basis.end(); it++)
        {
            diff.clear();
            std::set_difference(itemset.begin(), itemset.end(),
                                it->begin(), it->end(),
                                std::inserter(diff, diff.begin()));

            sub_sup = this->getFreq(diff);

            if (sub_sup < 1)
                continue;

            conf = (float)current_sup / sub_sup;

            if (conf < this->_min_conf)
            {
                basis.erase(it--);
            }
            else
            {
                this->_addRule(diff, *it, current_sup, conf);
                count++;
            }
        }
        if ((basis.size() < 2) ||
            (basis.back().size() == itemset.size() - 1)) break;
        basis = this->_genCands(basis);
    }
    return count;
}

// This func is only used when generating rules. Some key but useless parts are commented.
std::list<Itemset>Apriori::_genCands(std::list<Itemset>& basis)
{
    std::list<Itemset> result;

    if (basis.begin()->size() == 0) return result;

    auto it_end = --basis.end();

    if (basis.begin()->size() == 1)
    {
        for (auto it = basis.begin(); it != it_end; it++)
        {
            for (auto itt = std::next(it, 1); itt != basis.end(); itt++)
            {
                result.push_back(Itemset({ it->front(), itt->front() }));
            }
        }
        return result;
    }

    Itemset cand, temp_subset;
    // bool    success;
    // int     level = basis.front().size() + 1;

    for (auto it = basis.begin(); it != it_end; it++)
    {
        for (auto itt = std::next(it, 1); itt != basis.end(); itt++)
        {
            if (std::equal(it->begin(), --(it->end()), itt->begin()))
            {
                cand.clear();

                if (it->back() < itt->back())
                {
                    cand = *it;
                    cand.push_back(itt->back());
                }
                else
                {
                    cand = *itt;
                    cand.push_back(it->back());
                }

                // success = true;
                //
                // for (int i = 0; i < level; i++)
                // {
                //     temp_subset.clear();
                //     temp_subset = cand;
                //     temp_subset.erase(std::next(temp_subset.begin(), i));
                //
                //     auto hash_key = Apriori::genFreqTableKey(temp_subset);
                //
                //     if (this->freq_table.find(hash_key)
                //         == this->freq_table.end())
                //     {
                //         success = false;
                //         break;
                //     }
                // }


                // if (success == true)
                // {
                    result.push_back(cand);
                // }
            }
            else break;
        }
    }

    return result;
}

std::list<Rule>Apriori::getRules()
{
    return this->rules;
}

void Apriori::_setFreq(const Itemset& itemset, const int& sup)
{
    this->freq_table[itemset] = sup;
}

int Apriori::getFreq(const Itemset& itemset)
{
    return this->freq_table[itemset];
}

double Apriori::getRuleGenTime()
{
    return this->_time_rule_gen.count();
}

double Apriori::getFreqGenTime()
{
    return this->_time_freq_gen.count();
}

Transaction::Transaction(const Itemset& collection)
{
    this->collection = collection;
    this->collection.sort();
    this->collection.unique();
}
