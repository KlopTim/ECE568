#include "myTime.hpp"
using namespace std;

unordered_map<string,int> mymap = {
    {"Jan", 1},
    {"Feb", 2},
    {"Mar", 3},
    {"Apr", 4},
    {"May", 5},
    {"Jun", 6},
    {"Jul", 7},
    {"Aug", 8},
    {"Sep", 9},
    {"Oct", 10},
    {"Nov", 11},
    {"Dec", 12},
};


void fill_local_date(struct tm &t,string dt){
    string week = dt.substr(0,3);
    string mon = dt.substr(4,3);
    string date = dt.substr(8,2);
    string hour = dt.substr(11,2);
    string minute = dt.substr(14,2);
    string second = dt.substr(17,2);
    string year = dt.substr(20,4);

    t.tm_year = atoi(year.c_str())-1900;
    t.tm_mon = mymap[mon]-1;
    t.tm_mday = atoi(date.c_str());
    t.tm_hour = atoi(hour.c_str());
    t.tm_min = atoi(minute.c_str());
    t.tm_sec = atoi(second.c_str());
}

void fill_http_date(struct tm &t,string value){
    t.tm_year = atoi((value.substr(12,4)).c_str())-1900;
    t.tm_mon = mymap[value.substr(8,3)]-1;
    t.tm_mday = atoi((value.substr(5,2)).c_str());
    t.tm_hour = atoi((value.substr(17,2)).c_str());
    t.tm_min = atoi((value.substr(20,2)).c_str());
    t.tm_sec = atoi((value.substr(23,2)).c_str());
}

//已知Expires: Sat, 05 Mar 2022 16:19:40 GMT 中的 Sat, 05 Mar 2022 16:19:40 GMT
bool is_expired1(string value){
  time_t now = time(0);
  string dt = ctime(&now);
  tm *gmtm = gmtime(&now);
  dt = asctime(gmtm);

  struct tm t1 = {0};
  fill_local_date(t1,dt);

  struct tm t2 = {0};
  fill_http_date(t2,value);

  int seconds = difftime(mktime(&t2), mktime(&t1));//t2-t1
  return seconds<0;
}

//已知 Cache-Control: max-age=1209600 中的 int max_age
bool is_expired2(int max_age,string res_date){
  time_t now = time(0);
  string dt = ctime(&now);
  tm *gmtm = gmtime(&now);
  dt = asctime(gmtm);
 
  struct tm t1 = {0};
  fill_local_date(t1,dt);

  struct tm t2 = {0};
  fill_http_date(t2,res_date);

  int seconds = difftime(mktime(&t1), mktime(&t2));//t1-t2
  return seconds>max_age;
}


//get string expiretime
string get_expire_time(int max_age, string res_date){
  struct tm t={0};
  fill_http_date(t,res_date);
  time_t ans = mktime(&t)+max_age;
  string myans = ctime(&ans);/* 转换成 时间格式（UTC），给打印ID： cache expire at XXX 使用*/
  return myans;
}