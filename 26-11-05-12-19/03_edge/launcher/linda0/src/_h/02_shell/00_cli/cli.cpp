#include "03_edge/launcher/linda0/src/_h/02_shell/00_cli/_cli.h"

#include "00_vendored/CLIUtils/CLI11.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include "03_edge/launcher/linda0/src/__env.h"
#include "01_standard/phase00/error.hpp"
#include "02_shared/_utility/cliHelp.hpp"

using namespace std;

using namespace linda0::cli;
using namespace cliHelp;

//====----====----====----====----====----====----====----====---- PRIVATE : TYPE

namespace
{
  struct _STRUCT_operationInfo
  {
    CLI::App* app;

    string command;
    string subCommand;

    unordered_map<string, CLI::Option*> mapOperand;
    unordered_map<string, CLI::Option*> mapOptionValue;
    unordered_map<string, CLI::Option*> mapOptionFlag;
  };
}

//====----====----====----====----====----====----====----====---- PRIVATE : FDEC

namespace
{
  bool _is_helpRequested(int argc, char** argv);
  void _vd_registerCommandTree(CLI::App& app, const vector<STRUCT_commandInfo>& listCommandInfo, vector<CLI::Option*>& edit_listOptionVerbose, vector<_STRUCT_operationInfo>& out_listOperationInfo);
  void _vd_extractLineParsed(CLI::App& app, const vector<CLI::Option*>& listOptionVerbose, const vector<_STRUCT_operationInfo>& listOperationInfo, STRUCT_lineParsed& out_lineParsed);
  void _vd_initializeCommandInfo(vector<STRUCT_commandInfo>& out_listCommandInfo);
  void _vd_addCommonFlags(CLI::App* app, vector<CLI::Option*>& edit_listOptionVerbose);
  string _str_cliOptionSpec(const STRUCT_argumentInfo& info);
}

//====----====----====----====----====----====----====----====---- PUBLIC : FDEF

namespace linda0::cli
{
  TYPE_rc rc_parseLine(int argc, char** argv, STRUCT_lineParsed& out_lineParsed)
  {
    // NULLIFY : rc, out
    out_lineParsed = {};
    TYPE_rc rci = 0;

    vector<STRUCT_commandInfo> listCommandInfo;
    _vd_initializeCommandInfo(listCommandInfo);
    
    const string descriptionProgram = ENV_WHOAMI " is a virtual instrument development tool";

    const bool isHelpRequested = _is_helpRequested(argc, argv);
    if (isHelpRequested)
    {
      string command;
      string subCommand;
      const STRUCT_commandInfo* ptrCommand = nullptr;

      for (int index = 1; index < argc; ++index)
      {
        const string argument = argv[index];

        if (!ptrCommand)
        {
          ptrCommand = cliHelp::getPtr_commandInfo(listCommandInfo, argument);

          if (ptrCommand)
          {
            command = argument;
          }

          continue;
        }

        if (subCommand.empty())
        {
          const cliHelp::STRUCT_subCommandInfo* ptrSubCommand = cliHelp::getPtr_subCommandInfo(*ptrCommand, argument);

          if (ptrSubCommand)
          {
            subCommand = argument;
          }
        }
      }

      const string nameProgram = ENV_WHOAMI;
      const cliHelp::REQUEST_vd_printHelp requestHelp =
      {
        nameProgram,
        descriptionProgram,
        listCommandInfo,
        command,
        subCommand
      };
      cliHelp::vd_printHelp(requestHelp);
      return 0;
    }

    CLI::App app{descriptionProgram};
    vector<CLI::Option*> listOptionVerbose;
    vector<_STRUCT_operationInfo> listOperationInfo;
    _vd_registerCommandTree(app, listCommandInfo, listOptionVerbose, listOperationInfo);

    try
    {
      app.parse(argc, argv);
    }
    catch (const CLI::ParseError& e)
    {
      if (true)
      {
        const bool isExtrasErrorWithArgument =
        {
          e.get_name() == "ExtrasError"
          && argc >= 2
          && argv[1] != nullptr
        };
        // RCI : check for an unknown command
        rci--;
        if (isExtrasErrorWithArgument)
        {
          const string argument = argv[1];
          if (true)
          {
            const bool isUnknownCommand =
            {
              !argument.empty()
              && argument.front() != '-'
              && cliHelp::getPtr_commandInfo(listCommandInfo, argument) == nullptr
            };
            if (isUnknownCommand)
            {
              vd_printErrorRc(rci, "Unknown command: " + argument + ". Run '" + ENV_WHOAMI + " --help' for usage.");
              return rci;
            }
          }
        }
      }

      const int codeCliExit = app.exit(e);
      rci--;
      if (codeCliExit != 0)
      {
        vd_printErrorRc(rci, "Failed parsing command line: " + string(e.what()));
        return rci;
      }
      return 0;
    }

    _vd_extractLineParsed(app, listOptionVerbose, listOperationInfo, out_lineParsed);

    // END
    return 0;
  }
}

//====----====----====----====----====----====----====----====---- PRIVATE : FDEF

namespace
{
  bool _is_helpRequested(int argc, char** argv)
  {
    if (argc < 2)
    {
      return true;
    }

    for (int index = 1; index < argc; ++index)
    {
      const string argument = argv[index];
      if (argument == "-h" || argument == "--help" || argument == "-help")
      {
        return true;
      }
    }

    return false;
  }

  void _vd_registerCommandTree(CLI::App& app, const vector<STRUCT_commandInfo>& listCommandInfo, vector<CLI::Option*>& edit_listOptionVerbose, vector<_STRUCT_operationInfo>& out_listOperationInfo)
  {
    // INIT : CLI application
    app.name(ENV_WHOAMI);
    app.set_help_flag();
    app.get_formatter()->right_column_width(100);
    const string strVersion = to_string(ENV_VERSION_MAJOR) + "." + to_string(ENV_VERSION_MINOR) + "." + to_string(ENV_VERSION_PATCH);
    app.set_version_flag("-V,--version", strVersion, "Display version information");
    _vd_addCommonFlags(&app, edit_listOptionVerbose);

    // REGISTER : command tree
    for (const auto& commandInfo : listCommandInfo)
    {
      const string& command = commandInfo.name;
      string description = commandInfo.description;

      const cliHelp::STRUCT_subCommandInfo* directTarget = cliHelp::getPtr_subCommandInfo(commandInfo, "");
      if (description.empty() && directTarget)
      {
        description = directTarget->description;
      }

      CLI::App* appCommand = app.add_subcommand(command, description);
      appCommand->set_help_flag();
      appCommand->subcommand_fallthrough(false);

      if (
        commandInfo.listSubCommandInfo.size() > 1
        || !directTarget
      )
      {
        appCommand->require_subcommand(1);
      }

      if (!directTarget)
      {
        _vd_addCommonFlags(appCommand, edit_listOptionVerbose);
      }

      for (const auto& subCommandInfo : commandInfo.listSubCommandInfo)
      {
        const string& subCommand = subCommandInfo.name;
        CLI::App* appTarget = appCommand;

        if (!subCommand.empty())
        {
          appTarget = appCommand->add_subcommand(subCommand, subCommandInfo.description);
          appTarget->set_help_flag();
          appTarget->subcommand_fallthrough(false);
        }

        _STRUCT_operationInfo operationInfo;
        operationInfo.command = command;
        operationInfo.subCommand = subCommand;
        operationInfo.app = appTarget;

        for (const auto& operand : subCommandInfo.listOperand)
        {
          CLI::Option* option = appTarget->add_option(operand.name, operand.description);
          option->required();
          option->expected(1);

          if (!operand.meta.empty())
          {
            option->type_name(operand.meta);
          }

          operationInfo.mapOperand[operand.name] = option;
        }

        for (const auto& optionValue : subCommandInfo.listOptionValue)
        {
          CLI::Option* option = appTarget->add_option(_str_cliOptionSpec(optionValue), optionValue.description);
          option->expected(1);

          if (!optionValue.meta.empty())
          {
            option->type_name(optionValue.meta);
          }

          operationInfo.mapOptionValue[optionValue.name] = option;
        }

        for (const auto& optionFlag : subCommandInfo.listOptionFlag)
        {
          CLI::Option* option = appTarget->add_flag(_str_cliOptionSpec(optionFlag), optionFlag.description);
          operationInfo.mapOptionFlag[optionFlag.name] = option;
        }

        _vd_addCommonFlags(appTarget, edit_listOptionVerbose);
        out_listOperationInfo.push_back(operationInfo);
      }
    }

    // END
    return;
  }

  void _vd_extractLineParsed(CLI::App& app, const vector<CLI::Option*>& listOptionVerbose, const vector<_STRUCT_operationInfo>& listOperationInfo, STRUCT_lineParsed& out_lineParsed)
  {
    // READ : common flags
    for (const auto* option : listOptionVerbose)
    {
      if (option->count() > 0)
      {
        out_lineParsed.setOptionFlag.insert("--verbose");
        break;
      }
    }

    // FIND : active operation
    const _STRUCT_operationInfo* ptrActiveOperationInfo = nullptr;
    for (const auto& operationInfo : listOperationInfo)
    {
      if (!(*operationInfo.app))
      {
        continue;
      }

      ptrActiveOperationInfo = &operationInfo;
      break;
    }

    if (ptrActiveOperationInfo == nullptr)
    {
      cout << app.help();
      return;
    }

    const _STRUCT_operationInfo& operationInfo = *ptrActiveOperationInfo;
    out_lineParsed.command = operationInfo.command;
    out_lineParsed.subCommand = operationInfo.subCommand;

    // WRITE : operands and option values
    for (const auto& [name, option] : operationInfo.mapOperand)
    {
      if (!option->results().empty())
      {
        out_lineParsed.mapOperand[name] = option->results().back();
      }
    }

    for (const auto& [name, option] : operationInfo.mapOptionValue)
    {
      if (!option->results().empty())
      {
        out_lineParsed.mapOptionValue[name] = option->results().back();
      }
    }

    for (const auto& [name, option] : operationInfo.mapOptionFlag)
    {
      if (option->count() > 0)
      {
        out_lineParsed.setOptionFlag.insert(name);
      }
    }

    // END
    return;
  }

  void _vd_initializeCommandInfo(vector<STRUCT_commandInfo>& out_listCommandInfo)
  {
    out_listCommandInfo =
    { 
      { // list
        "list",
        "List available resources",
        {
          { 
            "support",
            "List available support modules", 
            {/**/}, 
            {/**/}, 
            { 
              { 
                "--language", 
                "-l", 
                "", 
                "List language support modules" 
              }, 
              { 
                "--audio", 
                "-a", 
                "", 
                "List audio support modules" 
              } 
            } 
          }, 
          { 
            "device",
            "List available audio devices", 
            { 
              { 
                "support", 
                "", 
                "NAME", 
                "Audio support module whose devices will be listed"
              }
            }, 
            {/**/}, 
            {/**/} 
          } 
        } 
      },
      { // scan
        "scan",
        "Scan source or device information",
        {
          { 
            "semantic",
            "Analyze a source file and display its semantics", 
            { 
              { 
                "source", 
                "", 
                "FILE", 
                "Path to the source file to scan" 
              },
              {
                "support",
                "",
                "NAME",
                "Language support module used to scan the source"
              } 
            }, 
            {/**/}, 
            {/**/} 
          }, 
          { 
            "channel",
            "Inspect an audio device and display its channels", 
            { 
              { 
                "support", 
                "", 
                "NAME", 
                "Audio support module used to access the device" 
              }
            }, 
            {
              { 
                "--device", 
                "", 
                "NAME", 
                "Name of the audio device to scan" 
              } 
            }, 
            {/**/} 
          } 
        } 
      }, 
      { // build
        "build",
        "",
        {
          { 
            "",
            "Build a RIG package and generate its runner file",
            { 
              { 
                "source", 
                "", 
                "FILE", 
                "Path to the source file to build" 
              },
              {
                "support",
                "",
                "NAME",
                "Language support module used to build the RIG package"
              } 
            }, 
            { 
              { 
                "--package", 
                "", 
                "DIR", 
                "Output directory for the RIG package\n" 
                "[DEFAULT] \"\\rig\" in parent directory of the <source>"
              }, 
              { 
                "--runner", 
                "", 
                "FILE", 
                "Output path for the runner file\n" 
                "[DEFAULT] \"runner\" in parent directory of the <source>"
              } 
            }, 
            {
              {
                "--absolute", 
                "-a", 
                "", 
                "Use absolute paths in runner and debug files\n"
                "[WARNING] Not recommended on package for distribution" 
              }
            } 
          } 
        } 
      }, 
      { // run
        "run",
        "",
        {
          { 
            "",
            "Start a GIG runtime from a RIG runner file", 
            { 
              { 
                "runner", 
                "", 
                "FILE", 
                "Path to a RIG runner file" 
              } 
            }, 
            {/**/}, 
            {
              {
                "--backup",
                "-b",
                "",
                "Create a runner backup after a successful run"
              },
              {
                "--debug",
                "-d",
                "",
                "Retain source diagnostics during runtime"
              }
            } 
          } 
        } 
      } 
    };

    // END
    return;
  }

  void _vd_addCommonFlags(CLI::App* app, vector<CLI::Option*>& edit_listOptionVerbose)
  {
    edit_listOptionVerbose.push_back(app->add_flag("-v,--verbose", "Enable verbose output"));
    app->set_help_flag("-h,--help", "Print help message and exit");

    // END
    return;
  }

  string _str_cliOptionSpec(const STRUCT_argumentInfo& info)
  {
    if (info.nameShort.empty())
    {
      return info.name;
    }

    return info.nameShort + "," + info.name;
  }
}
