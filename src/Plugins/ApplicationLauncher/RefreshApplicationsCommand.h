#pragma once

#include "../../Commands/ICommand.h"
#include "ApplicationFinder.h"

class RefreshApplicationsCommand : public ICommand {
public:
    std::wstring GetName() const override;
    std::wstring GetDescription() const override;
    CommandCategory GetCategory() const override;
    void Execute() override;
};
