#include "compat/inline_begin.h"
#include "compat/static_assert_begin.h"
#ifndef HasValue
#ifdef ValueType
#define HasValue 1
#else
#define HasValue 0
#endif
#endif
/*@self.public()*/
#include "btree_template_impl_g.h"
#undef HasValue
#include "compat/inline_end.h"
#include "compat/static_assert_end.h"
