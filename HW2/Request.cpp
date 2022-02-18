#include "Request.hpp"

string find_method(string start_line) {
    int len = start_line.find(' ');
    return start_line.substr(0, len);
}
string find_host(string hostname) {
    size_t end = hostname.find(":");
    return end == string::npos ? hostname : hostname.substr(0, end);
}
string find_port(string hostname) {
    size_t begin = hostname.find(":");
    return begin == string::npos ? "" : hostname.substr(begin + 1, hostname.size() - begin - 1);
}



Request::Request(string start_line, unordered_map<string, string> head_map, vector<char> all_msg, int headerSize) {
    this->head_map = head_map;
    this->all_msg = all_msg;
    this->method = find_method(start_line);
    this->headerSize = headerSize;
    this->start_line = start_line;
    if (head_map.find("Host") != head_map.end()) {
        string hostname = head_map["Host"];
        this->host = find_host(hostname);
        this->port = find_port(hostname);
    }
    else {
        this->host = "NULL";
        this->port = "NULL";
    }
}

string Request::get_method() {return method;}
string Request::get_host() {return host;}
string Request::get_port() {return port;}


int Request::get_headerSize() {
    return headerSize;
}

int Request::get_contentLen() {
    if (head_map.find("Content-Length") == head_map.end()) return -1;
    string len = head_map["Content-Length"];
    return stoi(len);
}

string Request::get_startLine() {
    return start_line;
}