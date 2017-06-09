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

std::map<string, pair<string, mutex *> > DATA_BASE;
std::map<string, pair<string, bool> > DATA_TASKS;

int connectSockets;
condition_variable cond_var;

void *processClient(void *);

void handleError(string msg)
{
  cerr << msg << " error code " << errno << " (" << strerror(errno) << ")\n";
  exit(1);
}


int main(int argc, char* argv[])
{
  int server_cnt = atoi(argv[1]), port;
  vector<int> ports(0);
  ports.push_back(28563);
  ports.push_back(28564);

  if (server_cnt == 0)
     port = ports[0];
      else
     if (server_cnt == 1)
      port = ports[1];
   else return 0;



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
  while (true) {
    connectSockets = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    ServerAddr.sin_family = AF_INET;
    ServerAddr.sin_addr.s_addr = inet_addr(serverIP);
    ServerAddr.sin_port = htons(ports[server_cnt^1]);
    if( connect( connectSockets, (sockaddr *) &ServerAddr, sizeof(ServerAddr)) == -1) {
    sleep(1);
      //handleError("connect failed:");
      cout << ":<" << endl;
      }
      else {
      cout << "YPAAAAAAAA! Connected"<<endl;
      break;
      }
  }
  cout << "server with port = " << port << " was made" << ports[server_cnt^1]<< endl;
  int first = 1;
  while (true)
  {
    int *clientSocket = new int[2];
    *clientSocket = accept(listenSocket, NULL, NULL);
    clientSocket[1] = first;
    if (*clientSocket < 0) handleError("accept failed:");
    pthread_t threadId;
    pthread_create(&threadId, NULL, processClient, (void*)clientSocket);
    first = 0;
  }
}

static pthread_mutex_t cs_mutex =  PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
static pthread_mutex_t create_data_mutex =  PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

int concurrentClientCount = 0;

void *processClient(void *dataPtr)
{
  pthread_mutex_lock( &cs_mutex );
  cout << ++concurrentClientCount << " concurrent clients are connected\n";
  pthread_mutex_unlock( &cs_mutex );

  int clientSocket = ((int*)dataPtr)[0];
  int first = ((int*)dataPtr)[1];
  delete[] (int*)dataPtr;
  
  string recvBuffer(50, '=');  //буфер приема
  //шаблон ответа
  string key, value;
  while (true)
  {
    key = "";
    value = "";
    int readBytesCount = 1, err;
    char choice;
    //получаем байты первой строки запроса
    err = recv(clientSocket, &recvBuffer[0], 1, 0);
    while (err > 0 && recvBuffer[readBytesCount-1] != ' ')
      err = recv(clientSocket, &recvBuffer[readBytesCount++], 1, 0);
    if (err < 0) handleError("recv failed:");
    if (err == 0) break; //клиент разорвал соединение
    choice = recvBuffer[readBytesCount-2];
    
    if (choice == '0') {  
    readBytesCount = 1;
    err = recv(clientSocket, &recvBuffer[0], 1, 0);
    while (err > 0 &&  recvBuffer[readBytesCount-1] != '\r' && recvBuffer[readBytesCount] != '\n') {
      err = recv(clientSocket, &recvBuffer[readBytesCount++], 1, 0);
      key += recvBuffer[readBytesCount - 2];
    }

    if (err < 0) handleError("recv failed:");
    if (err == 0) break;
    cout << "0 ((((((((( " << key << " ))))))))))" << endl;
    
    string answer = "";

     
    if (!DATA_BASE.count(key))
           answer = "was not declared!" ;
       else {
       
       	cout << "Entered to mutex\n";
       //	std::unique_lock<mutex> lock(*DATA_BASE[key].second);
       	DATA_BASE[key].second->lock();
       	answer = DATA_BASE[key].first;        
       		cout << "Read\n";	
      DATA_BASE[key].second->unlock();
      //lock.unlock();
       	cout << "Left mutex\n";
       }
   	   answer += "\r\n";
       send( clientSocket, &answer[0] , answer.size(), 0 );
    }

	else
	if (choice == '1' || choice == '2') {
	readBytesCount = 1;
	err = recv(clientSocket, &recvBuffer[0], 1, 0);
    while (err > 0 && recvBuffer[readBytesCount - 1] != ' ') {
      err = recv(clientSocket, &recvBuffer[readBytesCount++], 1, 0);
      key +=  recvBuffer[readBytesCount - 2];
    }

    if (err < 0) handleError("recv failed:");
    if (err == 0) break;
    cout << choice << " ((((((((( " << key << " ))))))))))" << endl;


	if (!DATA_BASE.count(key)) {
		 pthread_mutex_lock( &create_data_mutex);
   	     cout << "create mutext firstly" <<endl; 	
   	     DATA_BASE[key] = make_pair(value, new mutex());
   	     pthread_mutex_unlock( &create_data_mutex );
	   }
	if (choice == '1') {
	cout << "MUT LOCK"<<endl;
		DATA_BASE[key].second->lock();
		}
		else cout << "2 was meet"<<endl;
		
    readBytesCount = 1;
    err = recv(clientSocket, &recvBuffer[0], 1, 0);
    while (err > 0 &&  recvBuffer[readBytesCount-1] != '\r' && recvBuffer[readBytesCount] != '\n') {
      err = recv(clientSocket, &recvBuffer[readBytesCount++], 1, 0);
      value += recvBuffer[readBytesCount - 2];
    }
    cout << "1 ************ " << value << " ************" << endl;

    if (err < 0) handleError("recv failed:");
    if (err == 0) break; //клиент разорвал соединение

    string to_server = "1 "+ key + " " + value + "\r\n";
    if (!first && choice == '1') {
    cout << "send to " <<endl;
      DATA_TASKS[key].second = false;
      send(connectSockets, &to_server[0], to_server.size(), 0);
      }
    else {
     sleep(3);
     cout << "OK"<<endl;
     }
      
    if (choice == '2') {
    	 pthread_mutex_lock( &cs_mutex );
         cout << "write to task" <<endl; 	
   	     DATA_TASKS[key].second = true;
   	     pthread_mutex_unlock( &cs_mutex );	
    }
     	 
      
      if (choice != '2')
      {
      if (first) {
        to_server[0] = '2';
        send(connectSockets, &to_server[0], to_server.size(), 0);	
        cout << "it was sent to anothre server 2" <<endl;							
   	 }	else {
   	    while(!DATA_TASKS[key].second) { sleep(0.1); }
   		cout << "***************"<<endl;
   	}

      cout << "!!!writen!!!" <<endl; 
   	  DATA_BASE[key].first = value;     
   	 
   	   DATA_BASE[key].second->unlock();
   	   cout << "MUT UN"<<endl;
	}
   } 
   else {
    string error = "wrong command was entered\r\n";
     send( clientSocket, &error[0] , error.size(), 0 );
   }

   
 }


  pthread_mutex_lock( &cs_mutex );
  concurrentClientCount--;
  cout << " left = " << concurrentClientCount << endl;
  pthread_mutex_unlock( &cs_mutex );
}
