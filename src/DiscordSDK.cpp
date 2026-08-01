#include "DiscordSDK.hpp"

#define DISCORDPP_IMPLEMENTATION
#include "discordpp.h"

#include <battery/embed.hpp>

#include <iostream>
#include <fstream>

struct DiscordManager::DiscordData{
    std::shared_ptr<discordpp::Client> m_client;
};

DiscordManager::DiscordManager() 
    : m_discord(std::make_unique<DiscordData>())
{
    constexpr char discord_lib_file[] = DISCORD_LIB_FILE;

    const auto discord_lib = b::embed<DISCORD_LIB_NAME>();
    
    const std::string discord_lib_data = discord_lib.str();
    std::fstream out(discord_lib_file, std::ios::out | std::ios::binary);
    out.write(discord_lib_data.data(), discord_lib_data.size());
    out.flush();
    out.close();

    const auto embed_id = b::embed<"resources/discord_app_id.txt">();
    const uint64_t embed_id_uint64 = std::stoull(embed_id.str());

    m_discord->m_client = std::make_shared<discordpp::Client>();

    m_discord->m_client->SetApplicationId(embed_id_uint64);

    discordpp::Activity activity;
    activity.SetType(discordpp::ActivityTypes::Playing);
    activity.SetState("Chasing high scores");
    activity.SetDetails("Playing FlappyBlock");

    m_discord->m_client->UpdateRichPresence(activity, [](auto result) {
        if (!result.Successful()) {
            std::cerr << "❌ Failed to update Rich Presence: " << result.ToString() << "\n";
        }
    });

    m_update = std::thread([this]{
        while(m_running) {
            discordpp::RunCallbacks();
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    });
}

DiscordManager::~DiscordManager() {
    m_running = false;
    m_update.join();
}