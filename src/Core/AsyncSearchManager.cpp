#include "AsyncSearchManager.h"
#include <algorithm>
#include <cwctype>

AsyncSearchManager::AsyncSearchManager()
    : m_shouldStop(false)
    , m_searchPending(false)
    , m_cacheTimeout(std::chrono::seconds(300)) // 5 Minuten Cache
    , m_maxCacheSize(100)
    , m_debounceDelay(std::chrono::milliseconds(150)) // 150ms Debounce
{
    m_searchThread = std::thread(&AsyncSearchManager::SearchThreadMain, this);
}

AsyncSearchManager::~AsyncSearchManager() {
    m_shouldStop = true;
    m_searchCondition.notify_all();
    
    if (m_searchThread.joinable()) {
        m_searchThread.join();
    }
}

void AsyncSearchManager::SearchAsync(const std::wstring& query, SearchCallback callback) {
    std::lock_guard<std::mutex> lock(m_searchMutex);
    
    // Update pending search
    m_pendingQuery = query;
    m_pendingCallback = callback;
    m_lastSearchRequest = std::chrono::steady_clock::now();
    m_searchPending = true;
    
    // Prüfe Cache zuerst
    std::vector<ICommand*> cachedResults;
    if (GetFromCache(query, cachedResults)) {
        // Cache hit - sofort zurückgeben
        if (callback) {
            callback(cachedResults);
        }
        m_searchPending = false;
        return;
    }
    
    // Wake up search thread
    m_searchCondition.notify_one();
}

void AsyncSearchManager::CancelSearch() {
    std::lock_guard<std::mutex> lock(m_searchMutex);
    m_searchPending = false;
}

void AsyncSearchManager::ClearCache() {
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    m_searchCache.clear();
}

void AsyncSearchManager::SetCacheTimeout(std::chrono::seconds timeout) {
    m_cacheTimeout = timeout;
}

void AsyncSearchManager::SetDebounceDelay(std::chrono::milliseconds delay) {
    m_debounceDelay = delay;
}

void AsyncSearchManager::SetMaxCacheSize(size_t maxSize) {
    m_maxCacheSize = maxSize;
}

void AsyncSearchManager::SetSearchFunction(std::function<std::vector<ICommand*>(const std::wstring&)> searchFunc) {
    std::lock_guard<std::mutex> lock(m_searchFunctionMutex);
    m_searchFunction = searchFunc;
}

void AsyncSearchManager::SearchThreadMain() {
    while (!m_shouldStop) {
        std::unique_lock<std::mutex> lock(m_searchMutex);
        
        // Wait for search request
        m_searchCondition.wait(lock, [this] {
            return m_searchPending.load() || m_shouldStop.load();
        });
        
        if (m_shouldStop) {
            break;
        }
        
        if (m_searchPending) {
            ProcessSearch();
        }
    }
}

void AsyncSearchManager::ProcessSearch() {
    // Wait for debounce delay
    auto timeToWait = m_lastSearchRequest + m_debounceDelay;
    auto now = std::chrono::steady_clock::now();
    
    if (now < timeToWait) {
        auto delay = std::chrono::duration_cast<std::chrono::milliseconds>(timeToWait - now);
        std::this_thread::sleep_for(delay);
        
        // Check if a new search was requested during delay
        if (m_lastSearchRequest > timeToWait) {
            return; // New search request came in, abort this one
        }
    }
    
    // Get current search parameters
    std::wstring query;
    SearchCallback callback;
    {
        std::lock_guard<std::mutex> lock(m_searchMutex);
        query = m_pendingQuery;
        callback = m_pendingCallback;
        m_searchPending = false;
    }
    
    if (query.empty()) {
        if (callback) {
            callback({});
        }
        return;
    }
    
    // Check cache again (might have been updated)
    std::vector<ICommand*> results;
    if (GetFromCache(query, results)) {
        if (callback) {
            callback(results);
        }
        return;
    }
    
    // Perform actual search
    {
        std::lock_guard<std::mutex> lock(m_searchFunctionMutex);
        if (m_searchFunction) {
            try {
                results = m_searchFunction(query);
                
                // Add to cache
                AddToCache(query, results);
                
                // Callback with results
                if (callback) {
                    callback(results);
                }
            } catch (...) {
                // Search failed, return empty results
                if (callback) {
                    callback({});
                }
            }
        }
    }
}

bool AsyncSearchManager::GetFromCache(const std::wstring& query, std::vector<ICommand*>& results) {
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    
    auto now = std::chrono::steady_clock::now();
    
    // Cleanup expired entries
    CleanupCache();
    
    // Look for exact match first
    for (const auto& cache : m_searchCache) {
        if (cache.query == query) {
            if (now - cache.timestamp < m_cacheTimeout) {
                results = cache.results;
                return true;
            }
        }
    }
    
    // Look for substring matches that we can filter
    for (const auto& cache : m_searchCache) {
        if (IsSubstring(query, cache.query)) {
            if (now - cache.timestamp < m_cacheTimeout) {
                results = FilterCachedResults(cache.results, query);
                
                // Add filtered results to cache for future use
                if (!results.empty()) {
                    m_searchCache.emplace_back(query, results);
                }
                return true;
            }
        }
    }
    
    return false;
}

void AsyncSearchManager::AddToCache(const std::wstring& query, const std::vector<ICommand*>& results) {
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    
    // Check if already in cache
    for (auto& cache : m_searchCache) {
        if (cache.query == query) {
            cache.results = results;
            cache.timestamp = std::chrono::steady_clock::now();
            return;
        }
    }
    
    // Add new entry
    m_searchCache.emplace_back(query, results);
    
    // Limit cache size
    if (m_searchCache.size() > m_maxCacheSize) {
        // Remove oldest entries
        std::sort(m_searchCache.begin(), m_searchCache.end(), 
                  [](const SearchCache& a, const SearchCache& b) {
                      return a.timestamp < b.timestamp;
                  });
        
        size_t toRemove = m_searchCache.size() - m_maxCacheSize;
        m_searchCache.erase(m_searchCache.begin(), m_searchCache.begin() + toRemove);
    }
}

void AsyncSearchManager::CleanupCache() {
    auto now = std::chrono::steady_clock::now();
    
    m_searchCache.erase(
        std::remove_if(m_searchCache.begin(), m_searchCache.end(),
                       [this, now](const SearchCache& cache) {
                           return now - cache.timestamp >= m_cacheTimeout;
                       }),
        m_searchCache.end()
    );
}

bool AsyncSearchManager::IsSubstring(const std::wstring& query, const std::wstring& cachedQuery) {
    if (query.length() <= cachedQuery.length()) {
        return false; // Query must be longer than cached query for filtering
    }
    
    // Check if query starts with cached query
    std::wstring lowerQuery = query;
    std::wstring lowerCached = cachedQuery;
    
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::towlower);
    std::transform(lowerCached.begin(), lowerCached.end(), lowerCached.begin(), ::towlower);
    
    return lowerQuery.find(lowerCached) == 0;
}

std::vector<ICommand*> AsyncSearchManager::FilterCachedResults(const std::vector<ICommand*>& cachedResults, 
                                                               const std::wstring& query) {
    std::vector<ICommand*> filtered;
    std::wstring lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::towlower);
    
    for (ICommand* cmd : cachedResults) {
        if (!cmd) continue;
        
        std::wstring lowerName = cmd->GetName();
        std::wstring lowerDesc = cmd->GetDescription();
        
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::towlower);
        std::transform(lowerDesc.begin(), lowerDesc.end(), lowerDesc.begin(), ::towlower);
        
        // Check if command matches the more specific query
        if (lowerName.find(lowerQuery) != std::wstring::npos ||
            lowerDesc.find(lowerQuery) != std::wstring::npos) {
            filtered.push_back(cmd);
        }
    }
    
    return filtered;
} 