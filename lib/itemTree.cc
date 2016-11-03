#include "itemTree.h"

#include <iostream>

using namespace item;

ItemTree::ItemTree(const int & min_sup, std::vector<Item *> * item_table)
{
    if (item_table->size() < 1) return;
    
    this->_min_sup = min_sup;
    this->_hash_tree = false;
    Node *root = new Node();
    Node *new_node;
    Node *last_node = new Node(root, item_table->back()->count, item_table->back());
    int i = item_table->size() - 1;
    
    root->children[i] = last_node;
    
    for (; --i > -1;)
    {
        new_node = new Node(root, (*item_table)[i]->count, (*item_table)[i]);
        last_node->left = new_node;
        new_node->right = last_node;
        root->children[i] = new_node;
        last_node         = std::move(new_node);
    }
    this->_header_nodes.push_back(std::move(root));
    new_node = new Node();
    last_node->left = new_node;
    new_node->right = std::move(last_node);
    this->_header_nodes.push_back(std::move(new_node));
    this->_level = 1;
    this->_item_table = item_table;
    this->_cheated = false;
}

ItemTree::~ItemTree()
{
    for (auto & c: this->_header_nodes)
    {
        delete c;
    }
}

int ItemTree::grow()
{
//    if (this->_cheated == true)
//    {
//        if (this->_level == 1)
//            return this->_grow22Compact();
//        return this->_growCompact();
//    }
    
    if (this->_level == 1)
        return this->_grow22();
    return this->_grow();
    
}

int ItemTree::_grow22()
{
    
    this->_level=2;

    int result = 0;
    auto itn_end = this->_header_nodes[0]->children.end();
    auto it_end = std::next(itn_end, -1);
    std::map<int, Node*>::iterator itn;
    Node * new_node;
    Node * last_node = new Node();
    this->_header_nodes.push_back(last_node);
    
    for (auto it = this->_header_nodes[0]->children.begin(); it != it_end; it++)
    {
        for (itn = std::next(it, 1); itn != itn_end; itn++)
        {
                new_node = new Node(it->second, 0, itn->second->item_instance);
                it->second->children[itn->first] = new_node;
                new_node->left = last_node;
                last_node->right = new_node;
                last_node = std::move(new_node);
                result++;
        }
    }
    return result;
}


int ItemTree::_grow()
{
    int result = 0;
    
    std::map<int, Node *>::iterator it;
    std::map<int, Node *>::iterator it_end;
    std::map<int, Node *>::iterator itn;
    std::map<int, Node *>::iterator itn_end;
    
    Node *grandpa = this->_header_nodes[this->_level-1];
    Node *new_node;
    Node *last_node = new Node();
    this->_header_nodes.push_back(last_node);
    this->_level++;
    
    while (grandpa->right != nullptr)
    {
        grandpa = grandpa->right;
        
        // Remove infrequent itemsets
        // Infrequent itemsets are removed when apriori collects itemsets
//        for (it = grandpa->children.begin(); it != grandpa->children.end();)
//        {
//            if (it->second->count < this->_min_sup)
//            {
//                delete it->second;
//                it = grandpa->children.erase(it);
//            }
//            else
//            {
//                it++;
//            }
//        }
        
        if (grandpa->children.size() < 2)
        {
            if (this->_hash_tree == false)
            {
            // When use hash tree, there is no need to clear the branch, since in that case the item
            // tree is just used to generate candidates and there is no performance influence if the
            // redunduant branch is not pruned.
            
            // Clear the branch if current node has insufficient children to generate new itemset.
            // Go upstair until an ancedent with more than one children is found
            // In the last case, only the root node will be kept
            
                new_node = grandpa;
                grandpa = grandpa->left;
                while(new_node->parent->children.size() < 2 && new_node->parent->parent != nullptr)
                {
                    new_node = new_node->parent;
                }
                new_node->parent->children.erase(new_node->item);
                delete new_node;
            }
            continue;
        }
        
        // Combine each child node with those after it
        it_end  = --grandpa->children.end();
        itn_end = grandpa->children.end();
        for (it = grandpa->children.begin(); it != it_end; it++)
        {
            for (itn = std::next(it, 1); itn != itn_end; itn++)
            {
                new_node                         = new Node(it->second,
                                                                0, itn->second->item_instance);
                    it->second->children[itn->first] = new_node;
                    last_node->right                 = new_node;
                    new_node->left = last_node;
                    last_node                        = std::move(new_node);
                    result++;
            }
        }
    }
    return result;
}

//int ItemTree::_grow22Compact()
//{
//    int result = 0;
//    auto itn_end = this->_header_nodes[0]->children.end();
//    auto it_end = std::next(itn_end, -1);
//    std::map<int, Node*>::iterator itn;
//    Node * new_node;
//    Node * last_node = new Node();
//    this->_header_nodes.push_back(last_node);
//    
//    std::list<int> inter;
//    std::list<int>::iterator inter_it, inter_it_end;
//    for (auto it = this->_header_nodes[0]->children.begin(); it != it_end; it++)
//    {
//        inter_it = it->second->item_instance->trans.begin();
//        inter_it_end = it->second->item_instance->trans.end();
//        for (itn = std::next(it, 1); itn != itn_end; itn++)
//        {
//            inter.clear();
//            std::set_intersection(inter_it, inter_it_end,
//                                  itn->second->item_instance->trans.begin(),
//                                  itn->second->item_instance->trans.end(),
//                                  std::inserter(inter, inter.begin()));
//            if (inter.size()>=this->_min_sup)
//            {
//                new_node = new Node(it->second, 0, itn->second->item_instance, std::move(inter));
//                it->second->children[itn->first] = new_node;
//                new_node->left = last_node;
//                last_node->right = new_node;
//                last_node = std::move(new_node);
//                result++;
//            }
//        }
//    }
//    this->_level=2;
//    return result;
//}
//
//int ItemTree::_growCompact()
//{
//    int result = 0;
//    
//    std::map<int, Node *>::iterator it;
//    std::map<int, Node *>::iterator it_end;
//    std::map<int, Node *>::iterator itn;
//    std::map<int, Node *>::iterator itn_end;
//    
//    Node *grandpa = this->_header_nodes[this->_level-1];
//    Node *new_node;
//    Node *last_node = new Node();
//    this->_header_nodes.push_back(last_node);
//    this->_level++;
//    
//    while (grandpa->right != nullptr)
//    {
//        grandpa = grandpa->right;
//        if (grandpa->children.size() < 2)
//            continue;
//        std::list<int> inter;
//        std::list<int>::iterator inter_it, inter_it_end;
//        it_end  = --grandpa->children.end();
//        itn_end = grandpa->children.end();
//        for (it = grandpa->children.begin(); it != it_end; it++)
//        {
//            inter_it = it->second->trans.begin();
//            inter_it_end = it->second->trans.end();
//            for (itn = std::next(it, 1); itn != itn_end; itn++)
//            {
//                inter.clear();
//                std::set_intersection(inter_it, inter_it_end,
//                                      itn->second->trans.begin(),
//                                      itn->second->trans.end(),
//                                      std::inserter(inter, inter.begin()));
//                if (inter.size() >= this->_min_sup)
//                {
//                    new_node                         = new Node(it->second,
//                                                                0, itn->second->item_instance, std::move(inter));
//                    it->second->children[itn->first] = new_node;
//                    last_node->right                 = new_node;
//                    new_node->left = last_node;
//                    last_node                        = std::move(new_node);
//                    result++;
//                }
//            }
//        }
//    }
//    
//    return result;
//}

Node * ItemTree::destoryItemNode(item::Node *trash)
{
    Node * last_node = std::move(trash->left);
    trash->parent->children.erase(trash->item);
    delete trash;
    return last_node;
}

int ItemTree::getCurrentLevel()
{
    return this->_level;
}

int ItemTree::getMinSup()
{
    return this->_min_sup;
}

Node* ItemTree::getCurrentItemsets()
{
    return this->_header_nodes.back();
}

Node * ItemTree::getRoot()
{
    return this->_header_nodes[0];
}

void ItemTree::compactMode(const bool & open)
{
    this->_cheated = open;
}

void ItemTree::useHashTree( const bool &open)
{
    this->_hash_tree = true;
}
