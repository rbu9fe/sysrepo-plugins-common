#pragma once

#include <string>
#include <map>

#include <libyang-cpp/Context.hpp>
#include <libyang-cpp/DataNode.hpp>

#include <sysrepo-cpp/sysrepo-wrapper.hpp>

#include "module.hpp"
#include "context.hpp"

namespace ly = libyang;

namespace srpc
{

/**
 * @brief Extracts the key from the list XPath.
 *
 * @param list List name.
 * @param key Key name.
 * @param xpath XPath of the list.
 * @return std::string Key value.
 */
const std::string extractListKeyFromXPath(const std::string &list, const std::string &key, const std::string &xpath);

/**
 * @brief Extracts all the keys from the list XPath by list name.
 *
 * @param list List name.
 * @param xpath XPath of the list.
 * @return std::unordered_map<std::string, std::string> Key values.
 */
std::unordered_map<std::string, std::string> extractListKeysFromXpath(const std::string& list, const std::string& xpath);

/**
 * @brief Get meta value.
 *
 * @param meta Meta collection object.
 * @param name Meta name.
 * @return std::string Meta value.
 */
const std::string getMetaValue(const ly::MetaCollection &meta, const std::string &name);

/**
 * @brief Convert meta values list to a hash.
 *
 * @param meta Meta collection object.
 * @return std::map<std::string, std::string> Meta values hash.
 */
std::map<std::string, std::string> getMetaValuesHash(const ly::MetaCollection meta);

/**
 * Register all operational plugin subscriptions.
 *
 * @param sess Session to use for creating subscriptions.
 * @param ctx Plugin context.
 * @param mod Module to use.
 *
 */
template <PluginContext PluginContextType>
void registerOperationalSubscriptions(Sysrepo::Session &sess, PluginContextType &ctx,
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
void registerModuleChangeSubscriptions(Sysrepo::Session &sess, PluginContextType &ctx,
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
void registerRpcSubscriptions(Sysrepo::Session &sess, PluginContextType &ctx,
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
