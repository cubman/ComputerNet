#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include <stdlib.h>
#include <errno.h>
#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <cstring>
#include <pthread.h>
#include <map>
#include <algorithm>
#include <thread>
#include <mutex>
#include <string>
#include <condition_variable>
#include <boost/algorithm/string.hpp>

using namespace std;

void *processClient(void *);

void handleError(string msg)
{
  cerr << msg << " error code " << errno << " (" << strerror(errno) << ")\n";
  exit(1);
}

int port = 28563;

void return_to_user(int clientSocket, const map<char, int> & m) {
   for (auto x : m)
   {
     string to_user;
     to_user.push_back(x.first);
     to_user += " : " + to_string(x.second) + "\n";
     send( clientSocket, &to_user[0] , to_user.size(), 0 );
   }
   string t = "\n\n";
   send( clientSocket, &t[0] , t.size(), 0 );
}

int main(int argc, char* argv[])
{

struct sockaddr_in serverAddress, clientAddress;
  int listenSocket;  // впускающий сокет

  //создаем сокет для приема соединений всех клиентов
  listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  //разрешаем повторно использовать тот же порт серверу после перезапуска (нужно, если сервер упал во время открытого соединения)
  int turnOn = 1;
  if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &turnOn, sizeof(turnOn)) == -1)
    handleError("setsockopt failed:");

  // Setup the TCP listening socket
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_addr.s_addr = inet_addr("0.0.0.0");
  serverAddress.sin_port = htons(port);

  if (bind( listenSocket, (sockaddr *) &serverAddress, sizeof(serverAddress)) == -1)
    handleError("bind failed:");

  if (listen(listenSocket, 1000) == -1) handleError("listen failed:");

  char serverIP[] = "127.0.0.1";
  struct sockaddr_in ServerAddr;
  
    while (true)
  {
    int *clientSocket = new int[1];
    *clientSocket = accept(listenSocket, NULL, NULL);
    if (*clientSocket < 0) handleError("accept failed:");
    pthread_t threadId;
    pthread_create(&threadId, NULL, processClient, (void*)clientSocket);
  }
}

static pthread_mutex_t cs_mutex =  PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
int concurrentClientCount = 0;

void *processClient(void *dataPtr)
{
  pthread_mutex_lock( &cs_mutex );
  cout << ++concurrentClientCount << " concurrent clients are connected\n";
  pthread_mutex_unlock( &cs_mutex );

  int clientSocket = ((int*)dataPtr)[0];
  delete[] (int*)dataPtr;
  
  int readBytesCount = 1, err;
  string recvBuffer(10000, '=');  //буфер приема
  //шаблон ответа
  string seneded_massage; // то сообщение, что передал клиент
  while (true) {//начало работы для для сервера с клиентом

  map<char, int> my_map;
  seneded_massage = "";
    err = recv(clientSocket, &recvBuffer[1], 1, 0); //считывание по 1 байту

    //cout << recvBuffer[0] << recvBuffer[1] << recvBuffer[2];
    cout << err << recvBuffer[1] << recvBuffer[readBytesCount] << endl;
    while (err > 0 &&  recvBuffer[1] == '(' && recvBuffer[readBytesCount] != ')') {
      err = recv(clientSocket, &recvBuffer[++readBytesCount], 1, 0);
      char c = recvBuffer[readBytesCount - 1];// полученный символ
      if (c != '(' && c != ')' && c != '\r' && c != '\n')
         seneded_massage += c;
    }

    std::vector<std::string> strs;
    boost::split(strs, seneded_massage, boost::is_any_of(","));
    for (auto x : strs) {
      my_map[x[0]] = 0;
    }

    readBytesCount = 1;
    seneded_massage += "\n++\n\n";
    //получаем байты первой строки запроса
   err = recv(clientSocket, &recvBuffer[1], 1, 0); //считывание по 1 байту
    while (err > 0 &&  recvBuffer[readBytesCount-1] != '\r' && recvBuffer[readBytesCount] != '\n') {  
      err = recv(clientSocket, &recvBuffer[++readBytesCount], 1, 0);
      char c = recvBuffer[readBytesCount - 1];// полученный символ
      if (my_map.find(c) != my_map.end())
        ++my_map[c];
    if (c != '\r' && c != '\n')
         seneded_massage += c;
    }

    readBytesCount = 1;
    if (seneded_massage.empty())
        continue;
    if (err < 0) handleError("recv failed:");
    if (err == 0) break;
    
    return_to_user(clientSocket, my_map);//сформированное сообщение от ервера к клиенту
    
    cout << clientSocket << " entered command [ "<< seneded_massage  << " ]"<< endl;
  }
pthread_mutex_lock( &cs_mutex );
  concurrentClientCount--;
  pthread_mutex_unlock( &cs_mutex );
}
