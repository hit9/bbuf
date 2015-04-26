build: ./src/cc/*.cc ./src/c/*.c ./src/cc/*.hh ./src/c/*.h
	node-gyp configure rebuild

clean:
	rm -rf build
