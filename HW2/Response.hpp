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
    string start_line;
    vector<char> all_msg;
    unordered_map<string, string> head_map;
    int headerSize;
public:
    Response(string start_line, unordered_map<string, string> head_map, vector<char> all_msg, int headerSize);
    //Response(Response& other);
    bool isValid();
    bool isCashable();
    bool isExpire();
    bool isChunk();
    int get_contentLen();
    int get_headerSize();
    void set_allMsg(vector<char> all_msg);
    string get_cacheInfo();
};


#endif  