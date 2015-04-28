build: ./src/cc/*.cc ./src/c/*.c ./src/cc/*.hh ./src/c/*.h
	node-gyp configure rebuild

test: build
	mocha test.js

bench:
	@node bench/bench-v8-string.js
	@node bench/bench-v8-array-join.js
	@node bench/bench-node-buffer.js
	@node bench/bench-bbuf.js

clean:
	rm -rf build

.PHONY: bench
