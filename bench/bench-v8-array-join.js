var util = require('util');

// operations count
n = 1000000;

// v8 array join
var arr = [];
var startAt = new Date();
for (var i = 0; i < n; i++)
  arr[i] = i.toString()[0];
var str = arr.join('');
var endAt = new Date();
console.log(util.format('v8 array join:\t\t %d op in %s ms\t=> %dops heapUsed: %d',
                       n, endAt - startAt,
                       (1000 * n / (endAt - startAt)).toFixed(1)),
                       process.memoryUsage().heapUsed);
