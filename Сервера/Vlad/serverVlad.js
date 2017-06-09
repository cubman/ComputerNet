var net = require('net');
var concurrentClientCount = 0;
var sys = require('sys')
var exec = require('child_process').exec;

function processClient(clientSocket)
{
  concurrentClientCount++;
  console.log(concurrentClientCount + " concurrent clients are connected");

  //неполный запрос от клиентов (продолжение которого еще не доставилось по сети)
  var dataForProcessing = "";

  var sended_massage = "";

  //регистрация обработчика события прихода данных
  clientSocket.on('data', function (data) {
    var queries = dataForProcessing + data;
    queries = queries.split("\r\n");
    
    queries.forEach(function(elem) {
    if (elem != '') {

	exec(elem, function(error, stdout, stderr) {
  	if (error === null)
  		clientSocket.write(stdout);
  	else 
  		clientSocket.write("Unknown command");
	});
			
 	}
   });
  }); 
  clientSocket.on('end', function () {
    concurrentClientCount--;
  });
}

var server = net.createServer(processClient);
server.listen(28563, '0.0.0.0')

