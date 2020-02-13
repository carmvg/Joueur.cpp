#ifndef GAMES_COREMINER_JOB_H
#define GAMES_COREMINER_JOB_H

// Job
// Information about a Unit's job.

// DO NOT MODIFY THIS FILE
// Never try to directly create an instance of this class, or modify its member variables.
// Instead, you should only be reading its variables and calling its functions.

#include <vector>
#include <queue>
#include <deque>
#include <unordered_map>
#include <string>
#include <initializer_list>

#include "../../joueur/src/any.hpp"

#include "game_object.hpp"

#include "impl/coreminer_fwd.hpp"

// <<-- Creer-Merge: includes -->> - Code you add between this comment and the end comment will be preserved between Creer re-runs.
// you can add additional #includes here
// <<-- /Creer-Merge: includes -->>

namespace cpp_client
{

namespace coreminer
{

/// <summary>
/// Information about a Unit's job.
/// </summary>
class Job_ : public Game_object_
{
public:

    /// <summary>
    /// The amount of cargo capacity this Unit starts with.
    /// </summary>
    const int& cargo_capacity;

    /// <summary>
    /// The amount of starting health this Job has.
    /// </summary>
    const int& health;

    /// <summary>
    /// The maximum amount of cargo capacity this Unit can have.
    /// </summary>
    const int& max_cargo_capacity;

    /// <summary>
    /// The maximum amount of health this Job can have.
    /// </summary>
    const int& max_health;

    /// <summary>
    /// The maximum amount of mining power this Unit can have.
    /// </summary>
    const int& max_mining_power;

    /// <summary>
    /// The maximum number of moves this Job can have.
    /// </summary>
    const int& max_moves;

    /// <summary>
    /// The amount of mining power this Unit has per turn.
    /// </summary>
    const int& mining_power;

    /// <summary>
    /// The number of moves this Job can make per turn.
    /// </summary>
    const int& moves;

    /// <summary>
    /// The Job title. 'miner' or 'bomb'.
    /// </summary>
    const std::string& title;

    // <<-- Creer-Merge: member variables -->> - Code you add between this comment and the end comment will be preserved between Creer re-runs.
    // You can add additional member variables here. None of them will be tracked or updated by the server.
    // <<-- /Creer-Merge: member variables -->>



   // <<-- Creer-Merge: methods -->> - Code you add between this comment and the end comment will be preserved between Creer re-runs.
   // You can add additional methods here.
   // <<-- /Creer-Merge: methods -->>

   ~Job_();

   // ####################
   // Don't edit these!
   // ####################
   /// \cond FALSE
   Job_(std::initializer_list<std::pair<std::string, Any&&>> init);
   Job_() : Job_({}){}
   virtual void resize(const std::string& name, std::size_t size) override;
   virtual void change_vec_values(const std::string& name, std::vector<std::pair<std::size_t, Any>>& values) override;
   virtual void remove_key(const std::string& name, Any& key) override;
   virtual std::unique_ptr<Any> add_key_value(const std::string& name, Any& key, Any& value) override;
   virtual bool is_map(const std::string& name) override;
   virtual void rebind_by_name(Any* to_change, const std::string& member, std::shared_ptr<Base_object> ref) override;
    /// \endcond
    // ####################
    // Don't edit these!
    // ####################
};

} // coreminer

} // cpp_client

#endif // GAMES_COREMINER_JOB_H
