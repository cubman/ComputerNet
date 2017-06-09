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

using namespace std;

void *processClient(void *);

void handleError(string msg)
{
  cerr << msg << " error code " << errno << " (" << strerror(errno) << ")\n";
  exit(1);
}

int port = 28563;

string return_to_user(string command) {
   int command_int = -1; // поступившая команда
   try {
   command_int = stoi(command); // попытка преобразования
   } catch(exception & e) {

   }
   
   switch(command_int) {
   case 1 : return "Nice! You are good!\r\n";
   case 2 : return "Ok! Have a nice day!\r\n";
   case 3 : return "Hello! Let's go on testing program again!\r\n";
   default : return "Excuse me, but this command was not declared yet\r\n";
   }
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
  string recvBuffer(50, '=');  //буфер приема
  //шаблон ответа
  string seneded_massage; // то сообщение, что передал клиент
  while (true) {//начало работы для для сервера с клиентом

  seneded_massage = "";
    char choice; // выбор клиента
    //получаем байты первой строки запроса
    err = recv(clientSocket, &recvBuffer[0], 1, 0); //считывание по 1 байту
    while (err > 0 &&  recvBuffer[readBytesCount-1] != '\r' && recvBuffer[readBytesCount] != '\n') {
      err = recv(clientSocket, &recvBuffer[readBytesCount++], 1, 0);
      char c = recvBuffer[readBytesCount - 2];// полученный символ
      if (c != '\n')
         seneded_massage += c;
    }
    
    readBytesCount = 1;
    if (seneded_massage.empty())
        continue;
    if (err < 0) handleError("recv failed:");
    if (err == 0) break;
    
    string to_user = return_to_user(seneded_massage);//сформированное сообщение от ервера к клиенту
    send( clientSocket, &to_user[0] , to_user.size(), 0 );
    cout << clientSocket << " entered command [ "<< seneded_massage  << " ]"<< endl;
  }
pthread_mutex_lock( &cs_mutex );
  concurrentClientCount--;
  pthread_mutex_unlock( &cs_mutex );
}

