#include "03_edge/audio/linda.asio/src/_h/01_api/narrative/_narr.openAudioRuntime.h"

#include <atomic>
#include <iostream>
#include <new>
#include <set>
#include <string>
#include <vector>

#include "00_vendored/asio_sdk/common/asiosys.h"
#include "00_vendored/asio_sdk/common/asio.h"
#include "00_vendored/asio_sdk/host/asiodrivers.h"
#include "02_shared/_utility/pcm2Float4.hpp"

using namespace std;

//====----====----====----====----====----====----====----====---- TYPE

namespace lindaAsio::narr::openAudioRuntime
{
  namespace /* private */
  {
    struct _STRUCT_audioRuntimeConfig
    {
      bool valid = false;
      string error;
      
      string nameDriver;
      bool usePreferredSpb = false;
      uint32_t spb = 0;
      set<uint32_t> setIdInputChannel;
      set<uint32_t> setIdOutputChannel;
    };
  }
}

//====----====----====----====----====----====----====----====---- FDEC

namespace lindaAsio::narr::openAudioRuntime
{
  namespace /* private */
  {
    _STRUCT_audioRuntimeConfig _narr_to_audioRuntimeConfig(const TYPE_json& runner);
    pcm2Float4::ENUM_pcmFormat _narr_getEnum_pcmFormat(ASIOSampleType type);
  }
}

//====----====----====----====----====----====----====----====---- FDEF

namespace lindaAsio::narr::openAudioRuntime
{
  // [1]
  TYPE_rc narr_rc_resolveConfig(STRUCT_context& edit_context)
  {
    TYPE_rc rci = 0;

    rci--;
    if (edit_context.input.isRuntimeActive)
    {
      vd_printErrorRc(rci, "Audio runtime is already active");
      return rci;
    }

    _STRUCT_audioRuntimeConfig config = _narr_to_audioRuntimeConfig(edit_context.input.jsonRunner);
    rci--;
    if (!config.valid)
    {
      vd_printErrorRc(rci, "Invalid ASIO runtime configuration: " + config.error);
      return rci;
    }

    edit_context.state.nameDriver = config.nameDriver;
    edit_context.state.usePreferredSpb = config.usePreferredSpb;
    edit_context.state.spb = config.spb;
    edit_context.state.setIdInputChannel = config.setIdInputChannel;
    edit_context.state.setIdOutputChannel = config.setIdOutputChannel;

    // END
    return 0;
  }

  // [2]
  TYPE_rc narr_rc_initializeRuntime(STRUCT_context& edit_context)
  {
    TYPE_rc rci = 0;

    rci--;
    edit_context.state.handle_asioRuntime = new (nothrow) runtime::HANDLE_asioRuntime();
    if (edit_context.state.handle_asioRuntime == nullptr)
    {
      vd_printErrorRc(rci, "Failed allocating ASIO handle");
      return rci;
    }

    // Load the configured ASIO driver.
    lindaAsio::runtime::vd_bindAsioDriverManager(&edit_context.state.handle_asioRuntime->driverManager);
    vector<char> nameDriver(edit_context.state.nameDriver.begin(), edit_context.state.nameDriver.end());
    nameDriver.push_back('\0');
    rci--;
    if (!edit_context.state.handle_asioRuntime->driverManager.loadDriver(nameDriver.data()))
    {
      vd_printErrorRc(rci, "ASIO LOAD failed");
      runtime::vd_releaseAsioRuntime(edit_context.state.handle_asioRuntime);
      return rci;
    }
    edit_context.state.handle_asioRuntime->asioState = runtime::const_asioStateDriverLoaded;

    // Initialize the loaded ASIO driver.
    ASIODriverInfo infoDriver = {};
    infoDriver.asioVersion = 2;
    infoDriver.sysRef = GetDesktopWindow();
    const ASIOError asioErrorInit = ASIOInit(&infoDriver);
    rci--;
    if (asioErrorInit != ASE_OK)
    {
      const string messageErrorVendor =
        "INIT failed ("
        + to_string(static_cast<int32_t>(asioErrorInit))
        + "): "
        + string(infoDriver.errorMessage);
      vd_printErrorVendor("ASIO", messageErrorVendor);
      vd_printErrorRc(rci, "ASIO INIT failed");
      runtime::vd_releaseAsioRuntime(edit_context.state.handle_asioRuntime);
      return rci;
    }
    edit_context.state.handle_asioRuntime->asioState = runtime::const_asioStateInitialized;

    // Read the current driver SPS.
    ASIOSampleRate currentSps = 0;
    const ASIOError asioErrorGetSampleRate = ASIOGetSampleRate(&currentSps);
    rci--;
    if (asioErrorGetSampleRate != ASE_OK || currentSps <= 0)
    {
      if (asioErrorGetSampleRate != ASE_OK)
      {
        vd_printErrorVendor("ASIO", "GETSAMPLERATE failed (" + to_string(static_cast<int32_t>(asioErrorGetSampleRate)) + ")");
      }
      vd_printErrorRc(rci, "ASIO GETSAMPLERATE failed");
      runtime::vd_releaseAsioRuntime(edit_context.state.handle_asioRuntime);
      return rci;
    }
    edit_context.state.handle_asioRuntime->sps.store(static_cast<uint32_t>(currentSps), memory_order_release);

    // Resolve the fixed SPB for this runtime.
    runtime::STRUCT_spbCapability spbCapability = {};
    const ASIOError asioErrorGetBufferSize = ASIOGetBufferSize(&spbCapability.min, &spbCapability.max, &spbCapability.preferred, &spbCapability.granularity);
    rci--;
    if (asioErrorGetBufferSize != ASE_OK || !runtime::is_spbCapabilityValid(spbCapability))
    {
      if (asioErrorGetBufferSize != ASE_OK)
      {
        vd_printErrorVendor("ASIO", "GETBUFFERSIZE failed (" + to_string(static_cast<int32_t>(asioErrorGetBufferSize)) + ")");
      }
      vd_printErrorRc(rci, "ASIO GETBUFFERSIZE failed");
      runtime::vd_releaseAsioRuntime(edit_context.state.handle_asioRuntime);
      return rci;
    }

    edit_context.state.spb = edit_context.state.usePreferredSpb
      ? static_cast<uint32_t>(spbCapability.preferred)
      : edit_context.state.spb;
    if (!runtime::is_spbSupported(spbCapability, static_cast<long>(edit_context.state.spb)))
    {
      cerr << "WARNING: Configured SPB is unsupported; using the driver-preferred value" << endl;
      edit_context.state.spb = static_cast<uint32_t>(spbCapability.preferred);
    }

    // Read the physical channel counts and remove unavailable requests.
    long numInputDriver = 0;
    long numOutputDriver = 0;
    const ASIOError asioErrorGetChannels = ASIOGetChannels(&numInputDriver, &numOutputDriver);
    rci--;
    if (asioErrorGetChannels != ASE_OK || numInputDriver < 0 || numOutputDriver < 0)
    {
      if (asioErrorGetChannels != ASE_OK)
      {
        vd_printErrorVendor("ASIO", "GETCHANNELS failed (" + to_string(static_cast<int32_t>(asioErrorGetChannels)) + ")");
      }
      vd_printErrorRc(rci, "ASIO GETCHANNELS failed");
      runtime::vd_releaseAsioRuntime(edit_context.state.handle_asioRuntime);
      return rci;
    }
    for (set<uint32_t>::iterator it = edit_context.state.setIdInputChannel.begin(); it != edit_context.state.setIdInputChannel.end();)
    {
      if (*it > static_cast<uint32_t>(numInputDriver))
      {
        cerr << "WARNING: Ignored unavailable ASIO input channel ID " << *it << endl;
        it = edit_context.state.setIdInputChannel.erase(it);
      }
      else
      {
        ++it;
      }
    }
    for (set<uint32_t>::iterator it = edit_context.state.setIdOutputChannel.begin(); it != edit_context.state.setIdOutputChannel.end();)
    {
      if (*it > static_cast<uint32_t>(numOutputDriver))
      {
        cerr << "WARNING: Ignored unavailable ASIO output channel ID " << *it << endl;
        it = edit_context.state.setIdOutputChannel.erase(it);
      }
      else
      {
        ++it;
      }
    }
    rci--;
    if (edit_context.state.setIdInputChannel.empty() && edit_context.state.setIdOutputChannel.empty())
    {
      vd_printErrorRc(rci, "No available ASIO channels remain");
      runtime::vd_releaseAsioRuntime(edit_context.state.handle_asioRuntime);
      return rci;
    }

    // END
    return 0;
  }

  // [3]
  TYPE_rc narr_rc_configureChannels(STRUCT_context& edit_context)
  {
    TYPE_rc rci = 0;

    // Build the ASIO buffer list.
    for (uint32_t idChannel : edit_context.state.setIdInputChannel)
    {
      ASIOBufferInfo bufferInfo = {};
      bufferInfo.isInput = ASIOTrue;
      bufferInfo.channelNum = static_cast<long>(idChannel - 1);
      edit_context.state.handle_asioRuntime->listIdxBufferInput.push_back(static_cast<uint32_t>(edit_context.state.handle_asioRuntime->listBufferInfo.size()));
      edit_context.state.handle_asioRuntime->listBufferInfo.push_back(bufferInfo);
    }
    for (uint32_t idChannel : edit_context.state.setIdOutputChannel)
    {
      ASIOBufferInfo bufferInfo = {};
      bufferInfo.isInput = ASIOFalse;
      bufferInfo.channelNum = static_cast<long>(idChannel - 1);
      edit_context.state.handle_asioRuntime->listIdxBufferOutput.push_back(static_cast<uint32_t>(edit_context.state.handle_asioRuntime->listBufferInfo.size()));
      edit_context.state.handle_asioRuntime->listBufferInfo.push_back(bufferInfo);
    }

    // RCI : read ASIO channel information
    rci--;
    const TYPE_rc codeChannelInfo = rci;

    // RCI : validate the PCM format
    rci--;
    const TYPE_rc codePcmFormat = rci;

    // Resolve the PCM format of each selected channel.
    for (const ASIOBufferInfo& bufferInfo : edit_context.state.handle_asioRuntime->listBufferInfo)
    {
      ASIOChannelInfo channelInfo = {};
      channelInfo.channel = bufferInfo.channelNum;
      channelInfo.isInput = bufferInfo.isInput;
      const ASIOError asioErrorGetChannelInfo = ASIOGetChannelInfo(&channelInfo);
      if (asioErrorGetChannelInfo != ASE_OK)
      {
        vd_printErrorVendor("ASIO", "GETCHANNELINFO failed (" + to_string(static_cast<int32_t>(asioErrorGetChannelInfo)) + ")");
        vd_printErrorRc(codeChannelInfo, "ASIO GETCHANNELINFO failed");
        runtime::vd_releaseAsioRuntime(edit_context.state.handle_asioRuntime);
        return codeChannelInfo;
      }

      const pcm2Float4::ENUM_pcmFormat pcmFormat = _narr_getEnum_pcmFormat(channelInfo.type);
      pcm2Float4::CALLBACK_vd_readPcmBuffer readBuffer = nullptr;
      pcm2Float4::CALLBACK_vd_writePcmBuffer writeBuffer = nullptr;
      if (bufferInfo.isInput == ASIOTrue)
      {
        readBuffer = pcm2Float4::getPtr_pcmFloatReader(pcmFormat);
      }
      else
      {
        writeBuffer = pcm2Float4::getPtr_pcmFloatWriter(pcmFormat);
      }
      if (readBuffer == nullptr && writeBuffer == nullptr)
      {
        vd_printErrorRc(codePcmFormat, "ASIO sample type is unsupported");
        runtime::vd_releaseAsioRuntime(edit_context.state.handle_asioRuntime);
        return codePcmFormat;
      }
      edit_context.state.handle_asioRuntime->listReadBuffer.push_back(readBuffer);
      edit_context.state.handle_asioRuntime->listWriteBuffer.push_back(writeBuffer);
    }

    edit_context.state.handle_asioRuntime->llistInputToSemanticIn.resize(edit_context.state.handle_asioRuntime->listBufferInfo.size());
    edit_context.state.handle_asioRuntime->llistOutputFromSemanticOut.resize(edit_context.state.handle_asioRuntime->listBufferInfo.size());
    edit_context.state.handle_asioRuntime->listBufferFloat.resize(edit_context.state.handle_asioRuntime->listBufferInfo.size() * static_cast<size_t>(edit_context.state.spb));

    // END
    return 0;
  }

  // [4]
  TYPE_rc narr_rc_buildRoutes(STRUCT_context& edit_context)
  {
    // Build physical-input to semantic-input routes.
    for (const TYPE_json& routeInput : edit_context.input.jsonRunner["wire"]["listLindaIn"])
    {
      for (const TYPE_json& valueChannel : routeInput["listIdSrc"])
      {
        const uint32_t idChannel = valueChannel.get<uint32_t>();
        const uint32_t idxChannel = idChannel - 1;
        for (const TYPE_json& valueSemantic : routeInput["listIdDest"])
        {
          const uint32_t idSemantic = valueSemantic.get<uint32_t>();
          const uint32_t idxSemantic = idSemantic - 1;
          for (uint32_t idxBuffer = 0; idxBuffer < edit_context.state.handle_asioRuntime->listBufferInfo.size(); idxBuffer++)
          {
            if (
              edit_context.state.handle_asioRuntime->listBufferInfo[idxBuffer].isInput == ASIOTrue
              && edit_context.state.handle_asioRuntime->listBufferInfo[idxBuffer].channelNum == static_cast<long>(idxChannel)
            )
            {
              edit_context.state.handle_asioRuntime->llistInputToSemanticIn[idxBuffer].push_back(idxSemantic);
              break;
            }
          }
        }
      }
    }

    // Build semantic-output to physical-output routes.
    for (const TYPE_json& routeOutput : edit_context.input.jsonRunner["wire"]["listLindaOut"])
    {
      for (const TYPE_json& valueSemantic : routeOutput["listIdSrc"])
      {
        const uint32_t idSemantic = valueSemantic.get<uint32_t>();
        const uint32_t idxSemantic = idSemantic - 1;
        for (const TYPE_json& valueChannel : routeOutput["listIdDest"])
        {
          const uint32_t idChannel = valueChannel.get<uint32_t>();
          const uint32_t idxChannel = idChannel - 1;
          for (uint32_t idxBuffer = 0; idxBuffer < edit_context.state.handle_asioRuntime->listBufferInfo.size(); idxBuffer++)
          {
            if (
              edit_context.state.handle_asioRuntime->listBufferInfo[idxBuffer].isInput == ASIOFalse
              && edit_context.state.handle_asioRuntime->listBufferInfo[idxBuffer].channelNum == static_cast<long>(idxChannel)
            )
            {
              edit_context.state.handle_asioRuntime->llistOutputFromSemanticOut[idxBuffer].push_back(idxSemantic);
              break;
            }
          }
        }
      }
    }

    // END
    return 0;
  }

  // [5]
  TYPE_rc narr_rc_createBuffers(STRUCT_context& edit_context)
  {
    TYPE_rc rci = 0;

    const ASIOError asioErrorCreateBuffers = ASIOCreateBuffers(edit_context.state.handle_asioRuntime->listBufferInfo.data(), static_cast<long>(edit_context.state.handle_asioRuntime->listBufferInfo.size()), static_cast<long>(edit_context.state.spb), &edit_context.state.handle_asioRuntime->asioCallbacks);
    rci--;
    if (asioErrorCreateBuffers != ASE_OK)
    {
      vd_printErrorVendor("ASIO", "CREATEBUFFERS failed (" + to_string(static_cast<int32_t>(asioErrorCreateBuffers)) + ")");
      vd_printErrorRc(rci, "ASIO CREATEBUFFERS failed");
      runtime::vd_releaseAsioRuntime(edit_context.state.handle_asioRuntime);
      return rci;
    }
    edit_context.state.handle_asioRuntime->asioState = runtime::const_asioStateBuffersCreated;

    // Initialize both halves of every output buffer with format-correct silence.
    for (uint32_t idxBuffer : edit_context.state.handle_asioRuntime->listIdxBufferOutput)
    {
      const float* bufferSilence = edit_context.state.handle_asioRuntime->listBufferFloat.data() + static_cast<size_t>(idxBuffer) * edit_context.state.spb;
      for (uint32_t idxHalf = 0; idxHalf < 2; idxHalf++)
      {
        void* bufferOutput = edit_context.state.handle_asioRuntime->listBufferInfo[idxBuffer].buffers[idxHalf];
        edit_context.state.handle_asioRuntime->listWriteBuffer[idxBuffer](bufferSilence, bufferOutput, edit_context.state.spb);
      }
    }

    // END
    return 0;
  }

//====----====----====----====----====----====----====----====---- FDEF

  namespace /* private */
  {
    _STRUCT_audioRuntimeConfig _narr_to_audioRuntimeConfig(const TYPE_json& runner)
    {
      _STRUCT_audioRuntimeConfig config;
      if (!runner.is_object() || !runner.contains("wire") || !runner["wire"].is_object())
      {
        config.error = "wire must be an object";
        return config;
      }

      const TYPE_json& wire = runner["wire"];
      if (!wire.contains("audio") || !wire["audio"].is_object())
      {
        config.error = "wire.audio must be an object";
        return config;
      }
      if (!wire.contains("listLindaIn") || !wire["listLindaIn"].is_array())
      {
        config.error = "wire.listLindaIn must be an array";
        return config;
      }
      if (!wire.contains("listLindaOut") || !wire["listLindaOut"].is_array())
      {
        config.error = "wire.listLindaOut must be an array";
        return config;
      }

      const TYPE_json& audio = wire["audio"];
      if (!audio.contains("meta") || !audio["meta"].is_object())
      {
        config.error = "wire.audio.meta must be an object";
        return config;
      }

      const TYPE_json& meta = audio["meta"];
      if (!meta.contains("driver") || !meta["driver"].is_string() || meta["driver"].get_ref<const string&>().empty())
      {
        config.error = "wire.audio.meta.driver must be a non-empty string";
        return config;
      }
      if (!meta.contains("usePreferred") || !meta["usePreferred"].is_boolean())
      {
        config.error = "wire.audio.meta.usePreferred must be boolean";
        return config;
      }
      if (!audio.contains("spb") || !is_jsonValueUint32(audio["spb"]))
      {
        config.error = "wire.audio.spb must be an unsigned 32-bit integer";
        return config;
      }
      config.spb = audio["spb"].get<uint32_t>();

      config.nameDriver = meta["driver"].get<string>();
      config.usePreferredSpb = meta["usePreferred"].get<bool>();
      if (!config.usePreferredSpb && config.spb == 0)
      {
        config.error = "wire.audio.spb must be greater than 0 when usePreferred is false";
        return config;
      }

      for (const TYPE_json& route : wire["listLindaIn"])
      {
        if (!route.is_object() || !route.contains("listIdSrc") || !route["listIdSrc"].is_array())
        {
          config.error = "each Linda-In route must contain a listIdSrc array";
          return config;
        }
        if (!route.contains("listIdDest") || !route["listIdDest"].is_array())
        {
          config.error = "each Linda-In route must contain a listIdDest array";
          return config;
        }

        for (const TYPE_json& value : route["listIdSrc"])
        {
          if (!is_jsonValueUint32(value) || value.get<uint32_t>() == 0)
          {
            config.error = "Linda-In listIdSrc must contain positive 32-bit IDs";
            return config;
          }
          const uint32_t id = value.get<uint32_t>();
          config.setIdInputChannel.insert(id);
        }
        for (const TYPE_json& value : route["listIdDest"])
        {
          if (!is_jsonValueUint32(value) || value.get<uint32_t>() == 0)
          {
            config.error = "Linda-In listIdDest must contain positive 32-bit IDs";
            return config;
          }
        }
      }

      for (const TYPE_json& route : wire["listLindaOut"])
      {
        if (!route.is_object() || !route.contains("listIdSrc") || !route["listIdSrc"].is_array())
        {
          config.error = "each Linda-Out route must contain a listIdSrc array";
          return config;
        }
        if (!route.contains("listIdDest") || !route["listIdDest"].is_array())
        {
          config.error = "each Linda-Out route must contain a listIdDest array";
          return config;
        }

        for (const TYPE_json& value : route["listIdSrc"])
        {
          if (!is_jsonValueUint32(value) || value.get<uint32_t>() == 0)
          {
            config.error = "Linda-Out listIdSrc must contain positive 32-bit IDs";
            return config;
          }
        }
        for (const TYPE_json& value : route["listIdDest"])
        {
          if (!is_jsonValueUint32(value) || value.get<uint32_t>() == 0)
          {
            config.error = "Linda-Out listIdDest must contain positive 32-bit IDs";
            return config;
          }
          const uint32_t id = value.get<uint32_t>();
          config.setIdOutputChannel.insert(id);
        }
      }

      config.valid = true;
      return config;
    }

    pcm2Float4::ENUM_pcmFormat _narr_getEnum_pcmFormat(ASIOSampleType type)
    {
      switch (type)
      {
        case ASIOSTInt16MSB: return pcm2Float4::ENUM_pcmFormat::int16MSB;
        case ASIOSTInt24MSB: return pcm2Float4::ENUM_pcmFormat::int24MSB;
        case ASIOSTInt32MSB: return pcm2Float4::ENUM_pcmFormat::int32MSB;
        case ASIOSTFloat32MSB: return pcm2Float4::ENUM_pcmFormat::float32MSB;
        case ASIOSTFloat64MSB: return pcm2Float4::ENUM_pcmFormat::float64MSB;
        case ASIOSTInt32MSB16: return pcm2Float4::ENUM_pcmFormat::int32MSB16;
        case ASIOSTInt32MSB18: return pcm2Float4::ENUM_pcmFormat::int32MSB18;
        case ASIOSTInt32MSB20: return pcm2Float4::ENUM_pcmFormat::int32MSB20;
        case ASIOSTInt32MSB24: return pcm2Float4::ENUM_pcmFormat::int32MSB24;
        case ASIOSTInt16LSB: return pcm2Float4::ENUM_pcmFormat::int16LSB;
        case ASIOSTInt24LSB: return pcm2Float4::ENUM_pcmFormat::int24LSB;
        case ASIOSTInt32LSB: return pcm2Float4::ENUM_pcmFormat::int32LSB;
        case ASIOSTFloat32LSB: return pcm2Float4::ENUM_pcmFormat::float32LSB;
        case ASIOSTFloat64LSB: return pcm2Float4::ENUM_pcmFormat::float64LSB;
        case ASIOSTInt32LSB16: return pcm2Float4::ENUM_pcmFormat::int32LSB16;
        case ASIOSTInt32LSB18: return pcm2Float4::ENUM_pcmFormat::int32LSB18;
        case ASIOSTInt32LSB20: return pcm2Float4::ENUM_pcmFormat::int32LSB20;
        case ASIOSTInt32LSB24: return pcm2Float4::ENUM_pcmFormat::int32LSB24;
        default: return pcm2Float4::ENUM_pcmFormat::unsupported;
      }
    }
  }
}
