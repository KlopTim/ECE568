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
    string start_line;
    int headerSize;
public:
    Request(string start_line, unordered_map<string, string> head_map, vector<char> all_msg, int headerSize);
    string get_method();
    string get_startLine();
    string get_host();
    string get_port();
    int get_headerSize();
    int get_contentLen();
    vector<char> get_allMsg();
};

#endif  