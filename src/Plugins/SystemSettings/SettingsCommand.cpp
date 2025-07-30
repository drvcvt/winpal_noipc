#include "SettingsCommand.h"
#include <Windows.h>
#include <shellapi.h>

SettingsCommand::SettingsCommand(const std::wstring& name, const std::wstring& uri)
    : name(name), uri(uri), description(L"Opens the " + name + L" settings.") {}

void SettingsCommand::Execute()
{
    ShellExecuteW(NULL, L"open", uri.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

std::wstring SettingsCommand::GetName() const
{
    return name;
}

std::wstring SettingsCommand::GetDescription() const
{
    return description;
}

CommandCategory SettingsCommand::GetCategory() const
{
    return CommandCategory::SETTINGS;
} 