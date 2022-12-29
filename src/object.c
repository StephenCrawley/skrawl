#include "object.h"
#include <sys/mman.h>

// memory (buddy) allocation //

// memory is managed using a buddy allocation algorithm
// K objects occupy an x^2 block of memory
// the K object header is 16 bytes (see object.h for header layout) so objects need to be assigned a minimum of 16 bytes
// however 16B would only accommodate a 0-count object, and in practice we usually have objects with at least 1 element
// so instead the minimum size MIN_ALLOC is 32 bytes 
// TODO : this allocation scheme is performed for atoms and vectors. it is wasteful for atoms. we can improve this

// there is a global K array M
// this is where free memory buckets are stored
// an object in M can be a linked list
// suppose M[1] (64B block) is not a linked list, and a 64B object is "freed" with unref
// xK(M[1])[0] is set to the freed K object

#define HEADER_SIZE    16
#define MIN_ALLOC      32L 
#define BUCKET_SIZE(x) (u64)( MIN_ALLOC << MEM(x) ) //size of bucket containing object x
#define SIZEOF(x)      ( TYPE_SIZE[ABS(TYP(x))] )   //elemental size of x

// forward declarations
K k1(K x);
static void _printK(K x);

// mmap wrapper 
static void *alloc_n(u64 n){
    void *x = mmap(0,n,PROT_READ|PROT_WRITE,MAP_ANONYMOUS|MAP_PRIVATE,-1,0);
    if (x != MAP_FAILED)
        return x;
    else 
        printf("'memory! mmap failed. exiting...\n"), exit(1);
}

u64 WS=0;  // number of bytes allocated to objects (can be less than bytes requested from the OS)

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
        M[bucket] = *OBJ(x);
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

    // iteratively split the mmap'd block and add the new buckets to M
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

// size of each type     KK  KC  KI  KU  KV  '   /   \   ':  /:  \:
static i8 TYPE_SIZE[] = {8 , 1 , 8 , 1 , 1 , 8 , 8 , 8 , 8 , 8 , 8 };

// return a K object of type t and count n
K tn(i8 t, i64 n){
    K r = m1( MAX(1, n) * TYPE_SIZE[ABS(t)] );
    WS += BUCKET_SIZE(r);
    TYP(r) = t, REF(r) = 0, CNT(r) = n;
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

    i8  t = ABS(TYP(x));
    i64 m = CNT(x);
    i64 n = CNT(y) + m;
    u64 b = SIZEOF(x) * n; //number of bytes used by elements in joined object

    // if x has refcount=0 and enough space to append y's items, we append in place
    // otherwise we create a new object and copy in x and y
    if (REF(x) || BUCKET_SIZE(x) < b+HEADER_SIZE){
        x = xiy(tn(t,n), 0, x);
    }
    else {
        CNT(x) = n, TYP(x) = t;
    }

    return xiy(x, m, y);
}

K jk(K x, K y){
    return j2(x, k1(y));
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

// create int atom
K ki(i64 x){
    K r = tn(-KI, 1);
    INT(r)[0] = x;
    return r;
}

// create monadic verb
K ku(char c){
    K r = tn(KU, 1);
    char *s = VERB_STR;
    CHR(r)[0] = sc(s,c) - s;
    return r;
}

// create dyadic verb
K kv(char c){
    K r = tn(KV, 1);
    char *s = VERB_STR;
    CHR(r)[0] = sc(s,c) - s;
    return r;
}

// create adverb
K kw(i8 t, K x){
    K r = tn(t, 1);
    *OBJ(r) = x;
    return r;
}

// decrement K object refcount and place it back in M if no longer referenced
void unref(K x){
    // if ref not 0, decrement and return
    if (REF(x)--) return;
    
    // if generic K object, unref child objects
    if (IS_GENERIC_TYPE(x))
        for (i64 i=0, n=CNT(x); i<n; i++) unref( OBJ(x)[i] );

    // update used workspace, place object in the free list in M
    WS -= BUCKET_SIZE(x);
    *OBJ(x) = M[MEM(x)];
    M[MEM(x)] = x;
}

// increment refcount
K ref(K x){
    return REF(x)++, x;
}

// printer functions //

static void printInt(K x){
    for (i64 i=0, n=CNT(x), last=n-1; i<n; i++){
        printf("%ld", INT(x)[i]);
        if (i != last) putchar(' ');
    }
}

static void printAdverb(K x){
    i8 t = TYP(x);
    bool b = ( t >= KEP );
    if (b) t-=3;
    printf("(%c", ADVERB_STR[t - K_ADVERB_START]);
    if (b) putchar(':');
    putchar(';');
    _printK( *OBJ(x) );
    putchar(')');
}

static void _printK(K x){
    i8  t = TYP(x);
    i64 n = CNT(x);
    
    if (1==n && (!t || IS_SIMPLE_LIST(x))) putchar(',');

    switch(ABS(t)){
    case KK: putchar('('); for (i64 i=0, last=n-1; i<n; i++){ _printK( OBJ(x)[i] ); if(i!=last)putchar(';'); } putchar(')'); break;
    case KC: putchar('"'); for (i64 i=0; i<n; i++){ putchar(CHR(x)[i]); } putchar('"'); break;
    case KI: printInt(x); break;
    case KU: putchar(VERB_STR[*CHR(x)]); putchar(':'); break;
    case KV: putchar(VERB_STR[*CHR(x)]); break;
    case K_ADVERB_START ... K_ADVERB_END: printAdverb(x); break;
    default: printf("'nyi! print type %d\n", TYP(x));
    }
}

K printK(K x){
    _printK(x);
    putchar('\n');
    return x;
}
