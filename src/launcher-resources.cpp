#include <gio/gio.h>

#if defined (__ELF__) && ( __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 6))
# define SECTION __attribute__ ((section (".gresource.resources"), aligned (8)))
#else
# define SECTION
#endif

#ifdef _MSC_VER
static const SECTION union { const guint8 data[445]; const double alignment; void * const ptr;}  resources_resource_data = { {
  0107, 0126, 0141, 0162, 0151, 0141, 0156, 0164, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 
  0030, 0000, 0000, 0000, 0164, 0000, 0000, 0000, 0000, 0000, 0000, 0050, 0003, 0000, 0000, 0000, 
  0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0001, 0000, 0000, 0000, 0134, 0344, 0252, 0003, 
  0002, 0000, 0000, 0000, 0164, 0000, 0000, 0000, 0013, 0000, 0166, 0000, 0200, 0000, 0000, 0000, 
  0246, 0001, 0000, 0000, 0324, 0265, 0002, 0000, 0377, 0377, 0377, 0377, 0246, 0001, 0000, 0000, 
  0001, 0000, 0114, 0000, 0250, 0001, 0000, 0000, 0254, 0001, 0000, 0000, 0336, 0276, 0201, 0304, 
  0001, 0000, 0000, 0000, 0254, 0001, 0000, 0000, 0012, 0000, 0114, 0000, 0270, 0001, 0000, 0000, 
  0274, 0001, 0000, 0000, 0114, 0141, 0165, 0156, 0143, 0150, 0145, 0162, 0056, 0165, 0151, 0000, 
  0121, 0002, 0000, 0000, 0001, 0000, 0000, 0000, 0170, 0332, 0235, 0221, 0075, 0117, 0303, 0060, 
  0020, 0206, 0367, 0376, 0212, 0343, 0126, 0224, 0257, 0262, 0060, 0044, 0251, 0304, 0320, 0056, 
  0014, 0014, 0105, 0214, 0221, 0143, 0137, 0023, 0123, 0327, 0016, 0266, 0323, 0224, 0177, 0117, 
  0112, 0022, 0265, 0042, 0124, 0102, 0154, 0326, 0371, 0171, 0164, 0357, 0335, 0245, 0253, 0323, 
  0101, 0301, 0221, 0254, 0223, 0106, 0147, 0230, 0204, 0061, 0002, 0151, 0156, 0204, 0324, 0125, 
  0206, 0257, 0333, 0165, 0360, 0210, 0253, 0174, 0221, 0336, 0005, 0001, 0154, 0110, 0223, 0145, 
  0236, 0004, 0164, 0322, 0327, 0120, 0051, 0046, 0010, 0036, 0302, 0345, 0062, 0114, 0040, 0010, 
  0172, 0110, 0152, 0117, 0166, 0307, 0070, 0345, 0013, 0200, 0324, 0322, 0107, 0053, 0055, 0071, 
  0120, 0262, 0314, 0260, 0362, 0373, 0173, 0274, 0064, 0352, 0265, 0030, 0243, 0157, 0316, 0224, 
  0357, 0304, 0075, 0160, 0305, 0234, 0313, 0160, 0343, 0367, 0157, 0122, 0013, 0323, 0041, 0110, 
  0221, 0341, 0063, 0153, 0065, 0257, 0311, 0216, 0265, 0263, 0320, 0053, 0215, 0065, 0015, 0131, 
  0377, 0011, 0232, 0035, 0050, 0103, 0316, 0164, 0261, 0063, 0274, 0165, 0230, 0257, 0231, 0162, 
  0224, 0106, 0023, 0060, 0362, 0274, 0226, 0112, 0014, 0357, 0263, 0255, 0372, 0210, 0265, 0121, 
  0202, 0154, 0064, 0002, 0321, 0025, 0361, 0203, 0236, 0305, 0173, 0261, 0246, 0352, 0307, 0162, 
  0117, 0314, 0016, 0031, 0247, 0002, 0116, 0316, 0074, 0341, 0121, 0072, 0131, 0052, 0302, 0174, 
  0153, 0333, 0131, 0274, 0377, 0214, 0364, 0233, 0323, 0264, 0075, 0130, 0070, 0117, 0015, 0346, 
  0161, 0030, 0047, 0177, 0161, 0134, 0155, 0272, 0302, 0323, 0311, 0337, 0212, 0226, 0106, 0303, 
  0002, 0146, 0213, 0272, 0174, 0244, 0321, 0325, 0341, 0277, 0000, 0061, 0144, 0271, 0072, 0000, 
  0050, 0165, 0165, 0141, 0171, 0051, 0057, 0000, 0002, 0000, 0000, 0000, 0162, 0145, 0163, 0157, 
  0165, 0162, 0143, 0145, 0163, 0057, 0000, 0000, 0000, 0000, 0000, 0000
} };
#else /* _MSC_VER */
static const SECTION union { const guint8 data[445]; const double alignment; void * const ptr;}  resources_resource_data = {
  "\107\126\141\162\151\141\156\164\000\000\000\000\000\000\000\000"
  "\030\000\000\000\164\000\000\000\000\000\000\050\003\000\000\000"
  "\000\000\000\000\000\000\000\000\001\000\000\000\134\344\252\003"
  "\002\000\000\000\164\000\000\000\013\000\166\000\200\000\000\000"
  "\246\001\000\000\324\265\002\000\377\377\377\377\246\001\000\000"
  "\001\000\114\000\250\001\000\000\254\001\000\000\336\276\201\304"
  "\001\000\000\000\254\001\000\000\012\000\114\000\270\001\000\000"
  "\274\001\000\000\114\141\165\156\143\150\145\162\056\165\151\000"
  "\121\002\000\000\001\000\000\000\170\332\235\221\075\117\303\060"
  "\020\206\367\376\212\343\126\224\257\262\060\044\251\304\320\056"
  "\014\014\105\214\221\143\137\023\123\327\016\266\323\224\177\117"
  "\112\022\265\042\124\102\154\326\371\171\164\357\335\245\253\323"
  "\101\301\221\254\223\106\147\230\204\061\002\151\156\204\324\125"
  "\206\257\333\165\360\210\253\174\221\336\005\001\154\110\223\145"
  "\236\004\164\322\327\120\051\046\010\036\302\345\062\114\040\010"
  "\172\110\152\117\166\307\070\345\013\200\324\322\107\053\055\071"
  "\120\262\314\260\362\373\173\274\064\352\265\030\243\157\316\224"
  "\357\304\075\160\305\234\313\160\343\367\157\122\013\323\041\110"
  "\221\341\063\153\065\257\311\216\265\263\320\053\215\065\015\131"
  "\377\011\232\035\050\103\316\164\261\063\274\165\230\257\231\162"
  "\224\106\023\060\362\274\226\112\014\357\263\255\372\210\265\121"
  "\202\154\064\002\321\025\361\203\236\305\173\261\246\352\307\162"
  "\117\314\016\031\247\002\116\316\074\341\121\072\131\052\302\174"
  "\153\333\131\274\377\214\364\233\323\264\075\130\070\117\015\346"
  "\161\030\047\177\161\134\155\272\302\323\311\337\212\226\106\303"
  "\002\146\213\272\174\244\321\325\341\277\000\061\144\271\072\000"
  "\050\165\165\141\171\051\057\000\002\000\000\000\162\145\163\157"
  "\165\162\143\145\163\057\000\000\000\000\000\000" };
#endif /* !_MSC_VER */

static GStaticResource static_resource = { resources_resource_data.data, sizeof (resources_resource_data.data) - 1 /* nul terminator */, NULL, NULL, NULL };
extern GResource *resources_get_resource (void);
GResource *resources_get_resource (void)
{
  return g_static_resource_get_resource (&static_resource);
}
/*
  If G_HAS_CONSTRUCTORS is true then the compiler support *both* constructors and
  destructors, in a sane way, including e.g. on library unload. If not you're on
  your own.

  Some compilers need #pragma to handle this, which does not work with macros,
  so the way you need to use this is (for constructors):

  #ifdef G_DEFINE_CONSTRUCTOR_NEEDS_PRAGMA
  #pragma G_DEFINE_CONSTRUCTOR_PRAGMA_ARGS(my_constructor)
  #endif
  G_DEFINE_CONSTRUCTOR(my_constructor)
  static void my_constructor(void) {
   ...
  }

*/

#ifndef __GTK_DOC_IGNORE__

#if  __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 7)

#define G_HAS_CONSTRUCTORS 1

#define G_DEFINE_CONSTRUCTOR(_func) static void __attribute__((constructor)) _func (void);
#define G_DEFINE_DESTRUCTOR(_func) static void __attribute__((destructor)) _func (void);

#elif defined (_MSC_VER) && (_MSC_VER >= 1500)
/* Visual studio 2008 and later has _Pragma */

#define G_HAS_CONSTRUCTORS 1

/* We do some weird things to avoid the constructors being optimized
 * away on VS2015 if WholeProgramOptimization is enabled. First we
 * make a reference to the array from the wrapper to make sure its
 * references. Then we use a pragma to make sure the wrapper function
 * symbol is always included at the link stage. Also, the symbols
 * need to be extern (but not dllexport), even though they are not
 * really used from another object file.
 */

/* We need to account for differences between the mangling of symbols
 * for Win32 (x86) and x64 programs, as symbols on Win32 are prefixed
 * with an underscore but symbols on x64 are not.
 */
#ifdef _WIN64
#define G_MSVC_SYMBOL_PREFIX ""
#else
#define G_MSVC_SYMBOL_PREFIX "_"
#endif

#define G_DEFINE_CONSTRUCTOR(_func) G_MSVC_CTOR (_func, G_MSVC_SYMBOL_PREFIX)
#define G_DEFINE_DESTRUCTOR(_func) G_MSVC_DTOR (_func, G_MSVC_SYMBOL_PREFIX)

#define G_MSVC_CTOR(_func,_sym_prefix) \
  static void _func(void); \
  extern int (* _array ## _func)(void);              \
  int _func ## _wrapper(void) { _func(); g_slist_find (NULL,  _array ## _func); return 0; } \
  __pragma(comment(linker,"/include:" _sym_prefix # _func "_wrapper")) \
  __pragma(section(".CRT$XCU",read)) \
  __declspec(allocate(".CRT$XCU")) int (* _array ## _func)(void) = _func ## _wrapper;

#define G_MSVC_DTOR(_func,_sym_prefix) \
  static void _func(void); \
  extern int (* _array ## _func)(void);              \
  int _func ## _constructor(void) { atexit (_func); g_slist_find (NULL,  _array ## _func); return 0; } \
   __pragma(comment(linker,"/include:" _sym_prefix # _func "_constructor")) \
  __pragma(section(".CRT$XCU",read)) \
  __declspec(allocate(".CRT$XCU")) int (* _array ## _func)(void) = _func ## _constructor;

#elif defined (_MSC_VER)

#define G_HAS_CONSTRUCTORS 1

/* Pre Visual studio 2008 must use #pragma section */
#define G_DEFINE_CONSTRUCTOR_NEEDS_PRAGMA 1
#define G_DEFINE_DESTRUCTOR_NEEDS_PRAGMA 1

#define G_DEFINE_CONSTRUCTOR_PRAGMA_ARGS(_func) \
  section(".CRT$XCU",read)
#define G_DEFINE_CONSTRUCTOR(_func) \
  static void _func(void); \
  static int _func ## _wrapper(void) { _func(); return 0; } \
  __declspec(allocate(".CRT$XCU")) static int (*p)(void) = _func ## _wrapper;

#define G_DEFINE_DESTRUCTOR_PRAGMA_ARGS(_func) \
  section(".CRT$XCU",read)
#define G_DEFINE_DESTRUCTOR(_func) \
  static void _func(void); \
  static int _func ## _constructor(void) { atexit (_func); return 0; } \
  __declspec(allocate(".CRT$XCU")) static int (* _array ## _func)(void) = _func ## _constructor;

#elif defined(__SUNPRO_C)

/* This is not tested, but i believe it should work, based on:
 * http://opensource.apple.com/source/OpenSSL098/OpenSSL098-35/src/fips/fips_premain.c
 */

#define G_HAS_CONSTRUCTORS 1

#define G_DEFINE_CONSTRUCTOR_NEEDS_PRAGMA 1
#define G_DEFINE_DESTRUCTOR_NEEDS_PRAGMA 1

#define G_DEFINE_CONSTRUCTOR_PRAGMA_ARGS(_func) \
  init(_func)
#define G_DEFINE_CONSTRUCTOR(_func) \
  static void _func(void);

#define G_DEFINE_DESTRUCTOR_PRAGMA_ARGS(_func) \
  fini(_func)
#define G_DEFINE_DESTRUCTOR(_func) \
  static void _func(void);

#else

/* constructors not supported for this compiler */

#endif

#endif /* __GTK_DOC_IGNORE__ */

#ifdef G_HAS_CONSTRUCTORS

#ifdef G_DEFINE_CONSTRUCTOR_NEEDS_PRAGMA
#pragma G_DEFINE_CONSTRUCTOR_PRAGMA_ARGS(resource_constructor)
#endif
G_DEFINE_CONSTRUCTOR(resource_constructor)
#ifdef G_DEFINE_DESTRUCTOR_NEEDS_PRAGMA
#pragma G_DEFINE_DESTRUCTOR_PRAGMA_ARGS(resource_destructor)
#endif
G_DEFINE_DESTRUCTOR(resource_destructor)

#else
#warning "Constructor not supported on this compiler, linking in resources will not work"
#endif

static void resource_constructor (void)
{
  g_static_resource_init (&static_resource);
}

static void resource_destructor (void)
{
  g_static_resource_fini (&static_resource);
}
