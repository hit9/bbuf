#include <v8.h>
#include <node.h>
#include "buf.hh"

using namespace buf;


#define ASSERT_STRING_LIKE(val)                                              \
    if (!Buf::IsStringLike(val)) {                                           \
        return NanThrowTypeError("requires buf/string/buffer");              \
     }                                                                       \

#define ASSERT_ARGS_LEN(len)                                                 \
    if (args.Length() != len) {                                              \
        buf_t *err = buf_new(21);                                            \
        buf_sprintf(err, "takes exactly %d args", len);                      \
        NanThrowError(buf_str(err));                                         \
        buf_free(err);                                                       \
        return;                                                              \
    }                                                                        \

#define ASSERT_ARGS_LEN_GT(len)                                              \
    if (!(args.Length() > len)) {                                            \
        buf_t *err = buf_new(22);                                            \
        buf_sprintf(err, "takes at least %d args", len + 1);                 \
        NanThrowError(buf_str(err));                                         \
        buf_free(err);                                                       \
        return;                                                              \
    }                                                                        \

#define ASSERT_ARGS_LEN_LT(len)                                              \
    if (!(args.Length() < len)) {                                            \
        buf_t *err = buf_new(21);                                            \
        buf_sprintf(err, "takes at most %d args", len - 1);                  \
        NanThrowError(buf_str(err));                                         \
        buf_free(err);                                                       \
        return;                                                              \
    }                                                                        \

#define ASSERT_UINT8(val)                                                    \
    if (!val->IsUint32() || val->Uint32Value() > 255) {                      \
        return NanThrowTypeError("requires unsigned 8 bit integer");         \
     }

#define ASSERT_UINT32(val)                                                   \
    if (!val->IsUint32()) {                                                  \
        return NanThrowTypeError("requires unsigned integer");               \
     }

#define ASSERT_INT32(val)                                                    \
    if (!val->IsInt32()) {                                                   \
        return NanThrowTypeError("requires integer");                        \
     }


#define ASSERT_BUF_OK(operation)                                             \
    int buf_ret = operation;                                                 \
                                                                             \
    if (buf_ret == BUF_ENOMEM) {                                             \
        return NanThrowError("No memory");                                   \
    }                                                                        \
    if (buf_ret != BUF_OK) {                                                 \
        return NanThrowError("Buf operation failed") ;                       \
    }                                                                        \

#define TOCSTRING(v8string)                                                  \
    String::Utf8Value tmp(v8string);                                         \
    char *str = *tmp;                                                        \


Persistent<FunctionTemplate> Buf::constructor;

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
    Local<FunctionTemplate> ctor = NanNew<FunctionTemplate>(New);
    ctor->InstanceTemplate()->SetInternalFieldCount(1);
    ctor->SetClassName(NanNew("Buf"));
    // Persistents
    NanAssignPersistent(constructor, ctor);
    // Accessors
    ctor->InstanceTemplate()->SetAccessor(NanNew<String>("cap"), GetCap, SetCap);
    ctor->InstanceTemplate()->SetAccessor(NanNew<String>("length"), GetLength, SetLength);
    ctor->InstanceTemplate()->SetIndexedPropertyHandler(GetIndex, SetIndex);
    // Prototype
    NODE_SET_PROTOTYPE_METHOD(ctor, "grow", Grow);
    NODE_SET_PROTOTYPE_METHOD(ctor, "put", Put);
    NODE_SET_PROTOTYPE_METHOD(ctor, "pop", Pop);
    NODE_SET_PROTOTYPE_METHOD(ctor, "cmp", Cmp);
    NODE_SET_PROTOTYPE_METHOD(ctor, "clear", Clear);
    NODE_SET_PROTOTYPE_METHOD(ctor, "copy", Copy);
    NODE_SET_PROTOTYPE_METHOD(ctor, "slice", Slice);
    NODE_SET_PROTOTYPE_METHOD(ctor, "bytes", Bytes);
    NODE_SET_PROTOTYPE_METHOD(ctor, "charAt", CharAt);
    NODE_SET_PROTOTYPE_METHOD(ctor, "indexOf", IndexOf);
    NODE_SET_PROTOTYPE_METHOD(ctor, "equals", Equals);
    NODE_SET_PROTOTYPE_METHOD(ctor, "isSpace", IsSpace);
    NODE_SET_PROTOTYPE_METHOD(ctor, "startsWith", StartsWith);
    NODE_SET_PROTOTYPE_METHOD(ctor, "endsWith", EndsWith);
    NODE_SET_PROTOTYPE_METHOD(ctor, "inspect", Inspect);
    NODE_SET_PROTOTYPE_METHOD(ctor, "toString", ToString);
    // Class methods
    NODE_SET_METHOD(ctor->GetFunction(), "isBuf", IsBuf);
    // Exports
    exports->Set(NanNew<String>("Buf"), ctor->GetFunction());
}

bool Buf::HasInstance(Handle<Value> val) {
    return val->IsObject() && Buf::HasInstance(val.As<Object>());
}

bool Buf::HasInstance(Handle<Object> obj) {
    return obj->InternalFieldCount() == 1 &&
        NanHasInstance(constructor, obj);
}

bool Buf::IsStringOrBuffer(Handle<Value> val) {
    if (val->IsNumber())
        return false;
    return Buf::IsStringOrBuffer(val.As<Object>());
}

bool Buf::IsStringOrBuffer(Handle<Object> obj) {
    return obj->IsString() || Buffer::HasInstance(obj);
}

bool Buf::IsStringLike(Handle<Value> val) {
    if (val->IsNumber())
        return false;
    return Buf::IsStringLike(val.As<Object>());
}

bool Buf::IsStringLike(Handle<Object> obj) {
    return obj->IsString() || Buffer::HasInstance(obj) ||
        Buf::HasInstance(obj);
}

char *Buf::StringLikeToChars(Handle<Value> val) {
    return Buf::StringLikeToChars(val.As<Object>());
}

char *Buf::StringLikeToChars(Handle<Object> obj) {
    if (Buf::HasInstance(obj)) {
        Buf *buf = ObjectWrap::Unwrap<Buf>(obj);
        return buf_str(buf->buf);
    } else {
        Local<String> val = obj->ToString();
        String::Utf8Value str(val);
        return (char *)*str;
    }
}

/**
 * Public API: - new Buf  O(1)
 */
NAN_METHOD(Buf::New) {
    NanScope();
    ASSERT_ARGS_LEN(1);
    ASSERT_UINT32(args[0]);

    if (args.IsConstructCall()) {
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
    } else {
        // turn to construct call
        Local<Value> argv[1] = { args[0] };
        Local<FunctionTemplate> ctor = NanNew<FunctionTemplate>(constructor);
        NanReturnValue(ctor->GetFunction()->NewInstance(1, argv));
    }
}

/**
 * Public API: - Buf.isBuf O(1)
 */
NAN_METHOD(Buf::IsBuf) {
    NanScope();
    ASSERT_ARGS_LEN(1);
    NanReturnValue(NanNew<Boolean>(Buf::HasInstance(args[0])));
}

/**
 * Public API: - buf.cap O(1)
 */
NAN_GETTER(Buf::GetCap) {
    NanScope();
    Buf *holder = ObjectWrap::Unwrap<Buf>(args.Holder());
    NanReturnValue(NanNew<Number>(holder->buf->cap));
}

/**
 * Public API: - buf.cap (disabled helper) O(1)
 */
NAN_SETTER(Buf::SetCap) {
    NanScope();
    NanThrowError("cannot set buf.cap");
}

/**
 * Public API: - buf.length O(1)
 */
NAN_GETTER(Buf::GetLength) {
    NanScope();
    Buf *holder = ObjectWrap::Unwrap<Buf>(args.Holder());
    NanReturnValue(NanNew<Number>(holder->buf->size));
}

/**
 * Public API: - buf.length O(1)/O(k)
 */
NAN_SETTER(Buf::SetLength) {
    NanScope();
    ASSERT_UINT32(value);

    Buf *holder = ObjectWrap::Unwrap<Buf>(args.Holder());
    buf_t *buf = holder->buf;
    size_t len = value->Uint32Value();

    if (len < buf->size) {
        // truncate
        buf_rrm(buf, buf->size - len);
    }

    if (len > buf->size) {
        // append space
        ASSERT_BUF_OK(buf_grow(buf, len));
        while (buf->size < len)
            buf_putc(buf, 0x20);
    }

    NanReturnValue(NanNew<Number>(buf->size));
}

/**
 * Public API: - buf[idx] O(1)
 */
NAN_INDEX_GETTER(Buf::GetIndex) {
    NanScope();

    Buf *holder = ObjectWrap::Unwrap<Buf>(args.Holder());

    if (index >= holder->buf->size) {
        NanReturnUndefined();
    } else {
        NanReturnValue(NanNew<Number>(holder->buf->data[index]));
    }
}

/**
 * Public API: - buf[idx] = 'c' or buf[idx] = 97. O(1)
 */
NAN_INDEX_SETTER(Buf::SetIndex) {
    NanScope();

    Buf *holder = ObjectWrap::Unwrap<Buf>(args.Holder());
    buf_t *buf = holder->buf;

    if (!(index < buf->size))
        return NanThrowError("index should be 0 ~ size-1");

    if (value->IsNumber()) {
        // Byte
        ASSERT_UINT8(value);
        buf->data[index] = value->Uint32Value();
    } else if (Buf::HasInstance(value)) {
        // Buf
        Buf *b = ObjectWrap::Unwrap<Buf>(value->ToObject());

        if (b->buf->size != 1)
            return NanThrowError("requires only 1 byte");
        buf->data[index] = b->buf->data[0];
    } else if (Buf::IsStringOrBuffer(value)) {
        // String/Buffer
        TOCSTRING(value->ToString());

        if (strlen(str) != 1)
            return NanThrowError("requires only 1 byte");
        buf->data[index] = str[0];
    }

    NanReturnValue(NanNew(value));
}

/**
 * Public API: - Buf.prototype.charAt O(1)
 */
NAN_METHOD(Buf::CharAt) {
    NanScope();
    ASSERT_ARGS_LEN(1);
    ASSERT_UINT32(args[0]);
    Buf *holder = ObjectWrap::Unwrap<Buf>(args.Holder());
    size_t idx = args[0]->Uint32Value();

    if (idx >= holder->buf->size) {
        NanReturnUndefined();
    } else {
        // To make an array sized 2 but not 1:
        // on some plats, this `{0}` wont tell v8 where the
        // '\0' terminates the string.
        char s[2] = {0, 0};
        s[0] = holder->buf->data[idx];
        NanReturnValue(NanNew<String>(s));
    }
}

/**
 * Public API: - Buf.prototype.grow O(1)
 */
NAN_METHOD(Buf::Grow) {
    NanScope();
    ASSERT_ARGS_LEN(1);
    ASSERT_UINT32(args[0]);
    Buf *holder = ObjectWrap::Unwrap<Buf>(args.Holder());
    ASSERT_BUF_OK(buf_grow(holder->buf, args[0]->Uint32Value()));
    NanReturnValue(NanNew<Number>(holder->buf->cap));
}

/**
 * Public API: - Buf.prototype.put O(k)
 */
NAN_METHOD(Buf::Put) {
    NanScope();
    ASSERT_ARGS_LEN(1);

    Buf *holder = ObjectWrap::Unwrap<Buf>(args.Holder());
    buf_t *buf = holder->buf;
    size_t size = buf->size;

    if (Buf::HasInstance(args[0])) {
        // Buf
        Buf *b = ObjectWrap::Unwrap<Buf>(args[0]->ToObject());
        ASSERT_BUF_OK(buf_put(buf, b->buf->data, b->buf->size));
    } else if (Buf::IsStringOrBuffer(args[0])) {
        // String/Buffer
        TOCSTRING(args[0]->ToString());
        ASSERT_BUF_OK(buf_puts(buf, str));
    } else if (args[0]->IsArray()) {
        // Array
        Local<Value> item;
        Handle<Array> arr = Handle<Array>::Cast(args[0]);
        size_t idx;
        size_t len = arr->Length();
        ASSERT_BUF_OK(buf_grow(buf, buf->size + len));

        for (idx = 0; idx < len; idx++) {
            item = arr->Get(idx);
            ASSERT_UINT8(item);
            ASSERT_BUF_OK(buf_putc(buf, item->Uint32Value()));
        }
    } else if (args[0]->IsNumber()) {
        // Byte
        ASSERT_UINT8(args[0]);
        ASSERT_BUF_OK(buf_putc(buf, args[0]->Uint32Value()));
    } else {
        // Bad type
        return NanThrowTypeError("requires string/buffer/buf/array/number");
    }
    NanReturnValue(NanNew<Number>(buf->size - size));
}

/**
 * Public APi: - Buf.prototype.pop O(1)
 */
NAN_METHOD(Buf::Pop) {
    NanScope();
    ASSERT_ARGS_LEN(1);
    ASSERT_UINT32(args[0]);

    Buf *holder = ObjectWrap::Unwrap<Buf>(args.Holder());
    NanReturnValue(NanNew<Number>(
                buf_rrm(holder->buf, args[0]->Uint32Value())));
}

/**
 * Public API: - Buf.prototype.cmp O(n)
 */
NAN_METHOD(Buf::Cmp) {
    NanScope();
    ASSERT_ARGS_LEN(1);

    Buf *holder = ObjectWrap::Unwrap<Buf>(args.Holder());
    buf_t *buf = holder->buf;

    if (Buf::HasInstance(args[0])) {
        // Buf
        Buf *b = ObjectWrap::Unwrap<Buf>(args[0]->ToObject());
        NanReturnValue(NanNew<Number>(buf_cmp(buf, buf_str(b->buf))));
    } else if (Buf::IsStringOrBuffer(args[0])) {
        // String/Buffer
        TOCSTRING(args[0]->ToString());
        NanReturnValue(NanNew<Number>(buf_cmp(buf, str)));
    } else {
        // TODO: Array
        NanThrowTypeError("requires string/buffer/buf");
    }
}

/**
 * Public API: - Buf.prototype.equals O(n)
 */
NAN_METHOD(Buf::Equals) {
    NanScope();
    ASSERT_ARGS_LEN(1);

    Buf *holder = ObjectWrap::Unwrap<Buf>(args.Holder());
    buf_t *buf = holder->buf;

    if (Buf::HasInstance(args[0])) {
        // Buf
        Buf *b = ObjectWrap::Unwrap<Buf>(args[0]->ToObject());
        NanReturnValue(NanNew<Boolean>(buf_equals(buf, buf_str(b->buf))));
    } else if (Buf::IsStringOrBuffer(args[0])) {
        // String/Buffer
        TOCSTRING(args[0]->ToString());
        NanReturnValue(NanNew<Boolean>(buf_equals(buf, str)));
    } else {
        // TODO: Array
        NanThrowTypeError("requires string/buffer/buf");
    }
}


/**
 * Public API: - Buf.prototype.toString  O(1)
 */
NAN_METHOD(Buf::ToString) {
    NanScope();
    ASSERT_ARGS_LEN(0);

    Buf *holder = ObjectWrap::Unwrap<Buf>(args.Holder());
    NanReturnValue(NanNew<String>(buf_str(holder->buf)));
}

/**
 * Public API: - Buf.prototype.clear O(1)
 */
NAN_METHOD(Buf::Clear) {
    NanScope();
    ASSERT_ARGS_LEN(0);

    Buf *holder = ObjectWrap::Unwrap<Buf>(args.Holder());
    size_t size = holder->buf->size;
    buf_clear(holder->buf);
    NanReturnValue(NanNew<Number>(size));
}

/**
 * Public API: - Buf.prototype.bytes
 */
NAN_METHOD(Buf::Bytes) {
    NanScope();
    ASSERT_ARGS_LEN(0);

    Buf *holder = ObjectWrap::Unwrap<Buf>(args.Holder());

    size_t idx;
    buf_t *buf = holder->buf;

    Local<Array> bytes(NanNew<Array>());

    for (idx = 0; idx < buf->size; idx++)
        bytes->Set(idx, NanNew<Number>(buf->data[idx]));
    NanReturnValue(bytes);
}

/**
 * Public API: - Buf.prototype.copy O(n)
 */
NAN_METHOD(Buf::Copy) {
    NanScope();
    Buf *holder = ObjectWrap::Unwrap<Buf>(args.Holder());
    Local<Value> argv[1] = { NanNew<Number>(holder->buf->unit) };
    Local<FunctionTemplate> ctor = NanNew<FunctionTemplate>(constructor);
    Local<Object> inst = ctor->GetFunction()->NewInstance(1, argv);
    Buf *copy = ObjectWrap::Unwrap<Buf>(inst);
    ASSERT_BUF_OK(buf_put(copy->buf, holder->buf->data, holder->buf->size));
    NanReturnValue(inst);
}

/**
 * Public API: - Buf.prototype.slice O(k)
 */
NAN_METHOD(Buf::Slice) {
    NanScope();
    ASSERT_ARGS_LEN_GT(0);
    ASSERT_ARGS_LEN_LT(3);
    ASSERT_INT32(args[0]);

    if (args.Length() > 1)
        ASSERT_INT32(args[1]);

    Buf *holder = ObjectWrap::Unwrap<Buf>(args.Holder());

    // make a copy
    Local<Value> argv[1] = { NanNew<Number>(holder->buf->unit) };
    Local<FunctionTemplate> ctor = NanNew<FunctionTemplate>(constructor);
    Local<Object> inst = ctor->GetFunction()->NewInstance(1, argv);
    Buf *copy = ObjectWrap::Unwrap<Buf>(inst);

    // slice data

    long begin = args[0]->Int32Value();
    long end;

    size_t len;
    size_t size = holder->buf->size;

    if (args.Length() == 1)
        end = size;
    else
        end = args[1]->Int32Value();

    if (begin < 0) begin += size;
    if (begin < 0) begin = 0;

    if (end < 0) end += size;
    if (end > int(size)) end = size;

    if (begin < end) len = end - begin;
    if (begin >= end) len = 0;

    if (len > 0) {
        ASSERT_BUF_OK(buf_grow(copy->buf, len));
    }

    size_t idx = 0;

    while (idx < len)
        buf_putc(copy->buf, (holder->buf->data)[begin + idx++]);
    NanReturnValue(inst);
}

/**
 * Public API: - Buf.prototype.indexOf. Boyer Moore
 */
NAN_METHOD(Buf::IndexOf) {
    NanScope();
    ASSERT_ARGS_LEN_GT(0);
    ASSERT_ARGS_LEN_LT(3);
    ASSERT_STRING_LIKE(args[0]);

    size_t start = 0;
    if (args.Length() == 2)
        start = args[1]->Uint32Value();

    Buf *holder = ObjectWrap::Unwrap<Buf>(args.Holder());
    char *str = Buf::StringLikeToChars(args[0]);
    size_t idx = buf_index(holder->buf, str, start);

    if (idx == holder->buf->size) {
        NanReturnValue(NanNew<Number>(-1));
    } else {
        NanReturnValue(NanNew<Number>(idx));
    }
}

/**
 * Public API: - Buf.prototype.isSpace. O(n)
 */
NAN_METHOD(Buf::IsSpace) {
    NanScope();
    ASSERT_ARGS_LEN(0);
    Buf *holder = ObjectWrap::Unwrap<Buf>(args.Holder());
    NanReturnValue(NanNew<Boolean>(buf_isspace(holder->buf)));
}

/**
 * Public API: - Buf.prototype.startsWith. O(min(n, k))
 */
NAN_METHOD(Buf::StartsWith) {
    NanScope();
    ASSERT_ARGS_LEN(1);
    ASSERT_STRING_LIKE(args[0]);
    Buf *holder = ObjectWrap::Unwrap<Buf>(args.Holder());
    char *str = Buf::StringLikeToChars(args[0]);
    NanReturnValue(NanNew<Boolean>(buf_startswith(holder->buf, str)));
}

/**
 * Public API: - Buf.prototype.endsWith. O(min(n, k))
 */
NAN_METHOD(Buf::EndsWith) {
    NanScope();
    ASSERT_ARGS_LEN(1);
    ASSERT_STRING_LIKE(args[0]);
    Buf *holder = ObjectWrap::Unwrap<Buf>(args.Holder());
    char *str = Buf::StringLikeToChars(args[0]);
    NanReturnValue(NanNew<Boolean>(buf_endswith(holder->buf, str)));
}

/**
 * Public API: - Buf.prototype.inspect
 */
NAN_METHOD(Buf::Inspect) {
    NanScope();
    Buf *holder = ObjectWrap::Unwrap<Buf>(args.Holder());
    buf_t *buf = buf_new(holder->buf->unit);  // ensure not 0

    buf_sprintf(buf, "<bbuf [%d]", holder->buf->size);

    size_t idx;

    for (idx = 0; idx < holder->buf->size; idx++) {
        if (idx == 0)
            buf_putc(buf, ' ');

        if (idx > 32) {  // max display 33 chars
            buf_sprintf(buf, "..");
            break;
        } else {
            buf_sprintf(buf, "%02x", (holder->buf->data)[idx]);
        }

        if (idx != holder->buf->size - 1)
            buf_putc(buf, ' ');
    }

    buf_putc(buf, '>');

    Local<Value> val = NanNew<String>(buf_str(buf));
    buf_free(buf);
    NanReturnValue(val);
}
