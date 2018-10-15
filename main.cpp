//
//  main.cpp
//  compact_tree
//
//  Created by fabio deo on 03/04/2018.
//  Copyright © 2018 fabio deo. All rights reserved.
//


#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>

struct bdnode{
    std::string val;
    bdnode* left;
    bdnode* right;
    
};

typedef bdnode* bdt;

bdt buildcompactbdt(const std::vector<std::string>& fvalues);
std::string evalcompactbdt(bdt t, const std::string& input);
void build_tree_string_structure(std::vector<std::string>& tree_layout, int height_of_x, int depth_of_x);
void setting_ones(std::vector<std::string>& tree, const std::string& input);
bool spot_useless_nodes(std::vector<std::string>& tree_layout, int index, int input_length);
bdt generate_compact_tree(std::vector<std::string>& tree_layout, int index, int depth, int input_length);



/// FOR TESTING ONLY ----------------------------------------------------

#include <cstdlib>
#include <ctime>
#include <cmath>
#include <unordered_set>
#include <unistd.h>

void process_mem_usage(double& vm_usage, double& resident_set)
{
    vm_usage     = 0.0;
    resident_set = 0.0;
    
    // the two fields we want
    unsigned long vsize;
    long rss;
    {
        std::string ignore;
        std::ifstream ifs("/proc/self/stat", std::ios_base::in);
        ifs >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore
        >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore
        >> ignore >> ignore >> vsize >> rss;
    }
    
    long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
    vm_usage = vsize / 1024.0;
    resident_set = rss * page_size_kb;
}
bool is_leaf(bdt node){
    return (node->val.length() == 1); // only leafs has strings of length 1
}

void generate_combinations(std::string current, int len, std::vector<std::string>& combinations){
    if(len == 0){
        combinations.push_back(current);
        return;
    }
    generate_combinations(current + "0", len - 1, combinations);
    generate_combinations(current + "1", len - 1, combinations);
}

void test_all(const std::vector<std::string>& valid, bdt tree){
    std::unordered_set<std::string> ones(valid.begin(), valid.end());
    std::vector<std::string> combinations;
    generate_combinations("", valid[0].length(), combinations);
    for(std::string combination: combinations){
        std::string res = evalcompactbdt(tree, combination);
        std::cout << combination << ": " << res << "  ";
        if((res == "1" && ones.find(combination) == ones.end()) ||
           (res == "0" && ones.find(combination) != ones.end())){
            std::cout << "ERROR ###############";
        }
        std::cout << std::endl;
    }
}

int max(int a, int b){
    if(a > b)
        return a;
    return b;
}

int tree_height(bdt tree){
    if(is_leaf(tree))
        return 0;
    return max(tree_height(tree->left), tree_height(tree->right)) +1;
}

int count_nodes(bdt tree){
    if(is_leaf(tree))
        return 1;
    return 1 + count_nodes(tree->left) + count_nodes(tree->right);
}

void print_tree(bdt tree, std::string path){
    if(is_leaf(tree)){
        std::cout << path << ": " << tree->val << std::endl;
        return;
    }
    print_tree(tree->left, path + "L ");
    std::cout << path << ": " << tree->val << std::endl;
    print_tree(tree->right, path + "R ");
}

void create_rand_input(std::vector<std::string>& input, int len, bool print){
    int num_of_ones = rand() % int(pow(len, 2) - 1) + 1;
    for(int combination = 0; combination < num_of_ones; combination++){
        std::string curr = "";
        for(int i = 0; i < len; i++){
            curr.push_back(char( rand() % 2 ) + '0');
        }
        if(print)
            std::cout << curr << std::endl;
        input.push_back(curr);
    }
}

int main(){
    srand(time(NULL));
    int num_of_tests = 1;
    
    for(int len = 3; len < 4; len++){
        int tot_nodes = 0;
        int tot_height = 0;
        double time_elapsed = 0;
        for(int test = 0; test < num_of_tests; test++){
            std::vector<std::string> input;
            create_rand_input(input, len, false);
            
            std::clock_t start;
            start = std::clock();
            
            bdt tree = buildcompactbdt(input);
            
            time_elapsed += (std::clock() - start ) / (double) CLOCKS_PER_SEC;
            
            tot_nodes += count_nodes(tree);
            tot_height += tree_height(tree);
            
            print_tree(tree, "");
            test_all(input, tree);
            std::cout << "\n==========================================\n";

        }
        
        int def_nodes = (int)(2 * pow(2, len) - 1);
        int avg_nodes = tot_nodes / num_of_tests;
        double avg_time = time_elapsed / (double)num_of_tests;
        
        std::cout << "LEN: " << len;
        std::cout << "\nAvg heigth: " << tot_height / num_of_tests << "\nNodes count: " << avg_nodes;
        std::cout << "\nNormal tree nodes: " << def_nodes;
        std::cout << "\nReduced size is: " << avg_nodes * 100 / def_nodes << "% of the original";
        std::cout << "\nAvg time to build the tree: " << avg_time << std::endl << std::endl;
        using std::cout;
        using std::endl;
        
        double vm, rss;
        process_mem_usage(vm, rss);
        cout << "VM: " << vm << "; RSS: " << rss << endl;
    }
    return 0;
}

/// END OF TESTING ------------------------------------------------------


//build the tree data structure
bdt buildcompactbdt(const std::vector<std::string>& fvalues){
    int height = fvalues[0].length() - 1;
    int depth = 0;
    
    //generate the tree_layout in a vector of strings
    std::vector<std::string> tree_layout;
    build_tree_string_structure(tree_layout, height, depth);
    tree_layout.push_back("0");
    
    //for each input sequence set the 1 in the right leaf
    for (int i = 0; i < fvalues.size(); i++){
        std::string input = fvalues[i];
        setting_ones(tree_layout, input);
    }
    
    //generate the compact tree from the string
    int index = tree_layout.size()/2;
    bdt hd_tree = generate_compact_tree(tree_layout, index, 0, height + 1);
    
    return hd_tree;
}

//return from a input the corresponding value by traversing the tree
std::string evalcompactbdt(bdt t, const std::string& input){
    int height = input.length();
    
    while (t->val.length() > 1){
        int depth = std::stoi(t->val.substr(1), nullptr);
        
        if(input[depth - 1] == '0')
            t = t->left;
        else
            t = t->right;
    }

    return t->val;
}

//build the string rapresentation of the tree
void build_tree_string_structure(std::vector<std::string>& tree_layout, int height_of_x, int depth_of_x){

    //while it does not reach the leaves push back in the vector the values
    if(height_of_x != -1){
        build_tree_string_structure(tree_layout, height_of_x - 1, depth_of_x + 1);
        tree_layout.push_back("0");
        tree_layout.push_back("x" + std::to_string(depth_of_x + 1));
        build_tree_string_structure(tree_layout, height_of_x - 1, depth_of_x + 1);
    }
    
 }

//set leaves corresponding to inputs to 1
void setting_ones(std::vector<std::string>& tree, const std::string& input){
    int index = pow(2, input.size()) - 1;
    
    //given the information in the input change the index of the node to overwrite
    
    for(int i = 0; i < input.size() - 1; i++){
        int offset = pow(2, input.size() - i - 1);
        
        if (input[i] == '0')
            index = index - offset;
        else
            index = index + offset;
    }
    
    //...and overwrite a 1 in the leaf
    if(input[input.size() - 1] == '1')
        index++;
    else
        index--;
    
    tree[index] = "1";
}

//spot the nodes that can be erased in the compact version
bool spot_useless_nodes(std::vector<std::string>& tree_layout, int index, int input_length){

    int depth = std::stoi(tree_layout[index].substr(1), nullptr) - 1;
    int offset = pow(2, input_length - depth) - 1;
    for(int j = index - offset; j < index; j++){
        if(tree_layout[j] != tree_layout[j + offset + 1])
            return false;
    }
    return true;
}

//generate the tree from the structure stored in the string
bdt generate_compact_tree(std::vector<std::string>& tree_layout, int index, int depth, int input_length){

    //if leaf end recursive calls (BASE CASE)
    if (tree_layout[index].length() == 1){
        bdt leaf = new bdnode;
        leaf->val = tree_layout[index];
        leaf->left = NULL;
        leaf->right = NULL;
        return leaf;
    }
    
    bdt root = new bdnode;
    int offset = pow(2, input_length - depth - 1);
    
    //if node is useless just left subtree has to be considered and no node has to be created on this level
    if(spot_useless_nodes(tree_layout, index, input_length)){
        index = index - offset;
        root = generate_compact_tree(tree_layout, index, depth + 1, input_length);
    }
    
    //otherwise, generate the node and call the function again for left and right subtrees
    else{
        root->val = tree_layout[index];
        
        //left subtree recursive call
        index = index - offset;
        root->left = generate_compact_tree(tree_layout, index, depth + 1, input_length);
        
        //right subtree recursive call
        index = index + 2 * offset;
        root->right = generate_compact_tree(tree_layout, index, depth + 1, input_length);
    }

    return root;
}




