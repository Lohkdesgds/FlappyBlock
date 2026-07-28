#include "DiscordSDK.hpp"

#define DISCORDPP_IMPLEMENTATION
#include "discordpp.h"

#include <battery/embed.hpp>

#include <iostream>

struct DiscordManager::DiscordData{
    std::shared_ptr<discordpp::Client> m_client;
};

DiscordManager::DiscordManager() 
    : m_discord(std::make_unique<DiscordData>())
{
    const auto embed_id = b::embed<"resources/discord_app_id.txt">();
    const uint64_t embed_id_uint64 = std::stoull(embed_id.str());

    m_discord->m_client = std::make_shared<discordpp::Client>();

//    m_discord->m_client->AddLogCallback([](auto message, auto severity) {
//        std::cout << "[" << EnumToString(severity) << "] " << message << std::endl;
//    }, discordpp::LoggingSeverity::Info);
//
//    m_discord->m_client->SetStatusChangedCallback([client = m_discord->m_client](discordpp::Client::Status status, discordpp::Client::Error error, int32_t errorDetail) {
//        std::cout << "🔄 Status changed: " << discordpp::Client::StatusToString(status) << std::endl;
//
//        if (status == discordpp::Client::Status::Ready) {
//            std::cout << "✅ Client is ready! You can now call SDK functions.\n";
//        } else if (error != discordpp::Client::Error::None) {
//            std::cerr << "❌ Connection Error: " << discordpp::Client::ErrorToString(error) << " - Details: " << errorDetail << std::endl;
//        }
//    });

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