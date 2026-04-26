#pragma once

/**
 * @file Core.hpp
 * @brief All-in-one convenience include for CLAS12 Framework users
 *
 * Include this header to get access to all core framework components.
 */

// Core algorithm infrastructure
#include <clas12/framework/core/Algorithm.hpp>

// Application
#include <clas12/framework/core/Application.hpp>
#include <clas12/framework/core/ApplicationBuilder.hpp>

// Event and data model
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/TypedEventStore.hpp>

// Services
#include <clas12/framework/core/IService.hpp>
#include <clas12/framework/core/ServiceManager.hpp>

// Configuration
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/ConfigSchema.hpp>

// Status and error handling
#include <clas12/framework/core/StatusCode.hpp>
#include <clas12/framework/core/Result.hpp>
#include <clas12/framework/core/Error.hpp>

// Histograms
#include <clas12/framework/core/Histogram.hpp>
#include <clas12/framework/core/HistogramFiller.hpp>
#include <clas12/framework/core/ThreadLocalHistogram.hpp>

// Cutflow
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/AutoCutflow.hpp>

// Common services
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/services/ConfigService.hpp>
#include <clas12/framework/services/OutputFileService.hpp>
