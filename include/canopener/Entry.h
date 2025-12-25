#pragma once
#include <cstdint>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <string>
#include "canopener/DataView.h"
#include "castx.h"

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
		void set (T v) {
            switch (type) {
                case Type::INT8:    view.setInt8(0,castx<int8_t,T>(v)); return;
                case Type::INT16:   view.setInt16(0,castx<int16_t,T>(v),true); return;
                case Type::INT32:   view.setInt32(0,castx<int32_t,T>(v),true); return;
                case Type::UINT8:   view.setUint8(0,castx<uint8_t,T>(v)); return;
                case Type::UINT16:  view.setUint16(0,castx<uint16_t,T>(v),true); return;
                case Type::UINT32:  view.setUint32(0,castx<uint32_t,T>(v),true); return;
                case Type::FLOAT32: view.setFloat32(0,castx<float,T>(v),true); return;
                case Type::BOOL:    view.setUint8(0,castx<uint8_t,T>(v)); return;
                case Type::STRING:  view.setString(castx<std::string,T>(v)); return;
                default: 
 					throw std::logic_error("Type missing on set...");

                break;
            }
		}

		template<typename T>
	    T get() {
            switch (type) {
                case Type::INT8:    return castx<T,int8_t>(view.getInt8(0));
                case Type::INT16:   return castx<T,int16_t>(view.getInt16(0,true));
                case Type::INT32:   return castx<T,int32_t>(view.getInt32(0,true));
                case Type::UINT8:   return castx<T,uint8_t>(view.getUint8(0));
                case Type::UINT16:  return castx<T,uint16_t>(view.getUint16(0,true));
                case Type::UINT32:  return castx<T,uint32_t>(view.getUint32(0,true));
                case Type::FLOAT32: return castx<T,float>(view.getFloat32(0,true));
                case Type::BOOL:    return castx<T,uint8_t>(view.getUint8(0));
                case Type::STRING:  return castx<T,std::string>(view.getString());
                default: 
 					throw std::logic_error("Type missing on get...");

                break;
            }
	    }

	    std::vector<uint8_t>& raw() { return data; }
	    void setRaw(uint8_t* v, size_t size) { data.resize(size); memcpy(data.data(),v,size); }

	private:
		Type type;
		uint16_t index;
		uint8_t subindex;
		std::vector<uint8_t> data;
		DataView view;
		void setRawNum(uint64_t raw);
		uint32_t getRawNum();
		int getTypeSize();

		friend class Device;
	};
}
