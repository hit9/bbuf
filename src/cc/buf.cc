#include <v8.h>
#include <node.h>
#include "buf.hh"

using namespace buf;

Buf::Buf(size_t unit) {
    buf = buf_new(unit);
}

Buf::~Buf() {
    buf_free(buf);
}

/**
 * Register prototypes and exports
 */
void Buf::Initialize(Handle<Object> exports) {
    NanScope();
    Local<FunctionTemplate> t = NanNew<FunctionTemplate>(New);
    t->InstanceTemplate()->SetInternalFieldCount(1);
    NODE_SET_PROTOTYPE_METHOD(t, "cap", Cap);
    NODE_SET_PROTOTYPE_METHOD(t, "clear", Clear);
    NODE_SET_PROTOTYPE_METHOD(t, "inspect", Inspect);
    NODE_SET_PROTOTYPE_METHOD(t, "put", Put);
    NODE_SET_PROTOTYPE_METHOD(t, "size", Size);
    NODE_SET_PROTOTYPE_METHOD(t, "toString", ToString);
    NODE_SET_METHOD(t->GetFunction(), "isBuf", IsBuf);
    exports->Set(NanNew<String>("Buf"), t->GetFunction());
}

bool Buf::HasInstance(Handle<Value> val) {
    return val->IsObject() && Buf::HasInstance(val.As<Object>());
}

bool Buf::HasInstance(Handle<Object> obj) {
    return obj->InternalFieldCount() == 1 &&
        ObjectWrap::Unwrap<Buf>(obj) != NULL;
}

/**
 * Public API: - new Buf
 */
NAN_METHOD(Buf::New) {
    NanScope();

    if (args.Length() != 1) {
        NanThrowError("takes exactly 1 argument");
    } else if (!args[0]->IsUint32()){
        NanThrowTypeError("unsigned integer required");
    } else {
        Buf *buf = new Buf(args[0]->Uint32Value());
        buf->Wrap(args.This());
        NanReturnValue(args.This());
    }
}

/**
 * Public API: - Buf.isBuf
 */
NAN_METHOD(Buf::IsBuf) {
    NanScope();

    if (args.Length() != 1) {
        NanThrowError("takes exactly 1 argument");
    } else {
        NanReturnValue(NanNew<Boolean>(Buf::HasInstance(args[0])));
    }
}

/**
 * Public API: - Buf.prototype.cap
 */
NAN_METHOD(Buf::Cap) {
    NanScope();

    if (args.Length() != 0) {
        NanThrowError("takes no arguments");
    } else {
        Buf *self = ObjectWrap::Unwrap<Buf>(args.This());
        NanReturnValue(NanNew<Number>(self->buf->cap));
    }
}

/**
 * Public API: - Buf.prototype.size
 */
NAN_METHOD(Buf::Size) {
    NanScope();

    if (args.Length() != 0) {
        NanThrowError("takes no arguments");
    } else {
        Buf *self = ObjectWrap::Unwrap<Buf>(args.This());
        NanReturnValue(NanNew<Number>(self->buf->size));
    }
}

/**
 * Public API: - Buf.prototype.put
 */
NAN_METHOD(Buf::Put) {
    NanScope();

    if (args.Length() != 1) {
        NanThrowError("takes exactly 1 argument");
    } else if (!args[0]->IsString() && !Buffer::HasInstance(args[0]) &&
            !Buf::HasInstance(args[0])) {
        NanThrowTypeError("requires buf/string/buffer");
    } else {
        Buf *self = ObjectWrap::Unwrap<Buf>(args.This());
        String::Utf8Value tmp(args[0]->ToString());
        char *s = *tmp;
        int result = buf_puts(self->buf, s);
        switch(result) {
            case BUF_OK:
                // return new size
                NanReturnValue(NanNew<Number>(self->buf->size));
                break;
            case BUF_ENOMEM:
                NanThrowError("No memory");
                break;
        }
    }
}

/**
 * Public API: - Buf.prototype.toString
 */
NAN_METHOD(Buf::ToString) {
    NanScope();

    if (args.Length() != 0) {
        NanThrowError("takes no arguments");
    } else {
        Buf *self = ObjectWrap::Unwrap<Buf>(args.This());
        NanReturnValue(NanNew<String>(buf_str(self->buf)));
    }
}

/**
 * Public API: - Buf.prototype.clear
 */
NAN_METHOD(Buf::Clear) {
    NanScope();

    if (args.Length() != 0) {
        NanThrowError("takes no arguments");
    } else {
        Buf *self = ObjectWrap::Unwrap<Buf>(args.This());
        buf_clear(self->buf);
        NanReturnValue(NanNew<Boolean>(true));
    }
}

/**
 * Public API: - Buf.prototype.inspect
 */
NAN_METHOD(Buf::Inspect) {
    NanScope();
    Buf *self = ObjectWrap::Unwrap<Buf>(args.This());
    buf_t *buf = buf_new(self->buf->size);
    buf_sprintf(buf, "<buf [%d] '%.10s%s'>", self->buf->size,
            buf_str(self->buf), self->buf->size > 10 ? ".." : "");
    Local<Value> val = NanNew<String>(buf_str(buf));
    buf_free(buf);
    NanReturnValue(val);
}
