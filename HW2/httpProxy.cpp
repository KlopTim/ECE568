#include "httpProxy.hpp"

unordered_map<string, Response> cache_map;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_rwlock_t rw_mutex = PTHREAD_RWLOCK_INITIALIZER;
ofstream outfile("var/log/erss/proxy.log");
void build_proxy() {
  const char * hostname = NULL;
  const char * port = "12345";
  /* become a server */
  int socket_fd = create_service(hostname, port);
  /* loop forever */
  int client_id = 0;
  while (1) {
    /* accept connection from client */
    int client_fd = accept_connect(socket_fd);
    client client_msg;
    client_msg.client_id = client_id;
    client_msg.client_fd = client_fd;
    pthread_t thread;
    pthread_create(&thread, NULL, communicate, &client_msg);
    client_id++;
  }
}


void * communicate(void * msgs) {
  client * client_msg = (client *)msgs;
  int client_connection_fd = client_msg->client_fd;
  int client_id = client_msg->client_id;
  /* recv request */
  vector<char> all_msg;
  char msg[65535];
  int len = recv(client_connection_fd, msg, 65535, 0);
  if (len <= 0) {
    return NULL;
  }

  for (int i = 0; i < 65535; i++) all_msg.push_back(msg[i]);
  //printf("%s", msg);

  /* parsing the request */
  Parser req_par;
  req_par.parsing(all_msg);

  /* generate the http request pocket */
  Request req(req_par.getstartline(), req_par.getheaders(), all_msg, req_par.get_headerSize());
  if (req.get_method() != "CONNECT" && req.get_method() != "GET" && req.get_method() != "POST") {
    return NULL;
  }


  /* get the hostname and port name */
  string target_host = req.get_host();
  string target_port = req.get_port();
  if (target_port == "") {
    target_port = "80";
  }


  pthread_mutex_lock(&mutex);
  time_t now = time(0);
  char* dt = ctime(&now);
  outfile << client_id <<":  \"" << req_par.getstartline() << "\" from "<< target_host;
  outfile << " @ " <<  dt << endl;
  pthread_mutex_unlock(&mutex);


  /* connect to the target server */
  pthread_mutex_lock(&mutex);
  int toTarget_fd = connect_to(target_host.c_str(), target_port.c_str());
  pthread_mutex_unlock(&mutex);
  if (toTarget_fd == -1) {
    return  NULL;
  }

  pthread_mutex_lock(&mutex);
  outfile << client_id <<": Requesting \""<< req.get_startLine() << "\" from " << target_host << endl;
  pthread_mutex_unlock(&mutex);
  /* check the Method */
  string method = req.get_method();
  if (method == "CONNECT") {
    mode_connect(client_connection_fd, client_id, toTarget_fd);
    pthread_mutex_lock(&mutex);
    outfile << client_id <<": Tunnel closed\n";
    pthread_mutex_unlock(&mutex);
  } else if (method == "POST") {
    mode_post(client_connection_fd, client_id, toTarget_fd, msg, len, req);
  } else if (method == "GET") {
    mode_get(client_connection_fd, client_id, toTarget_fd, msg, len, req, req.get_startLine());
  }

  close(client_connection_fd);
  close(toTarget_fd);
  return NULL;
}



void mode_connect(int client_fd, int client_id, int server_fd) {
    send(client_fd, "HTTP/1.1 200 OK\r\n\r\n", 19, 0);
    pthread_mutex_lock(&mutex);
    outfile << client_id <<": Responding \"HTTP/1.1 200 OK\"\n";
    pthread_mutex_unlock(&mutex);

    fd_set main_set;
    fd_set read_fds;
    FD_ZERO(&main_set); 
    FD_ZERO(&read_fds);
    FD_SET(client_fd, &main_set);
    FD_SET(server_fd, &main_set);
    int fdmax = client_fd > server_fd ? client_fd : server_fd;
    while (1) {
        read_fds = main_set;
        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
            return;
        }
        if (FD_ISSET(client_fd, &read_fds)) {
            char msg[65535];
            int len = recv(client_fd, &msg, 65535, 0);
            if (len <= 0) {
              return;
            }
            send(server_fd, &msg, len, 0);
        }
        else if (FD_ISSET(server_fd, &read_fds)){
            char msg[65535];
            int len = recv(server_fd, &msg, 65535, 0);
            if (len <= 0) {
              pthread_mutex_lock(&mutex);
              outfile << client_id <<": no longer receive\n\n";
              pthread_mutex_unlock(&mutex);
              return;
            }
            send(client_fd, &msg, len, 0);
        }
    }
}

void mode_get(int client_fd, int client_id, int server_fd, char * msg, int len, Request & req, string startLine) {
  /* check cache */
  if (try_used_cache(client_fd, client_id, server_fd, msg, len, req, startLine)) return;
  /* send the request from the client to the target server */
  send(server_fd, msg, len, 0);
  /* get "part of" Response from the target server */
  char server_msg[65536] = {0};
  int mes_len = recv(server_fd, server_msg, sizeof(server_msg), 0);
  if (mes_len <= 0) {
    return;
  }
  /* parsing response */
  vector<char> res_msg(server_msg, server_msg + mes_len);
  Parser res_par;
  res_par.parsing(res_msg);
  /* transfer to Resposnse */
  Response res(res_par.getstartline(), res_par.getheaders(), res_msg, res_par.get_headerSize());
  pthread_mutex_lock(&mutex);
  outfile << client_id <<": Received \""<< res.get_startLine() << "\" from " << req.get_host() << endl;
  pthread_mutex_unlock(&mutex);
  /* check if chunk */
  if (res.isChunk()) {
    /* chunk */
    pthread_mutex_lock(&mutex);
    outfile << client_id <<": not cacheable because response is CHUNK\n";
    pthread_mutex_unlock(&mutex);
    // easy: send message back and forward
    do {
      send(client_fd, server_msg, mes_len, 0);
      mes_len = recv(server_fd, server_msg, sizeof(server_msg), 0);
      if (mes_len <= 0) {return;}
    } while(1);
  }
  else {
    /* non-chunk */
    // get content len
    int content_len = res.get_contentLen();
    if (content_len == -1) {
      send(client_fd, server_msg, mes_len, 0);
      pthread_mutex_lock(&mutex);
      outfile << client_id <<": Responding \""<< res.get_startLine() << "\""<< endl;
      pthread_mutex_unlock(&mutex);
      return;
    }
    // keep receive all response
    do{
      if (res_msg.size() >= content_len + res.get_headerSize()) break;
      mes_len = recv(server_fd, server_msg, sizeof(server_msg), 0);
      if (mes_len <= 0) break;
      vector<char> temp_msg(server_msg, server_msg + mes_len);
      res_msg.insert(res_msg.end(), temp_msg.begin(), temp_msg.end());
    } while(1);
    /* send response to client*/
    send(client_fd, res_msg.data(), res_msg.size(), 0);
    /* try cache */
    res.set_allMsg(res_msg);
    try_cache(res, startLine, client_id);
  }
  pthread_mutex_lock(&mutex);
  outfile << client_id <<": Responding \""<< res.get_startLine() << "\""<< endl;
  pthread_mutex_unlock(&mutex);
  return;
}


void mode_post(int client_fd, int client_id, int server_fd, char * msg, int len, Request & req) {
  // POST should receive all message first
  /* send request to server */
  send(server_fd, msg, len, 0);
  /* receive response from server */
  char server_msg[65535];
  int res_len = recv(server_fd, &server_msg, 65535, 0);
  if (res_len <= 0) {
    pthread_mutex_lock(&mutex);
    outfile << client_id <<": Post Response fail\n";
    pthread_mutex_unlock(&mutex);
  }
  vector<char> res_msg(server_msg, server_msg + res_len);
  Parser res_par;
  res_par.parsing(res_msg);
  Response res(res_par.getstartline(), res_par.getheaders(), res_msg, res_par.get_headerSize());
  pthread_mutex_lock(&mutex);
  outfile << client_id <<": Received \""<< res.get_startLine() << "\" from " << req.get_host() << endl;
  pthread_mutex_unlock(&mutex);
  send(client_fd, res_msg.data(), res_len, 0);
  pthread_mutex_lock(&mutex);
  outfile << client_id <<": Responding \""<< res.get_startLine() << "\""<< endl;
  pthread_mutex_unlock(&mutex);
}

void try_cache(Response & res, string startLine, int client_id) {
  // check no store
  string cacheInfo = res.get_cacheInfo();
  if (cacheInfo.find("no-store") != string::npos) {
    pthread_mutex_lock(&mutex);
    cout << client_id <<": no cacheable because no-store\n";
    pthread_mutex_unlock(&mutex);
    return;
  }
  // add write lock here
  pthread_rwlock_wrlock(&rw_mutex);
  Response temp = res;
  cache_map.insert(pair<string,Response>(startLine,temp));
  // erase cache
  if (cache_map.size() > 20) {
    cache_map.erase(cache_map.begin());
  }
  pthread_rwlock_unlock(&rw_mutex);
  string exprTime = res.get_expireTime();
  bool revalid = res.has_noCache() || res.has_mustRevalidate();
  if (revalid) {
    pthread_mutex_lock(&mutex);
    outfile << client_id <<": cached, but requires re-validation\n";
    pthread_mutex_unlock(&mutex);
  }
  else {
    if (exprTime != "") {
      pthread_mutex_lock(&mutex);
      outfile << client_id <<": cached, expires at " << exprTime << endl;
      pthread_mutex_unlock(&mutex);
    }
    else {
      pthread_mutex_lock(&mutex);
      outfile << client_id <<": cached" << endl;
      pthread_mutex_unlock(&mutex);
    }
  }
}

bool try_used_cache(int client_fd, int client_id, int server_fd, char * msg, int len, Request & req, string startLine) {
  /* check if there is a cache avaible */
  pthread_rwlock_rdlock(&rw_mutex);
  if (cache_map.find(req.get_startLine()) == cache_map.end()) {
    pthread_rwlock_unlock(&rw_mutex);
    pthread_mutex_lock(&mutex);
    outfile << client_id <<": not in cache\n";
    pthread_mutex_unlock(&mutex);
    return false;
  }
  // available cache found
  Response res = (cache_map.find(req.get_startLine()))->second;
  pthread_rwlock_unlock(&rw_mutex);
  /* revalidate check */
  if (res.has_noCache() || (res.has_mustRevalidate() && res.isExpire())) {
    // check e_tag and Last-Modified
    if (!res.has_eTag() && !res.has_LastModified()) {
      // the response can be used
      pthread_mutex_lock(&mutex);
      outfile << client_id <<": in cache, valid\n";
      pthread_mutex_unlock(&mutex);
      vector<char> all_msg = res.get_allMsg();
      send(client_fd, all_msg.data(), all_msg.size(), 0);
      return true;
    } 
    else {
      // add If-None-Match or If-Modified-Since
      pthread_mutex_lock(&mutex);
      outfile << client_id <<": in cache, requires validation\n";
      pthread_mutex_unlock(&mutex);
      string extra;
      if (res.has_eTag()) {
        extra = "If-None-Match: " + res.get_eTag(); 
      }
      else {
        extra = "If-Modified-Since: " + res.get_LastModified();
      }
      extra += "\r\n";
      vector<char> vec_extra(extra.begin(),extra.end());
      vector<char> req_msg = req.get_allMsg();
      req_msg.insert(req_msg.begin()+req.get_headerSize()-2, vec_extra.begin(), vec_extra.end());
      // send modified Request to Server
      send(server_fd, req_msg.data(), req_msg.size(), 0);
      // Receive (all) Response from Server
      char server_msg[65535];
      vector<char> new_res_msg;
      do{
        int mes_len = recv(server_fd, server_msg, sizeof(server_msg), 0);
        if (mes_len <= 0) break;
        vector<char> temp_msg(server_msg, server_msg + mes_len);
        new_res_msg.insert(new_res_msg.end(), temp_msg.begin(), temp_msg.end());
      } while(1);
      // parsing response
      Parser res_par;
      res_par.parsing(new_res_msg);
      Response new_res(res_par.getstartline(), res_par.getheaders(), new_res_msg, res_par.get_headerSize());
      // check status 304 or 200
      string status_line = new_res.get_startLine();
      // if 304 -> cache can be used
      if (status_line.find("304") != string::npos) {
        vector<char> all_msg = res.get_allMsg();
        send(client_fd, all_msg.data(), all_msg.size(), 0);
      }
      // if 200 -> return new Response to user
      else {
        // erase the expiration cache
        pthread_rwlock_wrlock(&rw_mutex);
        cache_map.erase(req.get_startLine());
        pthread_rwlock_unlock(&rw_mutex);
        send(client_fd, new_res_msg.data(), new_res_msg.size(), 0);
        try_cache(new_res, status_line, client_id);
      }
      return true;
    }
  }
  else {
    // check age and expire
    /* expire time check */
    if (res.isExpire()) {
      // erase the expiration cache
      pthread_rwlock_wrlock(&rw_mutex);
      cache_map.erase(req.get_startLine());
      pthread_rwlock_unlock(&rw_mutex);
      // Ask for a new Response
      string exprTime = res.get_expireTime();
      pthread_mutex_lock(&mutex);
      //cout << client_id << ": EXPIRE!\n";
      //cout << client_id << exprTime << endl;
      outfile << client_id << ": in cache, but expired at " << exprTime << endl;
      pthread_mutex_unlock(&mutex);
      return false;
    }
    else {
      // return the current cache to the client
      pthread_mutex_lock(&mutex);
      // cout << client_id <<": NON-EXPIRE\n";
      outfile << client_id << ": in cache, valid" << endl;
      pthread_mutex_unlock(&mutex);
      vector<char> all_msg = res.get_allMsg();
      send(client_fd, all_msg.data(), all_msg.size(), 0);
      return true;
    }
    return false;
  }
}



