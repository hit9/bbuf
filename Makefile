build: ./src/cc/*.cc ./src/c/*.c ./src/cc/*.hh ./src/c/*.h
	node-gyp configure rebuild

test: build
	mocha test.js

bench: build
	node bench.js

clean:
	rm -rf build
