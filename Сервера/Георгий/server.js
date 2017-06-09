var net = require('net');
var concurrentClientCount = 0;

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
    console.log(queries)
    queries.forEach(function(elem) {
    if (elem != '') {
      var map = new Object()
      var left = elem.substring(1, elem.indexOf(')')).split(',')
      var right = elem.substring(elem.indexOf(')') + 1)

      for (var i = 0; i < left.length; i++) 
        map[left[i]] = 0
      
      for (var i = 0; i < right.length; i++) 
        if (map.hasOwnProperty(right[i]))
          ++map[right[i]]
        else
          console.log(right[i])

      for (var key in map) 
        clientSocket.write(key + ' : ' + map[key] + '\n');

      clientSocket.write("\n\n")
  
      }
      
    });
  }); 
  clientSocket.on('end', function () {
    concurrentClientCount--;
  });
}



var server = net.createServer(processClient);
server.listen(28563, '0.0.0.0')
