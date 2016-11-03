
#include "apriori.h"

using namespace apriori;

Apriori::Apriori()
{
    this->_trans_manager = new manager::Manager();
    this->_opstream = nullptr;
    this->_hash_tree = false;
}

Apriori::~Apriori()
{
    delete this->_trans_manager;
}

void Apriori::addTransaction(const int& id, const Itemset& collection)
{
    this->_trans_manager->addTrans(collection);
}

void Apriori::setOutputStream(std::ostream * output_stream)
{
    this->_opstream = output_stream;
}

void Apriori::useHashTree(const int unsigned& hash_range,
                          const int unsigned& max_leafsize)
{
    this->_trans_manager->useHashTree(hash_range, max_leafsize);
    this->_hash_tree = true;
}

int Apriori::run(const int unsigned& min_support,
                  const float       & min_conf)
{
    this->_min_sup      = min_support;
    this->_min_conf     = min_conf;
    this->_rule_num     = 0;
    
    std::chrono::time_point<std::chrono::high_resolution_clock>
    clock_rule_gen;
    std::chrono::time_point<std::chrono::high_resolution_clock>
    clock_freq_gen;

    this->_trans_manager->ready(this->_min_sup, this->_min_conf);

    int cands = this->_collect();
    std::cout << "  Frequent 1-Itemset:" << cands << std::endl;

    int level = 1;

    while (cands > 1)
    {
        level++;
        
        // generate candidates
        std::cout << "Generating Candidates..." << std::endl;
        cands = this->_trans_manager->go();
        std::cout << "  " << level << "-Itemset Candidates: " << cands << std::endl;
        if (cands == 0) break;

        // count support
        std::cout << "Counting Support..." << std::endl;
        clock_freq_gen       = std::chrono::high_resolution_clock::now();
        this->_trans_manager->count();
        this->_time_freq_gen += std::chrono::high_resolution_clock::now() - clock_freq_gen;
        
        // collect frequent itemset and generate rules if necessary
        std::cout << "Generating Rules..." << std::endl;
        clock_rule_gen = std::chrono::high_resolution_clock::now();
        cands = this->_collect();
        std::cout << "  Frequent " << level << "-Itemset: " << cands << std::endl;
        this->_time_rule_gen += std::chrono::high_resolution_clock::now() - clock_rule_gen;
    }
    return this->_rule_num;
}

int Apriori::_collect()
{
    auto cand = this->_trans_manager->getCurrentItemsets();
    
    if (cand == nullptr) return 0;
    
    int success = 0;
    std::list<item::ItemID> temp;
    
    int itemset_size = cand->right->itemset.size();
    std::list<int> temp_itemset;
    while (cand->right != nullptr)
    {
        cand = cand->right;
        if (cand->count < this->_min_sup)
        {
            cand = this->_trans_manager->destoryItemNode(cand);
            continue;
        }
        else
        {
            temp_itemset = this->_trans_manager->solCode(cand->itemset);
            temp_itemset.sort();
            if (this->_min_conf < 0)
            {
                this->_addRule(temp_itemset, Itemset(), cand->count, -1);
            }
            else
            {
                this->_setFreq(temp_itemset, cand->count);
                if (itemset_size > 1)
                    this->_genRules(temp_itemset, cand->count);
            }
            success++;
        }
    }
    
    return success;
}

int Apriori::_genRules(const std::list<int>& itemset, const int & current_sup)
{
    std::list<std::list<int> > basis;
    std::list<int> diff;
    int     count       = 0;
    float   conf;
    int sub_sup;
    
    for (auto & item : itemset)
    {
        basis.push_back(std::list<int>({ item }));
    }
    while (!basis.empty())
    {
        for (auto it = basis.begin(); it != basis.end(); )
        {
            diff.clear();
            std::set_difference(itemset.begin(), itemset.end(),
                                it->begin(), it->end(), std::inserter(diff, diff.begin()));

            sub_sup = this->getFreq(diff);
            
            conf = (float)current_sup / sub_sup;
            
            if (conf < this->_min_conf)
            {
                it = basis.erase(it);
            }
            else
            {
                this->_addRule(diff, *it, current_sup, conf);
                count++;
                it++;
            }
        }
        if ((basis.size() < 2) ||
            (basis.back().size() == itemset.size() - 1)) break;
        basis = this->_genCands(basis);
    }
    return count;
}

// This func is only used when generating rules. Some key but useless parts are commented.
std::list<Itemset>Apriori::_genCands(const std::list<std::list<int> >& basis)
{
    std::list<std::list<int>> result;
//    int basis_size = basis.begin()->size();
//    
//    if (basis_size == 0) return result;
//    
    auto it_end = --basis.end();
    auto itt_end = basis.end();
//
//    if (basis_size == 1)
//    {
//        for (auto it = basis.begin(); it != it_end; it++)
//        {
//            for (auto itt = std::next(it, 1); itt != itt_end; itt++)
//            {
//                result.push_back(Itemset({ it->front(), itt->front() }));
//            }
//        }
//        return result;
//    }
    
    Itemset cand, temp_subset;
    // bool    success;
    // int     level = basis.front().size() + 1;
    
    for (auto it = basis.begin(); it != it_end; it++)
    {
        for (auto itt = std::next(it, 1); itt != itt_end; itt++)
        {
            if (std::equal(it->begin(), --(it->end()), itt->begin()))
            {
                //                cand.clear();
                cand = *it;
                cand.push_back(itt->back());
                
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
                //     if (this->_freq_table.find(hash_key)
                //         == this->_freq_table.end())
                //     {
                //         success = false;
                //         break;
                //     }
                // }
                
                
                // if (success == true)
                // {
                result.push_back(std::move(cand));
                // }
            }
            else break;
        }
    }
    
    return result;
}

void Apriori::_addRule(const std::list<int> & lhs, const std::list<int> & rhs, const int & support, const float & confidence)
{
    this->_rule_num++;
    if (this->_opstream == nullptr)
        this->rules.emplace_back(lhs, rhs, support, confidence);
    else
    {
        std::ostringstream lhss;
        std::ostringstream rhss;
        
        Itemset solved_lhs = lhs;//this->_trans_manager->solCode(lhs);
        Itemset solved_rhs = rhs;//this->_trans_manager->solCode(rhs);
        
        std::copy(solved_lhs.begin(), solved_lhs.end(),
                  std::ostream_iterator<int>(lhss, ","));
        std::copy(solved_rhs.begin(), solved_rhs.end(),
                  std::ostream_iterator<int>(rhss, ","));
        
        *(this->_opstream) << "{" << lhss.str().substr(0, lhss.str().size() - 1) << "}|" << "{" << rhss.str().substr(0, rhss.str().size() - 1) << "}|" << support << "|" << confidence << std::endl;
    }
}

std::list<Rule>Apriori::getRules()
{
    return this->rules;
}

void Apriori::_setFreq(const std::list<int>& itemset, const int& sup)
{
    this->_freq_table[itemset] = sup;
}

int Apriori::getFreq(const std::list<int>& itemset)
{
    return this->_freq_table[itemset];
}

double Apriori::getRuleGenTime()
{
    return this->_time_rule_gen.count();
}

double Apriori::getFreqGenTime()
{
    return this->_time_freq_gen.count();
}
