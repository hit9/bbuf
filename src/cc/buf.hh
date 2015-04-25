#include <v8.h>
#include <node.h>
#include <buf.h>
#include "nan.h"

namespace buf {
using namespace v8;
using namespace node;

class Buf : public ObjectWrap {
public:
    Buf(size_t unit);
    ~Buf();

    static void Initialize(Handle<Object> exports);
    static bool HasInstance(Handle<Value> val);
    static bool HasInstance(Handle<Object> obj);
    static NAN_METHOD(IsBuf);
    static NAN_METHOD(New);
    static NAN_METHOD(Put);
    static NAN_METHOD(ToString);
    static NAN_METHOD(Clear);
    static NAN_METHOD(Inspect);
    static NAN_GETTER(GetCap);
    static NAN_SETTER(SetCap);
    static NAN_GETTER(GetLength);
    static NAN_SETTER(SetLength);
private:
    buf_t* buf;
};
};
