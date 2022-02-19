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
int cal_age(int begin, string cacheCtrl) {
    int ans = 0;
    for (int i = begin; i < cacheCtrl.size(); i++) {
        if (!isdigit(cacheCtrl[i])) break;
        ans *= 10;
        ans += (cacheCtrl[i] - '0');
    }
    return ans;
}


bool Response::isValid() {return false;}
bool Response::isCashable() {return false;}
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
vector<char> Response::get_allMsg() {
    return all_msg;
}

string Response::get_cacheInfo() {
    if (head_map.find("Cache-Control") == head_map.end()) return "";
    return head_map["Cache-Control"];
}

bool Response::has_noCache() {
    if (head_map.find("Cache-Control") == head_map.end()) return false;
    string head_value = head_map["Cache-Control"];
    if (head_value.find("no-cache") == string::npos) return false;
    return true;
}
bool Response::has_mustRevalidate() {
    if (head_map.find("Cache-Control") == head_map.end()) return false;
    string head_value = head_map["Cache-Control"];
    if (head_value.find("must-revalidate") == string::npos) return false;
    return true;
}
bool Response::isExpire() {
    // check max-age
    if (head_map.find("Cache-Control") != head_map.end()) {
        //!!!!!!!
        string cacheCtrl = head_map["Cache-Control"];
        if (cacheCtrl.find("max-age") != string::npos) {
            // get the number of max-age
            int begin = cacheCtrl.find("max-age") + 8;
            int max_age = cal_age(begin, cacheCtrl);
            
            // get the date
            string date = head_map["Date"];
            if (is_expired2(max_age, date)) {
                return true;
            }
        }
    }
    // check expire 
    if (head_map.find("Expires") == head_map.end()) return false;
    string time_val = head_map["Expires"];
    if (is_expired1(time_val)) {
        return true;
    }
    return false;
}

string Response::get_expireTime() {
    // check max-age
    if (head_map.find("Cache-Control") != head_map.end()) {
        //!!!!!!!
        string cacheCtrl = head_map["Cache-Control"];
        if (cacheCtrl.find("max-age") != string::npos) {
            // get the number of max-age
            int begin = cacheCtrl.find("max-age") + 8;
            int max_age = cal_age(begin, cacheCtrl);
            
            // get the date
            string date = head_map["Date"];
            if (is_expired2(max_age, date)) {
                return get_expire_time(max_age, date);
            }
        }
    }
    // check expire 
    if (head_map.find("Expires") == head_map.end()) return "";
    string time_val = head_map["Expires"];
    if (is_expired1(time_val)) {
        return time_val;
    }
    return "";
}
bool Response::has_eTag() {
    if (head_map.find("ETag") == head_map.end()) return false;
    return true;
}
bool Response::has_LastModified() {
    if (head_map.find("Last-Modified") == head_map.end()) return false;
    return true;
}

string Response::get_startLine() {
    return start_line;
}

string Response::get_eTag() {
    return head_map["ETag"];
}
string Response::get_LastModified() {
    return head_map["Last-Modified"];
}



