/**
    Copyright (c) 2016 <PeiXu xuxx0884@umn.edu>

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/**
    File: apriori.cc
    Purpose: Implementation of Apriori algorithm using Hash Tree
    @author Pei Xu
    @version 1.0 10/3/2016
*/

#include <iostream>

#include "apriori.h"

void Apriori::addTranscation(const Itemset& collection)
{
    this->transcations.push_back(Transcation(collection));
}

std::list<Rule>Apriori::run(const int unsigned& min_support,
                            const float       & min_conf,
                            const int unsigned& hash_range,
                            const int unsigned& max_leafsize)
{
    // Array containing current most requent item sets
    std::list<Itemset> candidates;
    this->candidates.clear();

    // Initialize Rules List
    this->rules.clear();

    // Initialize all kinds of iterators
    std::list<Dataset>::iterator cand;
    std::list<Transcation>::iterator trans;
    Itemset::iterator it;

    // Initialize ItemSet
    Itemset temp_set;
    std::map<int, int>::iterator item_table_it;

    for (trans = this->transcations.begin(); trans != this->transcations.end();
         ++trans)
    {
        for (it = trans->collection.begin(); it != trans->collection.end();
             it++)
        {
            item_table_it = this->item_table.find(*it);

            if (item_table_it  == this->item_table.end())
            {
                this->item_table[*it] = 1;
            }
            else
            {
                this->item_table[*it]++;
            }

            if (item_table_it->second == min_support)
            {
                // Generate Frequent 1-Rule
                temp_set.clear();
                temp_set.push_back(*it);
                candidates.push_back(temp_set);
            }
        }
    }

    // Generate Frequent 1-Itemset for each transcation
    for (trans = this->transcations.begin(); trans != this->transcations.end();
         ++trans)
    {
        for (it = trans->collection.begin(); it != trans->collection.end();
             ++it)
        {
            item_table_it = this->item_table.find(*it);

            if (item_table_it != this->item_table.end())
            {
                if (item_table_it->second >= min_support)
                {
                    temp_set.clear();
                    temp_set.push_back(*it);
                    trans->itemsets.push_back(temp_set);
                    this->freq_table[genFreqTableKey(temp_set)] =
                        item_table_it->second;
                }
            }
        }

        if (trans->itemsets.empty()) this->transcations.erase(trans);
    }

    Itemset subset;                   // the level-1 subset of a candidate
                                      // itemset
    std::list<Itemset> rule_basis;    // the cantainer of the above itemsets
    std::list<Itemset>::iterator rit; // the cantainer of the above itemsets
    int level = 1;


    while (candidates.size() > 1)
    {
        // TODO: Debug Info
        std::cout << "Level: " <<  level <<
            "; # of Candidates: " << candidates.size() << std::endl;

        candidates = genCandidates(candidates);

        if (candidates.size() < 1) break;

        level++;

        // TODO: Debug Info
        std::cout << "Level: " << level << "; # of Candidates: " <<
            candidates.size() << std::endl;

        // Generating Hash Tree
        HashTree tree(level, hash_range, max_leafsize);

        for (rit = candidates.begin(); rit != candidates.end();
             rit++)
        {
            this->candidates.push_back(Dataset(Itemset(*rit)));
            tree.insert(&(this->candidates.back()));
        }

        // Count Support
        bool found;
        Dataset *result;
        std::list<Itemset> temp_list;

        for (trans =  this->transcations.begin();
             trans != this->transcations.end();)
        {
            found = false;
            temp_list.clear();
            temp_list = trans->itemsets;
            trans->itemsets.clear();
            trans->itemsets = genCandidates(temp_list);

            // Something wrong with the following code.
            // trans->itemsets = genCandidates(trans->itemsets);

            for (rit = trans->itemsets.begin(); rit != trans->itemsets.end();
                 rit++)
            {
                result = tree.find(*rit);

                if (result == NULL)
                {
                    trans->itemsets.erase(rit);
                }
                else
                {
                    found = true;
                    result->count++;
                }
            }
            std::advance(trans, 1);

            // if (found == false) this->transcations.erase(std::next(trans,
            // -1));
        }

        // Generate Rules
        bool success;
        candidates.clear();

        for (cand = this->candidates.begin(); cand != this->candidates.end();
             cand++)
        {
            success = true;

            if (cand->count < min_support) success = false;
            else if (cand->data.size() > 2)
            {
                rule_basis.clear();

                for (it = cand->data.begin(); it != cand->data.end();
                     it++)
                {
                    subset.clear();
                    subset.push_back(*it);
                    rule_basis.push_back(subset);
                }

                success =
                    genRules(cand->data, rule_basis, min_conf, cand->count);
            }

            if (success == true)
            {
                candidates.push_back(cand->data);
                this->freq_table[genFreqTableKey(cand->data)] = cand->count;
            }
        }

        this->candidates.clear();

    }
    return this->rules;
}

bool Apriori::genRules(const Itemset   & item_set,
                       std::list<Itemset>basis,
                       const float     & min_conf,
                       const int       & current_support)
{
    Itemset diff;
    float   conf;

    for (std::list<Itemset>::iterator it = basis.begin();
         it != basis.end(); it++)
    {
        diff.clear();
        std::set_difference(item_set.begin(), item_set.end(),
                            it->begin(), it->end(),
                            std::inserter(diff, diff.begin()));

        if (diff.size() == 1)
        {
            conf = (float)current_support / this->item_table[*(diff.begin())];
        }
        else
        {
            conf = (float)current_support /
                   this->freq_table[genFreqTableKey(diff)];
        }

        if (conf < min_conf) basis.erase(it);
        else this->rules.push_back(Rule(diff, *it, current_support, conf));
    }

    if (basis.empty()) return false;
    else if (item_set.size() - (basis.begin())->size() > 1)
    {
        return genRules(item_set, genCandidates(basis), min_conf,
                        current_support);
    }

    else return true;
}

std::list<Itemset>Apriori::genCandidates(std::list<Itemset>& basis)
{
    std::list<Itemset> result;

    if ((basis.begin())->size() == 0) return result;

    std::list<Itemset>::iterator it;
    std::list<Itemset>::iterator itt;
    Itemset diff;

    // The algorithm below this block also works for the case generating
    // Frequent 2-Rule from Frequent 1-Rule.
    // But here I hope to increase the efficiency a little more when generating
    // 2-Rule from 1-Rule.
    if ((basis.begin())->size() == 1)
    {
        std::list<Itemset>::iterator it_end = std::next(basis.end(), -1);

        for (it = basis.begin(); it != it_end; it++)
        {
            for (itt = std::next(it, 1);
                 itt != basis.end(); itt++)
            {
                diff.clear();
                diff.push_back(it->front());
                diff.push_back(itt->front());
                result.push_back(diff);
            }
        }
        return result;
    }

    Itemset temp_subset;
    bool    success;

    for (it = basis.begin(); it != std::next(basis.end(), -1); it++)
    {
        for (itt = std::next(it, 1);
             itt != basis.end(); itt++)
        {
            // if ((this->freq_table.find(genFreqTableKey(*it)) ==
            //      this->freq_table.end()) ||
            //     (this->freq_table.find(genFreqTableKey(*itt)) ==
            //      this->freq_table.end()))
            // {
            //     break;
            // }

            diff.clear();
            std::set_difference(it->begin(), --(it->end()),
                                itt->begin(), --(itt->end()),
                                std::inserter(diff, diff.begin()));

            if (diff.empty()) // && it->back() < itt->back()
            {
                std::copy(it->begin(), it->end(),
                          std::inserter(diff, diff.begin()));
                diff.push_back(itt->back());

                success = true;

                // if (diff.size() > 2)
                // {
                // for (int i = 0; i < diff.size(); i++)
                // {
                //     temp_subset.clear();
                //     std::copy(diff.begin(), diff.end(),
                //               std::inserter(temp_subset,
                //                             temp_subset.begin()));
                //     temp_subset.erase(std::next(temp_subset.begin(), i));
                //
                //     if (std::find(basis.begin(), basis.end(),
                //                   temp_subset) == basis.end())
                //     {
                //         success = false;
                //         break;
                //     }
                // }
                // }

                if (success == true) result.push_back(diff);
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
              std::ostream_iterator<Item>(key_name, ","));
    return key_name.str();
}

Transcation::Transcation(const Itemset& collection)
{
    this->collection = collection;
    Itemset::iterator it = std::unique(
        this->collection.begin(), this->collection.end());
    this->collection.resize(std::distance(this->collection.begin(), it));
    std::sort(this->collection.begin(), this->collection.end());
}
