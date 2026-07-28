#pragma once

#include <memory>
#include <thread>

class DiscordManager {
public:
    DiscordManager();
    ~DiscordManager();

private:
    struct DiscordData;

    std::unique_ptr<DiscordData> m_discord;
    std::thread m_update;
    bool m_running = true;
};