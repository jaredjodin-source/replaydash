#include "Macro.hpp"

namespace replaydash {

void Macro::clear() {
    events.clear();
}

bool Macro::empty() const {
    return events.empty();
}

}
