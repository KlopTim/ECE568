#ifndef PARSER_H_H  
#define PARSER_H_H  
#include<stdlib.h>
#include<stdio.h>
#include<iostream>
#include<string>
#include<vector>
#include<unordered_map>
using namespace std;

class Parser{
    unordered_map<string,string>headers;
    string start_line;
public:
    Parser();
    void parsing(vector<char>all_msg);
    string mygetline(vector<char>all_msg,size_t& i);
    size_t findcolon(string str);
    string getstartline();
    unordered_map<string,string> getheaders();
};

#endif  