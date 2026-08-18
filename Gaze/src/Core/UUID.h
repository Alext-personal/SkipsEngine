#pragma once
#include <memory>
namespace Gaze {
	class UUID {
	public:
		UUID();
		UUID(uint64_t);
		operator uint64_t() const { return m_id; }
		bool GetFlag() const { return m_id & (1ULL << 63); }
	private:
		uint64_t m_id;
	};
	namespace std {
		template<>
		struct hash<UUID> {
			size_t operator()(const UUID& uuid) const {
				return hash<uint64_t>()((uint64_t)uuid);
			}
		};
	}
}