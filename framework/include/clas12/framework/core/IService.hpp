#pragma once

#include <memory>
#include <string>
#include <clas12/framework/core/StatusCode.hpp>

namespace clas12::framework {

/**
 * @brief Base interface for all services in the framework
 *
 * Services provide functionality that can be accessed by multiple algorithms,
 * such as particle identification, histogram management, calibrations, etc.
 * This follows the service-oriented architecture pattern used in Gaudi/Athena.
 */
class IService {
   public:
    virtual ~IService() = default;

    /**
     * @brief Initialize the service
     * @return StatusCode indicating success or failure
     */
    virtual auto initialize() -> StatusCode = 0;

    /**
     * @brief Finalize the service (cleanup)
     * @return StatusCode indicating success or failure
     */
    virtual auto finalize() -> StatusCode = 0;

    /**
     * @brief Get the service name
     * @return const std::string& Service name
     */
    virtual auto name() const -> const std::string& = 0;
};

/**
 * @brief Base class for service implementations
 */
class Service : public IService {
   public:
    explicit Service(std::string name) : m_name(std::move(name)) {}

    auto name() const -> const std::string& override { return m_name; }

    auto initialize() -> StatusCode override {
        return StatusCode::SUCCESS();
    }

    auto finalize() -> StatusCode override {
        return StatusCode::SUCCESS();
    }

   protected:
    std::string m_name;
};

}  // namespace clas12::framework
