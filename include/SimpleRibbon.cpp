#include "SimpleRibbon.h"
#ifdef SIMPLE_RIBBON
#include "../extlib/ribbon/ribbon.hpp"

template<size_t coeff_bits, size_t result_bits>
struct RetrievalConfig : public ribbon::RConfig<coeff_bits, result_bits,
        /* threshold mode */ (result_bits == 64) ? ribbon::ThreshMode::twobit : ribbon::ThreshMode::onebit,
        /* sparse */ false, /* interleaved */ true, /* cls */ false, /* bucket_sh */ 0, /* key type */ uint64_t> {
    static constexpr bool log = false;
    static constexpr bool kIsFilter = false;
    static constexpr bool kUseMHC = false;
};

template<size_t bits, size_t coeff_bits>
SimpleRibbon<bits, coeff_bits>::SimpleRibbon(std::vector<std::pair<uint64_t, uint8_t>> &data) {
    using Config = RetrievalConfig<coeff_bits, /*result_bits*/ bits>;
    using RibbonT = ribbon::ribbon_filter</*depth*/ 2, Config>;

    using namespace ribbon;
    IMPORT_RIBBON_CONFIG(Config);

    ribbon = new ribbon::ribbon_filter</*depth*/ 2, Config>(data.size(), 0.95, 42);
    static_cast<RibbonT*>(ribbon)->AddRange(data.begin(), data.end());
    static_cast<RibbonT*>(ribbon)->BackSubst();
}

template<size_t bits, size_t coeff_bits>
SimpleRibbon<bits, coeff_bits>::~SimpleRibbon() {
    using Config = RetrievalConfig<coeff_bits, /*result_bits*/ bits>;
    using RibbonT = ribbon::ribbon_filter</*depth*/ 2, Config>;

    if (ribbon != nullptr) {
        delete static_cast<RibbonT*>(ribbon);
    }
}

template<size_t bits, size_t coeff_bits>
uint8_t SimpleRibbon<bits, coeff_bits>::retrieve(uint64_t key) {
    using Config = RetrievalConfig<coeff_bits, /*result_bits*/ bits>;
    using RibbonT = ribbon::ribbon_filter</*depth*/ 2, Config>;

    return static_cast<RibbonT*>(ribbon)->QueryRetrieval(key);
}

template<size_t bits, size_t coeff_bits>
std::size_t SimpleRibbon<bits, coeff_bits>::size() {
    using Config = RetrievalConfig<coeff_bits, /*result_bits*/ bits>;
    using RibbonT = ribbon::ribbon_filter</*depth*/ 2, Config>;

    assert(static_cast<RibbonT*>(ribbon)->Size() > 0);
    return static_cast<RibbonT*>(ribbon)->Size();
}

template class SimpleRibbon<1, 32>;
template class SimpleRibbon<2, 32>;
template class SimpleRibbon<3, 32>;

template class SimpleRibbon<1, 64>;
template class SimpleRibbon<2, 64>;
template class SimpleRibbon<3, 64>;

#endif
