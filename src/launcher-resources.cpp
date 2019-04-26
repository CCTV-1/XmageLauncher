#include <gio/gio.h>

#if defined (__ELF__) && ( __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 6))
# define SECTION __attribute__ ((section (".gresource.resources"), aligned (8)))
#else
# define SECTION
#endif

#ifdef _MSC_VER
static const SECTION union { const guint8 data[381]; const double alignment; void * const ptr;}  resources_resource_data = { {
  0107, 0126, 0141, 0162, 0151, 0141, 0156, 0164, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 
  0030, 0000, 0000, 0000, 0164, 0000, 0000, 0000, 0000, 0000, 0000, 0050, 0003, 0000, 0000, 0000, 
  0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0001, 0000, 0000, 0000, 0134, 0344, 0252, 0003, 
  0002, 0000, 0000, 0000, 0164, 0000, 0000, 0000, 0013, 0000, 0166, 0000, 0200, 0000, 0000, 0000, 
  0145, 0001, 0000, 0000, 0324, 0265, 0002, 0000, 0377, 0377, 0377, 0377, 0145, 0001, 0000, 0000, 
  0001, 0000, 0114, 0000, 0150, 0001, 0000, 0000, 0154, 0001, 0000, 0000, 0336, 0276, 0201, 0304, 
  0001, 0000, 0000, 0000, 0154, 0001, 0000, 0000, 0012, 0000, 0114, 0000, 0170, 0001, 0000, 0000, 
  0174, 0001, 0000, 0000, 0114, 0141, 0165, 0156, 0143, 0150, 0145, 0162, 0056, 0165, 0151, 0000, 
  0126, 0001, 0000, 0000, 0001, 0000, 0000, 0000, 0170, 0332, 0225, 0217, 0261, 0156, 0002, 0061, 
  0020, 0104, 0173, 0276, 0142, 0263, 0155, 0344, 0063, 0220, 0206, 0302, 0076, 0072, 0150, 0122, 
  0046, 0112, 0031, 0031, 0173, 0271, 0063, 0230, 0065, 0261, 0175, 0041, 0371, 0373, 0030, 0070, 
  0004, 0155, 0272, 0325, 0352, 0075, 0315, 0214, 0132, 0376, 0034, 0002, 0174, 0123, 0312, 0076, 
  0262, 0306, 0131, 0063, 0105, 0040, 0266, 0321, 0171, 0356, 0064, 0276, 0277, 0255, 0304, 0002, 
  0227, 0355, 0104, 0075, 0011, 0001, 0153, 0142, 0112, 0246, 0220, 0203, 0223, 0057, 0075, 0164, 
  0301, 0070, 0202, 0227, 0146, 0076, 0157, 0146, 0040, 0104, 0205, 0074, 0027, 0112, 0133, 0143, 
  0251, 0235, 0000, 0250, 0104, 0137, 0203, 0117, 0224, 0041, 0370, 0215, 0306, 0256, 0354, 0237, 
  0361, 0036, 0124, 0265, 0051, 0312, 0013, 0027, 0067, 0073, 0262, 0005, 0154, 0060, 0071, 0153, 
  0134, 0227, 0375, 0207, 0147, 0027, 0117, 0010, 0336, 0151, 0174, 0065, 0003, 0333, 0236, 0322, 
  0370, 0073, 0013, 0125, 0071, 0246, 0170, 0244, 0124, 0176, 0201, 0315, 0201, 0064, 0132, 0303, 
  0237, 0333, 0150, 0207, 0214, 0355, 0312, 0204, 0114, 0112, 0336, 0200, 0221, 0267, 0275, 0017, 
  0356, 0172, 0237, 0355, 0120, 0053, 0366, 0061, 0070, 0112, 0162, 0004, 0344, 0003, 0361, 0037, 
  0132, 0311, 0153, 0373, 0072, 0136, 0076, 0254, 0377, 0003, 0232, 0366, 0147, 0353, 0000, 0050, 
  0165, 0165, 0141, 0171, 0051, 0057, 0000, 0000, 0002, 0000, 0000, 0000, 0162, 0145, 0163, 0157, 
  0165, 0162, 0143, 0145, 0163, 0057, 0000, 0000, 0000, 0000, 0000, 0000
} };
#else /* _MSC_VER */
static const SECTION union { const guint8 data[381]; const double alignment; void * const ptr;}  resources_resource_data = {
  "\107\126\141\162\151\141\156\164\000\000\000\000\000\000\000\000"
  "\030\000\000\000\164\000\000\000\000\000\000\050\003\000\000\000"
  "\000\000\000\000\000\000\000\000\001\000\000\000\134\344\252\003"
  "\002\000\000\000\164\000\000\000\013\000\166\000\200\000\000\000"
  "\145\001\000\000\324\265\002\000\377\377\377\377\145\001\000\000"
  "\001\000\114\000\150\001\000\000\154\001\000\000\336\276\201\304"
  "\001\000\000\000\154\001\000\000\012\000\114\000\170\001\000\000"
  "\174\001\000\000\114\141\165\156\143\150\145\162\056\165\151\000"
  "\126\001\000\000\001\000\000\000\170\332\225\217\261\156\002\061"
  "\020\104\173\276\142\263\155\344\063\220\206\302\076\072\150\122"
  "\046\112\031\031\173\271\063\230\065\261\175\041\371\373\030\070"
  "\004\155\272\325\352\075\315\214\132\376\034\002\174\123\312\076"
  "\262\306\131\063\105\040\266\321\171\356\064\276\277\255\304\002"
  "\227\355\104\075\011\001\153\142\112\246\220\203\223\057\075\164"
  "\301\070\202\227\146\076\157\146\040\104\205\074\027\112\133\143"
  "\251\235\000\250\104\137\203\117\224\041\370\215\306\256\354\237"
  "\361\036\124\265\051\312\013\027\067\073\262\005\154\060\071\153"
  "\134\227\375\207\147\027\117\010\336\151\174\065\003\333\236\322"
  "\370\073\013\125\071\246\170\244\124\176\201\315\201\064\132\303"
  "\237\333\150\207\214\355\312\204\114\112\336\200\221\267\275\017"
  "\356\172\237\355\120\053\366\061\070\112\162\004\344\003\361\037"
  "\132\311\153\373\072\136\076\254\377\003\232\366\147\353\000\050"
  "\165\165\141\171\051\057\000\000\002\000\000\000\162\145\163\157"
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
