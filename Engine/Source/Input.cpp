#include <Engine/Input.h>

namespace Engine {
	Modifier operator|(Modifier a, Modifier b) {
		return static_cast<Modifier>(static_cast<int>(a) | static_cast<int>(b));
	}

	Modifier operator&(Modifier a, Modifier b) {
		return static_cast<Modifier>(static_cast<int>(a) & static_cast<int>(b));
	}

	Modifier operator^(Modifier a, Modifier b) {
		return static_cast<Modifier>(static_cast<int>(a) ^ static_cast<int>(b));
	}

	Modifier operator~(Modifier a) {
		return static_cast<Modifier>(~static_cast<int>(a));
	}

	Modifier& operator|=(Modifier& a, Modifier b) {
		a = a | b;
		return a;
	}

	Modifier& operator&=(Modifier& a, Modifier b) {
		a = a & b;
		return a;
	}

	Modifier& operator^=(Modifier& a, Modifier b) {
		a = a ^ b;
		return a;
	}
}
