#pragma once
#include <cstdint>
#include <cstring>
#include <cmath>
#include <algorithm>

namespace canopener {
	class Entry {
	public:
	    enum Type {
	        INT8, UINT8,
	        INT16, UINT16,
	        INT32, UINT32,
	        FLOAT32, BOOL,
	        STRING
	    };

		Entry(uint16_t index_, uint8_t subindex_);
		Entry& setType(Type type);

		template<typename T>
		void set(T val) {
		    if constexpr (std::is_same<T, std::string>::value) {
		        if (type != Type::STRING) return;
		        data.resize(val.size() + 1);       // +1 for null terminator
		        std::copy(val.begin(), val.end(), data.begin());
		        data.back() = 0;                   // null terminator
		    }

		    else if constexpr (std::is_arithmetic<T>::value) {
			    switch (type) {
			        case Type::FLOAT32: {
			            float fval = static_cast<float>(val);
			            uint32_t raw;
			            memcpy(&raw, &fval, 4);
			            setRawNum(raw);
			            break;
			        }

			        case Type::INT8:
			        case Type::INT16:
			        case Type::INT32:
			        case Type::UINT8:
			        case Type::UINT16:
			        case Type::UINT32: {
			            int64_t ival = 0;
			            if constexpr (std::is_floating_point<T>::value) {
			                ival = static_cast<int64_t>(std::round(val));
			            } else {
			                ival = static_cast<int64_t>(val);
			            }

			            uint64_t raw = 0;
			            switch (type) {
			                case Type::INT8:   raw = static_cast<int8_t>(ival); break;
			                case Type::INT16:  raw = static_cast<int16_t>(ival); break;
			                case Type::INT32:  raw = static_cast<int32_t>(ival); break;
			                case Type::UINT8:  raw = static_cast<uint8_t>(ival); break;
			                case Type::UINT16: raw = static_cast<uint16_t>(ival); break;
			                case Type::UINT32: raw = static_cast<uint32_t>(ival); break;
			                default: break;
			            }

			            setRawNum(raw);
			            break;
			        }

			        case Type::BOOL: {
			            data[0] = val ? 1 : 0;
			            break;
			        }

			        default:
			            break;
			    }
		    }
		}

		template<typename T>
	    T get() {
	        if constexpr (std::is_same<T, std::string>::value) {
	            if (type != Type::STRING || data.empty()) return {};
	            auto end = std::find(data.begin(), data.end(), 0);
	            return std::string(data.begin(), end);
	        } else if constexpr (std::is_floating_point<T>::value) {
	            if (type == Type::FLOAT32) {
	                uint32_t raw = getRawNum();
	                float fval;
	                memcpy(&fval, &raw, 4);
	                return static_cast<T>(fval);
	            } else { // integer type
	                return static_cast<T>(getRawNum());
	            }
	        } else if constexpr (std::is_integral<T>::value) {
	            if (type == Type::BOOL) {
	                return static_cast<T>(data[0] != 0);
	            } else { // integer type
	                return static_cast<T>(getRawNum());
	            }
	        }
	    }

	    std::vector<uint8_t>& raw() { return data; }
	    void setRaw(uint8_t* v, size_t size) { data.resize(size); memcpy(data.data(),v,size); }

	private:
		Type type;
		uint16_t index;
		uint8_t subindex;
		std::vector<uint8_t> data;
		void setRawNum(uint64_t raw);
		uint32_t getRawNum();
		int getTypeSize();

		friend class Device;
	};
}
