#pragma once
#include "ClientState.hpp"
#include "WorldState.hpp"
#include "ClientManager.hpp"
#include "ItemManager.hpp"
#include "QuestManager.hpp"


class Model
{
public:
    using ActionFPtr = std::string(Model::*)(ClientState&, const std::vector<std::string>&); 

    std::string ProcessAction(ClientState&, const std::vector<std::string>&);

    bool Initialise();
    void Save();

    WorldState& GetWorldState() {return m_world_state; }
    ClientManager& GetClientManager() {return m_client_manager; }
    ItemManager& GetItemManager() {return m_item_manager; }
    QuestManager& GetQuestManager() {return m_quest_manager; }

private:
    std::string look(ClientState&, const std::vector<std::string>&);
    std::string say(ClientState&, const std::vector<std::string>&);
    std::string go(ClientState&, const std::vector<std::string>&);
    std::string help(ClientState&, const std::vector<std::string>&);
    std::string inv(ClientState&, const std::vector<std::string>&);
    std::string quests(ClientState&, const std::vector<std::string>&);
    std::string pickup(ClientState&, const std::vector<std::string>&);

    struct Action
    {
        Model::ActionFPtr m_func = nullptr;
        std::string m_trigger;
        int m_min_num_args = 0;
    };
    std::vector<Action> m_actions;

    WorldState m_world_state;
    ClientManager m_client_manager;
    ItemManager m_item_manager;
    QuestManager m_quest_manager;
};