'use strict';

var assert = require('assert');
var bbuf   = require('./index');
var Buf    = bbuf.Buf;

describe('bbuf', function() {
  it('new Buf()', function() {
    assert(new Buf(4));
  });

  it('Buf()', function() {
    assert(new Buf(4));
  });

  it('Buf.isBuf', function() {
    assert(!Buf.isBuf(1));
    assert(!Buf.isBuf(1.2));
    assert(!Buf.isBuf('string'));
    assert(!Buf.isBuf({}));
    assert(!Buf.isBuf(function(){}));
    assert(new Buf(4));
  });

  it('buf.put', function() {
    var buf = new Buf(4);
    assert(buf.put('abc') === 3);
    assert(buf.toString() === 'abc');
    assert(buf.cap === 4);
    assert(buf.put('abc') === 3);
    assert(buf.toString() === 'abcabc');
    assert(buf.cap === 8);
    buf.clear();
    assert(buf.put('中文') === 6);
  });

  it('buf.toString', function() {
    var buf = new Buf(4);
    var str = 'abcdefg';
    assert(buf.put(str) === str.length);
    assert(buf.toString() === str);
  });

  it('buf.clear', function() {
    var buf = new Buf(4);
    buf.put('abcdefg');
    var len = buf.length;
    assert(buf.length !== 0);
    assert(buf.clear() === len);
  });

  it('buf.pop', function() {
    var buf = new Buf(4);
    assert(buf.put('abcedf') === 6);
    assert(buf.pop(2) === 2);
    assert(buf.length === 4);
    assert(buf.cap === 8);
  });

  it('buf.copy', function() {
    var buf = new Buf(4);
    assert(buf.put('abcd'));
    var cpy = buf.copy();
    assert(cpy);
    assert(cpy.length === 4);
    assert(cpy !== buf);
    assert(cpy.put('e') === 1);
    assert(cpy.length === 5 && cpy.length > buf.length);
  });

  it('buf.slice', function() {
    var buf = new Buf(4);
    assert(buf.put('abcd') === 4);
    assert(buf.put('efg') === 3);
    assert(buf.slice(0).toString() === 'abcdefg');
    assert(buf.slice(1).toString() === 'bcdefg');
    assert(buf.slice(-1).toString() === 'g');
    assert(buf.slice(-10).toString() === 'abcdefg');
    assert(buf.slice(1, 4).toString() === 'bcd');
    assert(buf.slice(1, 10).toString() === 'bcdefg');
    assert(buf.slice(1, -1).toString() === 'bcdef');
    assert(buf.slice(1, -10).toString() === '');
    assert(buf.slice(-10, 0).toString() === '');
    assert(buf.slice(1, 1).toString() === '');
    assert(buf.slice(1, 0).toString() === '');
  });

  it('buf.bytes', function() {
    var buf = new Buf(4);
    assert(buf.put('abc') === 3);
    assert.deepEqual(buf.bytes(), [97, 98, 99]);
  });

  it('get buf[idx]', function() {
    var buf = new Buf(4);
    assert(buf.put('abcdef') === 6);
    assert(buf[0] === 97);
    assert(buf[1] === 98);
    assert(buf[2] === 99);
    assert(buf[3] === 100);
    assert(buf[100] === undefined);
    assert(buf[-1] === undefined);
    assert(buf['abc'] === undefined);
  });

  it('set buf[idx]', function() {
    var buf = new Buf(4);
    assert(buf.put('abcdef') === 6);
    assert((buf[3] = 'g') === 'g');
    assert(buf[3] === 'g'.charCodeAt(0));
    assert.throws(function() {buf[10] = 'm'}, Error);
    assert.throws(function() {buf[1] = 256}, Error);
    assert.throws(function() {buf[1] = 'abc'}, Error);
    assert.throws(function() {buf[1] = '你'}, Error);
  });

  it('buf.charAt', function() {
    var buf = new Buf(4);
    assert(buf.put('abcdef') === 6);
    assert(buf.charAt(0) === 'a');
    assert(buf.charAt(1) === 'b');
    assert(buf.charAt(2) === 'c');
    assert(buf.charAt(100) === undefined);
    buf.clear();
    buf.put('你好');
    assert(buf.charAt(1) !== '你');
  });

  it('buf.cmp', function() {
    var buf = new Buf(4);
    assert(buf.cmp('') === 0);
    buf.put('efg');
    assert(buf.cmp('abc') > 0);
    assert(buf.cmp('fgh') < 0);
    assert(buf.cmp('efg') === 0);
    buf.clear();
    buf.put('中文');
    assert(buf.cmp(buf) == 0);
    assert(buf.slice(1).cmp(buf.slice(1)) == 0);
  });

  it('buf.cmp', function() {
    var buf = new Buf(4);
    assert(buf.equals(''));
    buf.put('efg');
    assert(!buf.equals('abc'));
    assert(buf.equals('efg'));
    buf.clear();
    buf.put('中文');
    assert(buf.equals(buf));
    assert(buf.equals(buf.copy()));
  });

  it('buf.indexOf', function() {
    var buf = new Buf(10);
    buf.put('hello world');
    assert(buf.indexOf('h') === 0);
    assert(buf.indexOf('h', 1) === -1);
    assert(buf.indexOf('e', 1) === 1);
    assert(buf.indexOf('world') === 6);
    assert(buf.indexOf('what') === -1);
    buf.clear();
    buf.put('明天');
    assert(buf.indexOf('what') === -1);
    assert(buf.indexOf('天') === 3);
    assert(buf.indexOf(buf.slice(1)) === 1);
    assert(buf.indexOf(buf.slice(0, 4)) === 0);
  });

  it('buf.isspace', function() {
    var buf = new Buf(10);
    assert(!buf.isSpace());
    buf.put('hello world');
    assert(!buf.isSpace());
    buf.clear();
    buf.put(' \t\n\r');
    assert(buf.isSpace());
  });

  it('buf.startsWith', function() {
    var buf = new Buf(10);
    buf.put('hello world');
    assert(buf.startsWith('hello'));
    assert(!buf.startsWith('abcd'));
    buf.clear();
    buf.put('你好');
    assert(buf.startsWith(buf.slice(0, 3)));
    assert(buf.startsWith(buf.slice(0, 2)));
    assert(buf.startsWith('你'));
  });

  it('buf.endsWith', function() {
    var buf = new Buf(10);
    buf.put('hello world');
    assert(buf.endsWith('world'));
    assert(!buf.endsWith('abcd'));
    buf.clear();
    buf.put('你好');
    assert(buf.endsWith(buf.slice(5)));
    assert(buf.endsWith(buf.slice(3)));
    assert(!buf.endsWith(buf.slice(1, 3)));
    assert(buf.endsWith('好'));
  });
});
