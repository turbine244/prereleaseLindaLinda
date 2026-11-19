#pragma once

/*
  Utilities for PCM <-> float conversion

    reader  : PCM buffer -> float sample buffer
    writer  : float sample buffer -> PCM buffer        
*/

#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>

//====----====----====----====----====----====----====----====---- TYPE

namespace pcm2Float4
{
  enum class ENUM_pcmFormat : int32_t
  {
    // wtf
    unsupported,

    // MSB base formats
    int16MSB,
    int24MSB,
    int32MSB,
    float32MSB,
    float64MSB,

    // MSB aligned formats
    int32MSB16,
    int32MSB18,
    int32MSB20,
    int32MSB24,

    // LSB base formats
    int16LSB,
    int24LSB,
    int32LSB,
    float32LSB,
    float64LSB,

    // LSB aligned formats
    int32LSB16,
    int32LSB18,
    int32LSB20,
    int32LSB24
  };

  using CALLBACK_vd_readPcmBuffer = void(*)(const void* src, float* dest, uint32_t spb);
  using CALLBACK_vd_writePcmBuffer = void(*)(const float* src, void* dest, uint32_t spb);
}

//====----====----====----====----====----====----====----====---- FDEC

namespace pcm2Float4
{
  static inline CALLBACK_vd_readPcmBuffer getPtr_pcmFloatReader(ENUM_pcmFormat format);
  static inline CALLBACK_vd_writePcmBuffer getPtr_pcmFloatWriter(ENUM_pcmFormat format);

  namespace /* private */
  {
    inline float _clipSample(float sample);
    inline uint16_t _to_word16MSB(const uint8_t* src);
    inline uint16_t _to_word16LSB(const uint8_t* src);
    inline uint32_t _to_word24MSB(const uint8_t* src);
    inline uint32_t _to_word24LSB(const uint8_t* src);
    inline uint32_t _to_word32MSB(const uint8_t* src);
    inline uint32_t _to_word32LSB(const uint8_t* src);
    inline uint64_t _to_word64MSB(const uint8_t* src);
    inline uint64_t _to_word64LSB(const uint8_t* src);
    inline void _vd_writeWord16MSB(uint8_t* dest, uint16_t value);
    inline void _vd_writeWord16LSB(uint8_t* dest, uint16_t value);
    inline void _vd_writeWord24MSB(uint8_t* dest, uint32_t value);
    inline void _vd_writeWord24LSB(uint8_t* dest, uint32_t value);
    inline void _vd_writeWord32MSB(uint8_t* dest, uint32_t value);
    inline void _vd_writeWord32LSB(uint8_t* dest, uint32_t value);
    inline void _vd_writeWord64MSB(uint8_t* dest, uint64_t value);
    inline void _vd_writeWord64LSB(uint8_t* dest, uint64_t value);
    inline void _vd_readBufferInt16MSB(const void* src, float* dest, uint32_t spb);
    inline void _vd_readBufferInt16LSB(const void* src, float* dest, uint32_t spb);
    inline void _vd_readBufferInt24MSB(const void* src, float* dest, uint32_t spb);
    inline void _vd_readBufferInt24LSB(const void* src, float* dest, uint32_t spb);
    inline void _vd_readBufferInt32MSB(const void* src, float* dest, uint32_t spb);
    inline void _vd_readBufferInt32LSB(const void* src, float* dest, uint32_t spb);
    inline void _vd_readBufferInt32MSBValid(const void* src, float* dest, uint32_t spb, uint32_t numBit);
    inline void _vd_readBufferInt32LSBValid(const void* src, float* dest, uint32_t spb, uint32_t numBit);
    inline void _vd_readBufferInt32MSB16(const void* src, float* dest, uint32_t spb);
    inline void _vd_readBufferInt32MSB18(const void* src, float* dest, uint32_t spb);
    inline void _vd_readBufferInt32MSB20(const void* src, float* dest, uint32_t spb);
    inline void _vd_readBufferInt32MSB24(const void* src, float* dest, uint32_t spb);
    inline void _vd_readBufferInt32LSB16(const void* src, float* dest, uint32_t spb);
    inline void _vd_readBufferInt32LSB18(const void* src, float* dest, uint32_t spb);
    inline void _vd_readBufferInt32LSB20(const void* src, float* dest, uint32_t spb);
    inline void _vd_readBufferInt32LSB24(const void* src, float* dest, uint32_t spb);
    inline void _vd_readBufferFloat32MSB(const void* src, float* dest, uint32_t spb);
    inline void _vd_readBufferFloat32LSB(const void* src, float* dest, uint32_t spb);
    inline void _vd_readBufferFloat64MSB(const void* src, float* dest, uint32_t spb);
    inline void _vd_readBufferFloat64LSB(const void* src, float* dest, uint32_t spb);
    inline void _vd_writeBufferInt16MSB(const float* src, void* dest, uint32_t spb);
    inline void _vd_writeBufferInt16LSB(const float* src, void* dest, uint32_t spb);
    inline void _vd_writeBufferInt24MSB(const float* src, void* dest, uint32_t spb);
    inline void _vd_writeBufferInt24LSB(const float* src, void* dest, uint32_t spb);
    inline void _vd_writeBufferInt32MSB(const float* src, void* dest, uint32_t spb);
    inline void _vd_writeBufferInt32LSB(const float* src, void* dest, uint32_t spb);
    inline void _vd_writeBufferInt32MSBValid(const float* src, void* dest, uint32_t spb, uint32_t numBit);
    inline void _vd_writeBufferInt32LSBValid(const float* src, void* dest, uint32_t spb, uint32_t numBit);
    inline void _vd_writeBufferInt32MSB16(const float* src, void* dest, uint32_t spb);
    inline void _vd_writeBufferInt32MSB18(const float* src, void* dest, uint32_t spb);
    inline void _vd_writeBufferInt32MSB20(const float* src, void* dest, uint32_t spb);
    inline void _vd_writeBufferInt32MSB24(const float* src, void* dest, uint32_t spb);
    inline void _vd_writeBufferInt32LSB16(const float* src, void* dest, uint32_t spb);
    inline void _vd_writeBufferInt32LSB18(const float* src, void* dest, uint32_t spb);
    inline void _vd_writeBufferInt32LSB20(const float* src, void* dest, uint32_t spb);
    inline void _vd_writeBufferInt32LSB24(const float* src, void* dest, uint32_t spb);
    inline void _vd_writeBufferFloat32MSB(const float* src, void* dest, uint32_t spb);
    inline void _vd_writeBufferFloat32LSB(const float* src, void* dest, uint32_t spb);
    inline void _vd_writeBufferFloat64MSB(const float* src, void* dest, uint32_t spb);
    inline void _vd_writeBufferFloat64LSB(const float* src, void* dest, uint32_t spb);
  }
}

//====----====----====----====----====----====----====----====---- FDEF

namespace pcm2Float4
{
  static inline CALLBACK_vd_readPcmBuffer getPtr_pcmFloatReader(ENUM_pcmFormat format)
  {
    switch (format)
    {
      case ENUM_pcmFormat::int16MSB:   return _vd_readBufferInt16MSB;
      case ENUM_pcmFormat::int24MSB:   return _vd_readBufferInt24MSB;
      case ENUM_pcmFormat::int32MSB:   return _vd_readBufferInt32MSB;
      case ENUM_pcmFormat::float32MSB: return _vd_readBufferFloat32MSB;
      case ENUM_pcmFormat::float64MSB: return _vd_readBufferFloat64MSB;
      case ENUM_pcmFormat::int32MSB16: return _vd_readBufferInt32MSB16;
      case ENUM_pcmFormat::int32MSB18: return _vd_readBufferInt32MSB18;
      case ENUM_pcmFormat::int32MSB20: return _vd_readBufferInt32MSB20;
      case ENUM_pcmFormat::int32MSB24: return _vd_readBufferInt32MSB24;
      case ENUM_pcmFormat::int16LSB:   return _vd_readBufferInt16LSB;
      case ENUM_pcmFormat::int24LSB:   return _vd_readBufferInt24LSB;
      case ENUM_pcmFormat::int32LSB:   return _vd_readBufferInt32LSB;
      case ENUM_pcmFormat::float32LSB: return _vd_readBufferFloat32LSB;
      case ENUM_pcmFormat::float64LSB: return _vd_readBufferFloat64LSB;
      case ENUM_pcmFormat::int32LSB16: return _vd_readBufferInt32LSB16;
      case ENUM_pcmFormat::int32LSB18: return _vd_readBufferInt32LSB18;
      case ENUM_pcmFormat::int32LSB20: return _vd_readBufferInt32LSB20;
      case ENUM_pcmFormat::int32LSB24: return _vd_readBufferInt32LSB24;
      default: return nullptr;
    }
  }
  
  static inline CALLBACK_vd_writePcmBuffer getPtr_pcmFloatWriter(ENUM_pcmFormat format)
  {
    switch (format)
    {
      case ENUM_pcmFormat::int16MSB:   return _vd_writeBufferInt16MSB;
      case ENUM_pcmFormat::int24MSB:   return _vd_writeBufferInt24MSB;
      case ENUM_pcmFormat::int32MSB:   return _vd_writeBufferInt32MSB;
      case ENUM_pcmFormat::float32MSB: return _vd_writeBufferFloat32MSB;
      case ENUM_pcmFormat::float64MSB: return _vd_writeBufferFloat64MSB;
      case ENUM_pcmFormat::int32MSB16: return _vd_writeBufferInt32MSB16;
      case ENUM_pcmFormat::int32MSB18: return _vd_writeBufferInt32MSB18;
      case ENUM_pcmFormat::int32MSB20: return _vd_writeBufferInt32MSB20;
      case ENUM_pcmFormat::int32MSB24: return _vd_writeBufferInt32MSB24;
      case ENUM_pcmFormat::int16LSB:   return _vd_writeBufferInt16LSB;
      case ENUM_pcmFormat::int24LSB:   return _vd_writeBufferInt24LSB;
      case ENUM_pcmFormat::int32LSB:   return _vd_writeBufferInt32LSB;
      case ENUM_pcmFormat::float32LSB: return _vd_writeBufferFloat32LSB;
      case ENUM_pcmFormat::float64LSB: return _vd_writeBufferFloat64LSB;
      case ENUM_pcmFormat::int32LSB16: return _vd_writeBufferInt32LSB16;
      case ENUM_pcmFormat::int32LSB18: return _vd_writeBufferInt32LSB18;
      case ENUM_pcmFormat::int32LSB20: return _vd_writeBufferInt32LSB20;
      case ENUM_pcmFormat::int32LSB24: return _vd_writeBufferInt32LSB24;
      default: return nullptr;
    }
  }
  
  namespace /* private */
  {
    inline float _clipSample(float sample)
    {
      return std::clamp(sample, -1.0f, 1.0f);
    }

    inline uint16_t _to_word16MSB(const uint8_t* src)
    {
      return static_cast<uint16_t>(src[0]) << 8 | src[1];
    }

    inline uint16_t _to_word16LSB(const uint8_t* src)
    {
      return static_cast<uint16_t>(src[1]) << 8 | src[0];
    }

    inline uint32_t _to_word24MSB(const uint8_t* src)
    {
      return static_cast<uint32_t>(src[0]) << 16 | static_cast<uint32_t>(src[1]) << 8 | src[2];
    }

    inline uint32_t _to_word24LSB(const uint8_t* src)
    {
      return static_cast<uint32_t>(src[2]) << 16 | static_cast<uint32_t>(src[1]) << 8 | src[0];
    }

    inline uint32_t _to_word32MSB(const uint8_t* src)
    {
      return static_cast<uint32_t>(src[0]) << 24 | static_cast<uint32_t>(src[1]) << 16 | static_cast<uint32_t>(src[2]) << 8 | src[3];
    }

    inline uint32_t _to_word32LSB(const uint8_t* src)
    {
      return static_cast<uint32_t>(src[3]) << 24 | static_cast<uint32_t>(src[2]) << 16 | static_cast<uint32_t>(src[1]) << 8 | src[0];
    }

    inline uint64_t _to_word64MSB(const uint8_t* src)
    {
      uint64_t value = 0;
      for (uint32_t i = 0; i < 8; i++)
      {
        value = value << 8 | src[i];
      }
      return value;
    }

    inline uint64_t _to_word64LSB(const uint8_t* src)
    {
      uint64_t value = 0;
      for (uint32_t i = 0; i < 8; i++)
      {
        value |= static_cast<uint64_t>(src[i]) << (i * 8);
      }
      return value;
    }

    inline void _vd_writeWord16MSB(uint8_t* dest, uint16_t value)
    {
      dest[0] = static_cast<uint8_t>(value >> 8);
      dest[1] = static_cast<uint8_t>(value);
    }

    inline void _vd_writeWord16LSB(uint8_t* dest, uint16_t value)
    {
      dest[0] = static_cast<uint8_t>(value);
      dest[1] = static_cast<uint8_t>(value >> 8);
    }

    inline void _vd_writeWord24MSB(uint8_t* dest, uint32_t value)
    {
      dest[0] = static_cast<uint8_t>(value >> 16);
      dest[1] = static_cast<uint8_t>(value >> 8);
      dest[2] = static_cast<uint8_t>(value);
    }

    inline void _vd_writeWord24LSB(uint8_t* dest, uint32_t value)
    {
      dest[0] = static_cast<uint8_t>(value);
      dest[1] = static_cast<uint8_t>(value >> 8);
      dest[2] = static_cast<uint8_t>(value >> 16);
    }

    inline void _vd_writeWord32MSB(uint8_t* dest, uint32_t value)
    {
      dest[0] = static_cast<uint8_t>(value >> 24);
      dest[1] = static_cast<uint8_t>(value >> 16);
      dest[2] = static_cast<uint8_t>(value >> 8);
      dest[3] = static_cast<uint8_t>(value);
    }

    inline void _vd_writeWord32LSB(uint8_t* dest, uint32_t value)
    {
      dest[0] = static_cast<uint8_t>(value);
      dest[1] = static_cast<uint8_t>(value >> 8);
      dest[2] = static_cast<uint8_t>(value >> 16);
      dest[3] = static_cast<uint8_t>(value >> 24);
    }

    inline void _vd_writeWord64MSB(uint8_t* dest, uint64_t value)
    {
      for (uint32_t i = 0; i < 8; i++)
      {
        dest[7 - i] = static_cast<uint8_t>(value >> (i * 8));
      }
    }

    inline void _vd_writeWord64LSB(uint8_t* dest, uint64_t value)
    {
      for (uint32_t i = 0; i < 8; i++)
      {
        dest[i] = static_cast<uint8_t>(value >> (i * 8));
      }
    }

    inline void _vd_readBufferInt16MSB(const void* src, float* dest, uint32_t spb)
    {
      const uint8_t* srcByte = static_cast<const uint8_t*>(src);
      for (uint32_t i = 0; i < spb; i++)
      {
        dest[i] = static_cast<float>(static_cast<int16_t>(_to_word16MSB(srcByte + static_cast<std::size_t>(i) * 2))) / 32768.0f;
      }
    }

    inline void _vd_readBufferInt16LSB(const void* src, float* dest, uint32_t spb)
    {
      const uint8_t* srcByte = static_cast<const uint8_t*>(src);
      for (uint32_t i = 0; i < spb; i++)
      {
        dest[i] = static_cast<float>(static_cast<int16_t>(_to_word16LSB(srcByte + static_cast<std::size_t>(i) * 2))) / 32768.0f;
      }
    }

    inline void _vd_readBufferInt24MSB(const void* src, float* dest, uint32_t spb)
    {
      const uint8_t* srcByte = static_cast<const uint8_t*>(src);
      for (uint32_t i = 0; i < spb; i++)
      {
        uint32_t raw = _to_word24MSB(srcByte + static_cast<std::size_t>(i) * 3);
        int32_t value = (raw & 0x00800000u) != 0 ? static_cast<int32_t>(raw | 0xFF000000u) : static_cast<int32_t>(raw);
        dest[i] = static_cast<float>(value) / 8388608.0f;
      }
    }

    inline void _vd_readBufferInt24LSB(const void* src, float* dest, uint32_t spb)
    {
      const uint8_t* srcByte = static_cast<const uint8_t*>(src);
      for (uint32_t i = 0; i < spb; i++)
      {
        uint32_t raw = _to_word24LSB(srcByte + static_cast<std::size_t>(i) * 3);
        int32_t value = (raw & 0x00800000u) != 0 ? static_cast<int32_t>(raw | 0xFF000000u) : static_cast<int32_t>(raw);
        dest[i] = static_cast<float>(value) / 8388608.0f;
      }
    }

    inline void _vd_readBufferInt32MSB(const void* src, float* dest, uint32_t spb)
    {
      const uint8_t* srcByte = static_cast<const uint8_t*>(src);
      for (uint32_t i = 0; i < spb; i++)
      {
        dest[i] = static_cast<float>(static_cast<double>(static_cast<int32_t>(_to_word32MSB(srcByte + static_cast<std::size_t>(i) * 4))) / 2147483648.0);
      }
    }

    inline void _vd_readBufferInt32LSB(const void* src, float* dest, uint32_t spb)
    {
      const uint8_t* srcByte = static_cast<const uint8_t*>(src);
      for (uint32_t i = 0; i < spb; i++)
      {
        dest[i] = static_cast<float>(static_cast<double>(static_cast<int32_t>(_to_word32LSB(srcByte + static_cast<std::size_t>(i) * 4))) / 2147483648.0);
      }
    }

    inline void _vd_readBufferInt32MSBValid(const void* src, float* dest, uint32_t spb, uint32_t numBit)
    {
      const uint8_t* srcByte = static_cast<const uint8_t*>(src);
      uint32_t shift = 32 - numBit;
      for (uint32_t i = 0; i < spb; i++)
      {
        int32_t value = static_cast<int32_t>(_to_word32MSB(srcByte + static_cast<std::size_t>(i) * 4) << shift);
        dest[i] = static_cast<float>(static_cast<double>(value) / 2147483648.0);
      }
    }

    inline void _vd_readBufferInt32LSBValid(const void* src, float* dest, uint32_t spb, uint32_t numBit)
    {
      const uint8_t* srcByte = static_cast<const uint8_t*>(src);
      uint32_t shift = 32 - numBit;
      for (uint32_t i = 0; i < spb; i++)
      {
        int32_t value = static_cast<int32_t>(_to_word32LSB(srcByte + static_cast<std::size_t>(i) * 4) << shift);
        dest[i] = static_cast<float>(static_cast<double>(value) / 2147483648.0);
      }
    }

    inline void _vd_readBufferInt32MSB16(const void* src, float* dest, uint32_t spb)
    {
      _vd_readBufferInt32MSBValid(src, dest, spb, 16);
    }

    inline void _vd_readBufferInt32MSB18(const void* src, float* dest, uint32_t spb)
    {
      _vd_readBufferInt32MSBValid(src, dest, spb, 18);
    }

    inline void _vd_readBufferInt32MSB20(const void* src, float* dest, uint32_t spb)
    {
      _vd_readBufferInt32MSBValid(src, dest, spb, 20);
    }

    inline void _vd_readBufferInt32MSB24(const void* src, float* dest, uint32_t spb)
    {
      _vd_readBufferInt32MSBValid(src, dest, spb, 24);
    }

    inline void _vd_readBufferInt32LSB16(const void* src, float* dest, uint32_t spb)
    {
      _vd_readBufferInt32LSBValid(src, dest, spb, 16);
    }

    inline void _vd_readBufferInt32LSB18(const void* src, float* dest, uint32_t spb)
    {
      _vd_readBufferInt32LSBValid(src, dest, spb, 18);
    }

    inline void _vd_readBufferInt32LSB20(const void* src, float* dest, uint32_t spb)
    {
      _vd_readBufferInt32LSBValid(src, dest, spb, 20);
    }

    inline void _vd_readBufferInt32LSB24(const void* src, float* dest, uint32_t spb)
    {
      _vd_readBufferInt32LSBValid(src, dest, spb, 24);
    }

    inline void _vd_readBufferFloat32MSB(const void* src, float* dest, uint32_t spb)
    {
      const uint8_t* srcByte = static_cast<const uint8_t*>(src);
      for (uint32_t i = 0; i < spb; i++)
      {
        dest[i] = std::bit_cast<float>(_to_word32MSB(srcByte + static_cast<std::size_t>(i) * 4));
      }
    }

    inline void _vd_readBufferFloat32LSB(const void* src, float* dest, uint32_t spb)
    {
      std::memcpy(dest, src, sizeof(float) * spb);
    }

    inline void _vd_readBufferFloat64MSB(const void* src, float* dest, uint32_t spb)
    {
      const uint8_t* srcByte = static_cast<const uint8_t*>(src);
      for (uint32_t i = 0; i < spb; i++)
      {
        dest[i] = static_cast<float>(std::bit_cast<double>(_to_word64MSB(srcByte + static_cast<std::size_t>(i) * 8)));
      }
    }

    inline void _vd_readBufferFloat64LSB(const void* src, float* dest, uint32_t spb)
    {
      const double* srcDouble = static_cast<const double*>(src);
      for (uint32_t i = 0; i < spb; i++)
      {
        dest[i] = static_cast<float>(srcDouble[i]);
      }
    }

    inline void _vd_writeBufferInt16MSB(const float* src, void* dest, uint32_t spb)
    {
      uint8_t* destByte = static_cast<uint8_t*>(dest);
      for (uint32_t i = 0; i < spb; i++)
      {
        float sample = _clipSample(src[i]);
        int16_t value = sample >= 0.0f ? static_cast<int16_t>(sample * 32767.0f) : static_cast<int16_t>(sample * 32768.0f);
        _vd_writeWord16MSB(destByte + static_cast<std::size_t>(i) * 2, static_cast<uint16_t>(value));
      }
    }

    inline void _vd_writeBufferInt16LSB(const float* src, void* dest, uint32_t spb)
    {
      int16_t* destInteger = static_cast<int16_t*>(dest);
      for (uint32_t i = 0; i < spb; i++)
      {
        float sample = _clipSample(src[i]);
        destInteger[i] = sample >= 0.0f ? static_cast<int16_t>(sample * 32767.0f) : static_cast<int16_t>(sample * 32768.0f);
      }
    }

    inline void _vd_writeBufferInt24MSB(const float* src, void* dest, uint32_t spb)
    {
      uint8_t* destByte = static_cast<uint8_t*>(dest);
      for (uint32_t i = 0; i < spb; i++)
      {
        float sample = _clipSample(src[i]);
        int32_t value = sample >= 0.0f ? static_cast<int32_t>(sample * 8388607.0f) : static_cast<int32_t>(sample * 8388608.0f);
        _vd_writeWord24MSB(destByte + static_cast<std::size_t>(i) * 3, static_cast<uint32_t>(value));
      }
    }

    inline void _vd_writeBufferInt24LSB(const float* src, void* dest, uint32_t spb)
    {
      uint8_t* destByte = static_cast<uint8_t*>(dest);
      for (uint32_t i = 0; i < spb; i++)
      {
        float sample = _clipSample(src[i]);
        int32_t value = sample >= 0.0f ? static_cast<int32_t>(sample * 8388607.0f) : static_cast<int32_t>(sample * 8388608.0f);
        _vd_writeWord24LSB(destByte + static_cast<std::size_t>(i) * 3, static_cast<uint32_t>(value));
      }
    }

    inline void _vd_writeBufferInt32MSB(const float* src, void* dest, uint32_t spb)
    {
      uint8_t* destByte = static_cast<uint8_t*>(dest);
      for (uint32_t i = 0; i < spb; i++)
      {
        float sample = _clipSample(src[i]);
        int32_t value = sample >= 0.0f ? static_cast<int32_t>(static_cast<double>(sample) * 2147483647.0) : static_cast<int32_t>(static_cast<double>(sample) * 2147483648.0);
        _vd_writeWord32MSB(destByte + static_cast<std::size_t>(i) * 4, static_cast<uint32_t>(value));
      }
    }

    inline void _vd_writeBufferInt32LSB(const float* src, void* dest, uint32_t spb)
    {
      int32_t* destInteger = static_cast<int32_t*>(dest);
      for (uint32_t i = 0; i < spb; i++)
      {
        float sample = _clipSample(src[i]);
        destInteger[i] = sample >= 0.0f ? static_cast<int32_t>(static_cast<double>(sample) * 2147483647.0) : static_cast<int32_t>(static_cast<double>(sample) * 2147483648.0);
      }
    }

    inline void _vd_writeBufferInt32MSBValid(const float* src, void* dest, uint32_t spb, uint32_t numBit)
    {
      uint8_t* destByte = static_cast<uint8_t*>(dest);
      uint32_t shift = 32 - numBit;
      double maxPositive = static_cast<double>((uint64_t{ 1 } << (numBit - 1)) - 1);
      double maxNegative = static_cast<double>(uint64_t{ 1 } << (numBit - 1));
      for (uint32_t i = 0; i < spb; i++)
      {
        float sample = _clipSample(src[i]);
        int32_t value = sample >= 0.0f ? static_cast<int32_t>(sample * maxPositive) : static_cast<int32_t>(sample * maxNegative);
        _vd_writeWord32MSB(destByte + static_cast<std::size_t>(i) * 4, static_cast<uint32_t>(value) & (0xFFFFFFFFu >> shift));
      }
    }

    inline void _vd_writeBufferInt32LSBValid(const float* src, void* dest, uint32_t spb, uint32_t numBit)
    {
      uint8_t* destByte = static_cast<uint8_t*>(dest);
      uint32_t shift = 32 - numBit;
      double maxPositive = static_cast<double>((uint64_t{ 1 } << (numBit - 1)) - 1);
      double maxNegative = static_cast<double>(uint64_t{ 1 } << (numBit - 1));
      for (uint32_t i = 0; i < spb; i++)
      {
        float sample = _clipSample(src[i]);
        int32_t value = sample >= 0.0f ? static_cast<int32_t>(sample * maxPositive) : static_cast<int32_t>(sample * maxNegative);
        _vd_writeWord32LSB(destByte + static_cast<std::size_t>(i) * 4, static_cast<uint32_t>(value) & (0xFFFFFFFFu >> shift));
      }
    }

    inline void _vd_writeBufferInt32MSB16(const float* src, void* dest, uint32_t spb)
    {
      _vd_writeBufferInt32MSBValid(src, dest, spb, 16);
    }

    inline void _vd_writeBufferInt32MSB18(const float* src, void* dest, uint32_t spb)
    {
      _vd_writeBufferInt32MSBValid(src, dest, spb, 18);
    }

    inline void _vd_writeBufferInt32MSB20(const float* src, void* dest, uint32_t spb)
    {
      _vd_writeBufferInt32MSBValid(src, dest, spb, 20);
    }

    inline void _vd_writeBufferInt32MSB24(const float* src, void* dest, uint32_t spb)
    {
      _vd_writeBufferInt32MSBValid(src, dest, spb, 24);
    }

    inline void _vd_writeBufferInt32LSB16(const float* src, void* dest, uint32_t spb)
    {
      _vd_writeBufferInt32LSBValid(src, dest, spb, 16);
    }

    inline void _vd_writeBufferInt32LSB18(const float* src, void* dest, uint32_t spb)
    {
      _vd_writeBufferInt32LSBValid(src, dest, spb, 18);
    }

    inline void _vd_writeBufferInt32LSB20(const float* src, void* dest, uint32_t spb)
    {
      _vd_writeBufferInt32LSBValid(src, dest, spb, 20);
    }

    inline void _vd_writeBufferInt32LSB24(const float* src, void* dest, uint32_t spb)
    {
      _vd_writeBufferInt32LSBValid(src, dest, spb, 24);
    }

    inline void _vd_writeBufferFloat32MSB(const float* src, void* dest, uint32_t spb)
    {
      uint8_t* destByte = static_cast<uint8_t*>(dest);
      for (uint32_t i = 0; i < spb; i++)
      {
        _vd_writeWord32MSB(destByte + static_cast<std::size_t>(i) * 4, std::bit_cast<uint32_t>(_clipSample(src[i])));
      }
    }

    inline void _vd_writeBufferFloat32LSB(const float* src, void* dest, uint32_t spb)
    {
      float* destFloat = static_cast<float*>(dest);
      for (uint32_t i = 0; i < spb; i++)
      {
        destFloat[i] = _clipSample(src[i]);
      }
    }

    inline void _vd_writeBufferFloat64MSB(const float* src, void* dest, uint32_t spb)
    {
      uint8_t* destByte = static_cast<uint8_t*>(dest);
      for (uint32_t i = 0; i < spb; i++)
      {
        _vd_writeWord64MSB(destByte + static_cast<std::size_t>(i) * 8, std::bit_cast<uint64_t>(static_cast<double>(_clipSample(src[i]))));
      }
    }

    inline void _vd_writeBufferFloat64LSB(const float* src, void* dest, uint32_t spb)
    {
      double* destDouble = static_cast<double*>(dest);
      for (uint32_t i = 0; i < spb; i++)
      {
        destDouble[i] = _clipSample(src[i]);
      }
    }
  }
}
