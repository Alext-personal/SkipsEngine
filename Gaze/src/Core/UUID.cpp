#include "Core/UUID.h"
#include <random>
static std::random_device s_randomDevice;
static std::mt19937_64 s_engine(s_randomDevice());
static std::uniform_int_distribution<uint64_t> s_uniformDistribution;
UUID::UUID() //random uuid with last bit set to 1 (0 is rezerved for engine assets)
{
	m_id = s_uniformDistribution(s_engine);
	m_id |= (1ULL << 63); // set last bit  to 1
}
UUID::UUID(uint64_t id) { // non-random uuid with last bet set to 0(engine asset)
	m_id = id;
	m_id &= ~(1ULL << 63); // set last bit to 0
}
