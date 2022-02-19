#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <unordered_map>
#include <string>
#include <iostream>
using namespace std;


void fill_local_date(struct tm &t,string dt);
void fill_http_date(struct tm &t,string value);
// check expire
bool is_expired1(string value);
// check max age expire
bool is_expired2(int max_age,string res_date);
string get_expire_time(int max_age,string res_date);

