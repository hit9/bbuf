IOBuf
=====

```
|-------------------------
| UNIT | UNIT | UNIT | ..       => Buf (cap, size)
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
buf.clear()               // => 15.
buf.cap                   // => 0

var bufa = new Buf(4), bufb = new Buf(4);
bufa.put('abc');
bufb.put('efg');
bufa.put(bufb);  // bufa => <buf [6] 'abcefg'> (with cap = 8) 
```

License
--------

MIT. (c) Chao Wang <hit9@icloud.com>
