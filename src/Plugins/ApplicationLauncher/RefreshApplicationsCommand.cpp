#include "RefreshApplicationsCommand.h"

std::wstring RefreshApplicationsCommand::GetName() const {
    return L"Refresh Applications";
}

std::wstring RefreshApplicationsCommand::GetDescription() const {
    return L"Rebuilds the cached application list.";
}

CommandCategory RefreshApplicationsCommand::GetCategory() const {
    return CommandCategory::APPLICATION_LAUNCHER;
}

void RefreshApplicationsCommand::Execute() {
    ApplicationFinder::Instance().RefreshApplications();
}
