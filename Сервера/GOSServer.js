var net = require('net');
var concurrentClientCount = 0;
var first_global = 1;
var ports = [28563, 28564];
var main_map = [];
var another_server;

function getExperimentParam(paramName) {
    return main_map[paramName];
}

function processClient(clientSocket)
{
  concurrentClientCount++;
  console.log(concurrentClientCount + " concurrent clients are connected");

  //неполный запрос от клиентов (продолжение которого еще не доставилось по сети)
  var dataForProcessing = "";

  var answer = "";

  //регистрация обработчика события прихода данных
  clientSocket.on('data', function (data) {
    var queries = dataForProcessing + data;
    queries = queries.split("\r\n");

    queries.forEach(function(data) {
      if (data == "")
        return;
      var t = getExperimentParam(data);
      clientSocket.write((t == undefined ? "was not found" : t) + "\r\n");
    
    
      //clientSocket.write("wrong command!\r\n");
     });
  });

  //регистрация обработчика события отключения клиента
  clientSocket.on('end', function () {
    concurrentClientCount--;
  });
}


main_map["1"] = "aaa"

var server = net.createServer(processClient);
server.listen(28563, '0.0.0.0');