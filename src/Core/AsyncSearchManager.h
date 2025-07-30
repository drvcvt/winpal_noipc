#pragma once

#include <vector>
#include <string>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <unordered_map>
#include <functional>
#include <chrono>
#include "../Commands/ICommand.h"

// Cache für Suchergebnisse
struct SearchCache {
    std::wstring query;
    std::vector<ICommand*> results;
    std::chrono::steady_clock::time_point timestamp;
    
    SearchCache(const std::wstring& q, const std::vector<ICommand*>& r) 
        : query(q), results(r), timestamp(std::chrono::steady_clock::now()) {}
};

class AsyncSearchManager {
public:
    using SearchCallback = std::function<void(const std::vector<ICommand*>&)>;
    
    AsyncSearchManager();
    ~AsyncSearchManager();
    
    // Startet eine asynchrone Suche mit Debouncing
    void SearchAsync(const std::wstring& query, SearchCallback callback);
    
    // Stoppt alle laufenden Suchen
    void CancelSearch();
    
    // Cache-Management
    void ClearCache();
    void SetCacheTimeout(std::chrono::seconds timeout);
    
    // Performance-Einstellungen
    void SetDebounceDelay(std::chrono::milliseconds delay);
    void SetMaxCacheSize(size_t maxSize);
    
    // Thread-sichere Methode zum Setzen der Suchfunktion
    void SetSearchFunction(std::function<std::vector<ICommand*>(const std::wstring&)> searchFunc);
    
private:
    // Threading
    std::thread m_searchThread;
    std::mutex m_searchMutex;
    std::condition_variable m_searchCondition;
    std::atomic<bool> m_shouldStop;
    std::atomic<bool> m_searchPending;
    
    // Current search state
    std::wstring m_pendingQuery;
    SearchCallback m_pendingCallback;
    std::chrono::steady_clock::time_point m_lastSearchRequest;
    
    // Cache
    std::vector<SearchCache> m_searchCache;
    std::mutex m_cacheMutex;
    std::chrono::seconds m_cacheTimeout;
    size_t m_maxCacheSize;
    
    // Performance settings
    std::chrono::milliseconds m_debounceDelay;
    
    // Search function
    std::function<std::vector<ICommand*>(const std::wstring&)> m_searchFunction;
    std::mutex m_searchFunctionMutex;
    
    // Thread functions
    void SearchThreadMain();
    void ProcessSearch();
    
    // Cache functions
    bool GetFromCache(const std::wstring& query, std::vector<ICommand*>& results);
    void AddToCache(const std::wstring& query, const std::vector<ICommand*>& results);
    void CleanupCache();
    
    // Helper functions
    bool IsSubstring(const std::wstring& query, const std::wstring& cachedQuery);
    std::vector<ICommand*> FilterCachedResults(const std::vector<ICommand*>& cachedResults, 
                                               const std::wstring& query);
}; 