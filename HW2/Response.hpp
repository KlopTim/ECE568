#ifndef RESPONSE_H_H  
#define RESPONSE_H_H  

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
using namespace std;

class Response {
private:
    vector<char> all_msg;
    unordered_map<string, string> head_map;
public:
    Response(string start_line, unordered_map<string, string> head_map, vector<char> all_msg);
    bool isValid();
    bool isCashable();
    bool isExpire();
};


#endif  