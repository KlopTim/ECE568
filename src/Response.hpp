#ifndef RESPONSE_H_H  
#define RESPONSE_H_H
#include "myTime.hpp"  
#include <ctype.h>
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
    bool isChunk();
    int get_contentLen();
    int get_headerSize();
    void set_allMsg(vector<char> all_msg);
    vector<char> get_allMsg();
    string get_cacheInfo();
    string get_startLine();

    bool has_noCache();
    bool has_mustRevalidate();
    bool isExpire();
    bool has_eTag();
    bool has_LastModified();
    string get_eTag();
    string get_LastModified();
    string get_expireTime();
};


#endif  