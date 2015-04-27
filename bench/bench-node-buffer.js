var util = require('util');

// operations count
n = 1000000;

// Node Buffer
var buffer = new Buffer(n);
var startAt = new Date();
for (var i = 0; i < n; i++) {
  buffer.write(i.toString()[0]);
}
var endAt = new Date();
console.log(util.format('node buf fixed size:\t %d op in %s ms\t=> %dops heapUsed: %d',
                       n, endAt - startAt,
                       (1000 * n / (endAt - startAt)).toFixed(1)),
                       process.memoryUsage().heapUsed);
