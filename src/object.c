#include "object.h"
#define __USE_MISC //for MAP_ANONYMOUS declaration
#include <sys/mman.h>
#undef __USE_MISC

// memory (buddy) allocation //

// the K type is an unsigned 64-bit int
// K objects are either a tagged int or a pointer to data

// tagged objects contain the type in the upper 8 bits, and the value in the lower 56 bits
// memory for tagged objects doesn't need to be managed manually 

// otherwise the K is a pointer to data (NB: error objects are an exception)
// pointed-to K objects are allocated on the heap using a buddy allocation algorithm
// these objects occupy an x^2 bucket of memory
// the K object header is 16 bytes (see object.h for header layout) so objects need to be assigned a minimum of 16 bytes
// however 16B would only accommodate a 0-count object, and in practice we usually have objects with at least 1 element
// so instead the minimum size MIN_ALLOC is 32 bytes 
// the K pointer points to just after the header, to the start of the data. see skrawl.h for header and data accessors
// example: if we wanted to allocate an array of 22 chars, we would need 16(header) + 22(data) = 38 bytes
// 38 bytes fits into a 64-byte bucket. this leaves 26 bytes of space in the tail
// this may seems wasteful but allows for in-place appends if there are no references to the object

// free K buckets are available in the global M array
// K objects are ref counted with ref(increment) and unref(decrement)
// when an object is "freed" (unref called when object ref is already 0) it is placed into M
// each element of M contains a linked list of free objects
// M[n] contains a linked list of buckets of size ( MIN_ALLOC<<n )
// so M[0] has 32B buckets, M[1] 64B buckets, M[2] 128B, ...
// NB: mmap'd memory is never returned to the OS, and free buckets are never coalesced with their buddies into larger buckets
// TODO: implement coalesce&free functionality

#define HEADER_SIZE    16
#define MIN_ALLOC      32L 
#define BUCKET_SIZE(x) (u64)( MIN_ALLOC << MEM(x) ) //size of bucket containing object x
#define SIZEOF(x)      ( TYPE_SIZE[ABS(TYP(x))] )   //elemental size of x

// forward declarations
K k1(K x);
static void _printK(K x);

// global vars
static const char   *VERB_STR=":+-*%,?.@!$#_^&=<>~|;";
static const char *ADVERB_STR="'/\\'/\\";

// mmap wrapper 
static void *alloc_n(u64 n){
    void *x = mmap(0,n,PROT_READ|PROT_WRITE,MAP_ANONYMOUS|MAP_PRIVATE,-1,0);
    if (x != MAP_FAILED)
        return x;
    else 
        printf("'mmap failed! exiting...\n"), exit(1);
}

#ifdef DBG_WS
u64 WS=0;  // number of bytes allocated to objects (can be less than bytes requested from the OS)
u64 WT=0;  // total number of bytes allocated on the heap
#endif

K M[31]; // array of free memory buckets

// return smallest bucket with at least n bytes
static K m1(u64 n){
    K x, r;
    i8 bucket, j;

    // find which bucket we need to assign from
    // the K header is 16 bytes
    // we add 15 so the arg to clzll is guarenteed to be at least 16 (n must be >0) 
    // we subtract the clz from 59 as we want bucket 0 to be a size of 2^5 bytes
    // (if 0 bytes was the smallest allocation we would subtract from 64)
    bucket = 59 - __builtin_clzll(n + 15);
    j = bucket;
    
    // if the bucket exists, return it
    x = M[bucket];
    if (x){
        M[bucket] = *OBJ(x); //set the next bucket in the linked list as the top of the free list in M
        return x;
    }

    // else we search for larger bucket in M we can split
    // if none exists we mmap a new block of memory with alloc_n
    // MAX(19, bucket) means 32L<<19 will always be the minimum allocation (16KiB)
    do {
        ++j;
        x = ( j < 31 ) ?
            M[j] :
            HEADER_SIZE + (K)alloc_n(MIN_ALLOC << (j=MAX(19, bucket)));
    } while (!x);

    MEM(x) = bucket, M[j] = *OBJ(x), r = x;

    // iteratively split the mmap'd block into smaller buckets and add the buckets to M
    while (bucket < j){
        // jump to next bucket
        x += BUCKET_SIZE(x),
        // assign bucket val in header
        MEM(x) = bucket,
        // add new bucket to the free bucket array
        M[bucket] = x,
        // 0 its 1st element (not a linked list)
        *OBJ(x) = 0,
        // next bucket
        ++bucket;
    }

    return r;
}

// size of each type     KK  KX, KC  KI  KF  KS  KU  KV  KW  '   /   \   ':  /:  \:  ::
static i8 TYPE_SIZE[] = {8 , 1 , 1 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 8};

// return a K object of type t and count n
K tn(i8 t, i64 n){
    K r = m1( MAX(1, n) * TYPE_SIZE[ABS(t)] );

#ifdef DBG_WS
    WS += BUCKET_SIZE(r);
    WT += BUCKET_SIZE(r);
#endif

    HDR_TYP(r) = t, REF(r) = 0, HDR_CNT(r) = n;
    return r;
}

// copy y into x starting at x[i]
static K xiy(K x, i64 i, K y){
    i8 s = SIZEOF(x);
    memcpy(CHR(x) + s*i, (void*)y, s*CNT(y));
    if (!TYP(y)){
        for (i64 i=0, n=CNT(y); i<n; i++) ref( OBJ(y)[i] );
    }
    return unref(y), x;
}

// join 2 K objects
K j2(K x, K y){
    // if x is empty, return y
    if (!CNT(x)) return unref(x), y;

    i8  s = SIZEOF(x);
    i8  t = ABS(TYP(x));
    i64 m = CNT(x);
    i64 n = CNT(y) + m;
    u64 b = s * n; //number of bytes used by elements in joined object

    // if x has refcount=0 and enough space to append y's items, we append in place
    // otherwise we create a new object and copy in x and y
    if (REF(x) || BUCKET_SIZE(x) < b+HEADER_SIZE){
        x = xiy(tn(t,n), 0, x);
    }
    else {
        HDR_CNT(x) = n, HDR_TYP(x) = t;
    }

    return TAG_TYP(y) ? (K)memcpy(CHR(x)+s*m,&y,s) : xiy(x,m,y);
}

K jk(K x, K y){
    return j2(x, k1(y));
}

// join list of strings x with separator c
K js(K x, char c){
    // if c!=0 we need xn extra chars for the separators
    i64 xn=CNT(x), n=c?xn:0;
    // add the length of each string
    for (i64 i=0; i<xn; i++) n+=CNT(OBJ(x)[i]);
    // create return object
    K r=tn(KC,n), t;
    // ignore last separator
    if (c) --HDR_CNT(r);
    u8 *s=CHR(r);
    // append strings and separator
    for (i64 i=0; i<xn; i++){
        t=OBJ(x)[i];
        n=CNT(t);
        s=n+(u8*)memcpy(s,CHR(t),n);
        if (c) *s++ = c;
    }
    return UNREF_X(r);
}

// vector from atom. assumes atomic argument
K va(K x){
    i8 rt=TAG_TYP(x)?TAG_TYP(x):-HDR_TYP(x);
    K r=(K)memcpy(CHR(tn(rt,1)), TAG_TYP(x)?CHR(&x):CHR(x), TYPE_SIZE[rt]);
    return unref(x), r;
}

// enlist x
K k1(K x){
    K r = tn(KK, 1);
    *OBJ(r) = x;
    return r;
}

// return (x;y)
K k2(K x, K y){
    K r = tn(KK, 2);
    OBJ(r)[0] = x, OBJ(r)[1] = y;
    return r;
}

// return (x;y;z)
K k3(K x, K y, K z){
    K r = tn(KK, 3);
    OBJ(r)[0] = x, OBJ(r)[1] = y, OBJ(r)[2] = z;
    return r;
}

// create byte atom
K kx(u8 c){ K r; return r=tn(-KX,1), *CHR(r)=c, r; }
// create 2-byte vector
K kx2(u8 x,u8 y){ K r; return r=tn(KX,2), *CHR(r)=x, CHR(r)[1]=y, r; }

// create char atom
K kc(u8 c){ K r; return r=tn(-KC,1), *CHR(r)=c, r; }

// create char vector with count n from string
K kCn(char *s, i64 n){
    return (K)memcpy((void*)tn(KC,n),s,n);
}
// create char vector with 0-terminated string
K kC0(char *s){
    return kCn(s,strlen(s));
}

// create int atom
K ki(i64 x){
    K r = tn(-KI, 1);
    INT(r)[0] = x;
    return r;
}

// create float atom
K kf(double x){
    K r = tn(-KF, 1);
    FLT(r)[0] = x;
    return r;
}

// create int atom
K ks(i64 x){
    K r = tn(-KS, 1);
    INT(r)[0] = x;
    return r;
}

// make dict
K kD(K x, K y){
    return tx(KD,k2(x,y));
}

// create monadic verb
K ku(u64 i){ return SET_TAG(KU, i); }

// create dyadic verb
K kv(u64 i){ return SET_TAG(KV, i); }

// create adverb
K kw(i8 t){ return SET_TAG(KW, t); }

// create adverb-modified object 
K kwx(i8 t, K x){
    K r = tn(t, 1);
    *OBJ(r) = x;
    return r;
}

u8    cverb(u8 i){ return   VERB_STR[i]; }
u8  cadverb(u8 i){ return ADVERB_STR[i]; }
u64 iadverb(char c){ return ic((char*)ADVERB_STR,c); }
K kuc(char c){ return ku(ic((char*)VERB_STR,c)); }
K kvc(char c){ return kv(ic((char*)VERB_STR,c)); }

// create magic value (elided list/function args)
K km(){ return SET_TAG(KM,0); } 

// create tagged pointer to error string
K kerr(K x){ return SET_TAG(KE,x); }

// decrement K object refcount and place it back in M if no longer referenced
void unref(K x){
    // if ref not 0, decrement and return
    if (TAG_TYP(x) || REF(x)--) return;
    
    // if generic K object, unref child objects
    if (IS_GENERIC(x))
        for (i64 i=0, n=CNT(x); i<n; i++) unref( OBJ(x)[i] );

#ifdef DBG_WS
    WS -= BUCKET_SIZE(x);
#endif

    // link top of free list, then set object as top of free list in M
    *OBJ(x) = M[MEM(x)];
    M[MEM(x)] = x;
}

// increment refcount
K ref(K x){
    return TAG_TYP(x) ? x : (REF(x)++, x);
}

// squeeze into more compact form if possible
// ("a";"b") -> "ab"
K squeeze(K x){
    K r;
    i8 t, rt;
    i64 n = CNT(x);

    // if not a general list or count=0, return x
    if (TYP(x) || !n) return x;

    // if 1st element not atom, return x
    t = TYP( *OBJ(x) );
    if (0<=t) return x;

    // if not all same type, return x
    for (i64 i=1; i<n; i++) 
        if (t!=TYP( OBJ(x)[i] )) return x;

    rt = -t;
    r = tn(rt, n);
    switch(rt){
    case KI:
    case KS: for (i64 i=0; i<n; i++) INT(r)[i] = *INT(OBJ(x)[i]); break; 
    case KC: for (i64 i=0; i<n; i++) CHR(r)[i] = *CHR(OBJ(x)[i]); break;
    case KF: for (i64 i=0; i<n; i++) FLT(r)[i] = *FLT(OBJ(x)[i]); break;
    default: printf("'squeeze! type %d unsupported\n",rt), exit(0);
    }

    return unref(x), r;
}

// printer functions //

static void printInt(K x){
    for (i64 i=0, n=CNT(x), last=n-1; i<n; i++){
        i64 j=INT(x)[i];
        INULL==j ? printf("0N") : printf("%ld", j);
        if (i != last) putchar(' ');
    }
}

static void printFlt(K x){
    for (i64 i=0, n=CNT(x), last=n-1; i<n; i++){
        double f=FLT(x)[i];
        f!=f ? printf("0n") : printf("%f", f);
        if (i != last) putchar(' ');
    }
}

static void printSym(K x){
    // syms are encoded in i64, so max 8 chars
    char s[10] = {'`', 0, 0, 0, 0, 0, 0, 0, 0, 0};
    for (i64 i=0, n=CNT(x), b=sizeof(i64); i<n; i++){
        memcpy(&s[1], (void*)(INT(x)+i), b);
        printf("%s", s);
    }
}

static void printAdverb(K x){
    i8 t = TYP(x);
    printf("(%c", ADVERB_STR[t - K_ADVERB_START]);
    if (t >= KEP) putchar(':');
    putchar(';');
    _printK( *OBJ(x) );
    putchar(')');
}

static void _printK(K x){
    i8  t = IS_ADVERB_MOD(x) ? K_ADVERB_START : TYP(x);
    if (KL==t){ x=*OBJ(x); }
    i64 n = CNT(x);
    
    if (1==n && (!t || IS_SIMPLE_LIST(x))) putchar(',');

    switch(ABS(t)){
    case KK: if(1!=n)putchar('('); for (i64 i=0, last=n-1; i<n; i++){ _printK( OBJ(x)[i] ); if(i!=last)putchar(';'); } if(1!=n)putchar(')'); break;
    case KX: printf("0x"); for (i64 i=0; i<n; i++){ printf("%02x",CHR(x)[i]); } break;
    case KC: putchar('"'); for (i64 i=0; i<n; i++){ putchar(CHR(x)[i]); } putchar('"'); break;
    case KI: printInt(x); break;
    case KF: printFlt(x); break;
    case KS: printSym(x); break;
    case KD: _printK(*OBJ(x)), putchar('!'), _printK(OBJ(x)[1]); break;
    case KU: putchar(VERB_STR[TAG_VAL(x)]); putchar(':'); break;
    case KV: putchar(VERB_STR[TAG_VAL(x)]); break;
    case KW: putchar(ADVERB_STR[TAG_VAL(x)]); if (2<TAG_VAL(x)) putchar(':'); break;
    case K_ADVERB_START: printAdverb(x); break;
    case KE: x=TAG_VAL(x), n=CNT(x); /*FALLTHROUGH*/
    case KL: fwrite(CHR(x), sizeof(char), n, stdout); break;
    case KM: break;
    default: printf("'nyi! print type %d\n", TYP(x));
    }
}

// error objects are special, they're the only true tagged pointer
// upper 8 bits == KE, lower 56 is pointer to (lists of) string
// tagging is useful so the object isn't freed as it propogates up the stack
K printErr(K x){
    // grab the pointer
    x=TAG_VAL(x);
    // if it points to list, join them with newlines
    if (KK==TYP(x)) x=js(x,'\n');
    // print
    _printK(kerr(x));
    // return
    return UNREF_X(ke());
}

K printK(K x){
    if (IS_NULL(x))  return x;
    if (IS_ERROR(x))
        x=printErr(x);
    else 
        _printK(x);
    putchar('\n');
    return unref(x), x;
}
