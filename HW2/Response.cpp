#include "Response.hpp"


Response::Response(string start_line, unordered_map<string, string> head_map, vector<char> all_msg) {
    this->head_map = head_map;
    this->all_msg = all_msg;
}
bool Response::isValid() {return false;}
bool Response::isCashable() {return false;}
bool Response::isExpire() {return false;}