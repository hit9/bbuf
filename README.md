bbuf
====

Bytes buffer with dynamic size for nodejs/iojs, a bit like the `bytearray` in Python.

```
+------- cap -------+
+------- size --+   | => buf (uint8 array)
|UNIT|UNIT|UNIT|UNIT|
```

Reasons to start this project:

* Want a dynamic bytes buffer, but node's Buffer requires fixed size.
* Don't want to hold multiple copies in memory via `Buffer.concat`, `s + s` etc.

Example
-------

```javascript
var Buf = require('bbuf').Buf;

var buf = new Buf(4); // created with buf unit <bbuf [0]>

buf.put('abc');      // 3    buf => <bbuf [3] 61 62 63>
buf.put('123');      // 3    buf => <bbuf [6] 61 62 63 31 32 33>
buf.pop(4);          // 4    buf => <bbuf [2] 61 62>
buf.length           // 2
buf.cap              // 8
buf.toString()       // 'ab'
buf.clear()          // 2    buf => <bbuf [0]>
buf.cap              // 0
```

API Refs
--------

### new Buf(BUF_UNIT)

Create a buf instance by buffer unit size.

### Buf.isBuf(object)

Test if an object is a Buf instance.

### buf.put(string/buffer/buf)

Put string/buffer/buf object to buf, return bytes put. O(k)

```js
buf.put('abcd');
buf.put(buf);
buf.put(new Buffer('abcd'));
// buf.toString() => 'abcdabcdabcd'
```

### buf.pop(size)

Pop buf on the right end, return bytes poped. O(1)

### buf.length

Get/Set buf size.

```js
buf.put('abcd'); // buf => <bbuf [4] 61 62 63 64>
buf.length  // 4
buf.length = 5;  // buf => <bbuf [5] 61 62 63 64 20> , appended a ' '
buf.length = 2;  // buf => <bbuf [2] 61 62>  // truncate
```

### buf.cap

Get buf capacity.

### buf.grow(size)

Grow buf capacity to given size.

(We dont need to grow size manually to put data onto buf,
 but this method can reduce the memory allocation times if the
 result's bytes size is known to us).

### buf.toString()

Return string from buf.

```js
buf.put('abcd');
buf.toString();  // 'abcd'
```

### buf.clear()

Clear buf. O(!)

```js
buf.put('abcd');
buf.clear();
buf.length   // 0
buf.cap      // 0
```

### buf.copy()

Copy buf into a new buf instance. O(n)

```js
buf.put('abcd');  // buf => <bbuf [4] 61 62 63 64>
buf.copy();  // <bbuf [4] 61 62 63 64>
```

### buf[idx], buf.charAt(idx)

Index accessors for a buf instance. all O(1)

- buf[idx] - Get byte (uint8 value) at index.
- buf[idx] = 'c' - Set byte at index by a single byte char, or by a byte.
- buf.charAt(idx) - Get byte as string at index.

```js
buf.put('abced');
buf[0];   // 97
buf[0] = 'c';  // 'c'
buf[2] = 97;  // 97
buf.toString(); // 'cbaed'
buf.charAt(0); // 'c'
```

### buf.slice(begin[, end])

Slice buf into a new buf instance. O(k)

```js
buf.put('abcd');  // 4. buf => <bbuf [4] 61 62 63 64>
buf.slice(0)  // <bbuf [4] 61 62 63 64>
buf.slice(-1)  // <bbuf [1] 64>
buf.slice(1, 3)  // <bbuf [2] 62 63>
```

Don't use `String.prototype.slice.apply(buf, [begin, end])`,
because we are playing with `bytes` but not `chars`:

```js
buf.put('你好');
String.prototype.slice.apply(buf, [0, 1]);  // '你'
buf.charAt(0) !== '你';  // true
```

### buf.bytes()

Get bytes array. O(n)

```js
buf.put('abcd');  // 4
buf.bytes();  // [ 97, 98, 99, 100  ]
```

### buf.cmp(string/buffer/buf)

Compare string/buffer/buf with this buf, similar to C's `strcmp`. O(min(m, n))

```js
buf.put('cde');
buf.cmp('abc');  // 2
buf.cmp('cde');  // 0
buf.cmp('mnp');  // -10
```

### buf.indexOf(string/buffer/buf[, startIndex])

Find the first index of string/buffer/buf in this buf. (Boyer-Moore algorithm)

```js
buf.put('abcde');
buf.indexOf('ab');  // 0
buf.indexOf('what');  // -1
buf.indexOf('d', 1);  // 3
```

Don't use `String.prototype.indexOf` for `buf`, if you are trying to find the `byte index`
instead of the `char index` (e.g. to parse bytes from a socket):

```js
buf.put('你好')
buf.indexOf('好')  // 3
String.prototype.indexOf.call(buf, '好')  // 1
```

Benchmark
---------

Simple [benchmark](bench.js) between `v8 string + operator`, `v8 array join`,
`node buffer.write` and `bbuf.put`:

```
v8 string + operator:    1000000 op in 202 ms   => 4950495ops heapUsed: 76034848
v8 array join:           1000000 op in 379 ms   => 2638522.4ops heapUsed: 71710880
node buf fixed size:     1000000 op in 355 ms   => 2816901.4ops heapUsed: 14669800
bbuf dynamic size:       1000000 op in 371 ms   => 2695417.8ops heapUsed: 36397128
```

License
--------

MIT. (c) Chao Wang <hit9@icloud.com>
