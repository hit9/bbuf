#include <v8.h>
#include <node.h>
#include <buf.h>
#include "nan.h"

#define BUF_MAX_UNIT 1024 * 1024  // 1mb

namespace buf {
using namespace v8;
using namespace node;

class Buf : public ObjectWrap {
public:
    Buf(size_t unit);
    ~Buf();

    static Persistent<FunctionTemplate> constructor;
    static void Initialize(Handle<Object> exports);
    static bool HasInstance(Handle<Value> val);
    static bool HasInstance(Handle<Object> obj);
    static bool IsStringLike(Handle<Value> val);
    static bool IsStringLike(Handle<Object> obj);
    static NAN_METHOD(IsBuf);
    static NAN_METHOD(New);
    static NAN_METHOD(Grow);
    static NAN_METHOD(Put);
    static NAN_METHOD(Pop);
    static NAN_METHOD(Cmp);
    static NAN_METHOD(Copy);
    static NAN_METHOD(Slice);
    static NAN_METHOD(Bytes);
    static NAN_METHOD(ByteAt);
    static NAN_METHOD(Clear);
    static NAN_METHOD(ToString);
    static NAN_METHOD(Inspect);
    static NAN_GETTER(GetCap);
    static NAN_SETTER(SetCap);
    static NAN_GETTER(GetLength);
    static NAN_SETTER(SetLength);
    static NAN_INDEX_GETTER(GetIndex);
    static NAN_INDEX_SETTER(SetIndex);
private:
    buf_t* buf;
};
};
