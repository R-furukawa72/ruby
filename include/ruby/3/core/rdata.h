/**                                                     \noop-*-C++-*-vi:ft=cpp
 * @file
 * @author     Ruby developers <ruby-core@ruby-lang.org>
 * @copyright  This  file  is   a  part  of  the   programming  language  Ruby.
 *             Permission  is hereby  granted,  to  either redistribute  and/or
 *             modify this file, provided that  the conditions mentioned in the
 *             file COPYING are met.  Consult the file for details.
 * @warning    Symbols   prefixed   with   either  `RUBY3`   or   `ruby3`   are
 *             implementation details.   Don't take  them as canon.  They could
 *             rapidly appear then vanish.  The name (path) of this header file
 *             is also an  implementation detail.  Do not expect  it to persist
 *             at the place it is now.  Developers are free to move it anywhere
 *             anytime at will.
 * @note       To  ruby-core:  remember  that   this  header  can  be  possibly
 *             recursively included  from extension  libraries written  in C++.
 *             Do not  expect for  instance `__VA_ARGS__` is  always available.
 *             We assume C99  for ruby itself but we don't  assume languages of
 *             extension libraries. They could be written in C++98.
 * @brief      Defines struct ::RData.
 */
#ifndef  RUBY3_RDATA_H
#define  RUBY3_RDATA_H
#include "ruby/3/config.h"

#ifdef STDC_HEADERS
# include <stddef.h>
#endif

#include "ruby/3/attr/deprecated.h"
#include "ruby/3/attr/warning.h"
#include "ruby/3/cast.h"
#include "ruby/3/core/rbasic.h"
#include "ruby/3/dllexport.h"
#include "ruby/3/fl_type.h"
#include "ruby/3/token_paste.h"
#include "ruby/3/value.h"
#include "ruby/3/value_type.h"
#include "ruby/defines.h"

#ifdef RUBY_UNTYPED_DATA_WARNING
# /* Take that. */
#elif defined(RUBY_EXPORT)
# define RUBY_UNTYPED_DATA_WARNING 1
#else
# define RUBY_UNTYPED_DATA_WARNING 0
#endif

/** @cond INTERNAL_MACRO */
#define RUBY3_DATA_FUNC(f) RUBY3_CAST((void (*)(void *))(f))
#define RUBY3_ATTRSET_UNTYPED_DATA_FUNC() \
    RUBY3_ATTR_WARNING(("untyped Data is unsafe; use TypedData instead")) \
    RUBY3_ATTR_DEPRECATED(("by TypedData"))
/** @endcond */

#define RDATA(obj)                RUBY3_CAST((struct RData *)(obj))
#define DATA_PTR(obj)             RDATA(obj)->data
#define RUBY_MACRO_SELECT         RUBY3_TOKEN_PASTE
#define RUBY_DEFAULT_FREE         RUBY3_DATA_FUNC(-1)
#define RUBY_NEVER_FREE           RUBY3_DATA_FUNC(0)
#define RUBY_UNTYPED_DATA_FUNC(f) f RUBY3_ATTRSET_UNTYPED_DATA_FUNC()

/*
#define RUBY_DATA_FUNC(func) ((void (*)(void*))(func))
*/
typedef void (*RUBY_DATA_FUNC)(void*);

struct RData {
    struct RBasic basic;
    RUBY_DATA_FUNC dmark;
    RUBY_DATA_FUNC dfree;
    void *data;
};

RUBY3_SYMBOL_EXPORT_BEGIN()
VALUE rb_data_object_wrap(VALUE klass, void *datap, RUBY_DATA_FUNC dmark, RUBY_DATA_FUNC dfree);
VALUE rb_data_object_zalloc(VALUE klass, size_t size, RUBY_DATA_FUNC dmark, RUBY_DATA_FUNC dfree);
RUBY3_SYMBOL_EXPORT_END()

#define Data_Wrap_Struct(klass, mark, free, sval) \
    rb_data_object_wrap(                          \
        (klass),                                  \
        (sval),                                   \
        RUBY3_DATA_FUNC(mark),                    \
        RUBY3_DATA_FUNC(free))

#define Data_Make_Struct0(result, klass, type, size, mark, free, sval)  \
    VALUE result = rb_data_object_zalloc(          \
        (klass),                                   \
        (size),                                    \
        RUBY3_DATA_FUNC(mark),                     \
        RUBY3_DATA_FUNC(free));                    \
    (sval) = RUBY3_CAST((type *)DATA_PTR(result)); \
    RUBY3_CAST(/*suppress unused variable warnings*/(void)(sval))

#ifdef HAVE_STMT_AND_DECL_IN_EXPR
#define Data_Make_Struct(klass, type, mark, free, sval) \
    RB_GNUC_EXTENSION({      \
        Data_Make_Struct0(   \
            data_struct_obj, \
            klass,           \
            type,            \
            sizeof(type),    \
            mark,            \
            free,            \
            sval);           \
        data_struct_obj;     \
    })
#else
#define Data_Make_Struct(klass, type, mark, free, sval) \
    rb_data_object_make(              \
        (klass),                      \
        RUBY3_DATA_FUNC(mark),        \
        RUBY3_DATA_FUNC(free),        \
        RUBY3_CAST((void **)&(sval)), \
        sizeof(type))
#endif

#define Data_Get_Struct(obj, type, sval) \
    ((sval) = RUBY3_CAST((type*)rb_data_object_get(obj)))

RUBY3_ATTRSET_UNTYPED_DATA_FUNC()
static inline VALUE
rb_data_object_wrap_warning(VALUE klass, void *ptr, RUBY_DATA_FUNC mark, RUBY_DATA_FUNC free)
{
    return rb_data_object_wrap(klass, ptr, mark, free);
}

static inline void *
rb_data_object_get(VALUE obj)
{
    Check_Type(obj, RUBY_T_DATA);
    return DATA_PTR(obj);
}

RUBY3_ATTRSET_UNTYPED_DATA_FUNC()
static inline void *
rb_data_object_get_warning(VALUE obj)
{
    return rb_data_object_get(obj);
}

#if defined(HAVE_BUILTIN___BUILTIN_CHOOSE_EXPR_CONSTANT_P)
# define rb_data_object_wrap_warning(klass, ptr, mark, free) \
    RB_GNUC_EXTENSION(                                       \
        __builtin_choose_expr(                               \
            __builtin_constant_p(klass) && !(klass),         \
            rb_data_object_wrap(klass, ptr, mark, free),     \
            (rb_data_object_wrap_warning)(klass, ptr, mark, free)))
#endif

static inline VALUE
rb_data_object_make(VALUE klass, RUBY_DATA_FUNC mark_func, RUBY_DATA_FUNC free_func, void **datap, size_t size)
{
    Data_Make_Struct0(result, klass, void, size, mark_func, free_func, *datap);
    return result;
}

RUBY3_ATTR_DEPRECATED(("by: rb_data_object_wrap"))
static inline VALUE
rb_data_object_alloc(VALUE klass, void *data, RUBY_DATA_FUNC dmark, RUBY_DATA_FUNC dfree)
{
    return rb_data_object_wrap(klass, data, dmark, dfree);
}

#define rb_data_object_wrap_0 rb_data_object_wrap
#define rb_data_object_wrap_1 rb_data_object_wrap_warning
#define rb_data_object_wrap   RUBY_MACRO_SELECT(rb_data_object_wrap_, RUBY_UNTYPED_DATA_WARNING)
#define rb_data_object_get_0  rb_data_object_get
#define rb_data_object_get_1  rb_data_object_get_warning
#define rb_data_object_get    RUBY_MACRO_SELECT(rb_data_object_get_, RUBY_UNTYPED_DATA_WARNING)
#define rb_data_object_make_0 rb_data_object_make
#define rb_data_object_make_1 rb_data_object_make_warning
#define rb_data_object_make   RUBY_MACRO_SELECT(rb_data_object_make_, RUBY_UNTYPED_DATA_WARNING)
#endif /* RUBY3_RDATA_H */
