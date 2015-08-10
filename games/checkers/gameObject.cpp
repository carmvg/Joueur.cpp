// Generated by Creer at 12:24AM on August 10, 2015 UTC, git hash: '44abe78df9634b90415f2d1791a923bdc6e4f5c6'
// An object in the game. The most basic class that all game classes should inherit from automatically.

#include "gameObject.h"
#include "gameManager.h"

// <<-- Creer-Merge: includes -->> - Code you add between this comment and the end comment will be preserved between Creer re-runs.
// you can add additional #includes(s) here.
// <<-- /Creer-Merge: includes -->>




void Checkers::GameObject::deltaUpdateField(const std::string& fieldName, boost::property_tree::ptree& delta)
{
    Joueur::BaseGameObject::deltaUpdateField(fieldName, delta);

    if (fieldName == "logs")
    {
        this->logs = this->gameManager->unserializeVector(delta, &this->logs);
    }
}



void Checkers::GameObject::log(std::string message)
{
    boost::property_tree::ptree args;
    args.put_child("message", *this->gameManager->serialize(message));

    auto returned = this->gameManager->runOnServer(*this, "log", args);
}


// <<-- Creer-Merge: methods -->> - Code you add between this comment and the end comment will be preserved between Creer re-runs.
// if you forward declaired additional methods to the GameObject then you can code them here.
// <<-- /Creer-Merge: methods -->>