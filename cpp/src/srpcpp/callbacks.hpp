#pragma once

#include <string>
#include <sysrepo-cpp/Session.hpp>
#include <sysrepo-cpp/Subscription.hpp>

namespace srpc
{
/**
 * @brief Operational callback struct.
 */
struct OperationalCallback
{
    std::string Module;                                                    ///< Module for the callback.
    std::string XPath;                                                     ///< XPath of the data.
    sysrepo::OperGetCb Callback;                                           ///< Callback function.
    sysrepo::SubscribeOptions opts = sysrepo::SubscribeOptions::OperMerge; ///< Subscription options.
};

/**
 * @brief Module change callback struct.
 */
struct ModuleChangeCallback
{
    std::string Module;                                                  ///< Module for the callback.
    std::string XPath;                                                   ///< XPath of the data.
    sysrepo::ModuleChangeCb Callback;                                    ///< Callback function.
    uint32_t priority = 0;                                               ///< Priority.
    sysrepo::SubscribeOptions opts = sysrepo::SubscribeOptions::Default; ///< Subscription options.
};

/**
 * @brief RPC callback struct.
 */
struct RpcCallback
{
    std::string XPath;                                                   ///< XPath of the data.
    sysrepo::RpcActionCb Callback;                                       ///< Callback function.
    uint32_t priority = 0;                                               ///< Priority.
    sysrepo::SubscribeOptions opts = sysrepo::SubscribeOptions::Default; ///< Subscription options.
};
} // namespace srpc
