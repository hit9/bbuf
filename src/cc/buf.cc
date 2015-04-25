#include <v8.h>
#include <node.h>
#include "buf.hh"

using namespace buf;


#define ASSERT_STRING_LIKE(val)                                             \
    if (!Buf::IsStringLike(val)) {                                          \
        return NanThrowTypeError("requires buf/string/buffer");             \
    }
      
#define ASSERT_ARGS_LEN(len)                                                \
    if (args.Length() != len) {                                             \
        buf_t *err = buf_new(16);                                           \
        buf_sprintf(err, "takes exactly %d args", len);                     \
        NanThrowError(buf_str(err));                                        \
        buf_free(err);                                                      \
        return;                                                             \
    }                                                                       \

#define ASSERT_UINT32(val)                                                  \
    if (!val->IsUint32()) {                                                 \
        return NanThrowTypeError("requires unsigned integer");              \
    }


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
    // Constructor
    Local<FunctionTemplate> cls = NanNew<FunctionTemplate>(New);
    cls->SetClassName(NanNew("Buf"));
    cls->InstanceTemplate()->SetInternalFieldCount(1);
    cls->InstanceTemplate()->SetAccessor(NanNew<String>("cap"), GetCap, SetCap);
    cls->InstanceTemplate()->SetAccessor(NanNew<String>("length"), GetLength, SetLength);
    cls->InstanceTemplate()->SetIndexedPropertyHandler(GetIndex, SetIndex);
    // Prototype
    NODE_SET_PROTOTYPE_METHOD(cls, "clear", Clear);
    NODE_SET_PROTOTYPE_METHOD(cls, "inspect", Inspect);
    NODE_SET_PROTOTYPE_METHOD(cls, "put", Put);
    NODE_SET_PROTOTYPE_METHOD(cls, "toString", ToString);
    // Class methods
    NODE_SET_METHOD(cls->GetFunction(), "isBuf", IsBuf);
    // exports
    exports->Set(NanNew<String>("Buf"), cls->GetFunction());
}

bool Buf::HasInstance(Handle<Value> val) {
    return val->IsObject() && Buf::HasInstance(val.As<Object>());
}

bool Buf::HasInstance(Handle<Object> obj) {
    return obj->InternalFieldCount() == 1 &&
        ObjectWrap::Unwrap<Buf>(obj) != NULL;
}

bool Buf::IsStringLike(Handle<Value> val) {
    return Buf::IsStringLike(val.As<Object>());
}

bool Buf::IsStringLike(Handle<Object> obj) {
    return obj->IsString() || Buffer::HasInstance(obj) ||
        Buf::HasInstance(obj);
}

/**
 * Public API: - new Buf
 */
NAN_METHOD(Buf::New) {
    NanScope();
    ASSERT_ARGS_LEN(1);
    ASSERT_UINT32(args[0]);

    size_t unit = args[0]->Uint32Value();

    if (unit == 0) {
        NanThrowError("buf unit should not be 0");
    } else if (unit > BUF_MAX_UNIT) {
        NanThrowError("buf unit is too large");
    } else {
        Buf *buf = new Buf(unit);
        buf->Wrap(args.This());
        NanReturnValue(args.This());
    }
}

/**
 * Public API: - Buf.isBuf
 */
NAN_METHOD(Buf::IsBuf) {
    NanScope();
    ASSERT_ARGS_LEN(1);
    NanReturnValue(NanNew<Boolean>(Buf::HasInstance(args[0])));
}

/**
 * Public API: - buf.cap
 */
NAN_GETTER(Buf::GetCap) {
    NanScope();
    Buf *self = ObjectWrap::Unwrap<Buf>(args.This());
    NanReturnValue(NanNew<Number>(self->buf->cap));
}

/**
 * Public API: - buf.cap (disabled helper)
 */
NAN_SETTER(Buf::SetCap) {
    NanScope();
    NanThrowError("cannot set buf.cap");
}

/**
 * Public API: - buf.length
 */
NAN_GETTER(Buf::GetLength) {
    NanScope();
    Buf *self = ObjectWrap::Unwrap<Buf>(args.This());
    NanReturnValue(NanNew<Number>(self->buf->size));
}

/**
 * Public API: - buf.length
 */
NAN_SETTER(Buf::SetLength) {
    NanScope();
    ASSERT_UINT32(value);

    Buf *self = ObjectWrap::Unwrap<Buf>(args.This());
    buf_t *buf = self->buf;
    size_t len = value->Uint32Value();

    if (len < buf->size) {
        // truncate
        buf_rrm(buf, buf->size - len);
    }

    if (len > buf->size) {
        // append space
        buf_grow(buf, len);

        while (buf->size < len)
            buf_putc(buf, 0x20);
    }

    NanReturnValue(NanNew<Number>(buf->size));
}

/**
 * Public API: - buf[idx]
 */
NAN_INDEX_GETTER(Buf::GetIndex) {
    NanScope();

    Buf *self = ObjectWrap::Unwrap<Buf>(args.This());

    if (index >= self->buf->size) {
        NanReturnUndefined();
    } else {
        char s[1] = {0};
        s[0] = (self->buf->data)[index];
        NanReturnValue(NanNew<String>(s));
    }
}

/**
 * Public API: - buf[idx] = 'c'
 */
NAN_INDEX_SETTER(Buf::SetIndex) {
    NanScope();
    ASSERT_STRING_LIKE(value);

    Buf *self = ObjectWrap::Unwrap<Buf>(args.This());

    if (index >= self->buf->size) {
        NanReturnValue(NanNew<Boolean>(false));
    } else {
        Local<String> val = value->ToString();

        if (val->Length() != 1) {
            NanThrowError("requires a single char");
        } else {
            String::Utf8Value tmp(val);
            char *s = *tmp;
            (self->buf->data)[index] = s[0];
            NanReturnValue(NanNew<String>(s));
        }
    }
}

/**
 * Public API: - Buf.prototype.put
 */
NAN_METHOD(Buf::Put) {
    NanScope();
    ASSERT_ARGS_LEN(1);
    ASSERT_STRING_LIKE(args[0]);

    Buf *self = ObjectWrap::Unwrap<Buf>(args.This());
    String::Utf8Value tmp(args[0]->ToString());
    char *val = *tmp;
    int result = buf_puts(self->buf, val);

    switch(result) {
        case BUF_OK:
            NanReturnValue(NanNew<Number>(self->buf->size));
            break;
        case BUF_ENOMEM:
            NanThrowError("No memory");
            break;
    }
}

/**
 * Public API: - Buf.prototype.toString
 */
NAN_METHOD(Buf::ToString) {
    NanScope();
    ASSERT_ARGS_LEN(0);

    Buf *self = ObjectWrap::Unwrap<Buf>(args.This());
    NanReturnValue(NanNew<String>(buf_str(self->buf)));
}

/**
 * Public API: - Buf.prototype.clear
 */
NAN_METHOD(Buf::Clear) {
    NanScope();
    ASSERT_ARGS_LEN(0);

    Buf *self = ObjectWrap::Unwrap<Buf>(args.This());
    buf_clear(self->buf);
    NanReturnValue(NanNew<Boolean>(true));
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
