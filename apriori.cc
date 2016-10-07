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
    @version 0.9 10/7/2016
 */

#include <iostream>

#include "apriori.h"
using namespace apriori;

void Apriori::addTransaction(const int& id, const Itemset& collection)
{
    this->transactions[id] = new Transaction(collection);
}

void Apriori::run(const int unsigned& min_support,
                  const float       & min_conf,
                  const int unsigned& hash_range,
                  const int unsigned& max_leafsize)
{
    std::cout << "# of transactions: " <<  this->transactions.size() << std::endl;

    // Array containing current most requent item sets
    std::list<Itemset> candidates;
    this->candidates.clear();

    // Initialize Rules List
    this->rules.clear();
    bool success;
    std::list<Itemset> rule_basis;

    // Initialize time recod
    std::chrono::time_point<std::chrono::high_resolution_clock> clock_rule_gen;
    std::chrono::time_point<std::chrono::high_resolution_clock> clock_freq_gen =
        std::chrono::high_resolution_clock::now();

    // Initialize ItemSet
    std::map<int, int>::iterator item_table_it;
    std::map<int, int> item_table;


    for (auto & trans : this->transactions)
    {
        for (auto & item : trans.second->collection)
        {
            item_table_it = item_table.find(item);

            if (item_table_it  == item_table.end())
            {
                item_table[item] = 1;
            }
            else
            {
                item_table[item]++;
            }

            if (item_table_it->second == min_support)
            {
                // Generate Frequent 1-Rule
                candidates.push_back(Itemset({ item }));
            }
        }
    }
    candidates.sort();

    // Erase Infrequent 1-Itemset from each transaction
    for (auto trans = this->transactions.begin();
         trans != this->transactions.end();
         ++trans)
    {
        for (auto item_it = trans->second->collection.begin();
             item_it != trans->second->collection.end(); item_it++)
        {
            item_table_it = item_table.find(*item_it);

            if (item_table_it != item_table.end())
            {
                if (item_table_it->second >= min_support)
                {
                    this->freq_table[Apriori::genFreqTableKey(Itemset({ *item_it }))
                    ] = item_table_it->second;
                }
                else
                {
                    std::advance(item_it, -1);
                    trans->second->collection.erase(std::next(item_it, 1));
                }
            }
        }

        if (trans->second->collection.empty()) this->transactions.erase(trans);
    }

    this->time_freq_gen = std::chrono::high_resolution_clock::now() -
                          clock_freq_gen;

    int level = 1;

    int multithread = 10;
    int iter_times;
    hashTree::HashTree *tree;
    bool *boom;

    while (candidates.size() > 1)
    {
        this->candidates.clear();

        // TODO: Debug Info
        std::cout << "Level: " <<  level <<
            "; # of Candidates: " << candidates.size() << std::endl;
        candidates = this->genCandidates(candidates);
        level++;

        // TODO: Debug Info
        std::cout << "Level: " << level << "; # of Candidates: " <<
            candidates.size() << std::endl;

        if (candidates.size() < 1) break;

        std::map<int,
                 Transaction *>::iterator it_begin = this->transactions.begin();
        std::map<int, Transaction *>::iterator it_end;
        std::list<int> obsoleted;
        std::vector<std::thread> threads;
        iter_times = this->transactions.size() / multithread;

        // Generating Hash Tree
        std::cout << "Planting Hash Tree ..." << std::endl;
        tree = new hashTree::HashTree(level, hash_range, max_leafsize);

        for (auto cand : candidates)
        {
            this->candidates.push_back(hashTree::Dataset(Itemset({ cand })));
            tree->insert(&(this->candidates.back()));
        }

        std::cout << "Hash Tree has grown up." << std::endl;
        candidates.clear();

        // Count Support
        clock_freq_gen = std::chrono::high_resolution_clock::now();

        boom = new bool(false);

        if (this->transactions.size() < 1000)
        {
            std::cout << "Begining counting support..." << std::endl;
            *boom = true;
            Apriori::countTransSupport(it_begin,
                                       this->transactions.end(), tree, boom,
                                       &(this->lock), &obsoleted);
        }
        else
        {
            std::cout << "Starting multi-thread..." << std::endl;
            multithread = this->transactions.size() / 1000;

            if (multithread == 0)
            {
                multithread++;
            }
            iter_times = this->transactions.size() / multithread;

            for (int i = 0; i < multithread - 1; i++)
            {
                it_end = std::next(it_begin, iter_times);
                threads.push_back(std::thread(Apriori::countTransSupport,
                                              it_begin,
                                              it_end, tree,
                                              boom, &(this->lock), &obsoleted,
                                              i + 1));
                it_begin = it_end;
            }

            threads.push_back(std::thread(Apriori::countTransSupport,
                                          it_begin,
                                          this->transactions.end(), tree,
                                          boom, &(this->lock), &obsoleted,
                                          multithread));

            std::cout << "Begining counting support..." << std::endl;
            *boom = true;

            for (auto & th: threads)
            {
                th.join();
            }
        }

        delete tree;
        delete boom;

        std::cout << "Abandoning abandoned transactions..." << std::endl;

        for (auto & o : obsoleted)
        {
            this->transactions.erase(o);
        }

        this->time_freq_gen +=
            (std::chrono::high_resolution_clock::now() - clock_freq_gen);

        // No need to generate Rules
        if (min_conf < 0)
        {
            for (auto & cand: this->candidates)
            {
                if (cand.count >= min_support)
                {
                    this->rules.push_back(Rule(Itemset(cand.data), Itemset(),
                                               cand.count, -1));
                    candidates.push_back(cand.data);
                    this->freq_table[Apriori::genFreqTableKey(cand.data)] =
                        cand.count;
                }
            }
            std::cout << std::endl;
            continue;
        }

        // Generate Rules
        std::cout << "Generating rules..." << std::endl;
        clock_rule_gen = std::chrono::high_resolution_clock::now();

        for (auto & cand : this->candidates)
        {
            success = true;

            if (cand.count < min_support)
            {
                success = false;

                // TODO: 删掉含有这个项集的事务中对应的项集
            }
            else if (level > 2)
            {
                rule_basis.clear();

                for (auto & item : cand.data)
                {
                    rule_basis.push_back(Itemset({ item }));
                }

                success = this->genRules(cand.data,
                                         rule_basis,
                                         min_conf,
                                         cand.count);
            }

            if (success == true)
            {
                candidates.push_back(cand.data);
                this->freq_table[Apriori::genFreqTableKey(cand.data)] =
                    cand.count;
            }
        }
        this->time_rule_gen +=
            (std::chrono::high_resolution_clock::now() - clock_rule_gen);

        std::cout << std::endl;
    }
}

int Apriori::genRules(const Itemset   & itemset,
                      std::list<Itemset>basis,
                      const float     & min_conf,
                      const int       & current_support)
{
    Itemset diff;
    float   conf;
    int     count = 0;

    for (auto it = basis.begin(); it != basis.end(); it++)
    {
        diff.clear();
        std::set_difference(itemset.begin(), itemset.end(),
                            it->begin(), it->end(),
                            std::inserter(diff, diff.begin()));
        // Reliable source of itemsets from genCandidates()
        conf = (float)current_support /
               this->freq_table[Apriori::genFreqTableKey(diff)];

        if (conf < min_conf)
        {
            basis.erase(it);
        }
        else
        {
            this->rules.push_back(Rule(diff, *it, current_support, conf));
            count++;
        }
    }

    if ((count > 0) &&  (itemset.size() - (basis.begin())->size() > 1))
    {
        return this->genRules(itemset, this->genCandidates(basis), min_conf,
                              current_support);
    }
    else
    {
        return count;
    }
}

std::list<Itemset>Apriori::genCandidates(std::list<Itemset>& basis)
{
    std::list<Itemset> result;

    if (basis.begin()->size() == 0) return result;

    auto it_end = --basis.end();

    // The algorithm below this block also works for the case generating
    // Frequent 2-Rule from Frequent 1-Rule.
    // But here I hope to increase the efficiency a little more when generating
    // 2-Rule from 1-Rule.
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
    bool    success;
    int     level = basis.front().size() + 1;

    for (auto it = basis.begin(); it != it_end; it++)
    {
        for (auto itt = std::next(it, 1); itt != basis.end(); itt++)
        {
            if (std::equal(it->begin(), --(it->end()), itt->begin()))

            // && it->back() < itt->back()
            {
                cand.clear();
                cand = *it;
                cand.push_back(itt->back());

                success = true;

                // The situation cand.size() == 2 has been dealt at the begining
                // of this function.
                // if (cand.size() > 2)
                // {

                for (int i = 0; i < level; i++)
                {
                    temp_subset.clear();
                    temp_subset = cand;
                    temp_subset.erase(std::next(temp_subset.begin(), i));

                    if (this->freq_table.find(Apriori::genFreqTableKey(temp_subset))
                        == this->freq_table.end())
                    {
                        success = false;
                        break;
                    }
                }

                if (success == true)
                {
                    result.push_back(cand);
                }

                // }
            }
            else break;
        }
    }

    return result;
}

inline std::string Apriori::genFreqTableKey(const Itemset& item_set)
{
    std::ostringstream key_name;
    std::copy(item_set.begin(), item_set.end(),
              std::ostream_iterator<hashTree::Item>(key_name, ","));
    return key_name.str();
}

Transaction::Transaction(const Itemset& collection)
{
    this->collection = collection;
    auto it          = std::unique(this->collection.begin(),
                                   this->collection.end());
    this->collection.resize(std::distance(this->collection.begin(), it));
    std::sort(this->collection.begin(), this->collection.end());
}

void Apriori::countTransSupport(
    const std::map<int, Transaction *>::iterator trans_begin,
    const std::map<int, Transaction *>::iterator trans_end,
    hashTree::HashTree                                *hashtree,
    bool                                    *boom,
    std::mutex                              *lock,
    std::list<int>                          *obsoleted,
    const int&                             tid)
{
    std::list<int> losers;
    bool search_res;
    while (*boom == false) std::this_thread::yield();

    for (auto it = trans_begin; it != trans_end; it++)
    {
        search_res = hashtree->findSubsetOf(it->second->collection, true);

        if (search_res == false)
        {
            losers.push_back(it->first);
        }
    }

    if (tid != 0)
    {
        std::cout << "Thread #" << tid << " completed." << std::endl;
    }

    lock->lock();

    for (auto & d : losers)
    {
        obsoleted->push_back(d);
    }
    lock->unlock();
}

inline Itemset Apriori::solveTableKey(const std::string& table_key)
{
    Itemset result;

    // TODO
    return result;
}

std::map<std::string, int>Apriori::getFreqTable()
{
    return this->freq_table;
}

std::list<Rule>Apriori::getRules()
{
    return this->rules;
}

double Apriori::getRuleGenTime()
{
    return this->time_rule_gen.count();
}

double Apriori::getFreqGenTime()
{
    return this->time_freq_gen.count();
}
