#pragma once
template<class T>
inline uint32_t hash32(T item, uint32_t seed = 0) {
	return Hash::BOBHash64((uint8_t*)&item, sizeof(T), seed);
}


inline int Get_Median(int result[], uint32_t length) {
	std::sort(result, result + length);
	return (length & 1) ? result[length >> 1]
		: (result[length >> 1] + result[(length >> 1) - 1]) / 2;
}