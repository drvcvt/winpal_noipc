#pragma once

#include <string>
#include <vector>
#include "../../Commands/ICommand.h"

class SettingsCommand : public ICommand
{
public:
    SettingsCommand(const std::wstring& name, const std::wstring& uri);
    void Execute() override;
    std::wstring GetName() const override;
    std::wstring GetDescription() const override;
    CommandCategory GetCategory() const override;

private:
    std::wstring name;
    std::wstring uri;
    std::wstring description;
}; 