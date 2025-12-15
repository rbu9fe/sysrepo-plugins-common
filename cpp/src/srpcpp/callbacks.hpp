#pragma once

#include <string>
#include <sysrepo-cpp/sysrepo-wrapper.hpp>

namespace srpc
{
/**
 * @brief Operational callback struct.
 */
struct OperationalCallback
{
    std::string Module;                                                    ///< Module for the callback.
    std::string XPath;                                                     ///< XPath of the data.
    Sysrepo::OperGetCb Callback;                                           ///< Callback function.
    sysrepo::SubscribeOptions opts = sysrepo::SubscribeOptions::OperMerge; ///< Subscription options.
};

/**
 * @brief Module change callback struct.
 */
struct ModuleChangeCallback
{
    std::string Module;                                                  ///< Module for the callback.
    std::string XPath;                                                   ///< XPath of the data.
    Sysrepo::ModuleChangeCb Callback;                                    ///< Callback function.
    uint32_t priority = 0;                                               ///< Priority.
    sysrepo::SubscribeOptions opts = sysrepo::SubscribeOptions::Default; ///< Subscription options.
};

/**
 * @brief RPC callback struct.
 */
struct RpcCallback
{
    std::string XPath;                                                   ///< XPath of the data.
    Sysrepo::RpcActionCb Callback;                                       ///< Callback function.
    uint32_t priority = 0;                                               ///< Priority.
    sysrepo::SubscribeOptions opts = sysrepo::SubscribeOptions::Default; ///< Subscription options.
};
} // namespace srpc
