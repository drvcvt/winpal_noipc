#pragma once

#include "../../Commands/ICommand.h"
#include "ApplicationFinder.h"

class GenericLaunchCommand : public ICommand {
public:
    GenericLaunchCommand();
    
    std::wstring GetName() const override;
    std::wstring GetDescription() const override;
    CommandCategory GetCategory() const override;
    void Execute() override;
    
    // Spezielle Methoden für die dynamische Anwendungssuche
    void SetSearchTerm(const std::wstring& searchTerm);
    std::vector<ApplicationInfo> GetMatchingApplications() const;
    void LaunchApplication(const ApplicationInfo& app);
    
private:
    ApplicationFinder& m_applicationFinder;
    std::wstring m_currentSearchTerm;
    
    // Hilfsmethoden
    bool LaunchByPath(const std::wstring& path);
    bool LaunchByName(const std::wstring& name);
    std::wstring ResolvePath(const std::wstring& input);
}; 