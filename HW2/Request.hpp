#ifndef REQUEST_H_H  
#define REQUEST_H_H  

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
using namespace std;

class Request {
private:
    vector<char> all_msg;
    unordered_map<string, string> head_map;
    string method;
    string host;
    string port;
public:
    Request(string start_line, unordered_map<string, string> head_map, vector<char> all_msg);
    string get_method();
    string get_host();
    string get_port();
};

#endif  