#include "abstract.hpp"
#include "Parser.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
using namespace std;


int main(){
  const char * hostname = NULL;
  const char * port = "12345";
  /* become a server */
  int socket_fd = create_service(hostname, port);
  int client_connection_fd = accept_connect(socket_fd);
  /* recv request */
  vector<char> all_msg;
  //recv_all(all_msg, client_connection_fd);
  char msg[65535];
  int len = recv(client_connection_fd, msg, 65535, 0);
  for (int i = 0; i < 65535; i++) all_msg.push_back(msg[i]);
  printf("%s", msg);
  //printf("%s", all_msg.data());


  /* parsing the request */
  Parser req_par;
  req_par.parsing(all_msg);


  /* generate the http request pocket */
  Request req(req_par.getstartline(), req_par.getheaders(), all_msg);


  /* get the hostname and port name */
  string target_host = req.get_host();
  string target_port = req.get_port();
  if (target_port == "") {
    target_port = "80";
  }
  cout << "\ntarget_host: " << target_host << endl << endl;
  cout << "\ntarget_port: " << target_port << endl << endl;


  /* connect to the target server */
  int toTarget_fd = connect_to(target_host.c_str(), target_port.c_str());
  if (toTarget_fd == -1) {
    cout << "connect to target fail\n";
  }
  cout << "\nConnect to target successfully\n";



  /* check the Method */

  string method = req.get_method();
  if (method == "CONNECT") {
    cout << "CONNECT\n";
    mode_connect(client_connection_fd, toTarget_fd);
  } else if (method == "POST") {
    cerr << "haven't implement yet\n";
    //mode_post();
  } else if (method == "GET") {
    cout << "GET\n";
    mode_request(client_connection_fd, toTarget_fd, msg, len, req);
  }

  close(client_connection_fd);
  close(toTarget_fd);
  return EXIT_SUCCESS;

}

