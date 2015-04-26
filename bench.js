// Simple bench on `append chars to string`.
var util = require('util');
var Buf = require('./index').Buf;

// operations count
n = 1000000;

// v8 String
var str = '';

var startAt = new Date();
for (var i = 0; i < n; i++)
  str += i.toString().slice(-1);

var endAt = new Date();
console.log(util.format('v8 string + operator:\t %d op in %s ms\t=> %dops',
                       n, endAt - startAt,
                       (1000 * n / (endAt - startAt)).toFixed(1)));


// Node Buffer
var buffer = new Buffer(str.length);
var startAt = new Date();
for (var i = 0; i < n; i++) {
  buffer.write(i.toString());
}
var endAt = new Date();
console.log(util.format('node buffer.write:\t %d op in %s ms\t=> %dops',
                       n, endAt - startAt,
                       (1000 * n / (endAt - startAt)).toFixed(1)));


// iobuf
var buf = new Buf(1);

var startAt = new Date();
for (var i = 0; i < n; i++)
  buf.put(i.toString().slice(-1));
var endAt = new Date();
console.log(util.format('node addon iobuf.put:\t %d op in %s ms\t=> %dops',
                       n, endAt - startAt,
                       (1000 * n / (endAt - startAt)).toFixed(1)));
