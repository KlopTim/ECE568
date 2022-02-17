#include"Parser.hpp"


Parser::Parser(){}

void Parser::parsing(vector<char>all_msg){
    size_t i = 0;
    start_line = mygetline(all_msg,i);
    while(i+1<all_msg.size()){
        if(all_msg[i]=='\r'&&all_msg[i+1]=='\n')break;
        size_t start = i;
        string curr_line = mygetline(all_msg,i);
        
        size_t pos = findcolon(curr_line);
        //cout<<curr_line<<endl;
        if(pos==-1)break;
        string key = curr_line.substr(0,pos);
        size_t value_start = pos + 1;
        size_t value_len = (i-2-start)-(pos+1);
        //size_t count = 0;
        while(curr_line[value_start]==' '){value_start++;value_len--;}
        string value = curr_line.substr(value_start,value_len);
        //cout<<key<<" : "<<key.size()<<endl;
        //cout<<value<<" : "<<value.size()<<endl;
        //cout<<value_len<<endl;
        headers[key]=value;
    }
}

string Parser::mygetline(vector<char>all_msg,size_t& i){
    vector<char> curr_line;
    while(all_msg[i]!='\r'){
        curr_line.push_back(all_msg[i]);
        i++;
    }
    //curr_line.push_back('\n');
    i+=2;
    return curr_line.data();
}


size_t Parser::findcolon(string str){
    size_t found = str.find(':');
    if(found!=std::string::npos){
        return found;
    }
    return -1;
}


string Parser::getstartline(){
    return start_line;
}

unordered_map<string,string> Parser::getheaders(){
    return headers;
}