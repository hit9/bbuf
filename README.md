IOBuf
=====

Utf8 buffer with dynamic size for nodejs/iojs.

```
+------- -cap ------+
+------- size --+   | => buf
|UNIT|UNIT|UNIT|UNIT|
```

Example
-------

```
var Buf = require('iobuf').Buf;

var buf = new Buf(16);   // created with buf unit

buf.put('abcd');          // => 4.  buf => <buf [4] 'abcd'>
buf.put('1234567890123')  // => 13. buf => <buf [17] 'abc123456..'>
buf.pop(2);               // => 2.  buf => <buf [2] 'ab'>
buf.length                // => 15.
buf.cap                   // => 32.
buf.toString()            // => 'abcd12345678901'
buf.clear()               // => 15.
buf.cap                   // => 0
```

API Refs
--------

### new Buf(BUF_UNIT)

Create a buf instance by buffer unit size.

### Buf.isBuf(object)

Test if an object is a Buf instance.

### buf.put(string/buffer/buf)

Put string/buffer/buf object to buf, return bytes put. O(k)

### buf.pop(size)

Pop buf on the right end, return bytes poped. O(1)

### buf.length

Get/Set buf size.

```js
buf.put('abcd'); // <buf [4] 'abcd'>
buf.length  // 4
buf.length = 5; console.log(buf);  // <buf [5] 'abcd '>
buf.length = 2; console.log(buf);  // <buf [2] 'ab'>
```

### buf.cap

Get buf capacity.

### buf.grow(size)

Grow buf capacity to given size.

### buf.toString()

Return string from buf.

```js
buf.put('abcd'); // <buf [4] 'abcd'>
buf.toString();  // 'abcd'
```

### buf.clear()

Clear buf.

```js
buf.put('abcd');
buf.clear();
buf.length   // 0
buf.cap      // 0
```

### buf.copy()

Copy buf into a new buf instance. O(n)

```js
buf.put('abcd');  // <buf [4] 'abcd'>
buf.copy()  // <buf [4] 'abcd'>
```

### buf.slice(begin[, end])

Slice buf into a new buf instance. O(k)

```js
buf.put('abcd');  // <buf [4] 'abcd'>
buf.slice(0)  // <buf [4] 'abcd'>
buf.slice(-1)  // <buf [1] 'd'>
buf.slice(1, 3)  // <buf [2] 'bc'>
```

License
--------

MIT. (c) Chao Wang <hit9@icloud.com>
