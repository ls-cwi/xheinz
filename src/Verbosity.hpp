#ifndef xHeinz_Verbosity_HPP
#define xHeinz_Verbosity_HPP

namespace xHeinz {

 typedef enum { VERBOSE_NONE
              , VERBOSE_ESSENTIAL
              , VERBOSE_NON_ESSENTIAL
              , VERBOSE_DEBUG
              } VerbosityLevel;

 extern VerbosityLevel g_verbosity;

} // namespace xHeinz

#endif /* xHeinz_Verbosity_HPP */
