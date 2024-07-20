#ifndef TYPEH
#define TYPEH

typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned char* uchar_ptr;

typedef enum {
    fail,
    done
}Status;

typedef enum{
    enco,
    deco,
    unsupported
}op_type;

#endif