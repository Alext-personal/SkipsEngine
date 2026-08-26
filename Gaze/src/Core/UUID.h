#pragma once
#include <memory>
namespace Gaze {
	class UUID {
	public:
		UUID();
		UUID(uint64_t);
		operator uint64_t() const { return m_id; }
		bool operator==(const UUID& id) { return m_id == (uint64_t)id.m_id; }
		bool GetFlag() const { return m_id & (1ULL << 63); }
	private:
		uint64_t m_id;
	};
	namespace ReservedUUID {
		inline const UUID TRIANGLE(0);
		inline const UUID QUAD(1);
		inline const UUID CUBE(2);
		inline const UUID DEFAULTSHADER(10);
	}
}
namespace std {
	template<>
	struct hash<Gaze::UUID> {
		size_t operator()(const Gaze::UUID& uuid) const {
			return hash<uint64_t>()((uint64_t)uuid);
		}
	};
}