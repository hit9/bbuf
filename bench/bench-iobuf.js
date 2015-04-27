var util = require('util');
var Buf = require('../index').Buf;

// operations count
n = 1000000;

// iobuf
var buf = new Buf(1024);
var startAt = new Date();
for (var i = 0; i < n; i++)
  buf.put(i.toString()[0]);
var endAt = new Date();
console.log(util.format('iobuf dynamic size:\t %d op in %s ms\t=> %dops heapUsed: %d',
                       n, endAt - startAt,
                       (1000 * n / (endAt - startAt)).toFixed(1)),
           process.memoryUsage().heapUsed);
