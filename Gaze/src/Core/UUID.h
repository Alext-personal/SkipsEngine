#pragma once
#include <memory>
namespace Gaze {
	class UUID {
	public:
		UUID();
		UUID(uint64_t,bool copy = false);
		uint64_t Get() const { return m_id; }
		bool operator==(const UUID& id) const { return m_id == id.m_id; }
		friend std::ostream& operator<<(std::ostream& os,const UUID& id ){
			os << id.m_id;
			return os;
		}
		bool GetFlag() const { return m_id & (1ULL << 63); }
		std::string ToString() const;
	private:
		uint64_t m_id;
	};
	namespace ReservedUUID {
		inline const UUID NONE(0);
		inline const UUID TRIANGLE(1);
		inline const UUID QUAD(2);
		inline const UUID CUBE(3);
		inline const UUID DEFAULTSHADER(10);
		inline const UUID DEFAULTTEXTURE(20);
		inline const UUID DEFAULTMATERIAL(30);
	}
}
namespace std {
	template<>
	struct hash<Gaze::UUID> {
		size_t operator()(const Gaze::UUID& uuid) const {
			return hash<uint64_t>()(uuid.Get());
		}
	};
}