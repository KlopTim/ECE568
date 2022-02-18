#include "Response.hpp"


Response::Response(string start_line, unordered_map<string, string> head_map, vector<char> all_msg, int headerSize) {
    this->start_line = start_line;
    this->head_map = head_map;
    this->all_msg = all_msg;
    this->headerSize = headerSize;
}

/*
Response::Response(Response& other) {
    this->head_map = other.head_map;
    this->all_msg = other.all_msg;
    this->headerSize = other.headerSize;
}
*/

bool Response::isValid() {return false;}
bool Response::isCashable() {return false;}
bool Response::isExpire() {return false;}
bool Response::isChunk() {
    if (head_map.find("Transfer-Encoding") != head_map.end()) {
        if (head_map["Transfer-Encoding"] == "chunked")
            return true;
    }
    return false;
}
int Response::get_contentLen() {
    if (head_map.find("Content-Length") == head_map.end()) return -1;
    string len = head_map["Content-Length"];
    return stoi(len);
}

int Response::get_headerSize() {
    return headerSize;
}

void Response::set_allMsg(vector<char> all_msg) {
    this->all_msg = all_msg;
}

string Response::get_cacheInfo() {
    if (head_map.find("Cache-Control") == head_map.end()) return "";
    return head_map["Cache-Control"];
}