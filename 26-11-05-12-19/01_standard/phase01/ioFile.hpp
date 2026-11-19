#pragma once

/*
  Project-wide file-io utilities
*/

#include <array>
#include <chrono>
#include <cstdio>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>

#if defined(_WIN32)
#include <windows.h>
#endif

#include "01_standard/phase00/error.hpp"
#include "01_standard/phase00/json.hpp"

//====----====----====----====----====----====----====----====---- FDEC

static inline TYPE_rc rc_readFileText(const std::filesystem::path& pathFile, std::string& out_text);
static inline TYPE_rc rc_readFileJson(const std::filesystem::path& pathFile, TYPE_json& out_value);

static inline TYPE_rc rc_writeFileText(const std::filesystem::path& pathFile, const std::string& text);
static inline TYPE_rc rc_writeFileJson(const std::filesystem::path& pathFile, const TYPE_json& value);
static inline TYPE_rc rc_writeFileJsonAtomic(const std::filesystem::path& pathFile, const TYPE_json& value);

static inline bool is_relativePathSafe(const std::filesystem::path& path);

//====----====----====----====----====----====----====----====---- FDEF

static inline TYPE_rc rc_readFileText(const std::filesystem::path& pathFile, std::string& out_text)
{
  // NULLIFY : rc, out
  out_text.clear();
  TYPE_rc rci = 0;

  // RCI : open the file
  std::ifstream fileIn;
  if (true)
  {
    fileIn.open(pathFile, std::ios::binary);

    rci--;
    if (!fileIn)
    {
      vd_printErrorRc(rci, "Failed opening file: " + pathFile.string());
      return rci;
    }
  }

  // RCI : read the file
  std::string text;
  if (true)
  {
    std::array<char, 4096> buffer = {};
    while (fileIn.read(buffer.data(), static_cast<std::streamsize>(buffer.size())))
    {
      text.append(buffer.data(), buffer.size());
    }

    const std::streamsize sizeRead = fileIn.gcount();
    if (sizeRead > 0)
    {
      text.append(buffer.data(), static_cast<std::string::size_type>(sizeRead));
    }

    rci--;
    if (fileIn.bad() || (!fileIn.eof() && fileIn.fail()))
    {
      vd_printErrorRc(rci, "Failed reading file: " + pathFile.string());
      return rci;
    }
  }

  // END : out
  out_text = text;
  return 0;
}

static inline TYPE_rc rc_readFileJson(const std::filesystem::path& pathFile, TYPE_json& out_value)
{
  // NULLIFY : rc, out
  out_value = nullptr;
  TYPE_rc rce = 0;
  TYPE_rc rci = 0;

  // RCE : read the file as a text
  std::string text;
  if (true)
  {
    rce = to_abs(rc_readFileText(pathFile, text));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed reading JSON file: " + pathFile.string());
      return rce;
    }
  }

  // RCI : parse the text as JSON
  TYPE_json temp_out_value;
  if (true)
  {
    temp_out_value = TYPE_json::parse(text, nullptr, false);

    rci--;
    if (temp_out_value.is_discarded())
    {
      vd_printErrorRc(rci, "Failed parsing JSON file: " + pathFile.string());
      return rci;
    }
  }
  
  // END : out
  out_value = temp_out_value;
  return 0;
}

static inline TYPE_rc rc_writeFileText(const std::filesystem::path& pathFile, const std::string& text)
{
  // NULLIFY : rc, out
  TYPE_rc rci = 0;

  // RCI : create parent directory
  if (true)
  {
    const std::filesystem::path pathDirParent = pathFile.parent_path();    
    
    rci--;
    if (!pathDirParent.empty())
    {
      std::error_code ecCreateDir;
      std::filesystem::create_directories(pathDirParent, ecCreateDir);
      
      if (ecCreateDir)
      {
        vd_printErrorRc(rci, "Failed creating parent directory: " + pathDirParent.string());
        return rci;
      }
    }
  }

  // RCI : open the file
  std::ofstream fileOut;
  if (true)
  {
    fileOut.open(pathFile, std::ios::binary | std::ios::trunc);

    rci--;
    if (!fileOut)
    {
      vd_printErrorRc(rci, "Failed opening file for writing: " + pathFile.string());
      return rci;
    }
  }    

  // RCI : write the file
  if (true)
  {
    fileOut << text;

    rci--;
    if (!fileOut)
    {
      vd_printErrorRc(rci, "Failed writing file: " + pathFile.string());
      return rci;
    }
  }

  // END
  return 0;
}

static inline TYPE_rc rc_writeFileJson(const std::filesystem::path& pathFile, const TYPE_json& value)
{
  // NULLIFY : rc, out
  TYPE_rc rce = 0;
  
  // RCE : write the file
  if (true)
  {
    rce = to_abs(rc_writeFileText(pathFile, value.dump(2)));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed writing JSON file: " + pathFile.string());
      return rce;
    }
  }

  // END
  return 0;
}

static inline TYPE_rc rc_writeFileJsonAtomic(const std::filesystem::path& pathFile, const TYPE_json& value)
{
  // NULLIFY : rc, out
  TYPE_rc rce = 0;

  // RCE : write temporary file (.tmp.)
  std::filesystem::path pathFileTemp;
  if (true)
  {
    const uint64_t nonce = static_cast<uint64_t>(std::chrono::steady_clock::now().time_since_epoch().count());
    pathFileTemp = pathFile.string() + ".tmp." + std::to_string(nonce);

    rce = to_abs(rc_writeFileJson(pathFileTemp, value));
    if (rce != 0)
    {
      std::error_code errorCode;
      std::filesystem::remove(pathFileTemp, errorCode);
      vd_printErrorRc(rce, "Failed writing temporary JSON file: " + pathFileTemp.string());
      return rce;
    }
  }    

  // RCI : atomic replacement
  if (true)
  {
#if defined(_WIN32)
    const bool isReplaced = MoveFileExW(pathFileTemp.c_str(), pathFile.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
    const bool isReplaced = std::rename(pathFileTemp.c_str(), pathFile.c_str()) == 0;
#endif
    TYPE_rc rci = 0;
    rci--;
    if (!isReplaced)
    {
      std::error_code ecRemoveFile;
      std::filesystem::remove(pathFileTemp, ecRemoveFile);
      vd_printErrorRc(rci, "Failed replacing file: " + pathFile.string());
      return rci;
    }
  }

  // END
  return 0;
}

static inline bool is_relativePathSafe(const std::filesystem::path& path)
{
  if (path.empty() || path.is_absolute() || path.has_root_path())
    return false;

  for (const auto& part : path)
  {
    if (part == "..")
      return false;
  }

  return true;
}