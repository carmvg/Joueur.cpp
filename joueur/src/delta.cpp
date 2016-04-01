#include "delta.hpp"
#include "attr_wrapper.hpp"
#include "exceptions.hpp"
#include "any.hpp"
#include "base_game.hpp"
#include "base_ai.hpp"
#include "base_object.hpp"
#include "delta_mergable.hpp"
#include "sgr.hpp"

#include <cstring>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>
#include <iterator>
#include <tuple>

namespace cpp_client
{

void morph_any(Any& to_morph, const rapidjson::Value& val)
{
   const bool no_type = (to_morph.type() == typeid(void));
   //otherwise just create the Any from it via this stupid if-else block
   if(val.IsBool())
   {
      if(!no_type && to_morph.type() != typeid(bool))
      {
         throw Bad_manipulation("Boolean assigned to non-Boolean.");
      }
      if(no_type)
      {
         to_morph = Any{val.GetBool()};
      }
      else
      {
         to_morph.as<bool>() = val.GetBool();
      }
   }
   else if(val.IsInt())
   {
      if(!no_type && to_morph.type() != typeid(int))
      {
         throw Bad_manipulation("Integer assigned to non-Integer.");
      }
      if(no_type)
      {
         to_morph = Any{val.GetInt()};
      }
      else
      {
         to_morph.as<int>() = val.GetInt();
      }
   }
   else if(val.IsString())
   {
      if(!no_type && to_morph.type() != typeid(std::string))
      {
         throw Bad_manipulation("String assigned to non-string.");
      }
      if(no_type)
      {
         to_morph = Any{std::string{val.GetString()}};
      }
      else
      {
         to_morph.as<std::string>() = std::string{val.GetString()};
      }
   }
   else if(val.IsNumber())
   {
      if(!no_type && to_morph.type() != typeid(double))
      {
         throw Bad_manipulation("Double assigned to non-double.");
      }
      if(no_type)
      {
         to_morph = Any{static_cast<double>(val.GetDouble())};
      }
      else
      {
         to_morph.as<double>() = static_cast<double>(val.GetDouble());
      }
   }
   else
   {
      //some kind of weird gross type that's icky
      throw Bad_response("Unknown JSON type received in a delta.");
   }
}

namespace
{

using vec_ref_t = std::vector<std::tuple<Base_object*,
                                         std::string,
                                         std::vector<std::pair<std::size_t, Any>>>>;

//returns the name of an object if the last thing added was an object reference
//returns an empty string otherwise
template<typename T>
inline std::string
   handle_itr(Base_game& context,
              Delta_mergable& apply_to,
              const rapidjson::Value::ConstMemberIterator& itr,
              T* owner,
              std::vector<std::tuple<std::shared_ptr<Base_object>*, std::string>>& refs,
              vec_ref_t& vec_refs);

}

void apply_delta(rapidjson::Value& delta, Base_game& apply_to)
{
   //can assume that the type is a delta
   const auto& data = attr_wrapper::get_loc(delta, "data")->value;
   if(!data.IsObject())
   {
      throw Bad_response("Delta's data field is not an object.");
   }
   std::vector<std::tuple<std::shared_ptr<Base_object>*, std::string>> refs;
   vec_ref_t vec_refs;
   for(auto data_iter = data.MemberBegin(); data_iter != data.MemberEnd(); ++data_iter)
   {
      auto to_add = handle_itr(apply_to, apply_to, data_iter, &apply_to, refs, vec_refs);
   }
   //now do the references
   for(auto&& ref : refs)
   {
      auto& obj = std::get<0>(ref);
      const auto& refer = std::get<1>(ref);
      *obj = std::static_pointer_cast<Base_object>(apply_to.get_objects()[refer]);
   }
   for(auto&& vec_ref : vec_refs)
   {
      auto& obj = std::get<0>(vec_ref);
      auto& name = std::get<1>(vec_ref);
      auto& changes = std::get<2>(vec_ref);
      //need to change the strings to pointers
      for(auto&& change : changes)
      {
         auto id = std::move(change.second.as<std::string>());
         change.second = std::static_pointer_cast<Base_object>(apply_to.get_objects()[id]);
      }
      obj->change_vec_values(name, changes);
   }
}

namespace
{

template<typename T>
inline std::string
   handle_itr(Base_game& context,
              Delta_mergable& apply_to,
              const rapidjson::Value::ConstMemberIterator& itr,
              T* owner,
              std::vector<std::tuple<std::shared_ptr<Base_object>*, std::string>>& refs,
              vec_ref_t& vec_refs)
{
   const auto& val = itr->value;
   const auto name = std::string(itr->name.GetString());
   auto& objects = context.get_objects();
   //check if it's an object
   if(val.IsObject())
   {
      //check if it's an "array"
      auto len_itr = val.FindMember(context.len_string().c_str());
      auto type_itr = val.FindMember("gameObjectName");
      auto id_itr = val.FindMember("id");
      if(len_itr != val.MemberEnd())
      {
         //array...
         //grab the length
         auto size = attr_wrapper::as<int>(len_itr->value);
         //resize the vector
         apply_to.resize(name, size);
         std::vector<std::pair<std::size_t, Any>> to_edit;
         std::vector<std::pair<std::size_t, Any>> temp_refs;
         for(auto data_iter = val.MemberBegin(); data_iter != val.MemberEnd(); ++data_iter)
         {
            //skip the length iterator
            if(data_iter == len_itr)
            {
               continue;
            }
            //either a game object or a primitive - if it's an object it's a game object
            if(data_iter->value.IsObject())
            {
               //make a base object and then edit it
               auto ptr = std::make_shared<Base_object>();
               auto str = handle_itr(context,
                                     *ptr,
                                     data_iter,
                                     &apply_to,
                                     refs,
                                     vec_refs);
               const auto num = atoi(data_iter->name.GetString());
               if(!str.empty())
               {
                  temp_refs.emplace_back(num, std::move(str));
               }
               else
               {
                  to_edit.emplace_back(num, std::move(ptr));
               }
            }
            else
            {
               Any to_add;
               morph_any(to_add, data_iter->value);
               to_edit.emplace_back(atoi(data_iter->name.GetString()),
                                    std::move(to_add));
            }
         }
         apply_to.change_vec_values(name, to_edit);
         //put vector object references in there too (if non-empty)
         if(!temp_refs.empty())
         {
            vec_refs.emplace_back(static_cast<Base_object*>(&apply_to),
                                  name,
                                  std::move(temp_refs));
         }
      }
      else if(type_itr != val.MemberEnd())
      {
         //new object...
         //give it a place in the objects
         objects[name] = context.generate_object(attr_wrapper::as<std::string>(type_itr->value));
         for(auto data_iter = val.MemberBegin(); data_iter != val.MemberEnd(); ++data_iter)
         {
            auto str = handle_itr(context,
                                  *objects[name],
                                  data_iter,
                                  &apply_to,
                                  refs,
                                  vec_refs);
            if(!str.empty())
            {
               auto target = std::string{data_iter->name.GetString()};
               refs.emplace_back(&objects[name]->variables_[target].as<std::shared_ptr<Base_object>>(),
                                 str);
            }
         }
      }
      else if(id_itr != val.MemberEnd())
      {
         //object reference...
         auto woop = attr_wrapper::as<std::string>(id_itr->value);
         return attr_wrapper::as<std::string>(id_itr->value);
      }
      else
      {
         //map...
         for(auto data_iter = val.MemberBegin(); data_iter != val.MemberEnd(); ++data_iter)
         {
            const auto target = std::string{data_iter->name.GetString()};
            if(data_iter->value.IsObject())
            {
               //need to tell if the object is a map - deal with it
               if(!apply_to.is_map(name))
               {
                  auto str = handle_itr(context,
                                        *apply_to.variables_[name].as<std::shared_ptr<Base_object>>(),
                                        data_iter,
                                        &apply_to,
                                        refs,
                                        vec_refs);
                  if(!str.empty())
                  {
                     refs.emplace_back(&apply_to.variables_[name].as<std::shared_ptr<Base_object>>(),
                                       std::move(str));
                  }
               }
               else
               {
                  //need to handle object references
                  Any key{std::string{target}};
                  Any value{};
                  value = apply_to.add_key_value(name, key, value);
                  if(value.type() == typeid(std::shared_ptr<Base_object>))
                  {
                     //make an object if needed
                     if(!value.as<std::shared_ptr<Base_object>>())
                     {
                        value = std::make_shared<Base_object>();
                     }
                     auto str = handle_itr(context,
                                           *value.as<std::shared_ptr<Base_object>>(),
                                           data_iter,
                                           &apply_to,
                                           refs,
                                           vec_refs);
                     Any dummy;
                     auto val = apply_to.add_key_value(name, key, dummy);
                     if(!str.empty())
                     {
                        refs.emplace_back(&val.as<std::shared_ptr<Base_object>>(), std::move(str));
                     }
                  }
                  else
                  {
                     //otherwise just morph it
                     morph_any(value, data_iter->value);
                     apply_to.add_key_value(name, key, value);
                  }
               }
            }
            else
            {
               //Figure this out: it's getting like void variables and junk
               morph_any(apply_to.variables_[target], data_iter->value);
            }
         }
      }
   }
   else if(!val.IsArray())
   {
      if(val.IsNull())
      {
         //need to do this a little differently so that it doesn't change type
         auto& change = apply_to.variables_[name].as<std::shared_ptr<Base_object>>();
         change.reset();
         return "";
      }
      else if(val.IsString() && val.GetString() == context.remove_string())
      {
         //remove this thing
         owner->erase(name);
         return "";
      }
      //do some checking (ignore name because of game weirdness)
      if(!apply_to.variables_.count(name) && name != "name")
      {
         std::cout << sgr::text_yellow
                   << "Warning: Unknown variable " << name << " added." << sgr::reset
                   << "\n"
                   ;
      }
      morph_any(apply_to.variables_[name], val);
   }
   else
   {
      //array - some kind of error
      throw Bad_response("Received a JSON array in a delta.");
   }
   return "";
}

}

} // cpp-client