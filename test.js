'use strict';

var assert = require('assert');
var iobuf  = require('./index');
var Buf    = iobuf.Buf;

describe('iobuf', function() {
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
});
