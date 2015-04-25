build: ./src/cc/*.cc ./src/c/*.c
	node-gyp configure rebuild

clean:
	rm -rf build
