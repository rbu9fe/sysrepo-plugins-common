#pragma once

#include <string>
#include <map>
#include <regex>

#include <libyang-cpp/Context.hpp>
#include <libyang-cpp/DataNode.hpp>

#include <sysrepo-cpp/Session.hpp>

#include "module.hpp"
#include "context.hpp"

namespace ly = libyang;

namespace srpc
{

/**
 * @brief Escape special characters in the incoming string so that it can be used in a regular expression.
 *
 * @param s Incoming string
 * @return std::string Escaped string
 */
static std::string EscapeForRegex(const std::string &s)
{
    static const std::regex meta_characters{R"([-[\]{}()*+?.\^$|\s])"};
    return std::regex_replace(s, meta_characters, R"(\$&)");
}

/**
 * @brief Extracts the key from the list XPath.
 *
 * @param list List name.
 * @param key Key name.
 * @param xpath XPath of the list.
 * @return std::string Key value.
 */
static const std::string extractListKeyFromXPath(const std::string &list, const std::string &key, const std::string &xpath)
{
    std::string value;

    std::stringstream ss;

    ss << EscapeForRegex(list) << "\\[" << key << "='([^']*)'\\]";

    const auto &xpath_expr = ss.str();

    std::regex re(xpath_expr);
    std::smatch xpath_match;

    if (std::regex_search(xpath, xpath_match, re))
    {
        value = xpath_match[1];
    }
    else
    {
        throw std::runtime_error("Failed to extract key from XPath.");
    }

    return value;
}

/**
 * @brief Extracts all the keys from the list XPath by list name.
 *
 * @param list List name.
 * @param xpath XPath of the list.
 * @return std::unordered_map<std::string, std::string> Key values.
 */
static std::unordered_map<std::string, std::string> extractListKeysFromXpath(const std::string& list, const std::string& xpath)
{
    std::unordered_map<std::string, std::string> keys_map;

    int starting_point = xpath.find(list + "[");

    std::string chunk(xpath.begin() + starting_point, xpath.end());

    int ending_point = chunk.find("]/");

    if(ending_point == std::string::npos){
        //this means it can be last
        ending_point = chunk.find_last_of(']');
    }

    chunk.erase(chunk.begin() + ending_point + 1, chunk.end());

    int begin = chunk.find('[');
    int end = chunk.find(']');
    // list is found, continue

    while (begin != std::string::npos || end != std::string::npos) {

        std::string mapstr(++chunk.begin() + begin, chunk.begin() + end);
        chunk.erase(begin, ++end - begin);

        // now split it by '=' in key value

        int eq_pos = mapstr.find('=');

        if (eq_pos == std::string::npos) {
            throw std::runtime_error("Failed to parse '=' sympol");
        }

        std::string key(mapstr.begin(), mapstr.begin() + eq_pos);
        // value shrink by one place begin to end to eliminate ' '
        std::string value(mapstr.begin() + eq_pos + 2, --mapstr.end());

        keys_map.insert(std::make_pair(key, value));

        begin = chunk.find('[');
        end = chunk.find(']');
    }

    return keys_map;
}

/**
 * @brief Get meta value.
 *
 * @param meta Meta collection object.
 * @param name Meta name.
 * @return std::string Meta value.
 */
static const std::string getMetaValue(const ly::MetaCollection &meta, const std::string &name)
{
    for (const auto &m : meta)
    {
        if (m.name() == name)
        {
            return m.valueStr();
        }
    }
    throw std::runtime_error("Failed to get meta value.");
}

/**
 * @brief Convert meta values list to a hash.
 *
 * @param meta Meta collection object.
 * @return std::map<std::string, std::string> Meta values hash.
 */
static std::map<std::string, std::string> getMetaValuesHash(const ly::MetaCollection meta)
{
    std::map<std::string, std::string> hash;

    for (const auto &m : meta)
    {
        hash[m.name()] = m.valueStr();
    }

    return hash;
}

/**
 * Register all operational plugin subscriptions.
 *
 * @param sess Session to use for creating subscriptions.
 * @param ctx Plugin context.
 * @param mod Module to use.
 *
 */
template <PluginContext PluginContextType>
static void registerOperationalSubscriptions(sysrepo::Session sess, PluginContextType &ctx,
                                      std::unique_ptr<srpc::IModule<PluginContextType>> &mod)
{
    const auto oper_callbacks = mod->getOperationalCallbacks();

    auto &sub_handle = ctx.getSubscriptionHandle();

    for (auto &cb : oper_callbacks)
    {
        SRPLG_LOG_INF(ctx.getPluginName(), "Creating operational subscription for xpath %s", cb.XPath.c_str());
        if (sub_handle.has_value())
        {
            sub_handle->onOperGet(cb.Module, cb.Callback, cb.XPath, cb.opts);
        }
        else
        {
            sub_handle.emplace(sess.onOperGet(cb.Module, cb.Callback, cb.XPath, cb.opts));
        }
    }
}

/**
 * Register all module change plugin subscriptions.
 *
 * @param sess Session to use for creating subscriptions.
 * @param ctx Plugin context.
 * @param mod Module to use.
 *
 */
template <PluginContext PluginContextType>
static void registerModuleChangeSubscriptions(sysrepo::Session sess, PluginContextType &ctx,
                                       std::unique_ptr<srpc::IModule<PluginContextType>> &mod)
{
    const auto change_callbacks = mod->getModuleChangeCallbacks();

    auto &sub_handle = ctx.getSubscriptionHandle();

    for (auto &cb : change_callbacks)
    {
        SRPLG_LOG_INF(ctx.getPluginName(), "Creating module change subscription for xpath %s", cb.XPath.c_str());
        if (sub_handle.has_value())
        {
            sub_handle->onModuleChange(cb.Module, cb.Callback, cb.XPath, cb.priority, cb.opts);
        }
        else
        {
            sub_handle.emplace(sess.onModuleChange(cb.Module, cb.Callback, cb.XPath, cb.priority, cb.opts));
        }
    }
}

/**
 * Register all RPC plugin subscriptions.
 *
 * @param sess Session to use for creating subscriptions.
 * @param ctx Plugin context.
 * @param mod Module to use.
 *
 */
template <PluginContext PluginContextType>
static void registerRpcSubscriptions(sysrepo::Session sess, PluginContextType &ctx,
                              std::unique_ptr<srpc::IModule<PluginContextType>> &mod)
{
    const auto rpc_callbacks = mod->getRpcCallbacks();

    auto &sub_handle = ctx.getSubscriptionHandle();

    for (auto &cb : rpc_callbacks)
    {
        SRPLG_LOG_INF(ctx.getPluginName(), "Creating RPC subscription for xpath %s", cb.XPath.c_str());
        if (sub_handle.has_value())
        {
            sub_handle->onRPCAction(cb.XPath, cb.Callback, cb.priority, cb.opts);
        }
        else
        {
            sub_handle.emplace(sess.onRPCAction(cb.XPath, cb.Callback, cb.priority, cb.opts));
        }
    }
}

} // namespace srpc
