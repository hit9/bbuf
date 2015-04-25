IOBuf
=====

```
|-------------------------
| UNIT | UNIT | UNIT | ..       => Buf
|-------------------------
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
```

License
--------

MIT. (c) Chao Wang <hit9@icloud.com>
