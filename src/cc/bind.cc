// Bytes buffer addon with dynamic size for nodejs/iojs
// Copyright (c) Chao Wang <hit9@icloud.com>

#include <v8.h>
#include <node.h>
#include "buf.hh"

using namespace v8;

extern "C" {
    static void init (Handle<Object> exports) {
        NanScope();
        buf::Buf::Initialize(exports);
    }
    NODE_MODULE(buf, init);
}
