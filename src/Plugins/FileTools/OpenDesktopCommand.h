#pragma once

#include "../../Commands/ICommand.h"

class OpenDesktopCommand : public ICommand {
public:
    std::wstring GetName() const override;
    std::wstring GetDescription() const override;
    CommandCategory GetCategory() const override;
    void Execute() override;
}; 