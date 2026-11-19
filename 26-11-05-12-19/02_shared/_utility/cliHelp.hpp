#pragma once

/*
  CLI help metadata & formatting utilities.
  Input structure: <COMMAND> <SUBCOMMAND> [<ARGUMENT>]

    Commands              : Top-level command names
    └─ Subcommands        : Named actions; an empty name means the command itself
       ├─ Operands        : Positional arguments
       ├─ Options (value) : Optional argument, require a value
       └─ Options (flag)  : Optional argument, do not take a value       
*/

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

//====----====----====----====----====----====----====----====---- TYPE

namespace cliHelp
{
  struct STRUCT_argumentInfo
  {
    std::string name = "";
    std::string nameShort = "";
    std::string meta = "";
    std::string description = "";
  };

  struct STRUCT_subCommandInfo
  {
    std::string name = "";
    std::string description = "";
    std::vector<STRUCT_argumentInfo> listOperand = {};
    std::vector<STRUCT_argumentInfo> listOptionValue = {};
    std::vector<STRUCT_argumentInfo> listOptionFlag = {};
  };

  struct STRUCT_commandInfo
  {
    std::string name = "";
    std::string description = "";
    std::vector<STRUCT_subCommandInfo> listSubCommandInfo = {};
  };

  struct REQUEST_vd_printHelp
  {
    const std::string& nameProgram;
    const std::string& description;
    const std::vector<STRUCT_commandInfo>& listCommandInfo;
    std::string nameCommand;
    std::string nameSubCommand;
  };

  namespace /* private */
  {
    using _TYPE_argumentRow = std::pair<std::string, std::string>; // face{shortname, name, meta}, description
    using _TYPE_listArgumentRow = std::vector<_TYPE_argumentRow>;

    struct _REQUEST_vd_getListArgumentRows
    {
      const std::vector<STRUCT_argumentInfo>& listArgumentInfo;
      size_t maxWidthName = 0;
      size_t maxWidthMeta = 0;
      bool isOperand = false;
    };
  }
}



//====----====----====----====----====----====----====----====---- FDEC

namespace cliHelp
{
  static inline const STRUCT_commandInfo* getPtr_commandInfo(const std::vector<STRUCT_commandInfo>& listCommandInfo, const std::string& nameCommand);
  static inline const STRUCT_subCommandInfo* getPtr_subCommandInfo(const STRUCT_commandInfo& commandInfo, const std::string& nameSubCommand);
  static inline void vd_printHelp(const REQUEST_vd_printHelp& request);

  namespace /* private */
  {
    inline std::string _str_argumentNameWithShort(const STRUCT_argumentInfo& info);
    inline std::string _str_argumentFacePadded(const STRUCT_argumentInfo& info, size_t widthName, size_t widthMeta);
    inline size_t _get_maxWidthArgumentFace(const _TYPE_listArgumentRow& rows);
    inline void _vd_printRows(const std::string& title, const _TYPE_listArgumentRow& rows, size_t externalMaxWidth);
    inline void _vd_getListArgumentRows(const _REQUEST_vd_getListArgumentRows& request, _TYPE_listArgumentRow& out_listArgumentRows);
    inline void _vd_printCommonFlags(bool includeVersion);
  }
}

//====----====----====----====----====----====----====----====---- FDEF

namespace cliHelp
{
  static inline const STRUCT_commandInfo* getPtr_commandInfo(const std::vector<STRUCT_commandInfo>& listCommandInfo, const std::string& nameCommand)
  {
    for (const auto& info : listCommandInfo)
    {
      if (info.name == nameCommand)
      {
        return &info;
      }
    }

    return nullptr;
  }

  static inline const STRUCT_subCommandInfo* getPtr_subCommandInfo(const STRUCT_commandInfo& commandInfo, const std::string& nameSubCommand)
  {
    for (const auto& info : commandInfo.listSubCommandInfo)
    {
      if (info.name == nameSubCommand)
      {
        return &info;
      }
    }

    return nullptr;
  }

  static inline void vd_printHelp(const REQUEST_vd_printHelp& request)
  {
    // PREPARE : copy the request for recursive help fallback
    REQUEST_vd_printHelp nextRequest = request;

    // RETURN : print top-level help (no command selected)
    if (request.nameCommand.empty())
    {
      std::cout << request.description << "\n\n";
      std::cout << "USAGE:\n  " << request.nameProgram << " [OPTIONS] [COMMAND]\n\n";

      _TYPE_listArgumentRow rows;
      for (const auto& info : request.listCommandInfo)
      {
        std::string description = info.description;
        const STRUCT_subCommandInfo* target = getPtr_subCommandInfo(info, "");
        if (description.empty() && target)
        {
          description = target->description;
        }
        rows.emplace_back(info.name, description);
      }

      _vd_printRows("COMMANDS", rows, 0);
      _vd_printCommonFlags(true);
      return;
    }

    // RESOLVE : find the requested command
    const STRUCT_commandInfo* ptrCommand = getPtr_commandInfo(request.listCommandInfo, request.nameCommand);
    if (ptrCommand == nullptr)
    {
      // RECURSE : fall back to top-level help
      nextRequest.nameCommand = "";
      nextRequest.nameSubCommand = "";
      vd_printHelp(nextRequest);
      return;
    }
    const auto& commandInfo = *ptrCommand;

    // RESOLVE : find the requested subcommand or command-level target
    const STRUCT_subCommandInfo* ptrTarget = getPtr_subCommandInfo(commandInfo, request.nameSubCommand);
    if (ptrTarget == nullptr)
    {
      if (!request.nameSubCommand.empty())
      {
        // RECURSE : fall back to command-level help
        nextRequest.nameSubCommand = "";
        vd_printHelp(nextRequest);
        return;
      }

      // RETURN : print available subcommands (no command-level target)
      std::cout << commandInfo.description << "\n\n";
      std::cout << "USAGE:\n  " << request.nameProgram << " " << request.nameCommand << " [OPTIONS] <SUBCOMMAND>\n\n";

      _TYPE_listArgumentRow rows;
      for (const auto& info : commandInfo.listSubCommandInfo)
      {
        rows.emplace_back(info.name, info.description);
      }

      _vd_printRows("SUBCOMMANDS", rows, 0);
      _vd_printCommonFlags(false);
      return;
    }
    const auto& targetInfo = *ptrTarget;
    std::cout << targetInfo.description << "\n\n";

    std::cout << "USAGE:\n  " << request.nameProgram << " " << request.nameCommand;
    if (!request.nameSubCommand.empty())
    {
      std::cout << " " << request.nameSubCommand;
    }
    std::cout << " [OPTIONS]";
    for (const auto& operand : targetInfo.listOperand)
    {
      std::cout << " <" << operand.name << ">";
    }
    std::cout << "\n\n";

    auto listFlag = targetInfo.listOptionFlag;
    listFlag.push_back({"--verbose", "-v", "", "Enable verbose output"});
    listFlag.push_back({"--help", "-h", "", "Display help and exit"});

    size_t widthName = 0;
    size_t widthMeta = 0;
    const auto updateWidths = [&widthName, &widthMeta](const auto& arguments, size_t nameExtra = 0)
    {
      for (const auto& argument : arguments)
      {
        widthName = std::max(widthName, _str_argumentNameWithShort(argument).size() + nameExtra);
        widthMeta = std::max(widthMeta, argument.meta.size());
      }
    };

    updateWidths(targetInfo.listOperand, 2);
    updateWidths(targetInfo.listOptionValue);

    _TYPE_listArgumentRow operandRows;
    _vd_getListArgumentRows({targetInfo.listOperand, widthName, widthMeta, true}, operandRows);
    _TYPE_listArgumentRow valueOptionRows;
    _vd_getListArgumentRows({targetInfo.listOptionValue, widthName, widthMeta}, valueOptionRows);
    _TYPE_listArgumentRow flagRows;
    _vd_getListArgumentRows({listFlag}, flagRows);

    size_t maxWidthRow = std::max(
      {
        _get_maxWidthArgumentFace(operandRows),
        _get_maxWidthArgumentFace(valueOptionRows),
        _get_maxWidthArgumentFace(flagRows)
      }
    );

    // pad
    maxWidthRow += 4;

    _vd_printRows("POSITIONAL OPERANDS", operandRows, maxWidthRow);
    _vd_printRows("VALUE OPTIONS", valueOptionRows, maxWidthRow);
    _vd_printRows("FLAG OPTIONS", flagRows, maxWidthRow);
  }

  namespace /* private */
  {
    inline std::string _str_argumentNameWithShort(const STRUCT_argumentInfo& info)
    {
      // return string: <NAME_SHORT>, <NAME>    
      std::string ret;

      if (!info.nameShort.empty())
      {
        ret += info.nameShort + ", ";
      }

      ret += info.name;

      return ret;
    }

    inline std::string _str_argumentFacePadded(const STRUCT_argumentInfo& info, size_t maxWidthName, size_t maxWidthMeta)
    {
      // return padded argument info string : NAME + padding + META
      std::string ret = _str_argumentNameWithShort(info);

      if (maxWidthName > 0)
      {
        ret += std::string(maxWidthName - ret.size() + 2, ' ');
      }

      if (!info.meta.empty())
      {
        ret += info.meta;
      }

      if (maxWidthMeta > 0)
      {
        ret += std::string(maxWidthMeta - info.meta.size(), ' ');
      }

      return ret;
    }

    inline size_t _get_maxWidthArgumentFace(const _TYPE_listArgumentRow& rows)
    {
      size_t width = 0;
      for (const auto& row : rows)
      {
        width = std::max(width, row.first.size());
      }

      return width;
    }

    inline void _vd_printRows(const std::string& title, const _TYPE_listArgumentRow& rows, size_t externalMaxWidth)
    {
      if (rows.empty())
      {
        return;
      }

      const size_t widthResolved = (externalMaxWidth == 0) ? (_get_maxWidthArgumentFace(rows) + 2) : (externalMaxWidth);

      std::cout << title << ":" << std::endl;

      for (const auto& [name, description] : rows)
      {
        std::istringstream stream(description);
        std::string line;
        bool first = true;

        while (std::getline(stream, line))
        {
          if (first)
          {
            std::cout
              << "  "
              << std::left
              << std::setw(static_cast<int>(widthResolved))
              << name
              << line
              << std::endl;
            first = false;
          }
          else
          {
            std::cout << std::string(widthResolved + 2, ' ') << line << std::endl;
          }
        }

        if (first)
        {
          std::cout << "  " << name << std::endl;
        }
      }

      std::cout << std::endl;
    }

    inline void _vd_getListArgumentRows(const _REQUEST_vd_getListArgumentRows& request, _TYPE_listArgumentRow& out_listArgumentRows)
    {
      out_listArgumentRows.clear();
      out_listArgumentRows.reserve(request.listArgumentInfo.size());

      for (const auto& argument : request.listArgumentInfo)
      {
        auto displayArgument = argument;
        if (request.isOperand)
        {
          displayArgument.name = "<" + displayArgument.name + ">";
        }
        out_listArgumentRows.emplace_back(_str_argumentFacePadded(displayArgument, request.maxWidthName, request.maxWidthMeta), argument.description);
      }
    }

    inline void _vd_printCommonFlags(bool includeVersion)
    {
      _TYPE_listArgumentRow rows;

      if (includeVersion)
      {
        rows.emplace_back("-V, --version", "Display version information");
      }

      rows.emplace_back("-v, --verbose", "Enable verbose output");
      rows.emplace_back("-h, --help", "Display help and exit");

      _vd_printRows("FLAG OPTIONS", rows, 0);
    }
  }
}
