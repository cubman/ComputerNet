var net = require('net');
var concurrentClientCount = 0;
var first_global = 1;
var ports = [28563, 28564];
var main_map = [];
var another_server;

function processClient(clientSocket)
{
  console.log("113");
  var first_local = first_global;
  //if ()
  first_global = 0;

  concurrentClientCount++;
  console.log(concurrentClientCount + " concurrent clients are connected");

  //неполный запрос от клиентов (продолжение которого еще не доставилось по сети)
  var dataForProcessing = "";

  var answer = "";

  //регистрация обработчика события прихода данных
  clientSocket.on('data', function (data) {
    var queries = dataForProcessing + data;
    queries = queries.split(" ");

    queries.forEach(function() {
    var work = [];
    if (queries[0] == '0'  && queries.length >= 2) {
      work = queries.splice(0, 2);
      work[1] = work[1].substring(0, work[1].length - 2);
    if (main_map[work[1]])
        clientSocket.write(main_map[work[1]] + "\r\n");
    else
   		clientSocket.write("ErRoR!!!\r\n");
    }
    else
    if (queries[0] == '1' && queries.length >= 3) {
      work = queries.splice(0, 3);
      work[2] = work[2].substring(0, work[2].length - 2);
      main_map[work[1]] = work[2];
    if (first_local == 1)
      another_server.write("1 "+work[1] + " " + work[2] + "\r\n");
    }
    else
      clientSocket.write("wrong command!\r\n");
     });
  });

  //регистрация обработчика события отключения клиента
  clientSocket.on('end', function () {
    concurrentClientCount--;
  });
}



var server = net.createServer(processClient);

console.log("111");
var choice = parseInt(process.argv[2]);


if (choice == 0) {
  server.listen(ports[0], '0.0.0.0');
 /* while (true) {
  setTimeout(function(){}, 1000);
  try {
    another_server = net.createConnection({port: ports[1]}, function()  {
       console.log("Connected to 2 server\n");
    });
      break;
  }
  catch(error) {

  }
  
 }*/
}
else
if (choice == 1) {
  server.listen(ports[1], '0.0.0.0');
   var b = 1;
   while (true) {
  setTimeout(function(){}, 1000);
  //try {
  //console.log(tcpPortUsed.check(ports[0]));
 /* if (portInUse(ports[0]))
   {
   console.log("!!!!!!!!!!!!!!!\n");
   b = 0;
   }*/
   
    another_server = net.createConnection({port: ports[0]}, function()  {
       console.log("Connected to 1 server\n");
       b = 0;
    });
    console.log(b);
    another_server.on('error', function(e) { console.log(")))))))))))))))))00\n")});

    if (b == 0)
      break;
/* }
  catch(error) {
		//console.log("exc!!\n");
  }*/
 }
 }
else
 return;

