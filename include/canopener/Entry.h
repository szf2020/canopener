#pragma once
#include <cstdint>
#include <cstring>

namespace canopener {
	class Entry {
	public:
		enum Type {
			INT32/*,
			FLOAT,
			BOOL*/
		};

		Entry(uint16_t index_, uint8_t subindex_);
		Entry& setType(Type type);

	    template <typename T>
	    Entry& set(T value) {
	        data.resize(sizeof(T));
	        std::memcpy(data.data(), &value, sizeof(T));
	        return *this;
	    }

	    template <typename T>
	    T get() {
	    	if (sizeof(T)>data.size())
	    		return 0;

	        T value;
	        std::memcpy(&value, data.data(), sizeof(T));
	        return value;
	    }

	    std::vector<uint8_t>& raw() { return data; }
	    void setRaw(uint8_t* v, size_t size) { data.resize(size); memcpy(data.data(),v,size); }

	private:
		Type type;
		uint16_t index;
		uint8_t subindex;
		std::vector<uint8_t> data;

		friend class Device;
	};
}
