#include "httpProxy.hpp"

unordered_map<string, Response> cache_map;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
ofstream outfile("var/log/erss/proxy.log");
void build_proxy() {
  const char * hostname = NULL;
  const char * port = "12345";
  /* become a server */
  int socket_fd = create_service(hostname, port);
  pthread_mutex_lock(&mutex);
  outfile << "successfully create proxy service\n";
  pthread_mutex_unlock(&mutex);
  /* loop forever */
  int client_id = 0;
  while (1) {
    /* accept connection from client */
    int client_fd = accept_connect(socket_fd);
    pthread_mutex_lock(&mutex);
    outfile << client_id <<":accept connection from new client\n";
    pthread_mutex_unlock(&mutex);
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
    pthread_mutex_lock(&mutex);
    outfile << client_id <<": This connection show no request\n";
    pthread_mutex_unlock(&mutex);
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
    pthread_mutex_lock(&mutex);
    outfile << client_id <<": no host connection\n";
    pthread_mutex_unlock(&mutex);
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

  cout << client_id <<":  \"" << req_par.getstartline() << "\" from "<< target_host;
  cout << " @ " <<  dt << endl;
  pthread_mutex_unlock(&mutex);


  /* connect to the target server */
  pthread_mutex_lock(&mutex);
  int toTarget_fd = connect_to(target_host.c_str(), target_port.c_str());
  pthread_mutex_unlock(&mutex);
  if (toTarget_fd == -1) {
    pthread_mutex_lock(&mutex);
    outfile << client_id <<": connect to target fail\n";
    cout << client_id <<": connect to target fail\n";
    pthread_mutex_unlock(&mutex);
  }
  pthread_mutex_lock(&mutex);
  outfile << client_id <<": Connect to target successfully\n";
  cout << client_id <<": Connect to target successfully\n";
  pthread_mutex_unlock(&mutex);


  /* check the Method */
  string method = req.get_method();
  if (method == "CONNECT") {
    pthread_mutex_lock(&mutex);
    outfile << client_id <<": CONNECT\n";
    cout << client_id <<": CONNECT\n";
    pthread_mutex_unlock(&mutex);
    mode_connect(client_connection_fd, client_id, toTarget_fd);
    pthread_mutex_lock(&mutex);
    outfile << client_id <<": Tunnel closed\n";
    pthread_mutex_unlock(&mutex);

  } else if (method == "POST") {
    pthread_mutex_lock(&mutex);
    outfile << client_id <<": POST\n";
    cout << client_id <<": POST\n";
    pthread_mutex_unlock(&mutex);
    mode_post(client_connection_fd, client_id, toTarget_fd, msg, len, req);
    //cerr << "haven't implement yet\n";
    //mode_post();
  } else if (method == "GET") {
    pthread_mutex_lock(&mutex);
    outfile << client_id <<": GET\n";
    cout << client_id <<": GET\n";
    pthread_mutex_unlock(&mutex);
    mode_get(client_connection_fd, client_id, toTarget_fd, msg, len, req, req.get_startLine());
  }

  close(client_connection_fd);
  close(toTarget_fd);
  return NULL;
}



void recv_all(vector<char>& all_msg, int sockt_fd) {
  pthread_mutex_lock(&mutex);
  outfile << ": begin Receving ...\n";
  pthread_mutex_unlock(&mutex);
  char msg[65535];
  int len = 0;
  do {
    pthread_mutex_lock(&mutex);
    outfile << ": Still Receving ...\n";
    pthread_mutex_unlock(&mutex);
    //cout << "Still Receving ...\n";
    len = recv(sockt_fd, msg, 65535, 0);
    if (len <= 0) return;
    for (int i = 0; i < len; i++) all_msg.push_back(msg[i]);
  } while (len > 0);
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
            pthread_mutex_lock(&mutex);
            outfile << client_id <<": Error: select\n\n";
            pthread_mutex_unlock(&mutex);
            return;
        }
        if (FD_ISSET(client_fd, &read_fds)) {
            //cout << "client msg\n";
            char msg[65535];
            int len = recv(client_fd, &msg, 65535, 0);
            if (len <= 0) {
              pthread_mutex_lock(&mutex);
              outfile << client_id <<": no longer receive\n\n";
              pthread_mutex_unlock(&mutex);
              return;
            }
            send(server_fd, &msg, len, 0);
            //cout << msg;
        }
        else if (FD_ISSET(server_fd, &read_fds)){
            //cout << "server msg\n";
            char msg[65535];
            int len = recv(server_fd, &msg, 65535, 0);
            if (len <= 0) {
              pthread_mutex_lock(&mutex);
              outfile << client_id <<": no longer receive\n\n";
              pthread_mutex_unlock(&mutex);
              return;
            }
            send(client_fd, &msg, len, 0);
            //cout << msg;
        }
    }
}

void mode_get(int client_fd, int client_id, int server_fd, char * msg, int len, Request & req, string startLine) {
  /* check cash */
  if (try_used_cache(client_fd, client_id, server_fd, msg, len, req, startLine)) {
    return;
  }
  pthread_mutex_lock(&mutex);
  cout << client_id <<": No cache available\n";
  pthread_mutex_unlock(&mutex);
  /* send the request from the client to the target server */
  send(server_fd, msg, len, 0);
  /* get "part of" Response from the target server */
  char server_msg[65536] = {0};
  int mes_len = recv(server_fd, server_msg, sizeof(server_msg), 0);
  if (mes_len <= 0) {
    pthread_mutex_lock(&mutex);
    outfile << client_id <<": Bad GET Request\n";
    cout << client_id <<": Bad GET Request\n";
    pthread_mutex_unlock(&mutex);
    return;
  }
  /* parsing response */
  vector<char> res_msg(server_msg, server_msg + mes_len);
  Parser res_par;
  res_par.parsing(res_msg);
  /* transfer to Resposnse */
  Response res(res_par.getstartline(), res_par.getheaders(), res_msg, res_par.get_headerSize());
  /* check if chunk */
  if (res.isChunk()) {
    /* chunk */
    pthread_mutex_lock(&mutex);
    outfile << client_id <<": It's chunked!\n";
    cout << client_id <<": It's chunked!\n";
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
    pthread_mutex_lock(&mutex);
    outfile << client_id <<": It's non-chunked!\n";
    cout << client_id <<": It's non-chunked!\n";
    pthread_mutex_unlock(&mutex);
    // get content len
    int content_len = res.get_contentLen();
    if (content_len == -1) {
      pthread_mutex_lock(&mutex);
      outfile << client_id <<": Error: no content len\n";
      cout << client_id <<": Error: no content len\n";
      cout << server_msg << endl;
      pthread_mutex_unlock(&mutex);
      send(client_fd, server_msg, mes_len, 0);
      return;
    }
    // keep receive all response
    do{
      pthread_mutex_lock(&mutex);
      cout << client_id <<": Still receiving\n";
      cout << client_id <<": res_msg size:" << res_msg.size()<< endl;
      cout << client_id <<": content Len size + header size::" << content_len + res.get_headerSize()<< endl;
      pthread_mutex_unlock(&mutex);
      if (res_msg.size() >= content_len + res.get_headerSize()) break;
      mes_len = recv(server_fd, server_msg, sizeof(server_msg), 0);
      if (mes_len <= 0) break;
      vector<char> temp_msg(server_msg, server_msg + mes_len);
      res_msg.insert(res_msg.end(), temp_msg.begin(), temp_msg.end());
    } while(1);
    pthread_mutex_lock(&mutex);
    cout << client_id <<": Finish receiving\n";
    pthread_mutex_unlock(&mutex);
    /* send response to client*/
    send(client_fd, res_msg.data(), res_msg.size(), 0);
    /* try cache */
    res.set_allMsg(res_msg);
    try_cache(res, startLine, client_id);
  }
  return;
}


void mode_post(int client_fd, int client_id, int server_fd, char * msg, int len, Request & req) {
  // POST should receive all message first
  /* send request to server */
  send(server_fd, msg, len, 0);
  /* receive response from server */
  char res_msg[65535];
  int res_len = recv(server_fd, &res_msg, 65535, 0);
  if (res_len <= 0) {
    pthread_mutex_lock(&mutex);
    outfile << client_id <<": Post Response fail\n";
    pthread_mutex_unlock(&mutex);
  }
  send(client_fd, &res_msg, res_len, 0);
}

void try_cache(Response & res, string startLine, int client_id) {
  pthread_mutex_lock(&mutex);
  cout << client_id <<": Try cache\n";
  pthread_mutex_unlock(&mutex);
  // check no store
  string cacheInfo = res.get_cacheInfo();
  if (cacheInfo == "" || cacheInfo.find("no-store") != string::npos) {
    pthread_mutex_lock(&mutex);
    cout << client_id <<": cannot cache the response\n";
    pthread_mutex_unlock(&mutex);
    return;
  }
  pthread_mutex_lock(&mutex);
  cout << client_id <<": cache the response\n";
  Response temp = res;
  cache_map.insert(pair<string,Response>(startLine,temp));
  pthread_mutex_unlock(&mutex);
}

bool try_used_cache(int client_fd, int client_id, int server_fd, char * msg, int len, Request & req, string startLine) {
  /* check if there is a cache avaible */
  if (cache_map.find(req.get_startLine()) == cache_map.end()) return false;
  // available cache found
  Response res = (cache_map.find(req.get_startLine()))->second;
  /* revalidate check */
  if (res.has_noCache() || (res.has_mustRevalidate() && res.isExpire())) {
    // check e_tag and Last-Modified
    if (!res.has_eTag() && !res.has_LastModified()) {
      // the response can be used
      vector<char> all_msg = res.get_allMsg();
      send(client_fd, all_msg.data(), all_msg.size(), 0);
      return true;
    } 
    else {
      // add If-None-Match or If-Modified-Since
      string extra = res.has_eTag() ? res.get_eTag() : res.get_LastModified();
      extra += "\r\n";
      vector<char> vec_extra(extra.begin(),extra.end());
      vector<char> req_msg = req.get_allMsg();
      req_msg.insert(req_msg.begin()+req.get_headerSize()-2, vec_extra.begin(), vec_extra.end());
      /* TODO: print out the req_mes */
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
        send(client_fd, new_res_msg.data(), new_res_msg.size(), 0);
        // try to cache the new response
        //try_cache(Response & res, string startLine, int client_id)
        try_cache(new_res, status_line, client_id);
      }
      return true;
    }
  }
  else {
    // no cache control
    /* expire time check */
    if (res.isExpire()) {
      // Ask for a new Response
      return false;
    }
    else {
      // return the current cache to the client
      vector<char> all_msg = res.get_allMsg();
      send(client_fd, all_msg.data(), all_msg.size(), 0);
      return true;
    }
  }
}



