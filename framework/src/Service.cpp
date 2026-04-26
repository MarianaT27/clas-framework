#include <clas12/framework/core/IService.hpp>
#include <clas12/framework/services/LoggingService.hpp>

namespace clas12::framework {

auto Service::resolve_required_services() -> StatusCode {
    for (std::size_t i = 0; i < m_required_service_retrievers.size(); ++i) {
        if (!m_required_service_retrievers[i]()) {
            Log::error(m_name,
                       "Required service not available: {}",
                       m_required_dependency_names[i]);
            return StatusCode::FAILURE();
        }
    }
    return StatusCode::SUCCESS();
}

}  // namespace clas12::framework
