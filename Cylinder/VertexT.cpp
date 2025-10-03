#include <format>
#include "VertexT.hpp"

// 'UNPACK' expand into all three (member) array elements
#define WRAP(F, ...) F __VA_OPT__((__VA_ARGS__))
#define UNPACK(member, ...) \
WRAP(__VA_OPT__(__VA_ARGS__,) member[0]), \
WRAP(__VA_OPT__(__VA_ARGS__,) member[1]), \
WRAP(__VA_OPT__(__VA_ARGS__,) member[2])

/*  pass a wrapper-function to 'UNPACK' to apply it on each member
    UNPACK(name)      --> name[0], name[1], name[2]
    UNPACK(name, int) --> int(name[0]), int(name[1]), int(name[2])
*/

std::string VertexT::Representation() const { return \
    std::format("[{: >+5.2f} {: >+5.2f} {: >+5.2f}]", UNPACK(coord))+
    std::format(" rgb(#{:02X}{:02X}{:02X})", UNPACK(255*color, int));
}
/* coord format is: space-padded, right-aligned, sign, 5-char width, 2-decimal float
  five-character width accounts for 3 digits, sign, and decimal-place
  [-0.75 +0.67 +0.00] rgb(#FF0000)
  [+0.75 +0.67 +0.00] rgb(#00FF00)
  [+0.00 -0.75 +0.00] rgb(#0000FF) */
