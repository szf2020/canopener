#pragma once
#include <vector>
#include <cstdint>
#include <cstring>
#include <stdexcept>

namespace canopener {
    class DataView {
    public:
        explicit DataView(std::vector<uint8_t>& buffer)
            : buf(buffer) {}

        uint8_t  getUint8 (size_t o) const { return read<uint8_t >(o, false); }
        int8_t   getInt8  (size_t o) const { return read<int8_t  >(o, false); }
        uint16_t getUint16(size_t o, bool le = false) const { return read<uint16_t>(o, le); }
        int16_t  getInt16 (size_t o, bool le = false) const { return read<int16_t >(o, le); }
        uint32_t getUint32(size_t o, bool le = false) const { return read<uint32_t>(o, le); }
        int32_t  getInt32 (size_t o, bool le = false) const { return read<int32_t >(o, le); }
        float    getFloat32(size_t o, bool le = false) const { return read<float >(o, le); }
        double   getFloat64(size_t o, bool le = false) const { return read<double>(o, le); }

        std::string getString() {
            if (!buf.empty() && buf.back()=='\0')
                return std::string(buf.begin(),buf.end()-1);

            return std::string(buf.begin(),buf.end());
        }

        void setUint8 (size_t o, uint8_t  v) { write<uint8_t >(o, v, false); }
        void setInt8  (size_t o, int8_t   v) { write<int8_t  >(o, v, false); }
        void setUint16(size_t o, uint16_t v, bool le = false) { write<uint16_t>(o, v, le); }
        void setInt16 (size_t o, int16_t  v, bool le = false) { write<int16_t >(o, v, le); }
        void setUint32(size_t o, uint32_t v, bool le = false) { write<uint32_t>(o, v, le); }
        void setInt32 (size_t o, int32_t  v, bool le = false) { write<int32_t >(o, v, le); }
        void setFloat32(size_t o, float  v, bool le = false) { write<float >(o, v, le); }
        void setFloat64(size_t o, double v, bool le = false) { write<double>(o, v, le); }

        void setString(std::string s) {
            buf.reserve(s.size()+1);
            for (char c: s)
                buf.push_back(static_cast<uint8_t>(c));
    
            buf.push_back(0);
        }

        size_t size() const { return buf.size(); }

    private:
        std::vector<uint8_t>& buf;

        void check(size_t o, size_t n) const {
            if (o + n > buf.size())
                throw std::out_of_range("DataView access out of range");
        }

        template<typename T>
        T read(size_t o, bool littleEndian) const {
            check(o, sizeof(T));

            T v;
            std::memcpy(&v, &buf[o], sizeof(T));

            if (sizeof(T) > 1 && littleEndian != isLittleEndian())
                byteSwap(v);

            return v;
        }

        template<typename T>
        void write(size_t o, T v, bool littleEndian) {
            check(o, sizeof(T));

            if (sizeof(T) > 1 && littleEndian != isLittleEndian())
                byteSwap(v);

            std::memcpy(&buf[o], &v, sizeof(T));
        }

        static bool isLittleEndian() {
            uint16_t x = 1;
            return *reinterpret_cast<uint8_t*>(&x) == 1;
        }

        template<typename T>
        static void byteSwap(T& v) {
            uint8_t* p = reinterpret_cast<uint8_t*>(&v);
            for (size_t i = 0, j = sizeof(T) - 1; i < j; ++i, --j)
                std::swap(p[i], p[j]);
        }
    };
}
